/* Copyright (C) 2000-2003 MySQL AB

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */


/*
 Functions to copy data to or from fields
 This could be done with a single short function but opencooding this
 gives much more speed.
 */

#include "mysql_priv.h"
#include <m_ctype.h>

static void do_field_eq(Copy_field *copy)
{
  memcpy(copy->to_ptr,copy->from_ptr,copy->from_length);
}

static void do_field_1(Copy_field *copy)
{
  copy->to_ptr[0]=copy->from_ptr[0];
}

static void do_field_2(Copy_field *copy)
{
  copy->to_ptr[0]=copy->from_ptr[0];
  copy->to_ptr[1]=copy->from_ptr[1];
}

static void do_field_3(Copy_field *copy)
{
  copy->to_ptr[0]=copy->from_ptr[0];
  copy->to_ptr[1]=copy->from_ptr[1];
  copy->to_ptr[2]=copy->from_ptr[2];
}

static void do_field_4(Copy_field *copy)
{
  copy->to_ptr[0]=copy->from_ptr[0];
  copy->to_ptr[1]=copy->from_ptr[1];
  copy->to_ptr[2]=copy->from_ptr[2];
  copy->to_ptr[3]=copy->from_ptr[3];
}

static void do_field_6(Copy_field *copy)
{						// For blob field
  copy->to_ptr[0]=copy->from_ptr[0];
  copy->to_ptr[1]=copy->from_ptr[1];
  copy->to_ptr[2]=copy->from_ptr[2];
  copy->to_ptr[3]=copy->from_ptr[3];
  copy->to_ptr[4]=copy->from_ptr[4];
  copy->to_ptr[5]=copy->from_ptr[5];
}

static void do_field_8(Copy_field *copy)
{
  copy->to_ptr[0]=copy->from_ptr[0];
  copy->to_ptr[1]=copy->from_ptr[1];
  copy->to_ptr[2]=copy->from_ptr[2];
  copy->to_ptr[3]=copy->from_ptr[3];
  copy->to_ptr[4]=copy->from_ptr[4];
  copy->to_ptr[5]=copy->from_ptr[5];
  copy->to_ptr[6]=copy->from_ptr[6];
  copy->to_ptr[7]=copy->from_ptr[7];
}


static void do_field_to_null_str(Copy_field *copy)
{
  if (*copy->from_null_ptr & copy->from_bit)
  {
    bzero(copy->to_ptr,copy->from_length);
    copy->to_null_ptr[0]=1;			// Always bit 1
  }
  else
  {
    copy->to_null_ptr[0]=0;
    memcpy(copy->to_ptr,copy->from_ptr,copy->from_length);
  }
}


static void do_outer_field_to_null_str(Copy_field *copy)
{
  if (*copy->null_row ||
      copy->from_null_ptr && (*copy->from_null_ptr & copy->from_bit))
  {
    bzero(copy->to_ptr,copy->from_length);
    copy->to_null_ptr[0]=1;			// Always bit 1
  }
  else
  {
    copy->to_null_ptr[0]=0;
    memcpy(copy->to_ptr,copy->from_ptr,copy->from_length);
  }
}


bool
set_field_to_null(Field *field)
{
  if (field->real_maybe_null())
  {
    field->set_null();
    field->reset();
    return 0;
  }
  field->reset();
  if (current_thd->count_cuted_fields)
  {
    current_thd->cuted_fields++;		// Increment error counter
    return 0;
  }
  if (!current_thd->no_errors)
    my_printf_error(ER_BAD_NULL_ERROR,ER(ER_BAD_NULL_ERROR),MYF(0),
		    field->field_name);
  return 1;
}


/*
  Set field to NULL or TIMESTAMP or to next auto_increment number

  SYNOPSIS
    set_field_to_null_with_conversions()
    field		Field to update
    no_conversion	Set to 1 if we should return 1 if field can't
			take null values.
			If set to 0 we will do store the 'default value'
			if the field is a special field. If not we will
			give an error.

  RETURN VALUES
    0		Field could take 0 or an automatic conversion was used
    1		Field could not take NULL and no conversion was used.
		If no_conversion was not set, an error message is printed
*/

