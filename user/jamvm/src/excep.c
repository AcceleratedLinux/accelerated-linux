/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007, 2008
 * Robert Lougher <rob@lougher.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include "jam.h"
#include "lock.h"
#include "symbol.h"
#include "excep.h"

static Class *ste_class, *ste_array_class, *throw_class, *vmthrow_class;
static MethodBlock *vmthrow_init_mb;
static int backtrace_offset;
static int inited = FALSE;

static Class *exceptions[MAX_EXCEPTION_ENUM];

static int exception_symbols[] = {
    EXCEPTIONS_DO(SYMBOL_NAME_ENUM)
};

void initialiseException() {
    FieldBlock *bcktrce;
    int i;

    ste_class = findSystemClass0(SYMBOL(java_lang_StackTraceElement));
    ste_array_class = findArrayClass(SYMBOL(array_java_lang_StackTraceElement));
    vmthrow_class = findSystemClass0(SYMBOL(java_lang_VMThrowable));
    throw_class = findSystemClass0(SYMBOL(java_lang_Throwable));
    bcktrce = findField(vmthrow_class, SYMBOL(backtrace), SYMBOL(sig_java_lang_Object));
    vmthrow_init_mb = findMethod(ste_class, SYMBOL(object_init),
                         SYMBOL(_java_lang_String_I_java_lang_String_java_lang_String_Z__V));

    if((bcktrce == NULL) || (vmthrow_init_mb == NULL)) {
        jam_fprintf(stderr, "Error initialising VM (initialiseException)\n");
        exitVM(1);
    }

    CLASS_CB(vmthrow_class)->flags |= VMTHROWABLE;
    backtrace_offset = bcktrce->offset;

    registerStaticClassRef(&ste_class);
    registerStaticClassRef(&ste_array_class);
    registerStaticClassRef(&vmthrow_class);
    registerStaticClassRef(&throw_class);

    /* Load and register the exceptions used within the VM.
       These are preloaded to speed up access.  The VM will
       abort if any can't be loaded */

    for(i = 0; i < MAX_EXCEPTION_ENUM; i++) {
        exceptions[i] = findSystemClass0(symbol_values[exception_symbols[i]]);
        registerStaticClassRef(&exceptions[i]);
    }

    inited = TRUE;
}

Object *exceptionOccurred() {
   return getExecEnv()->exception; 
}

void setException(Object *exp) {
    getExecEnv()->exception = exp;
}

void clearException() {
    ExecEnv *ee = getExecEnv();

    if(ee->overflow) {
        ee->overflow = FALSE;
        ee->stack_end -= STACK_RED_ZONE_SIZE;
    }
    ee->exception = NULL;
}

void signalChainedExceptionClass(Class *exception, char *message, Object *cause) {
    Object *exp = allocObject(exception);
    Object *str = message == NULL ? NULL : Cstr2String(message);
    MethodBlock *init = lookupMethod(exception, SYMBOL(object_init),
                                                SYMBOL(_java_lang_String__V));
    if(exp && init) {
        executeMethod(exp, init, str);

        if(cause && !exceptionOccurred()) {
            MethodBlock *mb = lookupMethod(exception, SYMBOL(initCause),
                                           SYMBOL(_java_lang_Throwable__java_lang_Throwable));
            if(mb)
                executeMethod(exp, mb, cause);
        }
        setException(exp);
    }
}

void signalChainedExceptionName(char *excep_name, char *message, Object *cause) {
    if(!inited) {
        jam_fprintf(stderr, "Exception occurred while VM initialising.\n");
        if(message)
            jam_fprintf(stderr, "%s: %s\n", excep_name, message);
        else
            jam_fprintf(stderr, "%s\n", excep_name);
        exit(1);
    } else {
        Class *exception = findSystemClass(excep_name);

        if(!exceptionOccurred())
            signalChainedExceptionClass(exception, message, cause);
    }
}

void signalChainedExceptionEnum(int excep_enum, char *message, Object *cause) {
    if(!inited) {
        char *excep_name = symbol_values[exception_symbols[excep_enum]];

        jam_fprintf(stderr, "Exception occurred while VM initialising.\n");
        if(message)
            jam_fprintf(stderr, "%s: %s\n", excep_name, message);
        else
            jam_fprintf(stderr, "%s\n", excep_name);
        exit(1);
    }

    signalChainedExceptionClass(exceptions[excep_enum], message, cause);
}

void printException() {
    ExecEnv *ee = getExecEnv();
    Object *exception = ee->exception;

    if(exception != NULL) {
        MethodBlock *mb = lookupMethod(exception->class, SYMBOL(printStackTrace),
                                                         SYMBOL(___V));
        clearException();
        executeMethod(exception, mb);

        /* If we're really low on memory we might have been able to throw
         * OutOfMemory, but then been unable to print any part of it!  In
         * this case the VM just seems to stop... */
        if(ee->exception) {
            jam_fprintf(stderr, "Exception occurred while printing exception (%s)...\n",
                            CLASS_CB(ee->exception->class)->name);
            jam_fprintf(stderr, "Original exception was %s\n", CLASS_CB(exception->class)->name);
        }
    }
}

