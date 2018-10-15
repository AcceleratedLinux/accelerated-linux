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


/* Sum functions (COUNT, MIN...) */

#ifdef __GNUC__
#pragma implementation				// gcc: Class implementation
#endif

#include "mysql_priv.h"


Item_sum::Item_sum(List<Item> &list)
{
  arg_count=list.elements;
  if ((args=(Item**) sql_alloc(sizeof(Item*)*arg_count)))
  {
    uint i=0;
    List_iterator_fast<Item> li(list);
    Item *item;

    while ((item=li++))
    {
      args[i++]= item;
    }
  }
  with_sum_func=1;
  list.empty();					// Fields are used
}


void Item_sum::make_field(Send_field *tmp_field)
{
  if (args[0]->type() == Item::FIELD_ITEM && keep_field_type())
  {
    ((Item_field*) args[0])->field->make_field(tmp_field);
    if (maybe_null)
      tmp_field->flags&= ~NOT_NULL_FLAG;
  }
  else
  {
    tmp_field->flags=0;
    if (!maybe_null)
      tmp_field->flags|= NOT_NULL_FLAG;
    if (unsigned_flag)
      tmp_field->flags |= UNSIGNED_FLAG;
    tmp_field->length=max_length;
    tmp_field->decimals=decimals;
    tmp_field->type=(result_type() == INT_RESULT ? FIELD_TYPE_LONG :
		     result_type() == REAL_RESULT ? FIELD_TYPE_DOUBLE :
		     FIELD_TYPE_VAR_STRING);
  }
  tmp_field->table_name=(char*)"";
  tmp_field->col_name=name;
}

void Item_sum::print(String *str)
{
  str->append(func_name());
  str->append('(');
  for (uint i=0 ; i < arg_count ; i++)
  {
    if (i)
      str->append(',');
    args[i]->print(str);
  }
  str->append(')');
}

void Item_sum::fix_num_length_and_dec()
{
  decimals=0;
  for (uint i=0 ; i < arg_count ; i++)
    set_if_bigger(decimals,args[i]->decimals);
  max_length=float_length(decimals);
}


String *
Item_sum_num::val_str(String *str)
{
  double nr=val();
  if (null_value)
    return 0;
  str->set(nr,decimals);
  return str;
}


String *
Item_sum_int::val_str(String *str)
{
  longlong nr=val_int();
  if (null_value)
    return 0;
  char buff[21];
  uint length= (uint) (longlong10_to_str(nr,buff,-10)-buff);
  str->copy(buff,length);
  return str;
}


bool
Item_sum_num::fix_fields(THD *thd,TABLE_LIST *tables)
{
  if (!thd->allow_sum_func)
  {
    my_error(ER_INVALID_GROUP_FUNC_USE,MYF(0));
    return 1;
  }
  thd->allow_sum_func=0;			// No included group funcs
  decimals=0;
  maybe_null=0;
  for (uint i=0 ; i < arg_count ; i++)
  {
    if (args[i]->fix_fields(thd,tables))
      return 1;
    if (decimals < args[i]->decimals)
      decimals=args[i]->decimals;
    maybe_null |= args[i]->maybe_null;
  }
  result_field=0;
  max_length=float_length(decimals);
  null_value=1;
  fix_length_and_dec();
  thd->allow_sum_func=1;			// Allow group functions
  return 0;
}


bool
Item_sum_hybrid::fix_fields(THD *thd,TABLE_LIST *tables)
{
  Item *item=args[0];
  if (!thd->allow_sum_func)
  {
    my_error(ER_INVALID_GROUP_FUNC_USE,MYF(0));
    return 1;
  }
  thd->allow_sum_func=0;			// No included group funcs
  if (item->fix_fields(thd,tables))
    return 1;
  hybrid_type=item->result_type();
  if (hybrid_type == INT_RESULT)
    max_length=20;
  else if (hybrid_type == REAL_RESULT)
    max_length=float_length(decimals);
  else
    max_length=item->max_length;
  decimals=item->decimals;
  /* MIN/MAX can return NULL for empty set indepedent of the used column */
  maybe_null= 1;
  binary=item->binary;
  unsigned_flag=item->unsigned_flag;
  result_field=0;
  null_value=1;
  fix_length_and_dec();
  thd->allow_sum_func=1;			// Allow group functions
  return 0;
}


