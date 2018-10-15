/* Copyright (C) 2000 MySQL AB & MySQL Finland AB & TCX DataKonsult AB

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


#ifdef __GNUC__
#pragma interface			/* gcc class implementation */
#endif

struct st_table_list;
void item_init(void);				/* Init item functions */

class Item {
  Item(const Item &);				/* Prevent use of these */
  void operator=(Item &);
public:
  static void *operator new(size_t size) {return (void*) sql_alloc((uint) size); }
  static void operator delete(void *ptr,size_t size) {} /*lint -e715 */

  enum Type {FIELD_ITEM,FUNC_ITEM,SUM_FUNC_ITEM,STRING_ITEM,
	     INT_ITEM,REAL_ITEM,NULL_ITEM,VARBIN_ITEM,
	     COPY_STR_ITEM,FIELD_AVG_ITEM, DEFAULT_ITEM,
	     PROC_ITEM,COND_ITEM,REF_ITEM,FIELD_STD_ITEM, CONST_ITEM};
  enum cond_result { COND_UNDEF,COND_OK,COND_TRUE,COND_FALSE };

  String str_value;			/* used to store value */
  my_string name;			/* Name from select */
  Item *next;
  uint32 max_length;
  uint8 marker,decimals;
  my_bool maybe_null;			/* If item may be null */
  my_bool null_value;			/* if item is null */
  my_bool binary;
  my_bool unsigned_flag;
  my_bool with_sum_func;


  // alloc & destruct is done as start of select using sql_alloc
  Item();
  virtual ~Item() { name=0; }		/*lint -e1509 */
  void set_name(char* str,uint length=0);
  void init_make_field(Send_field *tmp_field,enum enum_field_types type);
  virtual bool fix_fields(THD *,struct st_table_list *);
  virtual bool save_in_field(Field *field, bool no_conversions);
  virtual void save_org_in_field(Field *field)
  { (void) save_in_field(field, 1); }
  virtual bool save_safe_in_field(Field *field)
  { return save_in_field(field, 1); }
  virtual bool send(THD *thd, String *str);
  virtual bool eq(const Item *, bool binary_cmp) const;
  virtual Item_result result_type () const { return REAL_RESULT; }
  virtual enum Type type() const =0;
  virtual double val()=0;
  virtual longlong val_int()=0;
  virtual String *val_str(String*)=0;
  virtual void make_field(Send_field *field)=0;
  virtual Field *tmp_table_field(TABLE *t_arg=(TABLE *)0) { return 0; }
  virtual const char *full_name() const { return name ? name : "???"; }
  virtual double  val_result() { return val(); }
  virtual longlong val_int_result() { return val_int(); }
  virtual String *str_result(String* tmp) { return val_str(tmp); }
  /* bit map of tables used by item */
  virtual table_map used_tables() const { return (table_map) 0L; }
  /*
    Return table map of tables that can't be NULL tables (tables that are
    used in a context where if they would contain a NULL row generated
    by a LEFT or RIGHT join, the item would not be true).
    This expression is used on WHERE item to determinate if a LEFT JOIN can be
    converted to a normal join.
    Generally this function should return used_tables() if the function
    would return null if any of the arguments are null
    As this is only used in the beginning of optimization, the value don't
    have to be updated in update_used_tables()
  */
  virtual table_map not_null_tables() const { return used_tables(); }
  /*
    Returns true if this is a simple constant item like an integer, not
    a constant expression
  */
  virtual bool basic_const_item() const { return 0; }
  virtual Item *new_item() { return 0; }	/* Only for const items */
  virtual cond_result eq_cmp_result() const { return COND_OK; }
  inline uint float_length(uint decimals_par) const
  { return decimals != NOT_FIXED_DEC ? (DBL_DIG+2+decimals_par) : DBL_DIG+8;}
  virtual bool const_item() const { return used_tables() == 0; }
  virtual void print(String *str_arg) { str_arg->append(full_name()); }
  virtual void update_used_tables() {}
  virtual void split_sum_func(List<Item> &fields) {}
  virtual bool get_date(TIME *ltime,bool fuzzydate);
  virtual bool get_time(TIME *ltime);
  virtual bool get_date_result(TIME *ltime,bool fuzzydate)
  { return get_date(ltime,fuzzydate); }
  virtual bool is_null() { return 0; }
  virtual unsigned int size_of()= 0;
  virtual void top_level_item() {}
  virtual void set_result_field(Field *field) {}
  virtual bool is_result_field() { return 0; }
  virtual void save_in_result_field(bool no_conversions) {}
  virtual void no_rows_in_result() {}
};


