/* Dbg.c - Tcl Debugger - See cmdHelp() for commands

Written by: Don Libes, NIST, 3/23/93

Design and implementation of this program was paid for by U.S. tax
dollars.  Therefore it is public domain.  However, the author and NIST
would appreciate credit if this program or parts of it are used.

*/

#include <stdio.h>
#include <stdarg.h>

#include "Dbg_cf.h"
#if 0
/* tclInt.h drags in stdlib.  By claiming no-stdlib, force it to drag in */
/* Tcl's compat version.  This avoids having to test for its presence */
/* which is too tricky - configure can't generate two cf files, so when */
/* Expect (or any app) uses the debugger, there's no way to get the info */
/* about whether stdlib exists or not, except pointing the debugger at */
/* an app-dependent .h file and I don't want to do that. */
#define NO_STDLIB_H
#endif


#include "tclInt.h"
/*#include <varargs.h>		tclInt.h drags in varargs.h.  Since Pyramid */
/*				objects to including varargs.h twice, just */
/*				omit this one. */
/*#include "string.h"		tclInt.h drags this in, too! */
#include "Dbg.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

static int simple_interactor();
static int zero();

/* most of the static variables in this file may be */
/* moved into Tcl_Interp */

static Dbg_InterProc *interactor = simple_interactor;
static ClientData interdata = 0;
static Dbg_IgnoreFuncsProc *ignoreproc = zero;
static Dbg_OutputProc *printproc = 0;
static ClientData printdata = 0;

static void print(Tcl_Interp *interp, char *fmt, ...);

static int debugger_active = FALSE;

/* this is not externally documented anywhere as of yet */
char *Dbg_VarName = "dbg";

#define DEFAULT_COMPRESS	0
static int compress = DEFAULT_COMPRESS;
#define DEFAULT_WIDTH		75	/* leave a little space for printing */
					/*  stack level */
static int buf_width = DEFAULT_WIDTH;

static int main_argc = 1;
static char *default_argv = "application";
static char **main_argv = &default_argv;

static Tcl_Trace debug_handle;
static int step_count = 1;	/* count next/step */

#define FRAMENAMELEN 10		/* enough to hold strings like "#4" */
static char viewFrameName[FRAMENAMELEN];/* destination frame name for up/down */

static CallFrame *goalFramePtr;	/* destination for next/return */
static int goalNumLevel;	/* destination for Next */

static enum debug_cmd {
	none, step, next, ret, cont, up, down, where, Next
} debug_cmd;

/* info about last action to use as a default */
static enum debug_cmd last_action_cmd = next;
static int last_step_count = 1;

/* this acts as a strobe (while testing breakpoints).  It is set to true */
/* every time a new debugger command is issued that is an action */
static debug_new_action;

#define NO_LINE -1	/* if break point is not set by line number */

struct breakpoint {
	int id;
	char *file;	/* file where breakpoint is */
	int line;	/* line where breakpoint is */
	char *pat;	/* pattern defining where breakpoint can be */
	regexp *re;	/* regular expression to trigger breakpoint */
	char *expr;	/* expr to trigger breakpoint */
	char *cmd;	/* cmd to eval at breakpoint */
	struct breakpoint *next, *previous;
};

static struct breakpoint *break_base = 0;
static int breakpoint_max_id = 0;

static struct breakpoint *
breakpoint_new()
{
	struct breakpoint *b = (struct breakpoint *)ckalloc(sizeof(struct breakpoint));
	if (break_base) break_base->previous = b;
	b->next = break_base;
	b->previous = 0;
	b->id = breakpoint_max_id++;
	b->file = 0;
	b->line = NO_LINE;
	b->pat = 0;
	b->re = 0;
	b->expr = 0;
	b->cmd = 0;
	break_base = b;
	return(b);
}

static
void
breakpoint_print(interp,b)
Tcl_Interp *interp;
struct breakpoint *b;
{
	print(interp,"breakpoint %d: ",b->id);

	if (b->re) {
		print(interp,"-re \"%s\" ",b->pat);
	} else if (b->pat) {
		print(interp,"-glob \"%s\" ",b->pat);
	} else if (b->line != NO_LINE) {
		if (b->file) {
			print(interp,"%s:",b->file);
		}
		print(interp,"%d ",b->line);
	}

	if (b->expr)
		print(interp,"if {%s} ",b->expr);

	if (b->cmd)
		print(interp,"then {%s}",b->cmd);

	print(interp,"\n");
}