/***********************************************************************
** reset and add of sum_func
***********************************************************************/

void Item_sum_sum::reset()
{
  null_value=1; sum=0.0; Item_sum_sum::add();
}

bool Item_sum_sum::add()
{
  sum+=args[0]->val();
  if (!args[0]->null_value)
    null_value= 0;
  return 0;
}

double Item_sum_sum::val()
{
  return sum;
}


void Item_sum_count::reset()
{
  count=0; add();
}

bool Item_sum_count::add()
{
  if (!args[0]->maybe_null)
    count++;
  else
  {
    (void) args[0]->val_int();
    if (!args[0]->null_value)
      count++;
  }
  return 0;
}

longlong Item_sum_count::val_int()
{
  return (longlong) count;
}

/*
** Avgerage
*/

void Item_sum_avg::reset()
{
  sum=0.0; count=0; Item_sum_avg::add();
}

bool Item_sum_avg::add()
{
  double nr=args[0]->val();
  if (!args[0]->null_value)
  {
    sum+=nr;
    count++;
  }
  return 0;
}

double Item_sum_avg::val()
{
  if (!count)
  {
    null_value=1;
    return 0.0;
  }
  null_value=0;
  return sum/ulonglong2double(count);
}


/*
** Standard deviation
*/

void Item_sum_std::reset()
{
  sum=sum_sqr=0.0; count=0; (void) Item_sum_std::add();
}

bool Item_sum_std::add()
{
  double nr=args[0]->val();
  if (!args[0]->null_value)
  {
    sum+=nr;
    sum_sqr+=nr*nr;
    count++;
  }
  return 0;
}

double Item_sum_std::val()
{
  if (!count)
  {
    null_value=1;
    return 0.0;
  }
  null_value=0;
  /* Avoid problems when the precision isn't good enough */
  double tmp=ulonglong2double(count);
  double tmp2=(sum_sqr - sum*sum/tmp)/tmp;
  return tmp2 <= 0.0 ? 0.0 : sqrt(tmp2);
}


void Item_sum_std::reset_field()
{
  double nr=args[0]->val();
  char *res=result_field->ptr;

  if (args[0]->null_value)
    bzero(res,sizeof(double)*2+sizeof(longlong));
  else
  {
    float8store(res,nr);
    nr*=nr;
    float8store(res+sizeof(double),nr);
    longlong tmp=1;
    int8store(res+sizeof(double)*2,tmp);
  }
}

void Item_sum_std::update_field()
{
  double nr,old_nr,old_sqr;
  longlong field_count;
  char *res=result_field->ptr;

  float8get(old_nr, res);
  float8get(old_sqr, res+sizeof(double));
  field_count=sint8korr(res+sizeof(double)*2);

  nr=args[0]->val();
  if (!args[0]->null_value)
  {
    old_nr+=nr;
    old_sqr+=nr*nr;
    field_count++;
  }
  float8store(res,old_nr);
  float8store(res+sizeof(double),old_sqr);
  int8store(res+sizeof(double)*2,field_count);
}

/* min & max */

double Item_sum_hybrid::val()
{
  if (null_value)
    return 0.0;
  switch (hybrid_type) {
  case STRING_RESULT:
    String *res;  res=val_str(&str_value);
    return res ? atof(res->c_ptr()) : 0.0;
  case INT_RESULT:
    if (unsigned_flag)
      return ulonglong2double(sum_int);
    return (double) sum_int;
  case REAL_RESULT:
    return sum;
  }
  return 0;					// Keep compiler happy
}

longlong Item_sum_hybrid::val_int()
{
  if (null_value)
    return 0;
  if (hybrid_type == INT_RESULT)
    return sum_int;
  return (longlong) Item_sum_hybrid::val();
}


String *
Item_sum_hybrid::val_str(String *str)
{
  if (null_value)
    return 0;
  switch (hybrid_type) {
  case STRING_RESULT:
    return &value;
  case REAL_RESULT:
    str->set(sum,decimals);
    break;
  case INT_RESULT:
    if (unsigned_flag)
      str->set((ulonglong) sum_int);
    else
      str->set((longlong) sum_int);
    break;
  }
  return str;					// Keep compiler happy
}