bool
set_field_to_null_with_conversions(Field *field, bool no_conversions)
{
  if (field->real_maybe_null())
  {
    field->set_null();
    field->reset();
    return 0;
  }
  if (no_conversions)
    return 1;

  /*
    Check if this is a special type, which will get a special walue
    when set to NULL
  */
  if (field->type() == FIELD_TYPE_TIMESTAMP)
  {
    ((Field_timestamp*) field)->set_time();
    return 0;					// Ok to set time to NULL
  }
  field->reset();
  if (field == field->table->next_number_field)
    return 0;					// field is set in handler.cc
  if (current_thd->count_cuted_fields)
  {
    current_thd->cuted_fields++;		// Increment error counter
    return 0;
  }
  if (!current_thd->no_errors)
    my_printf_error(ER_BAD_NULL_ERROR,ER(ER_BAD_NULL_ERROR),MYF(0),
		    field->field_name);
  return 1;
}


static void do_skip(Copy_field *copy __attribute__((unused)))
{
}


static void do_copy_null(Copy_field *copy)
{
  if (*copy->from_null_ptr & copy->from_bit)
  {
    *copy->to_null_ptr|=copy->to_bit;
    copy->to_field->reset();
  }
  else
  {
    *copy->to_null_ptr&= ~copy->to_bit;
    (copy->do_copy2)(copy);
  }
}


static void do_outer_field_null(Copy_field *copy)
{
  if (*copy->null_row ||
      copy->from_null_ptr && (*copy->from_null_ptr & copy->from_bit))
  {
    *copy->to_null_ptr|=copy->to_bit;
    copy->to_field->reset();
  }
  else
  {
    *copy->to_null_ptr&= ~copy->to_bit;
    (copy->do_copy2)(copy);
  }
}


static void do_copy_not_null(Copy_field *copy)
{
  if (*copy->from_null_ptr & copy->from_bit)
  {
    current_thd->cuted_fields++;
    copy->to_field->reset();
  }
  else
    (copy->do_copy2)(copy);
}


static void do_copy_maybe_null(Copy_field *copy)
{
  *copy->to_null_ptr&= ~copy->to_bit;
  (copy->do_copy2)(copy);
}

/* timestamp and next_number has special handling in case of NULL values */

static void do_copy_timestamp(Copy_field *copy)
{
  if (*copy->from_null_ptr & copy->from_bit)
  {
    ((Field_timestamp*) copy->to_field)->set_time();// Same as set_field_to_null
  }
  else
    (copy->do_copy2)(copy);
}


static void do_copy_next_number(Copy_field *copy)
{
  if (*copy->from_null_ptr & copy->from_bit)
    copy->to_field->reset();			// Same as set_field_to_null
  else
    (copy->do_copy2)(copy);
}


static void do_copy_blob(Copy_field *copy)
{
  ulong length=((Field_blob*) copy->from_field)->get_length();
  ((Field_blob*) copy->to_field)->store_length(length);
  memcpy_fixed(copy->to_ptr,copy->from_ptr,sizeof(char*));
}

static void do_conv_blob(Copy_field *copy)
{
  copy->from_field->val_str(&copy->tmp,&copy->tmp);
  ((Field_blob *) copy->to_field)->store(copy->tmp.ptr(),
					 copy->tmp.length());
}

/* Save blob in copy->tmp for GROUP BY */

static void do_save_blob(Copy_field *copy)
{
  char buff[MAX_FIELD_WIDTH];
  String res(buff,sizeof(buff));
  copy->from_field->val_str(&res,&res);
  copy->tmp.copy(res);
  ((Field_blob *) copy->to_field)->store(copy->tmp.ptr(),
					 copy->tmp.length());
}


static void do_field_string(Copy_field *copy)
{
  char buff[MAX_FIELD_WIDTH];
  copy->tmp.set_quick(buff,sizeof(buff));
  copy->from_field->val_str(&copy->tmp,&copy->tmp);
  copy->to_field->store(copy->tmp.c_ptr_quick(),copy->tmp.length());
}