static void
save_re_matches(interp,re)
Tcl_Interp *interp;
regexp *re;
{
	int i;
	char name[20];
	char match_char;/* place to hold char temporarily */
			/* uprooted by a NULL */

	for (i=0;i<NSUBEXP;i++) {
		if (re->startp[i] == 0) break;

		sprintf(name,"%d",i);
		/* temporarily null-terminate in middle */
		match_char = *re->endp[i];
		*re->endp[i] = 0;
		Tcl_SetVar2(interp,Dbg_VarName,name,re->startp[i],0);

		/* undo temporary null-terminator */
		*re->endp[i] = match_char;
	}
}

/* return 1 to break, 0 to continue */
static int
breakpoint_test(interp,cmd,bp)
Tcl_Interp *interp;
char *cmd;		/* command about to be executed */
struct breakpoint *bp;	/* breakpoint to test */
{
	if (bp->re) {
#if TCL_MAJOR_VERSION == 6
		if (0 == regexec(bp->re,cmd)) return 0;
#else
		if (0 == TclRegExec(bp->re,cmd,cmd)) return 0;
#endif
		save_re_matches(interp,bp->re);
	} else if (bp->pat) {
		if (0 == Tcl_StringMatch(cmd,bp->pat)) return 0;
	} else if (bp->line != NO_LINE) {
		/* not yet implemented - awaiting support from Tcl */
		return 0;
	}

	if (bp->expr) {
		int value;

		/* ignore errors, since they are likely due to */
		/* simply being out of scope a lot */
		if (TCL_OK != Tcl_ExprBoolean(interp,bp->expr,&value)
		    || (value == 0)) return 0;
	}

	if (bp->cmd) {
#if TCL_MAJOR_VERSION == 6
		Tcl_Eval(interp,bp->cmd,0,(char **)0);
#else
		Tcl_Eval(interp,bp->cmd);
#endif
	} else {
		breakpoint_print(interp,bp);
	}

	return 1;
}

static char *already_at_top_level = "already at top level";

/* similar to TclGetFrame but takes two frame ptrs and a direction.
If direction is up,   search up stack from curFrame
If direction is down, simulate searching down stack by
		      seaching up stack from origFrame
*/
static
int
TclGetFrame2(interp, origFramePtr, string, framePtrPtr, dir)
    Tcl_Interp *interp;
    CallFrame *origFramePtr;	/* frame that is true top-of-stack */
    char *string;		/* String describing frame. */
    CallFrame **framePtrPtr;	/* Store pointer to frame here (or NULL
				 * if global frame indicated). */
    enum debug_cmd dir;	/* look up or down the stack */
{
    Interp *iPtr = (Interp *) interp;
    int level, result;
    CallFrame *framePtr;	/* frame currently being searched */

    CallFrame *curFramePtr = iPtr->varFramePtr;

    /*
     * Parse string to figure out which level number to go to.
     */