bool Item_sum_min::add()
{
  switch (hybrid_type) {
  case STRING_RESULT:
  {
    String *result=args[0]->val_str(&tmp_value);
    if (!args[0]->null_value &&
	(null_value ||
	 (binary ? stringcmp(&value,result) : sortcmp(&value,result)) > 0))
    {
      value.copy(*result);
      null_value=0;
    }
  }
  break;
  case INT_RESULT:
  {
    longlong nr=args[0]->val_int();
    if (!args[0]->null_value && (null_value ||
				 (unsigned_flag && 
				  (ulonglong) nr < (ulonglong) sum_int) ||
				 (!unsigned_flag && nr < sum_int)))
    {
      sum_int=nr;
      null_value=0;
    }
  }
  break;
  case REAL_RESULT:
  {
    double nr=args[0]->val();
    if (!args[0]->null_value && (null_value || nr < sum))
    {
      sum=nr;
      null_value=0;
    }
  }
  break;
  }
  return 0;
}


bool Item_sum_max::add()
{
  switch (hybrid_type) {
  case STRING_RESULT:
  {
    String *result=args[0]->val_str(&tmp_value);
    if (!args[0]->null_value &&
	(null_value ||
	 (binary ? stringcmp(&value,result) : sortcmp(&value,result)) < 0))
    {
      value.copy(*result);
      null_value=0;
    }
  }
  break;
  case INT_RESULT:
  {
    longlong nr=args[0]->val_int();
    if (!args[0]->null_value && (null_value ||
				 (unsigned_flag && 
				  (ulonglong) nr > (ulonglong) sum_int) ||
				 (!unsigned_flag && nr > sum_int)))
    {
      sum_int=nr;
      null_value=0;
    }
  }
  break;
  case REAL_RESULT:
  {
    double nr=args[0]->val();
    if (!args[0]->null_value && (null_value || nr > sum))
    {
      sum=nr;
      null_value=0;
    }
  }
  break;
  }
  return 0;
}


/* bit_or and bit_and */

longlong Item_sum_bit::val_int()
{
  return (longlong) bits;
}

void Item_sum_bit::reset()
{
  bits=reset_bits; add();
}

bool Item_sum_or::add()
{
  ulonglong value= (ulonglong) args[0]->val_int();
  if (!args[0]->null_value)
    bits|=value;
  return 0;
}

bool Item_sum_and::add()
{
  ulonglong value= (ulonglong) args[0]->val_int();
  if (!args[0]->null_value)
    bits&=value;
  return 0;
}

/************************************************************************
** reset result of a Item_sum with is saved in a tmp_table
*************************************************************************/

void Item_sum_num::reset_field()
{
  double nr=args[0]->val();
  char *res=result_field->ptr;

  if (maybe_null)
  {
    if (args[0]->null_value)
    {
      nr=0.0;
      result_field->set_null();
    }
    else
      result_field->set_notnull();
  }
  float8store(res,nr);
}


void Item_sum_hybrid::reset_field()
{
  if (hybrid_type == STRING_RESULT)
  {
    char buff[MAX_FIELD_WIDTH];
    String tmp(buff,sizeof(buff)),*res;

    res=args[0]->val_str(&tmp);
    if (args[0]->null_value)
    {
      result_field->set_null();
      result_field->reset();
    }
    else
    {
      result_field->set_notnull();
      result_field->store(res->ptr(),res->length());
    }
  }
  else if (hybrid_type == INT_RESULT)
  {
    longlong nr=args[0]->val_int();

    if (maybe_null)
    {
      if (args[0]->null_value)
      {
	nr=0;
	result_field->set_null();
      }
      else
	result_field->set_notnull();
    }
    result_field->store(nr);
  }
  else						// REAL_RESULT
  {
    double nr=args[0]->val();

    if (maybe_null)
    {
      if (args[0]->null_value)
      {
	nr=0.0;
	result_field->set_null();
      }
      else
	result_field->set_notnull();
    }
    result_field->store(nr);
  }
}