CodePntr findCatchBlockInMethod(MethodBlock *mb, Class *exception, CodePntr pc_pntr) {
    ExceptionTableEntry *table = mb->exception_table;
    int size = mb->exception_table_size;
    int pc = pc_pntr - ((CodePntr)mb->code);
    int i;
 
    for(i = 0; i < size; i++)
        if((pc >= table[i].start_pc) && (pc < table[i].end_pc)) {

            /* If the catch_type is 0 it's a finally block, which matches
               any exception.  Otherwise, the thrown exception class must
               be an instance of the caught exception class to catch it */

            if(table[i].catch_type != 0) {
                Class *caught_class = resolveClass(mb->class, table[i].catch_type, FALSE);
                if(caught_class == NULL) {
                    clearException();
                    continue;
                }
                if(!isInstanceOf(caught_class, exception))
                    continue;
            }
            return ((CodePntr)mb->code) + table[i].handler_pc;
        }

    return NULL;
}
    
CodePntr findCatchBlock(Class *exception) {
    Frame *frame = getExecEnv()->last_frame;
    CodePntr handler_pc = NULL;

    while(((handler_pc = findCatchBlockInMethod(frame->mb, exception, frame->last_pc)) == NULL)
                    && (frame->prev->mb != NULL)) {

        if(frame->mb->access_flags & ACC_SYNCHRONIZED) {
            Object *sync_ob = frame->mb->access_flags & ACC_STATIC ?
                    (Object*)frame->mb->class : (Object*)frame->lvars[0];
            objectUnlock(sync_ob);
        }
        frame = frame->prev;
    }

    getExecEnv()->last_frame = frame;

    return handler_pc;
}

int mapPC2LineNo(MethodBlock *mb, CodePntr pc_pntr) {
    int pc = pc_pntr - (CodePntr) mb->code;
    int i;

    if(mb->line_no_table_size > 0) {
        for(i = mb->line_no_table_size-1; i && pc < mb->line_no_table[i].start_pc; i--);
        return mb->line_no_table[i].line_no;
    }

    return -1;
}

Object *setStackTrace0(ExecEnv *ee, int max_depth) {
    Frame *bottom, *last = ee->last_frame;
    Object *array, *vmthrwble;
    uintptr_t *data;
    int depth = 0;

    if(last->prev == NULL) {
        if((array = allocTypeArray(sizeof(uintptr_t) == 4 ? T_INT : T_LONG, 0)) == NULL)
            return NULL;
        goto out2;
    }

    for(; last->mb != NULL && isInstanceOf(vmthrow_class, last->mb->class);
          last = last->prev);

    for(; last->mb != NULL && isInstanceOf(throw_class, last->mb->class);
          last = last->prev);

    bottom = last;
    do {
        for(; last->mb != NULL; last = last->prev, depth++)
            if(depth == max_depth)
                goto out;
    } while((last = last->prev)->prev != NULL);
    
out:
    if((array = allocTypeArray(sizeof(uintptr_t) == 4 ? T_INT : T_LONG, depth*2)) == NULL)
        return NULL;

    data = ARRAY_DATA(array);
    depth = 0;
    do {
        for(; bottom->mb != NULL; bottom = bottom->prev) {
            if(depth == max_depth)
                goto out2;

            data[depth++] = (uintptr_t)bottom->mb;
            data[depth++] = (uintptr_t)bottom->last_pc;
        }
    } while((bottom = bottom->prev)->prev != NULL);

out2:
    if((vmthrwble = allocObject(vmthrow_class)))
        INST_DATA(vmthrwble)[backtrace_offset] = (uintptr_t)array;

    return vmthrwble;
}

Object *convertStackTrace(Object *vmthrwble) {
    Object *array, *ste_array;
    int depth, i, j;
    uintptr_t *src;
    Object **dest;

    if((array = (Object *)INST_DATA(vmthrwble)[backtrace_offset]) == NULL)
        return NULL;

    src = ARRAY_DATA(array);
    depth = ARRAY_LEN(array);

    if((ste_array = allocArray(ste_array_class, depth/2, sizeof(Object*))) == NULL)
        return NULL;

    dest = ARRAY_DATA(ste_array);

    for(i = 0, j = 0; i < depth; j++) {
        MethodBlock *mb = (MethodBlock*)src[i++];
        CodePntr pc = (CodePntr)src[i++];
        ClassBlock *cb = CLASS_CB(mb->class);
        char *dot_name = slash2dots(cb->name);

        int isNative = mb->access_flags & ACC_NATIVE ? TRUE : FALSE;
        Object *filename = isNative ? NULL : (cb->source_file_name ?
                                             createString(cb->source_file_name) : NULL);
        Object *classname = createString(dot_name);
        Object *methodname = createString(mb->name);
        Object *ste = allocObject(ste_class);
        sysFree(dot_name);

        if(exceptionOccurred())
            return NULL;

        executeMethod(ste, vmthrow_init_mb, filename, isNative ? -1 : mapPC2LineNo(mb, pc),
                        classname, methodname, isNative);

        if(exceptionOccurred())
            return NULL;

        dest[j] = ste;
    }

    return ste_array;
}

/* GC support for marking classes referenced by a VMThrowable.
   In rare circumstances a stack backtrace may hold the only
   reference to a class */

void markVMThrowable(Object *vmthrwble, int mark, int mark_soft_refs) {
    Object *array;

    if((array = (Object *)INST_DATA(vmthrwble)[backtrace_offset]) != NULL) {
        uintptr_t *src = ARRAY_DATA(array);
        int i, depth = ARRAY_LEN(array);

        for(i = 0; i < depth; i += 2) {
            MethodBlock *mb = (MethodBlock*)src[i];
            markObject(mb->class, mark, mark_soft_refs);
        }
    }
}
