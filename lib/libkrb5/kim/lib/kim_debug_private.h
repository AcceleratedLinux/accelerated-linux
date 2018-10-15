/*
 * $Header$
 *
 * Copyright 2008 Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 * require a specific license from the United States Government.
 * It is the responsibility of any person or organization contemplating
 * export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */

#define kim_debug_printf(format, ...) __kim_debug_printf(__FUNCTION__, format, ## __VA_ARGS__)
void __kim_debug_printf (kim_string in_function, 
                         kim_string in_format, 
                         ...);

kim_error _check_error (kim_error  in_err, 
                        kim_string in_function, 
                        kim_string in_file, 
                        int          in_line);
#define check_error(err) _check_error(err, __FUNCTION__, __FILE__, __LINE__)

void kim_os_debug_print (kim_string in_string);