void Item_sum_sum::reset_field()
{
  double nr=args[0]->val();			// Nulls also return 0
  float8store(result_field->ptr,nr);
  if (args[0]->null_value)
    result_field->set_null();
  else
    result_field->set_notnull();
}


void Item_sum_count::reset_field()
{
  char *res=result_field->ptr;
  longlong nr=0;

  if (!args[0]->maybe_null)
    nr=1;
  else
  {
    (void) args[0]->val_int();
    if (!args[0]->null_value)
      nr=1;
  }
  int8store(res,nr);
}


void Item_sum_avg::reset_field()
{
  double nr=args[0]->val();
  char *res=result_field->ptr;

  if (args[0]->null_value)
    bzero(res,sizeof(double)+sizeof(longlong));
  else
  {
    float8store(res,nr);
    res+=sizeof(double);
    longlong tmp=1;
    int8store(res,tmp);
  }
}

void Item_sum_bit::reset_field()
{
  char *res=result_field->ptr;
  ulonglong nr=(ulonglong) args[0]->val_int();
  int8store(res,nr);
}

/*
** calc next value and merge it with field_value
*/

void Item_sum_sum::update_field()
{
  double old_nr,nr;
  char *res=result_field->ptr;

  float8get(old_nr,res);
  nr=args[0]->val();
  if (!args[0]->null_value)
  {
    old_nr+=nr;
    result_field->set_notnull();
  }
  float8store(res,old_nr);
}


void Item_sum_count::update_field()
{
  longlong nr;
  char *res=result_field->ptr;

  nr=sint8korr(res);
  if (!args[0]->maybe_null)
    nr++;
  else
  {
    (void) args[0]->val_int();
    if (!args[0]->null_value)
      nr++;
  }
  int8store(res,nr);
}


void Item_sum_avg::update_field()
{
  double nr,old_nr;
  longlong field_count;
  char *res=result_field->ptr;

  float8get(old_nr,res);
  field_count=sint8korr(res+sizeof(double));

  nr=args[0]->val();
  if (!args[0]->null_value)
  {
    old_nr+=nr;
    field_count++;
  }
  float8store(res,old_nr);
  res+=sizeof(double);
  int8store(res,field_count);
}

void Item_sum_hybrid::update_field()
{
  if (hybrid_type == STRING_RESULT)
    min_max_update_str_field();
  else if (hybrid_type == INT_RESULT)
    min_max_update_int_field();
  else
    min_max_update_real_field();
}


void
Item_sum_hybrid::min_max_update_str_field()
{
  String *res_str=args[0]->val_str(&value);

  if (!args[0]->null_value)
  {
    res_str->strip_sp();
    result_field->val_str(&tmp_value,&tmp_value);

    if (result_field->is_null() ||
	(cmp_sign * (binary ? stringcmp(res_str,&tmp_value) :
		 sortcmp(res_str,&tmp_value)) < 0))
      result_field->store(res_str->ptr(),res_str->length());
    result_field->set_notnull();
  }
}


void
Item_sum_hybrid::min_max_update_real_field()
{
  double nr,old_nr;

  old_nr=result_field->val_real();
  nr=args[0]->val();
  if (!args[0]->null_value)
  {
    if (result_field->is_null(0) ||
	(cmp_sign > 0 ? old_nr > nr : old_nr < nr))
      old_nr=nr;
    result_field->set_notnull();
  }
  else if (result_field->is_null(0))
    result_field->set_null();
  result_field->store(old_nr);
}


void
Item_sum_hybrid::min_max_update_int_field()
{
  longlong nr,old_nr;

  old_nr=result_field->val_int();
  nr=args[0]->val_int();
  if (!args[0]->null_value)
  {
    if (result_field->is_null(0))
      old_nr=nr;
    else
    {
      bool res=(unsigned_flag ?
		(ulonglong) old_nr > (ulonglong) nr :
		old_nr > nr);
      /* (cmp_sign > 0 && res) || (!(cmp_sign > 0) && !res) */
      if ((cmp_sign > 0) ^ (!res))
	old_nr=nr;
    }
    result_field->set_notnull();
  }
  else if (result_field->is_null(0))
    result_field->set_null();
  result_field->store(old_nr);
}


