/*
 * COPYRIGHT (c) 2006
 * The Regents of the University of Michigan
 * ALL RIGHTS RESERVED
 * 
 * Permission is granted to use, copy, create derivative works
 * and redistribute this software and such derivative works
 * for any purpose, so long as the name of The University of
 * Michigan is not used in any advertising or publicity
 * pertaining to the use of distribution of this software
 * without specific, written prior authorization.  If the
 * above copyright notice or any other identification of the
 * University of Michigan is included in any copy of any
 * portion of this software, then the disclaimer below must
 * also be included.
 * 
 * THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION
 * FROM THE UNIVERSITY OF MICHIGAN AS TO ITS FITNESS FOR ANY
 * PURPOSE, AND WITHOUT WARRANTY BY THE UNIVERSITY OF
 * MICHIGAN OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING
 * WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE
 * REGENTS OF THE UNIVERSITY OF MICHIGAN SHALL NOT BE LIABLE
 * FOR ANY DAMAGES, INCLUDING SPECIAL, INDIRECT, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OF THE SOFTWARE, EVEN
 * IF IT HAS BEEN OR IS HEREAFTER ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGES.
 */

/*
 * Create a key given random data.  It is assumed that random_key has
 * already been initialized and random_key->contents have been allocated
 * with the correct length.
 */
#include "k5-int.h"
#include "etypes.h"

krb5_error_code KRB5_CALLCONV
krb5_c_random_to_key(krb5_context context, krb5_enctype enctype,
		     krb5_data *random_data, krb5_keyblock *random_key)
{
    int i;
    krb5_error_code ret;
    const struct krb5_enc_provider *enc;

    if (random_data == NULL || random_key == NULL)
	return(EINVAL);

    if (random_key->contents == NULL)
	return(EINVAL);

    for (i=0; i<krb5_enctypes_length; i++) {
	if (krb5_enctypes_list[i].etype == enctype)
	    break;
    }

    if (i == krb5_enctypes_length)
	return(KRB5_BAD_ENCTYPE);

    enc = krb5_enctypes_list[i].enc;

    if (random_key->length != enc->keylength)
	return(KRB5_BAD_KEYSIZE);

    ret = ((*(enc->make_key))(random_data, random_key));

    if (ret) {
	memset(random_key->contents, 0, random_key->length);
    }

    return(ret);
}