static void do_field_int(Copy_field *copy)
{
  longlong value=copy->from_field->val_int();
  copy->to_field->store(value);
}

static void do_field_real(Copy_field *copy)
{
  double value=copy->from_field->val_real();
  copy->to_field->store(value);
}


static void do_cut_string(Copy_field *copy)
{						// Shorter string field
  memcpy(copy->to_ptr,copy->from_ptr,copy->to_length);

  /* Check if we loosed any important characters */
  char *ptr,*end;
  for (ptr=copy->from_ptr+copy->to_length,end=copy->from_ptr+copy->from_length ;
       ptr != end ;
       ptr++)
  {
    if (!isspace(*ptr))
    {
      current_thd->cuted_fields++;		// Give a warning
      break;
    }
  }
}


static void do_expand_string(Copy_field *copy)
{
  memcpy(copy->to_ptr,copy->from_ptr,copy->from_length);
  bfill(copy->to_ptr+copy->from_length,copy->to_length-copy->from_length,' ');
}

static void do_varstring(Copy_field *copy)
{
  uint length=uint2korr(copy->from_ptr);
  if (length > copy->to_length-2)
  {
    length=copy->to_length-2;
    if (current_thd->count_cuted_fields)
      current_thd->cuted_fields++;		// Increment error counter
  }
  int2store(copy->to_ptr,length);
  memcpy(copy->to_ptr+2, copy->from_ptr,length);
}

/***************************************************************************
** The different functions that fills in a Copy_field class
***************************************************************************/

/*
  copy of field to maybe null string.
  If field is null then the all bytes are set to 0.
  if field is not null then the first byte is set to 1 and the rest of the
  string is the field value.
  The 'to' buffer should have a size of field->pack_length()+1
*/

void Copy_field::set(char *to,Field *from)
{
  from_ptr=from->ptr;
  to_ptr=to;
  from_length=from->pack_length();
  if (from->maybe_null())
  {
    from_null_ptr=from->null_ptr;
    from_bit=	  from->null_bit;
    to_ptr[0]=	  1;				// Null as default value
    to_null_ptr=  (uchar*) to_ptr++;
    to_bit=	  1;
    if (from->table->maybe_null)
    {
      null_row=   &from->table->null_row;
      do_copy=	  do_outer_field_to_null_str;
    }
    else
      do_copy=	  do_field_to_null_str;
  }
  else
  {
    to_null_ptr=  0;				// For easy debugging
    do_copy=	  do_field_eq;
  }
}



void Copy_field::set(Field *to,Field *from,bool save)
{
  if (to->type() == FIELD_TYPE_NULL)
  {
    to_null_ptr=0;				// For easy debugging
    to_ptr=0;
    do_copy=do_skip;
    return;
  }
  from_field=from;
  to_field=to;
  from_ptr=from->ptr;
  from_length=from->pack_length();
  to_ptr=  to->ptr;
  to_length=to_field->pack_length();

  // set up null handling
  from_null_ptr=to_null_ptr=0;
  if (from->maybe_null())
  {
    from_null_ptr=	from->null_ptr;
    from_bit=		from->null_bit;
    if (to_field->real_maybe_null())
    {
      to_null_ptr=	to->null_ptr;
      to_bit=		to->null_bit;
      if (from_null_ptr)
	do_copy=	do_copy_null;
      else
      {
	null_row=	&from->table->null_row;
	do_copy=	do_outer_field_null;
      }
    }
    else
      do_copy=do_copy_not_null;
  }
  else if (to_field->real_maybe_null())
  {
    to_null_ptr=	to->null_ptr;
    to_bit=		to->null_bit;
    if (to_field->type() == FIELD_TYPE_TIMESTAMP)
      do_copy=do_copy_timestamp;		// Automatic timestamp
    else if (to_field == to_field->table->next_number_field)
      do_copy=do_copy_next_number;
    else
      do_copy=do_copy_maybe_null;
  }
  else
   do_copy=0;

  if ((to->flags & BLOB_FLAG) && save)
    do_copy2= do_save_blob;
  else
    do_copy2= get_copy_func(to,from);
  if (!do_copy)					// Not null
    do_copy=do_copy2;
}