void Item_sum_or::update_field()
{
  ulonglong nr;
  char *res=result_field->ptr;

  nr=uint8korr(res);
  nr|= (ulonglong) args[0]->val_int();
  int8store(res,nr);
}


void Item_sum_and::update_field()
{
  ulonglong nr;
  char *res=result_field->ptr;

  nr=uint8korr(res);
  nr&= (ulonglong) args[0]->val_int();
  int8store(res,nr);
}


Item_avg_field::Item_avg_field(Item_sum_avg *item)
{
  name=item->name;
  decimals=item->decimals;
  max_length=item->max_length;
  field=item->result_field;
  maybe_null=1;
}

double Item_avg_field::val()
{
  double nr;
  longlong count;
  float8get(nr,field->ptr);
  char *res=(field->ptr+sizeof(double));
  count=sint8korr(res);

  if (!count)
  {
    null_value=1;
    return 0.0;
  }
  null_value=0;
  return nr/(double) count;
}

String *Item_avg_field::val_str(String *str)
{
  double nr=Item_avg_field::val();
  if (null_value)
    return 0;
  str->set(nr,decimals);
  return str;
}

Item_std_field::Item_std_field(Item_sum_std *item)
{
  name=item->name;
  decimals=item->decimals;
  max_length=item->max_length;
  field=item->result_field;
  maybe_null=1;
}

double Item_std_field::val()
{
  double sum,sum_sqr;
  longlong count;
  float8get(sum,field->ptr);
  float8get(sum_sqr,(field->ptr+sizeof(double)));
  count=sint8korr(field->ptr+sizeof(double)*2);

  if (!count)
  {
    null_value=1;
    return 0.0;
  }
  null_value=0;
  double tmp= (double) count;
  double tmp2=(sum_sqr - sum*sum/tmp)/tmp;
  return tmp2 <= 0.0 ? 0.0 : sqrt(tmp2);
}

String *Item_std_field::val_str(String *str)
{
  double nr=val();
  if (null_value)
    return 0;
  str->set(nr,decimals);
  return str;
}

/****************************************************************************
** COUNT(DISTINCT ...)
****************************************************************************/

#include "sql_select.h"

static int simple_raw_key_cmp(void* arg, byte* key1, byte* key2)
{
  return memcmp(key1, key2, *(uint*) arg);
}

static int simple_str_key_cmp(void* arg, byte* key1, byte* key2)
{
  return my_sortcmp((char*) key1, (char*) key2, *(uint*) arg);
}

/*
  Did not make this one static - at least gcc gets confused when
  I try to declare a static function as a friend. If you can figure
  out the syntax to make a static function a friend, make this one
  static
*/

int composite_key_cmp(void* arg, byte* key1, byte* key2)
{
  Item_sum_count_distinct* item = (Item_sum_count_distinct*)arg;
  Field **field    = item->table->field;
  Field **field_end= field + item->table->fields;
  uint32 *lengths=item->field_lengths;
  for (; field < field_end; ++field)
  {
    Field* f = *field;
    int len = *lengths++;
    int res = f->key_cmp(key1, key2);
    if (res)
      return res;
    key1 += len;
    key2 += len;
  }
  return 0;
}

/*
  helper function for walking the tree when we dump it to MyISAM -
  tree_walk will call it for each leaf
*/

int dump_leaf(byte* key, uint32 count __attribute__((unused)),
		     Item_sum_count_distinct* item)
{
  byte* buf = item->table->record[0];
  int error;
  /*
    The first item->rec_offset bytes are taken care of with
    restore_record(table,2) in setup()
  */
  memcpy(buf + item->rec_offset, key, item->tree.size_of_element);
  if ((error = item->table->file->write_row(buf)))
  {
    if (error != HA_ERR_FOUND_DUPP_KEY &&
	error != HA_ERR_FOUND_DUPP_UNIQUE)
      return 1;
  }
  return 0;
}


Item_sum_count_distinct::~Item_sum_count_distinct()
{
  if (table)
    free_tmp_table(current_thd, table);
  delete tmp_table_param;
  if (use_tree)
    delete_tree(&tree);
}

