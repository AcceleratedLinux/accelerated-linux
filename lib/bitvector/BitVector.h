#ifndef MODULE_BIT_VECTOR
#define MODULE_BIT_VECTOR
#ifdef __cplusplus
extern "C"
{
#endif
/*****************************************************************************/
/*  MODULE NAME:  BitVector.h                           MODULE TYPE:  (adt)  */
/*****************************************************************************/
/*  MODULE IMPORTS:                                                          */
/*****************************************************************************/
#include <stdlib.h>                                 /*  MODULE TYPE:  (sys)  */
#include <limits.h>                                 /*  MODULE TYPE:  (sys)  */
#include <string.h>                                 /*  MODULE TYPE:  (sys)  */
#include <ctype.h>                                  /*  MODULE TYPE:  (sys)  */
#include "ToolBox.h"                                /*  MODULE TYPE:  (dat)  */
/*****************************************************************************/
/*  MODULE INTERFACE:                                                        */
/*****************************************************************************/

typedef enum
    {
        ErrCode_Ok = 0,    /* everything went allright                       */

        ErrCode_Type,      /* types word and size_t have incompatible sizes  */
        ErrCode_Bits,      /* bits of word and sizeof(word) are inconsistent */
        ErrCode_Word,      /* size of word is less than 16 bits              */
        ErrCode_Long,      /* size of word is greater than size of long      */
        ErrCode_Powr,      /* number of bits of word is not a power of two   */
        ErrCode_Loga,      /* error in calculation of logarithm              */

        ErrCode_Null,      /* unable to allocate memory                      */

        ErrCode_Indx,      /* index out of range                             */
        ErrCode_Ordr,      /* minimum > maximum index                        */
        ErrCode_Size,      /* bit vector size mismatch                       */
        ErrCode_Pars,      /* input string syntax error                      */
        ErrCode_Ovfl,      /* numeric overflow error                         */
        ErrCode_Same,      /* operands must be distinct                      */
        ErrCode_Expo,      /* exponent must be positive                      */
        ErrCode_Zero       /* division by zero error                         */
    } ErrCode;

typedef wordptr *listptr;

/* ===> MISCELLANEOUS BASIC FUNCTIONS: <=== */

charptr BitVector_Error      (ErrCode error);  /* return string for err code */

ErrCode BitVector_Boot       (void);                 /* 0 = ok, 1..7 = error */

N_word  BitVector_Size       (N_int bits);  /* bit vector size (# of words)  */
N_word  BitVector_Mask       (N_int bits);  /* bit vector mask (unused bits) */

/* ===> CLASS METHODS: <=== */

charptr BitVector_Version    (void);                /* return version string */

N_int   BitVector_Word_Bits  (void);     /* return # of bits in machine word */
N_int   BitVector_Long_Bits  (void);    /* return # of bits in unsigned long */

/* ===> CONSTRUCTOR METHODS: <=== */

wordptr BitVector_Create     (N_int bits, boolean clear);          /* malloc */
listptr BitVector_Create_List(N_int bits, boolean clear, N_int count);

wordptr BitVector_Resize     (wordptr oldaddr, N_int bits);       /* realloc */

wordptr BitVector_Shadow     (wordptr addr); /* make new same size but empty */
wordptr BitVector_Clone      (wordptr addr);         /* make exact duplicate */

wordptr BitVector_Concat     (wordptr X, wordptr Y); /* return concatenation */

/* ===> DESTRUCTOR METHODS: <=== */

void    BitVector_Dispose            (charptr string);             /* string */
void    BitVector_Destroy            (wordptr addr);               /* bitvec */
void    BitVector_Destroy_List       (listptr list, N_int count);  /* list   */

/* ===> OBJECT METHODS: <=== */

/* ===> bit vector copy function: */

void    BitVector_Copy       (wordptr X, wordptr Y);              /* X = Y   */

/* ===> bit vector initialization: */

void    BitVector_Empty      (wordptr addr);                      /* X = {}  */
void    BitVector_Fill       (wordptr addr);                      /* X = ~{} */
void    BitVector_Flip       (wordptr addr);                      /* X = ~X  */

void    BitVector_Primes     (wordptr addr);

/* ===> miscellaneous functions: */

void    BitVector_Reverse    (wordptr X, wordptr Y);

/* ===> bit vector interval operations and functions: */

void    BitVector_Interval_Empty     (wordptr addr, N_int lower, N_int upper);
void    BitVector_Interval_Fill      (wordptr addr, N_int lower, N_int upper);
void    BitVector_Interval_Flip      (wordptr addr, N_int lower, N_int upper);
void    BitVector_Interval_Reverse   (wordptr addr, N_int lower, N_int upper);

boolean BitVector_interval_scan_inc  (wordptr addr, N_int start,
                                      N_intptr min, N_intptr max);
boolean BitVector_interval_scan_dec  (wordptr addr, N_int start,
                                      N_intptr min, N_intptr max);

void    BitVector_Interval_Copy      (wordptr X, wordptr Y, N_int Xoffset,
                                      N_int Yoffset, N_int length);

wordptr BitVector_Interval_Substitute(wordptr X, wordptr Y,
                                      N_int Xoffset, N_int Xlength,
                                      N_int Yoffset, N_int Ylength);

/* ===> bit vector test functions: */

boolean BitVector_is_empty   (wordptr addr);                  /* X == {} ?   */
boolean BitVector_is_full    (wordptr addr);                  /* X == ~{} ?  */

boolean BitVector_equal      (wordptr X, wordptr Y);          /* X == Y ?    */
Z_int   BitVector_Lexicompare(wordptr X, wordptr Y);          /* X <,=,> Y ? */
Z_int   BitVector_Compare    (wordptr X, wordptr Y);          /* X <,=,> Y ? */

/* ===> bit vector string conversion functions: */

charptr BitVector_to_Hex     (wordptr addr);
ErrCode BitVector_from_Hex   (wordptr addr, charptr string);

charptr BitVector_to_Bin     (wordptr addr);
ErrCode BitVector_from_Bin   (wordptr addr, charptr string);

charptr BitVector_to_Dec     (wordptr addr);
ErrCode BitVector_from_Dec   (wordptr addr, charptr string);

charptr BitVector_to_Enum    (wordptr addr);
ErrCode BitVector_from_Enum  (wordptr addr, charptr string);

/* ===> bit vector bit operations, functions & tests: */

void    BitVector_Bit_Off    (wordptr addr, N_int index); /*  X = X \ {x}    */
void    BitVector_Bit_On     (wordptr addr, N_int index); /*  X = X + {x}    */
boolean BitVector_bit_flip   (wordptr addr, N_int index); /* (X+{x})\(X*{x}) */

boolean BitVector_bit_test   (wordptr addr, N_int index); /*  {x} in X ?     */

void    BitVector_Bit_Copy   (wordptr addr, N_int index, boolean bit);

/* ===> bit vector bit shift & rotate functions: */

void    BitVector_LSB                (wordptr addr, boolean bit);
void    BitVector_MSB                (wordptr addr, boolean bit);
boolean BitVector_lsb_               (wordptr addr);
boolean BitVector_msb_               (wordptr addr);
boolean BitVector_rotate_left        (wordptr addr);
boolean BitVector_rotate_right       (wordptr addr);
boolean BitVector_shift_left         (wordptr addr, boolean carry_in);
boolean BitVector_shift_right        (wordptr addr, boolean carry_in);
void    BitVector_Move_Left          (wordptr addr, N_int bits);
void    BitVector_Move_Right         (wordptr addr, N_int bits);

/* ===> bit vector insert/delete bits: */

void    BitVector_Insert     (wordptr addr, N_int offset, N_int count,
                              boolean clear);
void    BitVector_Delete     (wordptr addr, N_int offset, N_int count,
                              boolean clear);

/* ===> bit vector arithmetic: */

boolean BitVector_increment  (wordptr addr);                        /*  X++  */
boolean BitVector_decrement  (wordptr addr);                        /*  X--  */

boolean BitVector_compute    (wordptr X, wordptr Y, wordptr Z, boolean minus,
                                                               boolean *carry);
boolean BitVector_add        (wordptr X, wordptr Y, wordptr Z, boolean *carry);
boolean BitVector_sub        (wordptr X, wordptr Y, wordptr Z, boolean *carry);
boolean BitVector_inc        (wordptr X, wordptr Y);
boolean BitVector_dec        (wordptr X, wordptr Y);

void    BitVector_Negate     (wordptr X, wordptr Y);
void    BitVector_Absolute   (wordptr X, wordptr Y);
Z_int   BitVector_Sign       (wordptr addr);
ErrCode BitVector_Mul_Pos    (wordptr X, wordptr Y, wordptr Z, boolean strict);
ErrCode BitVector_Multiply   (wordptr X, wordptr Y, wordptr Z);
ErrCode BitVector_Div_Pos    (wordptr Q, wordptr X, wordptr Y, wordptr R);
ErrCode BitVector_Divide     (wordptr Q, wordptr X, wordptr Y, wordptr R);
ErrCode BitVector_GCD        (wordptr X, wordptr Y, wordptr Z);
ErrCode BitVector_GCD2       (wordptr U, wordptr V, wordptr W,      /*   O   */
                                         wordptr X, wordptr Y);     /*   I   */
ErrCode BitVector_Power      (wordptr X, wordptr Y, wordptr Z);

/* ===> direct memory access functions: */

void    BitVector_Block_Store(wordptr addr, charptr buffer, N_int length);
charptr BitVector_Block_Read (wordptr addr, N_intptr length);

/* ===> word array functions: */

void    BitVector_Word_Store (wordptr addr, N_int offset, N_int value);
N_int   BitVector_Word_Read  (wordptr addr, N_int offset);

void    BitVector_Word_Insert(wordptr addr, N_int offset, N_int count,
                              boolean clear);
void    BitVector_Word_Delete(wordptr addr, N_int offset, N_int count,
                              boolean clear);

/* ===> arbitrary size chunk functions: */

void    BitVector_Chunk_Store(wordptr addr, N_int chunksize,
                              N_int offset, N_long value);
N_long  BitVector_Chunk_Read (wordptr addr, N_int chunksize,
                              N_int offset);

/* ===> set operations: */

void    Set_Union            (wordptr X, wordptr Y, wordptr Z); /* X = Y + Z */
void    Set_Intersection     (wordptr X, wordptr Y, wordptr Z); /* X = Y * Z */
void    Set_Difference       (wordptr X, wordptr Y, wordptr Z); /* X = Y \ Z */
void    Set_ExclusiveOr      (wordptr X, wordptr Y, wordptr Z); /*(Y+Z)\(Y*Z)*/
void    Set_Complement       (wordptr X, wordptr Y);            /* X = ~Y    */

/* ===> set functions: */

boolean Set_subset           (wordptr X, wordptr Y);            /* X in Y ?  */

N_int   Set_Norm             (wordptr addr);                    /* = | X |   */
N_int   Set_Norm2            (wordptr addr);                    /* = | X |   */
N_int   Set_Norm3            (wordptr addr);                    /* = | X |   */
Z_long  Set_Min              (wordptr addr);                    /* = min(X)  */
Z_long  Set_Max              (wordptr addr);                    /* = max(X)  */

/* ===> matrix-of-booleans operations: */

void    Matrix_Multiplication(wordptr X, N_int rowsX, N_int colsX,
                              wordptr Y, N_int rowsY, N_int colsY,
                              wordptr Z, N_int rowsZ, N_int colsZ);

void    Matrix_Product       (wordptr X, N_int rowsX, N_int colsX,
                              wordptr Y, N_int rowsY, N_int colsY,
                              wordptr Z, N_int rowsZ, N_int colsZ);

void    Matrix_Closure       (wordptr addr, N_int rows, N_int cols);

void    Matrix_Transpose     (wordptr X, N_int rowsX, N_int colsX,
                              wordptr Y, N_int rowsY, N_int colsY);

/*****************************************************************************/
/*  MODULE RESOURCES:                                                        */
/*****************************************************************************/

#define bits_(BitVector) *(BitVector-3)
#define size_(BitVector) *(BitVector-2)
#define mask_(BitVector) *(BitVector-1)

#define  ERRCODE_TYPE  "sizeof(word) > sizeof(size_t)"
#define  ERRCODE_BITS  "bits(word) != sizeof(word)*8"
#define  ERRCODE_WORD  "bits(word) < 16"
#define  ERRCODE_LONG  "bits(word) > bits(long)"
#define  ERRCODE_POWR  "bits(word) != 2^x"
#define  ERRCODE_LOGA  "bits(word) != 2^ld(bits(word))"
#define  ERRCODE_NULL  "unable to allocate memory"
#define  ERRCODE_INDX  "index out of range"
#define  ERRCODE_ORDR  "minimum > maximum index"
#define  ERRCODE_SIZE  "bit vector size mismatch"
#define  ERRCODE_PARS  "input string syntax error"
#define  ERRCODE_OVFL  "numeric overflow error"
#define  ERRCODE_SAME  "result vector(s) must be distinct"
#define  ERRCODE_EXPO  "exponent must be positive"
#define  ERRCODE_ZERO  "division by zero error"
#define  ERRCODE_OOPS  "unexpected internal error - please contact author"

extern const N_int BitVector_BYTENORM[256];
/*
{
    0x00, 0x01, 0x01, 0x02,  0x01, 0x02, 0x02, 0x03,
    0x01, 0x02, 0x02, 0x03,  0x02, 0x03, 0x03, 0x04,
    0x01, 0x02, 0x02, 0x03,  0x02, 0x03, 0x03, 0x04,
    0x02, 0x03, 0x03, 0x04,  0x03, 0x04, 0x04, 0x05,
    0x01, 0x02, 0x02, 0x03,  0x02, 0x03, 0x03, 0x04,
    0x02, 0x03, 0x03, 0x04,  0x03, 0x04, 0x04, 0x05,
    0x02, 0x03, 0x03, 0x04,  0x03, 0x04, 0x04, 0x05,
    0x03, 0x04, 0x04, 0x05,  0x04, 0x05, 0x05, 0x06,
    0x01, 0x02, 0x02, 0x03,  0x02, 0x03, 0x03, 0x04,
    0x02, 0x03, 0x03, 0x04,  0x03, 0x04, 0x04, 0x05,
    0x02, 0x03, 0x03, 0x04,  0x03, 0x04, 0x04, 0x05,
    0x03, 0x04, 0x04, 0x05,  0x04, 0x05, 0x05, 0x06,
    0x02, 0x03, 0x03, 0x04,  0x03, 0x04, 0x04, 0x05,
    0x03, 0x04, 0x04, 0x05,  0x04, 0x05, 0x05, 0x06,
    0x03, 0x04, 0x04, 0x05,  0x04, 0x05, 0x05, 0x06,
    0x04, 0x05, 0x05, 0x06,  0x05, 0x06, 0x06, 0x07,
    0x01, 0x02, 0x02, 0x03,  0x02, 0x03, 0x03, 0x04,
    0x02, 0x03, 0x03, 0x04,  0x03, 0x04, 0x04, 0x05,
    0x02, 0x03, 0x03, 0x04,  0x03, 0x04, 0x04, 0x05,
    0x03, 0x04, 0x04, 0x05,  0x04, 0x05, 0x05, 0x06,
    0x02, 0x03, 0x03, 0x04,  0x03, 0x04, 0x04, 0x05,
    0x03, 0x04, 0x04, 0x05,  0x04, 0x05, 0x05, 0x06,
    0x03, 0x04, 0x04, 0x05,  0x04, 0x05, 0x05, 0x06,
    0x04, 0x05, 0x05, 0x06,  0x05, 0x06, 0x06, 0x07,
    0x02, 0x03, 0x03, 0x04,  0x03, 0x04, 0x04, 0x05,
    0x03, 0x04, 0x04, 0x05,  0x04, 0x05, 0x05, 0x06,
    0x03, 0x04, 0x04, 0x05,  0x04, 0x05, 0x05, 0x06,
    0x04, 0x05, 0x05, 0x06,  0x05, 0x06, 0x06, 0x07,
    0x03, 0x04, 0x04, 0x05,  0x04, 0x05, 0x05, 0x06,
    0x04, 0x05, 0x05, 0x06,  0x05, 0x06, 0x06, 0x07,
    0x04, 0x05, 0x05, 0x06,  0x05, 0x06, 0x06, 0x07,
    0x05, 0x06, 0x06, 0x07,  0x06, 0x07, 0x07, 0x08
};
*/

/*****************************************************************************/
/*  MODULE IMPLEMENTATION:                                                   */
/*****************************************************************************/

/*****************************************************************************/
/*  VERSION:  6.4                                                            */
/*****************************************************************************/
/*  VERSION HISTORY:                                                         */
/*****************************************************************************/
/*                                                                           */
/*    Version 6.4  03.10.04  Added C++ comp. directives. Improved "Norm()".  */
/*    Version 6.3  28.09.02  Added "Create_List()" and "GCD2()".             */
/*    Version 6.2  15.09.02  Overhauled error handling. Fixed "GCD()".       */
/*    Version 6.1  08.10.01  Make VMS linker happy: _lsb,_msb => _lsb_,_msb_ */
/*    Version 6.0  08.10.00  Corrected overflow handling.                    */
/*    Version 5.8  14.07.00  Added "Power()". Changed "Copy()".              */
/*    Version 5.7  19.05.99  Quickened "Div_Pos()". Added "Product()".       */
/*    Version 5.6  02.11.98  Leading zeros eliminated in "to_Hex()".         */
/*    Version 5.5  21.09.98  Fixed bug of uninitialized "error" in Multiply. */
/*    Version 5.4  07.09.98  Fixed bug of uninitialized "error" in Divide.   */
/*    Version 5.3  12.05.98  Improved Norm. Completed history.               */
/*    Version 5.2  31.03.98  Improved Norm.                                  */
/*    Version 5.1  09.03.98  No changes.                                     */
/*    Version 5.0  01.03.98  Major additions and rewrite.                    */
/*    Version 4.2  16.07.97  Added is_empty, is_full.                        */
/*    Version 4.1  30.06.97  Added word-ins/del, move-left/right, inc/dec.   */
/*    Version 4.0  23.04.97  Rewrite. Added bit shift and bool. matrix ops.  */
/*    Version 3.2  04.02.97  Added interval methods.                         */
/*    Version 3.1  21.01.97  Fixed bug on 64 bit machines.                   */
/*    Version 3.0  12.01.97  Added flip.                                     */
/*    Version 2.0  14.12.96  Efficiency and consistency improvements.        */
/*    Version 1.1  08.01.96  Added Resize and ExclusiveOr.                   */
/*    Version 1.0  14.12.95  First version under UNIX (with Perl module).    */
/*    Version 0.9  01.11.93  First version of C library under MS-DOS.        */
/*    Version 0.1  ??.??.89  First version in Turbo Pascal under CP/M.       */
/*                                                                           */
/*****************************************************************************/
/*  AUTHOR:                                                                  */
/*****************************************************************************/
/*                                                                           */
/*    Steffen Beyer                                                          */
/*    mailto:sb@engelschall.com                                              */
/*    http://www.engelschall.com/u/sb/download/                              */
/*                                                                           */
/*****************************************************************************/
/*  COPYRIGHT:                                                               */
/*****************************************************************************/
/*                                                                           */
/*    Copyright (c) 1995 - 2004 by Steffen Beyer.                            */
/*    All rights reserved.                                                   */
/*                                                                           */
/*****************************************************************************/
/*  LICENSE:                                                                 */
/*****************************************************************************/
/*                                                                           */
/*    This library is free software; you can redistribute it and/or          */
/*    modify it under the terms of the GNU Library General Public            */
/*    License as published by the Free Software Foundation; either           */
/*    version 2 of the License, or (at your option) any later version.       */
/*                                                                           */
/*    This library is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU       */
/*    Library General Public License for more details.                       */
/*                                                                           */
/*    You should have received a copy of the GNU Library General Public      */
/*    License along with this library; if not, write to the                  */
/*    Free Software Foundation, Inc.,                                        */
/*    59 Temple Place, Suite 330, Boston, MA 02111-1307 USA                  */
/*                                                                           */
/*    or download a copy from ftp://ftp.gnu.org/pub/gnu/COPYING.LIB-2.0      */
/*                                                                           */
/*****************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