    result = 1;
    if (*string == '#') {
	if (Tcl_GetInt(interp, string+1, &level) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (level < 0) {
	    levelError:
	    Tcl_AppendResult(interp, "bad level \"", string, "\"",
		    (char *) NULL);
	    return TCL_ERROR;
	}
	framePtr = origFramePtr; /* start search here */
	
    } else if (isdigit(*string)) {
	if (Tcl_GetInt(interp, string, &level) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (dir == up) {
		if (curFramePtr == 0) {
			Tcl_SetResult(interp,already_at_top_level,TCL_STATIC);
			return TCL_ERROR;
		}
		level = curFramePtr->level - level;
		framePtr = curFramePtr; /* start search here */
	} else {
		if (curFramePtr != 0) {
			level = curFramePtr->level + level;
		}
		framePtr = origFramePtr; /* start search here */
	}
    } else {
	level = curFramePtr->level - 1;
	result = 0;
    }

    /*
     * Figure out which frame to use.
     */

    if (level == 0) {
	framePtr = NULL;
    } else {
	for (;framePtr != NULL;	framePtr = framePtr->callerVarPtr) {
	    if (framePtr->level == level) {
		break;
	    }
	}
	if (framePtr == NULL) {
	    goto levelError;
	}
    }
    *framePtrPtr = framePtr;
    return result;
}


static char *printify(s)
char *s;
{
	static int destlen = 0;
	char *d;		/* ptr into dest */
	unsigned int need;
	static char buf_basic[DEFAULT_WIDTH+1];
	static char *dest = buf_basic;

	if (s == 0) return("<null>");

	/* worst case is every character takes 4 to printify */
	need = strlen(s)*4;
	if (need > destlen) {
		if (dest && (dest != buf_basic)) ckfree(dest);
		dest = (char *)ckalloc(need+1);
		destlen = need;
	}

	for (d = dest;*s;s++) {
		/* since we check at worst by every 4 bytes, play */
		/* conservative and subtract 4 from the limit */
		if (d-dest > destlen-4) break;

		if (*s == '\b') {
			strcpy(d,"\\b");		d += 2;
		} else if (*s == '\f') {
			strcpy(d,"\\f");		d += 2;
		} else if (*s == '\v') {
			strcpy(d,"\\v");		d += 2;
		} else if (*s == '\r') {
			strcpy(d,"\\r");		d += 2;
		} else if (*s == '\n') {
			strcpy(d,"\\n");		d += 2;
		} else if (*s == '\t') {
			strcpy(d,"\\t");		d += 2;
		} else if ((unsigned)*s < 0x20) { /* unsigned strips parity */
			sprintf(d,"\\%03o",*s);		d += 4;
		} else if (*s == 0177) {
			strcpy(d,"\\177");		d += 4;
		} else {
			*d = *s;			d += 1;
		}
	}
	*d = '\0';
	return(dest);
}

static
char *
print_argv(interp,argc,argv)
Tcl_Interp *interp;
int argc;
char *argv[];
{
	static int buf_width_max = DEFAULT_WIDTH;
	static char buf_basic[DEFAULT_WIDTH+1];	/* basic buffer */
	static char *buf = buf_basic;
	int space;		/* space remaining in buf */
	int len;
	char *bufp;
	int proc;		/* if current command is "proc" */
	int arg_index;

	if (buf_width > buf_width_max) {
		if (buf && (buf != buf_basic)) ckfree(buf);
		buf = (char *)ckalloc(buf_width + 1);
		buf_width_max = buf_width;
	}

	proc = (0 == strcmp("proc",argv[0]));
	sprintf(buf,"%.*s",buf_width,argv[0]);
	len = strlen(buf);
	space = buf_width - len;
	bufp = buf + len;
	argc--; argv++;
	arg_index = 1;
	
	while (argc && (space > 0)) {
		char *elementPtr;
		char *nextPtr;
		int wrap;

		/* braces/quotes have been stripped off arguments */
		/* so put them back.  We wrap everything except lists */
		/* with one argument.  One exception is to always wrap */
		/* proc's 2nd arg (the arg list), since people are */
		/* used to always seeing it this way. */

		if (proc && (arg_index > 1)) wrap = TRUE;
		else {
			(void) TclFindElement(interp,*argv,&elementPtr,
					&nextPtr,(int *)0,(int *)0);
			if (*elementPtr == '\0') wrap = TRUE;
			else if (*nextPtr == '\0') wrap = FALSE;
			else wrap = TRUE;
		}

		/* wrap lists (or null) in braces */
		if (wrap) {
			sprintf(bufp," {%.*s}",space-3,*argv);
		} else {
			sprintf(bufp," %.*s",space-1,*argv);
		}
		len = strlen(buf);
		space = buf_width - len;
		bufp = buf + len;
		argc--; argv++;
		arg_index++;
	}

	if (compress) {
		/* this copies from our static buf to printify's static buf */
		/* and back to our static buf */
		strncpy(buf,printify(buf),buf_width);
	}

	/* usually but not always right, but assume truncation if buffer is */
	/* full.  this avoids tiny but odd-looking problem of appending "}" */
	/* to truncated lists during {}-wrapping earlier */
	if (strlen(buf) == buf_width) {
		buf[buf_width-1] = buf[buf_width-2] = buf[buf_width-3] = '.';
	}

	return(buf);
}

static
void
PrintStackBelow(interp,curf,viewf)
Tcl_Interp *interp;
CallFrame *curf;	/* current FramePtr */
CallFrame *viewf;	/* view FramePtr */
{
	char ptr;	/* graphically indicate where we are in the stack */

	/* indicate where we are in the stack */
	ptr = ((curf == viewf)?'*':' ');

	if (curf == 0) {
		print(interp,"%c0: %s\n",
				ptr,print_argv(interp,main_argc,main_argv));
	} else {
		PrintStackBelow(interp,curf->callerVarPtr,viewf);
		print(interp,"%c%d: %s\n",ptr,curf->level,
			print_argv(interp,curf->argc,curf->argv));
	}
}

static
void
PrintStack(interp,curf,viewf,argc,argv,level)
Tcl_Interp *interp;
CallFrame *curf;	/* current FramePtr */
CallFrame *viewf;	/* view FramePtr */
int argc;
char *argv[];
char *level;
{
	PrintStackBelow(interp,curf,viewf);
	
	print(interp," %s: %s\n",level,print_argv(interp,argc,argv));
}

/* return 0 if goal matches current frame or goal can't be found */
/*	anywere in frame stack */
/* else return 1 */
/* This catches things like a proc called from a Tcl_Eval which in */
/* turn was not called from a proc but some builtin such as source */
/* or Tcl_Eval.  These builtin calls to Tcl_Eval lose any knowledge */
/* the FramePtr from the proc, so we have to search the entire */
/* stack frame to see if it's still there. */
static int
GoalFrame(goal,iptr)
CallFrame *goal;
Interp *iptr;
{
	CallFrame *cf = iptr->varFramePtr;

	/* if at current level, return success immediately */
	if (goal == cf) return 0;

	while (cf) {
		cf = cf->callerVarPtr;
		if (goal == cf) {
			/* found, but since it's above us, fail */
			return 1;
		}
	}
	return 0;
}

/* debugger's trace handler */
/*ARGSUSED*/
static void
debugger_trap(clientData,interp,level,command,cmdProc,cmdClientData,argc,argv)
ClientData clientData;		/* not used */
Tcl_Interp *interp;
int level;			/* positive number if called by Tcl, -1 if */
				/* called by Dbg_On in which case we don't */
				/* know the level */
char *command;
int (*cmdProc)();		/* not used */
ClientData cmdClientData;
int argc;
char *argv[];
{
	char level_text[6];	/* textual representation of level */

	int break_status;
	Interp *iPtr = (Interp *)interp;

	CallFrame *trueFramePtr;	/* where the pc is */
	CallFrame *viewFramePtr;	/* where up/down are */

	int print_command_first_time = TRUE;
	static int debug_suspended = FALSE;

	struct breakpoint *b;

	/* skip commands that are invoked interactively */
	if (debug_suspended) return;

	/* skip debugger commands */
	if (argv[0][1] == '\0') {
		switch (argv[0][0]) {
		case 'n':
		case 's':
		case 'c':
		case 'r':
		case 'w':
		case 'b':
		case 'u':
		case 'd': return;
		}
	}

	if ((*ignoreproc)(interp,argv[0])) return;

	/* if level is unknown, use "?" */
	sprintf(level_text,(level == -1)?"?":"%d",level);

	/* save so we can restore later */
	trueFramePtr = iPtr->varFramePtr;

	/* do not allow breaking while testing breakpoints */
	debug_suspended = TRUE;

	/* test all breakpoints to see if we should break */
	/* if any successful breakpoints, start interactor */
	debug_new_action = FALSE;	/* reset strobe */
	break_status = FALSE;		/* no successful breakpoints yet */
	for (b = break_base;b;b=b->next) {
		break_status |= breakpoint_test(interp,command,b);
	}
	if (break_status) {
		if (!debug_new_action) goto start_interact;

		/* if s or n triggered by breakpoint, make "s 1" */
		/* (and so on) refer to next command, not this one */
/*		step_count++;*/
		goto end_interact;
	}

	switch (debug_cmd) {
	case cont:
		goto finish;
	case step:
		step_count--;
		if (step_count > 0) goto finish;
		goto start_interact;
	case next:
		/* check if we are back at the same level where the next */
		/* command was issued.  Also test */
		/* against all FramePtrs and if no match, assume that */
		/* we've missed a return, and so we should break  */
/*		if (goalFramePtr != iPtr->varFramePtr) goto finish;*/
		if (GoalFrame(goalFramePtr,iPtr)) goto finish;
		step_count--;
		if (step_count > 0) goto finish;
		goto start_interact;
	case Next:
		/* check if we are back at the same level where the next */
		/* command was issued.  */
		if (goalNumLevel < iPtr->numLevels) goto finish;
		step_count--;
		if (step_count > 0) goto finish;
		goto start_interact;
	case ret:
		/* same comment as in "case next" */
		if (goalFramePtr != iPtr->varFramePtr) goto finish;
		goto start_interact;
	}

start_interact:
	if (print_command_first_time) {
		print(interp,"%s: %s\n",
				level_text,print_argv(interp,1,&command));
		print_command_first_time = FALSE;
	}
	/* since user is typing a command, don't interrupt it immediately */
	debug_cmd = cont;
	debug_suspended = TRUE;

	/* interactor won't return until user gives a debugger cmd */
	(*interactor)(interp,interdata);
end_interact:

	/* save this so it can be restored after "w" command */
	viewFramePtr = iPtr->varFramePtr;

	if (debug_cmd == up || debug_cmd == down) {
		/* calculate new frame */
		if (-1 == TclGetFrame2(interp,trueFramePtr,viewFrameName,
					&iPtr->varFramePtr,debug_cmd)) {
			print(interp,"%s\n",interp->result);
			Tcl_ResetResult(interp);
		}
		goto start_interact;
	}

	/* reset view back to normal */
	iPtr->varFramePtr = trueFramePtr;

#if 0
	/* allow trapping */
	debug_suspended = FALSE;
#endif

	switch (debug_cmd) {
	case cont:
	case step:
		goto finish;
	case next:
		goalFramePtr = iPtr->varFramePtr;
		goto finish;
	case Next:
		goalNumLevel = iPtr->numLevels;
		goto finish;
	case ret:
		goalFramePtr = iPtr->varFramePtr;
		if (goalFramePtr == 0) {
			print(interp,"nowhere to return to\n");
			break;
		}
		goalFramePtr = goalFramePtr->callerVarPtr;
		goto finish;
	case where:
		PrintStack(interp,iPtr->varFramePtr,viewFramePtr,argc,argv,level_text);
		break;
	}

	/* restore view and restart interactor */
	iPtr->varFramePtr = viewFramePtr;
	goto start_interact;

 finish:
	debug_suspended = FALSE;
}

/*ARGSUSED*/
static
int
cmdNext(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
	debug_new_action = TRUE;
	debug_cmd = *(enum debug_cmd *)clientData;
	last_action_cmd = debug_cmd;

	step_count = (argc == 1)?1:atoi(argv[1]);
	last_step_count = step_count;
	return(TCL_RETURN);
}

/*ARGSUSED*/
static
int
cmdDir(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
	debug_cmd = *(enum debug_cmd *)clientData;

	if (argc == 1) argv[1] = "1";
	strncpy(viewFrameName,argv[1],FRAMENAMELEN);

	return TCL_RETURN;
}

/*ARGSUSED*/
static
int
cmdSimple(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
	debug_new_action = TRUE;
	debug_cmd = *(enum debug_cmd *)clientData;
	last_action_cmd = debug_cmd;

	return TCL_RETURN;
}

static
void
breakpoint_destroy(b)
struct breakpoint *b;
{
	if (b->file) ckfree(b->file);
	if (b->pat) ckfree(b->pat);
	if (b->re) ckfree((char *)b->re);			
	if (b->cmd) ckfree(b->cmd);

	/* unlink from chain */
	if ((b->previous == 0) && (b->next == 0)) {
		break_base = 0;
	} else if (b->previous == 0) {
		break_base = b->next;
		b->next->previous = 0;
	} else if (b->next == 0) {
		b->previous->next = 0;
	} else {
		b->previous->next = b->next;
		b->next->previous = b->previous;
	}

	ckfree((char *)b);
}

static void
savestr(straddr,str)
char **straddr;
char *str;
{
	*straddr = ckalloc(strlen(str)+1);
	strcpy(*straddr,str);
}

/* return 1 if a string is substring of a flag */
static int
flageq(flag,string,minlen)
char *flag;
char *string;
int minlen;		/* at least this many chars must match */
{
	for (;*flag;flag++,string++,minlen--) {
		if (*string == '\0') break;
		if (*string != *flag) return 0;
	}
	if (*string == '\0' && minlen <= 0) return 1;
	return 0;
}

/*ARGSUSED*/
static
int
cmdWhere(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
	if (argc == 1) {
		debug_cmd = where;
		return TCL_RETURN;
	}

	argc--; argv++;

	while (argc) {
		if (flageq("-width",*argv,2)) {
			argc--; argv++;
			if (*argv) {
				buf_width = atoi(*argv);
				argc--; argv++;
			} else print(interp,"%d\n",buf_width);
		} else if (flageq("-compress",*argv,2)) {
			argc--; argv++;
			if (*argv) {
				compress = atoi(*argv);
				argc--; argv++;
			} else print(interp,"%d\n",compress);
		} else {
			print(interp,"usage: w [-width #] [-compress 0|1]\n");
			return TCL_ERROR;
		}
	}
	return TCL_OK;
}

#define breakpoint_fail(msg) {error_msg = msg; goto break_fail;}

/*ARGSUSED*/
static
int
cmdBreak(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
	struct breakpoint *b;
	char *error_msg;

	argc--; argv++;

	if (argc < 1) {
		for (b = break_base;b;b=b->next) breakpoint_print(interp,b);
		return(TCL_OK);
	}

	if (argv[0][0] == '-') {
		if (argv[0][1] == '\0') {
			while (break_base) {
				breakpoint_destroy(break_base);
			}
			breakpoint_max_id = 0;
			return(TCL_OK);
		} else if (isdigit(argv[0][1])) {
			int id = atoi(argv[0]+1);

			for (b = break_base;b;b=b->next) {
				if (b->id == id) {
					breakpoint_destroy(b);
					if (!break_base) breakpoint_max_id = 0;
					return(TCL_OK);
				}
			}
			Tcl_SetResult(interp,"no such breakpoint",TCL_STATIC);
			return(TCL_ERROR);
		}
	}

	b = breakpoint_new();

	if (flageq("-regexp",argv[0],2)) {
		argc--; argv++;
#if TCL_MAJOR_VERSION == 6
		if ((argc > 0) && (b->re = regcomp(argv[0]))) {
#else
		if ((argc > 0) && (b->re = TclRegComp(argv[0]))) {
#endif
			savestr(&b->pat,argv[0]);
			argc--; argv++;
		} else {
			breakpoint_fail("bad regular expression")
		}
	} else if (flageq("-glob",argv[0],2)) {
		argc--; argv++;
		if (argc > 0) {
			savestr(&b->pat,argv[0]);
			argc--; argv++;
		} else {
			breakpoint_fail("no pattern?");
		}
	} else if ((!(flageq("if",*argv,1)) && (!(flageq("then",*argv,1))))) {
		/* look for [file:]line */
		char *colon;
		char *linep;	/* pointer to beginning of line number */

		colon = strchr(argv[0],':');
		if (colon) {
			*colon = '\0';
			savestr(&b->file,argv[0]);
			*colon = ':';
			linep = colon + 1;
		} else {
			linep = argv[0];
			/* get file from current scope */
			/* savestr(&b->file, ?); */
		}

		if (TCL_OK == Tcl_GetInt(interp,linep,&b->line)) {
			argc--; argv++;
			print(interp,"setting breakpoints by line number is currently unimplemented - use patterns or expressions\n");
		} else {
			/* not an int? - unwind & assume it is an expression */

			if (b->file) ckfree(b->file);
		}
	}

	if (argc > 0) {
		int do_if = FALSE;

		if (flageq("if",argv[0],1)) {
			argc--; argv++;
			do_if = TRUE;
		} else if (!flageq("then",argv[0],1)) {
			do_if = TRUE;
		}

		if (do_if) {
			if (argc < 1) {
				breakpoint_fail("if what");
			}

			savestr(&b->expr,argv[0]);
			argc--; argv++;
		}
	}

	if (argc > 0) {
		if (flageq("then",argv[0],1)) {
			argc--; argv++;
		}

		if (argc < 1) {
			breakpoint_fail("then what?");
		}

		savestr(&b->cmd,argv[0]);
	}

	sprintf(interp->result,"%d",b->id);
	return(TCL_OK);

 break_fail:
	breakpoint_destroy(b);
	Tcl_SetResult(interp,error_msg,TCL_STATIC);
	return(TCL_ERROR);
}

static char *help[] = {
"s [#]		step into procedure",
"n [#]		step over procedure",
"N [#]		step over procedures, commands, and arguments",
"c		continue",
"r		continue until return to caller",
"u [#]		move scope up level",
"d [#]		move scope down level",
"		go to absolute frame if # is prefaced by \"#\"",
"w		show stack (\"where\")",
"w -w [#]	show/set width",
"w -c [0|1]	show/set compress",
"b		show breakpoints",
"b [-r regexp-pattern] [if expr] [then command]",
"b [-g glob-pattern]   [if expr] [then command]",
"b [[file:]#]          [if expr] [then command]",
"		if pattern given, break if command resembles pattern",
"		if # given, break on line #",
"		if expr given, break if expr true",
"		if command given, execute command at breakpoint",
"b -#		delete breakpoint",
"b -		delete all breakpoints",
0};

/*ARGSUSED*/
static
int
cmdHelp(clientData, interp, argc, argv)
ClientData clientData;
Tcl_Interp *interp;
int argc;
char **argv;
{
	char **hp;

	for (hp=help;*hp;hp++) {
		print(interp,"%s\n",*hp);
	}

	return(TCL_OK);
}

/* occasionally, we print things larger buf_max but not by much */
/* see print statements in PrintStack routines for examples */
#define PAD 80

static void print(Tcl_Interp *interp, char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	if (!printproc) vprintf(fmt,args);
	else {
		static int buf_width_max = DEFAULT_WIDTH+PAD;
		static char buf_basic[DEFAULT_WIDTH+PAD+1];
		static char *buf = buf_basic;

		if (buf_width+PAD > buf_width_max) {
			if (buf && (buf != buf_basic)) ckfree(buf);
			buf = (char *)ckalloc(buf_width+PAD+1);
			buf_width_max = buf_width+PAD;
		}

		vsprintf(buf,fmt,args);
		(*printproc)(interp,buf,printdata);
	}
	va_end(args);
}

/*ARGSUSED*/
Dbg_InterStruct
Dbg_Interactor(interp,inter_proc,data)
Tcl_Interp *interp;
Dbg_InterProc *inter_proc;
ClientData data;
{
	Dbg_InterStruct tmp;

	tmp.func = interactor;
	tmp.data = interdata;
	interactor = (inter_proc?inter_proc:simple_interactor);
	interdata = data;
	return tmp;
}

/*ARGSUSED*/
Dbg_IgnoreFuncsProc *
Dbg_IgnoreFuncs(interp,proc)
Tcl_Interp *interp;
Dbg_IgnoreFuncsProc *proc;
{
	Dbg_IgnoreFuncsProc *tmp = ignoreproc;
	ignoreproc = (proc?proc:zero);
	return tmp;
}

/*ARGSUSED*/
Dbg_OutputStruct
Dbg_Output(interp,proc,data)
Tcl_Interp *interp;
Dbg_OutputProc *proc;
ClientData data;
{
	Dbg_OutputStruct tmp;

	tmp.func = printproc;
	tmp.data = printdata;
	printproc = proc;
	printdata = data;
	return tmp;
}

/*ARGSUSED*/
int
Dbg_Active(interp)
Tcl_Interp *interp;
{
	return debugger_active;
}

char **
Dbg_ArgcArgv(argc,argv,copy)
int argc;
char *argv[];
int copy;
{
	char **alloc;

	main_argc = argc;

	if (!copy) {
		main_argv = argv;
		alloc = 0;
	} else {
		main_argv = alloc = (char **)ckalloc((argc+1)*sizeof(char *));
		while (argc-- >= 0) {
			*main_argv++ = *argv++;
		}
		main_argv = alloc;
	}
	return alloc;
}

static struct cmd_list {
	char *cmdname;
	Tcl_CmdProc *cmdproc;
	enum debug_cmd cmdtype;
} cmd_list[]  = {
		{"n", cmdNext,   next},
		{"s", cmdNext,   step},
		{"N", cmdNext,   Next},
		{"c", cmdSimple, cont},
		{"r", cmdSimple, ret},
		{"w", cmdWhere,  none},
		{"b", cmdBreak,  none},
		{"u", cmdDir,    up},
		{"d", cmdDir,    down},
		{"h", cmdHelp,   none},
		{0}
};

/* this may seem excessive, but this avoids the explicit test for non-zero */
/* in the caller, and chances are that that test will always be pointless */
/*ARGSUSED*/
static int zero(interp,string)
Tcl_Interp *interp;
char *string;
{
	return 0;
}

static int
simple_interactor(interp)
Tcl_Interp *interp;
{
	int rc;
	char *ccmd;		/* pointer to complete command */
	char line[BUFSIZ+1];	/* space for partial command */
	int newcmd = TRUE;
	Interp *iPtr = (Interp *)interp;

#if TCL_MAJOR_VERSION == 6
	Tcl_CmdBuf buffer;

	if (!(buffer = Tcl_CreateCmdBuf())) {
		Tcl_AppendElement(interp,"no more space for cmd buffer",0);
		return(TCL_ERROR);
	}
#else
	Tcl_DString dstring;
	Tcl_DStringInit(&dstring);
#endif


	newcmd = TRUE;
	while (TRUE) {
		struct cmd_list *c;

		if (newcmd) {
			print(interp,"dbg%d.%d> ",iPtr->numLevels,iPtr->curEventNum+1);
		} else {
			print(interp,"dbg+> ");
		}
		fflush(stdout);

		if (0 >= (rc = read(0,line,BUFSIZ))) {
			if (!newcmd) line[0] = 0;
			else exit(0);
		} else line[rc] = '\0';

#if TCL_MAJOR_VERSION == 6
		if (NULL == (ccmd = Tcl_AssembleCmd(buffer,line))) {
#else
		ccmd = Tcl_DStringAppend(&dstring,line,rc);
		if (!Tcl_CommandComplete(ccmd)) {
#endif
			newcmd = FALSE;
			continue;	/* continue collecting command */
		}
		newcmd = TRUE;

#if TCL_MAJOR_VERSION != 6
		/* if user pressed return with no cmd, use previous one */
		if ((ccmd[0] == '\n' || ccmd[0] == '\r') && ccmd[1] == '\0') {

			/* this loop is guaranteed to exit through break */
			for (c = cmd_list;c->cmdname;c++) {
				if (c->cmdtype == last_action_cmd) break;
			}

			/* recreate textual version of command */
			Tcl_DStringAppend(&dstring,c->cmdname,-1);

			if (c->cmdtype == step ||
			    c->cmdtype == next ||
			    c->cmdtype == Next) {
				char num[10];

				sprintf(num," %d",last_step_count);
				Tcl_DStringAppend(&dstring,num,-1);
			}
		}
#endif

#if TCL_MAJOR_VERSION == 6
		rc = Tcl_Eval(interp,ccmd, 0, 0);
#elif TCL_MAJOR_VERSION == 7 && TCL_MINOR_VERSION < 4
		rc = Tcl_RecordAndEval(interp,ccmd,0);
#else
		rc = Tcl_RecordAndEval(interp,ccmd,TCL_NO_EVAL);
		rc = Tcl_Eval(interp,ccmd);
#endif
#if TCL_MAJOR_VERSION != 6
		Tcl_DStringFree(&dstring);
#endif
		switch (rc) {
		case TCL_OK:
			if (*interp->result != 0)
				print(interp,"%s\n",interp->result);
			continue;
		case TCL_ERROR:
			print(interp,"%s\n",Tcl_GetVar(interp,"errorInfo",TCL_GLOBAL_ONLY));
			/* since user is typing by hand, we expect lots
			   of errors, and want to give another chance */
			continue;
		case TCL_BREAK:
		case TCL_CONTINUE:
#define finish(x)	{rc = x; goto done;}
			finish(rc);
		case TCL_RETURN:
			finish(TCL_OK);
		default:
			/* note that ccmd has trailing newline */
			print(interp,"error %d: %s\n",rc,ccmd);
			continue;
		}
	}
	/* cannot fall thru here, must jump to label */
 done:
#if TCL_MAJOR_VERSION == 6
	/* currently, code guarantees buffer is valid */
	Tcl_DeleteCmdBuf(buffer);
#else
	Tcl_DStringFree(&dstring);
#endif

	return(rc);
}

static char init_auto_path[] = "lappend auto_path $dbg_library";

static void
init_debugger(interp)
Tcl_Interp *interp;
{
	struct cmd_list *c;

	for (c = cmd_list;c->cmdname;c++) {
		Tcl_CreateCommand(interp,c->cmdname,c->cmdproc,
			(ClientData)&c->cmdtype,(Tcl_CmdDeleteProc *)0);
	}

	debug_handle = Tcl_CreateTrace(interp,
				10000,debugger_trap,(ClientData)0);

	debugger_active = TRUE;
	Tcl_SetVar2(interp,Dbg_VarName,"active","1",0);
#ifdef DBG_SCRIPTDIR
	Tcl_SetVar(interp,"dbg_library",DBG_SCRIPTDIR,0);
#endif
#if TCL_MAJOR_VERSION == 6
	Tcl_Eval(interp,"source " DEBUGGER_INIT, 0, 0);
#else
	Tcl_Eval(interp,init_auto_path);
#endif

}

/* allows any other part of the application to jump to the debugger */
/*ARGSUSED*/
void
Dbg_On(interp,immediate)
Tcl_Interp *interp;
int immediate;		/* if true, stop immediately */
			/* should only be used in safe places */
			/* i.e., when Tcl_Eval can be called */
{
	if (!debugger_active) init_debugger(interp);

	debug_cmd = step;
	step_count = 1;

	if (immediate) {
		static char *fake_cmd = "--interrupted-- (command_unknown)";

		debugger_trap((ClientData)0,interp,-1,fake_cmd,(int (*)())0,
					(ClientData)0,1,&fake_cmd);
/*		(*interactor)(interp);*/
	}
}

void
Dbg_Off(interp)
Tcl_Interp *interp;
{
	struct cmd_list *c;

	if (!debugger_active) return;

	for (c = cmd_list;c->cmdname;c++) {
		Tcl_DeleteCommand(interp,c->cmdname);
	}

	Tcl_DeleteTrace(interp,debug_handle);
	debugger_active = FALSE;
	Tcl_UnsetVar(interp,Dbg_VarName,TCL_GLOBAL_ONLY);
}