void (*Copy_field::get_copy_func(Field *to,Field *from))(Copy_field*)
{
  if (to->flags & BLOB_FLAG)
  {
    if (!(from->flags & BLOB_FLAG))
      return do_conv_blob;
    if (from_length != to_length ||
	to->table->db_low_byte_first != from->table->db_low_byte_first)
    {
      // Correct pointer to point at char pointer
      to_ptr+=to_length - to->table->blob_ptr_size;
      from_ptr+=from_length- from->table->blob_ptr_size;
      return do_copy_blob;
    }
  }
  else
  {
    // Check if identical fields
    if (from->result_type() == STRING_RESULT)
    {
      if (to->real_type() != from->real_type() ||
	  to->table->db_low_byte_first != from->table->db_low_byte_first)
      {
	if (from->real_type() == FIELD_TYPE_ENUM ||
	    from->real_type() == FIELD_TYPE_SET)
	  if (to->result_type() != STRING_RESULT)
	    return do_field_int;		// Convert SET to number
	return do_field_string;
      }
      if (to->real_type() == FIELD_TYPE_ENUM ||
	  to->real_type() == FIELD_TYPE_SET)
      {
	if (!to->eq_def(from))
	  return do_field_string;
      }
      else if (to->real_type() == FIELD_TYPE_VAR_STRING && to_length !=
	       from_length)
	return do_varstring;
      else if (to_length < from_length)
	return do_cut_string;
      else if (to_length > from_length)
	return do_expand_string;
    }
    else if (to->real_type() != from->real_type() ||
	     to_length != from_length ||
	     to->table->db_low_byte_first != from->table->db_low_byte_first)
    {
      if (to->real_type() == FIELD_TYPE_DECIMAL ||
	  to->result_type() == STRING_RESULT)
	return do_field_string;
      if (to->result_type() == INT_RESULT)
	return do_field_int;
      return do_field_real;
    }
    else
    {
      if (!to->eq_def(from) ||
	  to->table->db_low_byte_first != from->table->db_low_byte_first)
      {
	if (to->real_type() == FIELD_TYPE_DECIMAL)
	  return do_field_string;
	if (to->result_type() == INT_RESULT)
	  return do_field_int;
	else
	  return do_field_real;
      }
    }
  }
    /* Eq fields */
  switch (to_length) {
  case 1: return do_field_1;
  case 2: return do_field_2;
  case 3: return do_field_3;
  case 4: return do_field_4;
  case 6: return do_field_6;
  case 8: return do_field_8;
  }
  return do_field_eq;
}


/* Simple quick field convert that is called on insert */

void field_conv(Field *to,Field *from)
{
  if (to->real_type() == from->real_type())
  {
    if (to->pack_length() == from->pack_length() &&
	to->real_type() != FIELD_TYPE_ENUM &&
	to->real_type() != FIELD_TYPE_SET &&
	to->table->db_low_byte_first == from->table->db_low_byte_first)
    {						// Identical fields
      memcpy(to->ptr,from->ptr,to->pack_length());
      return;
    }
  }
  if (to->type() == FIELD_TYPE_BLOB)
  {						// Be sure the value is stored
    Field_blob *blob=(Field_blob*) to;
    from->val_str(&blob->value,&blob->value);
    if (!blob->value.is_alloced() &&
	from->real_type() != FIELD_TYPE_STRING)
      blob->value.copy();
    blob->store(blob->value.ptr(),blob->value.length());
    return;
  }
  if ((from->result_type() == STRING_RESULT &&
       (to->result_type() == STRING_RESULT ||
	(from->real_type() != FIELD_TYPE_ENUM &&
	 from->real_type() != FIELD_TYPE_SET))) ||
      to->type() == FIELD_TYPE_DECIMAL)
  {
    char buff[MAX_FIELD_WIDTH];
    String result(buff,sizeof(buff));
    from->val_str(&result,&result);
    to->store(result.c_ptr_quick(),result.length());
  }
  else if (from->result_type() == REAL_RESULT)
    to->store(from->val_real());
  else
    to->store(from->val_int());
}