class Item_ident :public Item
{
public:
  const char *db_name;
  const char *table_name;
  const char *field_name;
  Item_ident(const char *db_name_par,const char *table_name_par,
	     const char *field_name_par)
    :db_name(db_name_par),table_name(table_name_par),field_name(field_name_par)
    { name = (char*) field_name_par; }
  const char *full_name() const;
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_field :public Item_ident
{
  void set_field(Field *field);
public:
  Field *field,*result_field;
  // Item_field() {}

  Item_field(const char *db_par,const char *table_name_par,
	     const char *field_name_par)
    :Item_ident(db_par,table_name_par,field_name_par),field(0),result_field(0)
  {}
  Item_field(Field *field);
  enum Type type() const { return FIELD_ITEM; }
  bool eq(const Item *item, bool binary_cmp) const;
  double val();
  longlong val_int();
  String *val_str(String*);
  double val_result();
  longlong val_int_result();
  String *str_result(String* tmp);
  bool send(THD *thd, String *str_arg)
  {
    return result_field->send(thd,str_arg);
  }
  void make_field(Send_field *field);
  bool fix_fields(THD *,struct st_table_list *);
  bool save_in_field(Field *field,bool no_conversions);
  void save_org_in_field(Field *field);
  table_map used_tables() const;
  enum Item_result result_type () const
  {
    return field->result_type();
  }
  Field *tmp_table_field(TABLE *t_arg=(TABLE *)0) { return result_field; }
  bool get_date(TIME *ltime,bool fuzzydate);
  bool get_date_result(TIME *ltime,bool fuzzydate);
  bool get_time(TIME *ltime);
  bool is_null() { return field->is_null(); }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_null :public Item
{
public:
  Item_null(char *name_par=0)
    { maybe_null=null_value=TRUE; name= name_par ? name_par : (char*) "NULL";}
  enum Type type() const { return NULL_ITEM; }
  bool eq(const Item *item, bool binary_cmp) const;
  double val();
  longlong val_int();
  String *val_str(String *str);
  void make_field(Send_field *field);
  bool save_in_field(Field *field, bool no_conversions);
  bool save_safe_in_field(Field *field);
  enum Item_result result_type () const
  { return STRING_RESULT; }
  bool send(THD *thd, String *str);
  bool basic_const_item() const { return 1; }
  Item *new_item() { return new Item_null(name); }
  bool is_null() { return 1; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_int :public Item
{
public:
  const longlong value;
  Item_int(int32 i,uint length=11) :value((longlong) i)
    { max_length=length;}
#ifdef HAVE_LONG_LONG
  Item_int(longlong i,uint length=21) :value(i)
    { max_length=length;}
#endif
  Item_int(const char *str_arg,longlong i,uint length) :value(i)
    { max_length=length; name=(char*) str_arg;}
  Item_int(const char *str_arg) :
    value(str_arg[0] == '-' ? strtoll(str_arg,(char**) 0,10) :
	  (longlong) strtoull(str_arg,(char**) 0,10))
    { max_length= (uint) strlen(str_arg); name=(char*) str_arg;}
  enum Type type() const { return INT_ITEM; }
  virtual enum Item_result result_type () const { return INT_RESULT; }
  longlong val_int() { return value; }
  double val() { return (double) value; }
  String *val_str(String*);
  void make_field(Send_field *field);
  bool save_in_field(Field *field, bool no_conversions);
  bool basic_const_item() const { return 1; }
  Item *new_item() { return new Item_int(name,value,max_length); }
  void print(String *str);
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_uint :public Item_int
{
public:
  Item_uint(const char *str_arg, uint length) :
    Item_int(str_arg, (longlong) strtoull(str_arg,(char**) 0,10), length) {}
  Item_uint(uint32 i) :Item_int((longlong) i, 10) {}
  double val() { return ulonglong2double(value); }
  String *val_str(String*);
  void make_field(Send_field *field);
  Item *new_item() { return new Item_uint(name,max_length); }
  bool fix_fields(THD *thd,struct st_table_list *table_list)
  {
    unsigned_flag= 1;
    return 0;
  }
  void print(String *str);
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_real :public Item
{
public:
  const double value;
  // Item_real() :value(0) {}
  Item_real(const char *str_arg,uint length) :value(atof(str_arg))
  {
    name=(char*) str_arg;
    decimals=(uint8) nr_of_decimals(str_arg);
    max_length=length;
  }
  Item_real(const char *str,double val_arg,uint decimal_par,uint length)
    :value(val_arg)
  {
    name=(char*) str;
    decimals=(uint8) decimal_par;
    max_length=length;
  }
  Item_real(double value_par) :value(value_par) {}
  bool save_in_field(Field *field, bool no_conversions);
  enum Type type() const { return REAL_ITEM; }
  double val() { return value; }
  longlong val_int() { return (longlong) (value+(value > 0 ? 0.5 : -0.5));}
  String *val_str(String*);
  void make_field(Send_field *field);
  bool basic_const_item() const { return 1; }
  Item *new_item() { return new Item_real(name,value,decimals,max_length); }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_float :public Item_real
{
public:
  Item_float(const char *str,uint length) :Item_real(str,length)
  {
    decimals=NOT_FIXED_DEC;
    max_length=DBL_DIG+8;
  }
  unsigned int size_of() { return sizeof(*this);}  
};

class Item_string :public Item
{
public:
  Item_string(const char *str,uint length)
  {
    str_value.set(str,length);
    max_length=length;
    name=(char*) str_value.ptr();
    decimals=NOT_FIXED_DEC;
  }
  Item_string(const char *name_par,const char *str,uint length)
  {
    str_value.set(str,length);
    max_length=length;
    name=(char*) name_par;
    decimals=NOT_FIXED_DEC;
  }
  ~Item_string() {}
  enum Type type() const { return STRING_ITEM; }
  double val() { return atof(str_value.ptr()); }
  longlong val_int() { return strtoll(str_value.ptr(),(char**) 0,10); }
  String *val_str(String*) { return (String*) &str_value; }
  bool save_in_field(Field *field, bool no_conversions);
  void make_field(Send_field *field);
  enum Item_result result_type () const { return STRING_RESULT; }
  bool basic_const_item() const { return 1; }
  bool eq(const Item *item, bool binary_cmp) const;
  Item *new_item() { return new Item_string(name,str_value.ptr(),max_length); }
  String *const_string() { return &str_value; }
  inline void append(char *str,uint length) { str_value.append(str,length); }
  void print(String *str);
  unsigned int size_of() { return sizeof(*this);}
};


/* For INSERT ... VALUES (DEFAULT) */

class Item_default :public Item
{
public:
  Item_default() { name= (char*) "DEFAULT"; }
  enum Type type() const { return DEFAULT_ITEM; }
  void make_field(Send_field *field) {}
  bool save_in_field(Field *field, bool no_conversions)
  {
    field->set_default();
    return 0;
  }
  virtual double val() { return 0.0; }
  virtual longlong val_int() { return 0; }
  virtual String *val_str(String *str) { return 0; }
  bool basic_const_item() const { return 1; }
  unsigned int size_of() { return sizeof(*this);}
};


/* for show tables */

class Item_datetime :public Item_string
{
public:
  Item_datetime(const char *item_name): Item_string(item_name,"",0)
  { max_length=19;}
  void make_field(Send_field *field);
  unsigned int size_of() { return sizeof(*this);}  
};

class Item_empty_string :public Item_string
{
public:
  Item_empty_string(const char *header,uint length) :Item_string("",0)
    { name=(char*) header; max_length=length;}
  void make_field(Send_field *field);
  unsigned int size_of() { return sizeof(*this);}  
};

class Item_varbinary :public Item
{
public:
  Item_varbinary(const char *str,uint str_length);
  ~Item_varbinary() {}
  enum Type type() const { return VARBIN_ITEM; }
  double val() { return (double) Item_varbinary::val_int(); }
  longlong val_int();
  String *val_str(String*) { return &str_value; }
  bool save_in_field(Field *field, bool no_conversions);
  void make_field(Send_field *field);
  enum Item_result result_type () const;
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_result_field :public Item	/* Item with result field */
{
public:
  Field *result_field;				/* Save result here */
  Item_result_field() :result_field(0) {}
  ~Item_result_field() {}			/* Required with gcc 2.95 */
  Field *tmp_table_field(TABLE *t_arg=(TABLE *)0) { return result_field; }
  table_map used_tables() const { return 1; }
  virtual void fix_length_and_dec()=0;
  unsigned int size_of() { return sizeof(*this);}  
  void set_result_field(Field *field) { result_field= field; }
  bool is_result_field() { return 1; }
  void save_in_result_field(bool no_conversions)
  {
    save_in_field(result_field, no_conversions);
  }
};


class Item_ref :public Item_ident
{
public:
  Field *result_field;				/* Save result here */
  Item **ref;
  Item_ref(char *db_par,char *table_name_par,char *field_name_par)
    :Item_ident(db_par,table_name_par,field_name_par),ref(0) {}
  Item_ref(Item **item, char *table_name_par,char *field_name_par)
    :Item_ident(NullS,table_name_par,field_name_par),ref(item) {}
  enum Type type() const		{ return REF_ITEM; }
  bool eq(const Item *item, bool binary_cmp) const
  { return (*ref)->eq(item, binary_cmp); }
  ~Item_ref() { if (ref) delete *ref; }
  double val()
  {
    double tmp=(*ref)->val_result();
    null_value=(*ref)->null_value;
    return tmp;
  }
  longlong val_int()
  {
    longlong tmp=(*ref)->val_int_result();
    null_value=(*ref)->null_value;
    return tmp;
  }
  String *val_str(String* tmp)
  {
    tmp=(*ref)->str_result(tmp);
    null_value=(*ref)->null_value;
    return tmp;
  }
  bool is_null()
  {
    (void) (*ref)->val_int_result();
    return (*ref)->null_value;
  }
  bool get_date(TIME *ltime,bool fuzzydate)
  {
    return (null_value=(*ref)->get_date_result(ltime,fuzzydate));
  }
  bool send(THD *thd, String *tmp)	{ return (*ref)->send(thd, tmp); }
  void make_field(Send_field *field)	{ (*ref)->make_field(field); }
  bool fix_fields(THD *,struct st_table_list *);
  bool save_in_field(Field *field, bool no_conversions)
  { return (*ref)->save_in_field(field, no_conversions); }
  void save_org_in_field(Field *field)	{ (*ref)->save_org_in_field(field); }
  enum Item_result result_type () const { return (*ref)->result_type(); }
  table_map used_tables() const		{ return (*ref)->used_tables(); }
  unsigned int size_of() { return sizeof(*this);}  
  void set_result_field(Field *field) { result_field= field; }
  bool is_result_field() { return 1; }
  void save_in_result_field(bool no_conversions)
  {
    (*ref)->save_in_field(result_field, no_conversions);
  }
};


/*
  The following class is used to optimize comparing of date columns
  We need to save the original item, to be able to set the field to the
  original value in 'opt_range'.
*/

class Item_int_with_ref :public Item_int
{
  Item *ref;
public:
  Item_int_with_ref(longlong i, Item *ref_arg) :Item_int(i), ref(ref_arg)
  {}
  bool save_in_field(Field *field, bool no_conversions)
  {
    return ref->save_in_field(field, no_conversions);
  }
  unsigned int size_of() { return sizeof(*this);}  
};


#include "item_sum.h"
#include "item_func.h"
#include "item_cmpfunc.h"
#include "item_strfunc.h"
#include "item_timefunc.h"
#include "item_uniq.h"

class Item_copy_string :public Item
{
public:
  Item *item;
  Item_copy_string(Item *i) :item(i)
  {
    null_value=maybe_null=item->maybe_null;
    decimals=item->decimals;
    max_length=item->max_length;
    name=item->name;
  }
  ~Item_copy_string() { delete item; }
  enum Type type() const { return COPY_STR_ITEM; }
  enum Item_result result_type () const { return STRING_RESULT; }
  double val()
  { return null_value ? 0.0 : atof(str_value.c_ptr()); }
  longlong val_int()
  { return null_value ? LL(0) : strtoll(str_value.c_ptr(),(char**) 0,10); }
  String *val_str(String*);
  void make_field(Send_field *field) { item->make_field(field); }
  void copy();
  table_map used_tables() const { return (table_map) 1L; }
  bool const_item() const { return 0; }
  bool is_null() { return null_value; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_buff :public Sql_alloc
{
public:
  my_bool null_value;
  Item_buff() :null_value(0) {}
  virtual bool cmp(void)=0;
  virtual ~Item_buff(); /*line -e1509 */
  unsigned int size_of() { return sizeof(*this);}  
};

class Item_str_buff :public Item_buff
{
  Item *item;
  String value,tmp_value;
public:
  Item_str_buff(Item *arg) :item(arg),value(arg->max_length) {}
  bool cmp(void);
  ~Item_str_buff();				// Deallocate String:s
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_real_buff :public Item_buff
{
  Item *item;
  double value;
public:
  Item_real_buff(Item *item_par) :item(item_par),value(0.0) {}
  bool cmp(void);
  unsigned int size_of() { return sizeof(*this);}  
};

class Item_int_buff :public Item_buff
{
  Item *item;
  longlong value;
public:
  Item_int_buff(Item *item_par) :item(item_par),value(0) {}
  bool cmp(void);
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_field_buff :public Item_buff
{
  char *buff;
  Field *field;
  uint length;

public:
  Item_field_buff(Item_field *item)
  {
    field=item->field;
    buff= (char*) sql_calloc(length=field->pack_length());
  }
  bool cmp(void);
  unsigned int size_of() { return sizeof(*this);}  
};

extern Item_buff *new_Item_buff(Item *item);
extern Item_result item_cmp_type(Item_result a,Item_result b);
extern Item *resolve_const_item(Item *item,Item *cmp_item);
extern bool field_is_equal_to_item(Field *field,Item *item);