bool Item_sum_count_distinct::fix_fields(THD *thd,TABLE_LIST *tables)
{
  if (Item_sum_num::fix_fields(thd,tables) ||
      !(tmp_table_param= new TMP_TABLE_PARAM))
    return 1;
  return 0;
}

bool Item_sum_count_distinct::setup(THD *thd)
{
  List<Item> list;
  /* Create a table with an unique key over all parameters */
  for (uint i=0; i < arg_count ; i++)
  {
    Item *item=args[i];
    if (list.push_back(item))
      return 1;					// End of memory
    if (item->const_item())
    {
      (void) item->val_int();
      if (item->null_value)
	always_null=1;
    }
  }
  if (always_null)
    return 0;
  count_field_types(tmp_table_param,list,0);
  if (table)
  {
    free_tmp_table(thd, table);
    tmp_table_param->cleanup();
  }
  if (!(table=create_tmp_table(thd, tmp_table_param, list, (ORDER*) 0, 1,
			       0, 0,
			       current_lex->select->options | thd->options)))
    return 1;
  table->file->extra(HA_EXTRA_NO_ROWS);		// Don't update rows
  table->no_rows=1;


  // no blobs, otherwise it would be MyISAM
  if (table->db_type == DB_TYPE_HEAP)
  {
    qsort_cmp2 compare_key;
    void* cmp_arg;

    // to make things easier for dump_leaf if we ever have to dump to MyISAM
    restore_record(table,2);

    if (table->fields == 1)
    {
      /*
	If we have only one field, which is the most common use of
	count(distinct), it is much faster to use a simpler key
	compare method that can take advantage of not having to worry
	about other fields
      */
      Field* field = table->field[0];
      switch(field->type())
      {
	/*
	  If we have a string, we must take care of charsets and case
	  sensitivity
	*/
      case FIELD_TYPE_STRING:
      case FIELD_TYPE_VAR_STRING:
	compare_key = (qsort_cmp2)(field->binary() ? simple_raw_key_cmp:
				   simple_str_key_cmp);
	break;
      default:
	/*
	  Since at this point we cannot have blobs anything else can
	  be compared with memcmp
	*/
	compare_key = (qsort_cmp2)simple_raw_key_cmp;
	break;
      }
      key_length = field->pack_length();
      cmp_arg = (void*) &key_length;
      rec_offset = 1;
    }
    else // too bad, cannot cheat - there is more than one field
    {
      bool all_binary = 1;
      Field** field, **field_end;
      field_end = (field = table->field) + table->fields;
      uint32 *lengths;
      if (!(field_lengths= 
	    (uint32*) thd->alloc(sizeof(uint32) * table->fields)))
	return 1;

      for (key_length = 0, lengths=field_lengths; field < field_end; ++field)
      {
	uint32 length= (*field)->pack_length();
	key_length += length;
	*lengths++ = length;
	if (!(*field)->binary())
	  all_binary = 0;			// Can't break loop here
      }
      rec_offset = table->reclength - key_length;
      if (all_binary)
      {
	compare_key = (qsort_cmp2)simple_raw_key_cmp;
	cmp_arg = (void*) &key_length;
      }
      else
      {
	compare_key = (qsort_cmp2) composite_key_cmp ;
	cmp_arg = (void*) this;
      }
    }

    init_tree(&tree, min(thd->variables.max_heap_table_size,
			 thd->variables.sortbuff_size/16), 0,
	      key_length, compare_key, 0, NULL, cmp_arg);
    use_tree = 1;

    /*
      The only time key_length could be 0 is if someone does
      count(distinct) on a char(0) field - stupid thing to do,
      but this has to be handled - otherwise someone can crash
      the server with a DoS attack
    */
    max_elements_in_tree = ((key_length) ? 
			    thd->variables.max_heap_table_size/key_length : 1);
  }
  return 0;
}


int Item_sum_count_distinct::tree_to_myisam()
{
  if (create_myisam_from_heap(current_thd, table, tmp_table_param,
			      HA_ERR_RECORD_FILE_FULL, 1) ||
      tree_walk(&tree, (tree_walk_action)&dump_leaf, (void*)this,
		left_root_right))
    return 1;
  delete_tree(&tree);
  use_tree = 0;
  return 0;
}

