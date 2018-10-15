/* Copyright (C) 2002 Jean-Marc Valin 
   File: stack_alloc.h
   
   Temporary memory allocation on stack

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   
   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
   
   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
   
   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.
   
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STACK_ALLOC_H
#define STACK_ALLOC_H

#if 0

/*Aligns the stack to a 'size' boundary */
#define ALIGN(stack, size) (stack=(void*)((((int)stack)+((size)-1)) & (-(size))))
/*Aligns the stack to a 'size' boundary minus k */
#define ALIGN_1(stack, size, k) (stack=(void*)(((((int)stack)+((size)-1+(k))) & (-(size)))-(k)))

/* Allocates 'size' elements of type 'type' on the stack */
#define PUSH(stack, size, type) (ALIGN(stack,sizeof(type)),stack=(void*)(((int)stack)+((size)*sizeof(type))),(type*)(((int)stack)-((size)*sizeof(type))))


#else

/*Aligns the stack to a 'size' boundary */
#define ALIGN(stack, size) ((stack) += ((size) - (int)(stack)) & ((size) - 1))

/* Allocates 'size' elements of type 'type' on the stack */
#define PUSH(stack, size, type) (ALIGN((stack),sizeof(type)),(stack)+=((size)*sizeof(type)),(type*)((stack)-((size)*sizeof(type))))

/* Allocates a struct stack */
#define PUSHS(stack, type) (ALIGN((stack),sizeof(long)),(stack)+=(sizeof(type)),(type*)((stack)-(sizeof(type))))


#endif

#endif
