/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007
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

#define OS_ARCH "x86_64"

#define HANDLER_TABLE_T static const void
#define DOUBLE_1_BITS 0x3ff0000000000000LL

#define READ_DBL(v,p,l)	v = ((u8)p[0]<<56)|((u8)p[1]<<48)|((u8)p[2]<<40) \
                            |((u8)p[3]<<32)|((u8)p[4]<<24)|((u8)p[5]<<16) \
                            |((u8)p[6]<<8)|(u8)p[7]; p+=8

extern void setDoublePrecision();
#define FPU_HACK setDoublePrecision()

#define COMPARE_AND_SWAP(addr, old_val, new_val)   \
({                                                 \
    char result;                                   \
    int read_val;                                  \
    __asm__ __volatile__ ("                        \
        lock;                                      \
        cmpxchgq %5, %1;                           \
        sete %0"                                   \
    : "=q" (result), "=m" (*addr), "=a" (read_val) \
    : "m" (*addr), "a" ((uintptr_t)old_val),       \
      "r" ((uintptr_t)new_val)                     \
    : "memory");                                   \
    result;                                        \
})

#define FLUSH_CACHE(addr, length)

#define LOCKWORD_READ(addr) *addr
#define LOCKWORD_WRITE(addr, value) *addr = value
#define LOCKWORD_COMPARE_AND_SWAP(addr, old_val, new_val) \
        COMPARE_AND_SWAP(addr, old_val, new_val)

#define UNLOCK_MBARRIER() __asm__ __volatile__ ("" ::: "memory")
#define JMM_LOCK_MBARRIER() __asm__ __volatile__ ("" ::: "memory")
#define JMM_UNLOCK_MBARRIER() __asm__ __volatile__ ("" ::: "memory")
#define MBARRIER() __asm__ __volatile__ ("lock; addl $0,0(%%rsp)" ::: "memory")