void Item_sum_count_distinct::reset()
{
  if (use_tree)
    reset_tree(&tree);
  else if (table)
  {
    table->file->extra(HA_EXTRA_NO_CACHE);
    table->file->delete_all_rows();
    table->file->extra(HA_EXTRA_WRITE_CACHE);
  }
  (void) add();
}

bool Item_sum_count_distinct::add()
{
  int error;
  if (always_null)
    return 0;
  copy_fields(tmp_table_param);
  copy_funcs(tmp_table_param->items_to_copy);

  for (Field **field=table->field ; *field ; field++)
    if ((*field)->is_real_null(0))
      return 0;					// Don't count NULL

  if (use_tree)
  {
    /*
      If the tree got too big, convert to MyISAM, otherwise insert into the
      tree.
    */
    if (tree.elements_in_tree > max_elements_in_tree)
    {
      if (tree_to_myisam())
	return 1;
    }
    else if (!tree_insert(&tree, table->record[0] + rec_offset, 0))
      return 1;
  }
  else if ((error=table->file->write_row(table->record[0])))
  {
    if (error != HA_ERR_FOUND_DUPP_KEY &&
	error != HA_ERR_FOUND_DUPP_UNIQUE)
    {
      if (create_myisam_from_heap(current_thd, table, tmp_table_param, error,
				  1))
	return 1;				// Not a table_is_full error
    }
  }
  return 0;
}

longlong Item_sum_count_distinct::val_int()
{
  if (!table)					// Empty query
    return LL(0);
  if (use_tree)
    return tree.elements_in_tree;
  table->file->info(HA_STATUS_VARIABLE | HA_STATUS_NO_LOCK);
  return table->file->records;
}

/****************************************************************************
** Functions to handle dynamic loadable aggregates
** Original source by: Alexis Mikhailov <root@medinf.chuvashia.su>
** Adapted for UDAs by: Andreas F. Bobak <bobak@relog.ch>.
** Rewritten by: Monty.
****************************************************************************/

#ifdef HAVE_DLOPEN

void Item_udf_sum::reset()
{
  DBUG_ENTER("Item_udf_sum::reset");
  udf.reset(&null_value);
  DBUG_VOID_RETURN;
}

bool Item_udf_sum::add()
{
  DBUG_ENTER("Item_udf_sum::add");
  udf.add(&null_value);
  DBUG_RETURN(0);
}

double Item_sum_udf_float::val()
{
  DBUG_ENTER("Item_sum_udf_float::val");
  DBUG_PRINT("info",("result_type: %d  arg_count: %d",
		     args[0]->result_type(), arg_count));
  DBUG_RETURN(udf.val(&null_value));
}

String *Item_sum_udf_float::val_str(String *str)
{
  double nr=val();
  if (null_value)
    return 0;					/* purecov: inspected */
  else
    str->set(nr,decimals);
  return str;
}


longlong Item_sum_udf_int::val_int()
{
  DBUG_ENTER("Item_sum_udf_int::val_int");
  DBUG_PRINT("info",("result_type: %d  arg_count: %d",
		     args[0]->result_type(), arg_count));
  DBUG_RETURN(udf.val_int(&null_value));
}

String *Item_sum_udf_int::val_str(String *str)
{
  longlong nr=val_int();
  if (null_value)
    return 0;
  else
    str->set(nr);
  return str;
}

/* Default max_length is max argument length */

void Item_sum_udf_str::fix_length_and_dec()
{
  DBUG_ENTER("Item_sum_udf_str::fix_length_and_dec");
  max_length=0;
  for (uint i = 0; i < arg_count; i++)
    set_if_bigger(max_length,args[i]->max_length);
  DBUG_VOID_RETURN;
}

String *Item_sum_udf_str::val_str(String *str)
{
  DBUG_ENTER("Item_sum_udf_str::str");
  String *res=udf.val_str(str,&str_value);
  null_value = !res;
  DBUG_RETURN(res);
}

#endif /* HAVE_DLOPEN */
