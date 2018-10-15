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


/* This file defines all string functions */

#ifdef __GNUC__
#pragma interface			/* gcc class implementation */
#endif

class Item_str_func :public Item_func
{
public:
  Item_str_func() :Item_func() { decimals=NOT_FIXED_DEC; }
  Item_str_func(Item *a) :Item_func(a) {decimals=NOT_FIXED_DEC; }
  Item_str_func(Item *a,Item *b) :Item_func(a,b) { decimals=NOT_FIXED_DEC; }
  Item_str_func(Item *a,Item *b,Item *c) :Item_func(a,b,c) { decimals=NOT_FIXED_DEC; }
  Item_str_func(Item *a,Item *b,Item *c,Item *d) :Item_func(a,b,c,d) {decimals=NOT_FIXED_DEC; }
  Item_str_func(Item *a,Item *b,Item *c,Item *d, Item* e) :Item_func(a,b,c,d,e) {decimals=NOT_FIXED_DEC; }
  Item_str_func(List<Item> &list) :Item_func(list) {decimals=NOT_FIXED_DEC; }
  longlong val_int();
  double val();
  enum Item_result result_type () const { return STRING_RESULT; }
  void left_right_max_length();
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_md5 :public Item_str_func
{
  String tmp_value;
public:
  Item_func_md5(Item *a) :Item_str_func(a) {}
  String *val_str(String *);
  void fix_length_and_dec();
  const char *func_name() const { return "md5"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_sha :public Item_str_func
{
public:
  Item_func_sha(Item *a) :Item_str_func(a) {}  
  String *val_str(String *);    
  void fix_length_and_dec();      
  const char *func_name() const { return "sha"; }	
};

class Item_func_aes_encrypt :public Item_str_func
{
public:
  Item_func_aes_encrypt(Item *a, Item *b) :Item_str_func(a,b) {}
  String *val_str(String *);
  void fix_length_and_dec();
  const char *func_name() const { return "aes_encrypt"; }
};

class Item_func_aes_decrypt :public Item_str_func	
{
public:
  Item_func_aes_decrypt(Item *a, Item *b) :Item_str_func(a,b) {}
  String *val_str(String *);
  void fix_length_and_dec();
  const char *func_name() const { return "aes_decrypt"; }
};


class Item_func_concat :public Item_str_func
{
  String tmp_value;
public:
  Item_func_concat(List<Item> &list) :Item_str_func(list) {}
  Item_func_concat(Item *a,Item *b) :Item_str_func(a,b) {}
  String *val_str(String *);
  void fix_length_and_dec();
  const char *func_name() const { return "concat"; }
  unsigned int size_of() { return sizeof(*this);}  
};

class Item_func_concat_ws :public Item_str_func
{
  Item *separator;
  String tmp_value;

public:
  Item_func_concat_ws(Item *a,List<Item> &list) 
    :Item_str_func(list),separator(a) {}
  ~Item_func_concat_ws() { delete separator; }
  String *val_str(String *);
  void fix_length_and_dec();
  void update_used_tables();
  bool fix_fields(THD *thd,struct st_table_list *tlist)
  {
    return (separator->fix_fields(thd,tlist)
	    || Item_func::fix_fields(thd,tlist));
  }
  void split_sum_func(List<Item> &fields);
  const char *func_name() const { return "concat_ws"; }
};

class Item_func_reverse :public Item_str_func
{
public:
  Item_func_reverse(Item *a) :Item_str_func(a) {}
  String *val_str(String *);
  void fix_length_and_dec();
};


class Item_func_replace :public Item_str_func
{
  String tmp_value,tmp_value2;
public:
  Item_func_replace(Item *org,Item *find,Item *replace)
    :Item_str_func(org,find,replace) {}
  String *val_str(String *);
  void fix_length_and_dec();
  const char *func_name() const { return "replace"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_insert :public Item_str_func
{
  String tmp_value;
public:
  Item_func_insert(Item *org,Item *start,Item *length,Item *new_str)
    :Item_str_func(org,start,length,new_str) {}
  String *val_str(String *);
  void fix_length_and_dec();
  const char *func_name() const { return "insert"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_str_conv :public Item_str_func
{
public:
  Item_str_conv(Item *item) :Item_str_func(item) {}
  void fix_length_and_dec() { max_length = args[0]->max_length; }
};


class Item_func_lcase :public Item_str_conv
{
public:
  Item_func_lcase(Item *item) :Item_str_conv(item) {}
  String *val_str(String *);
  const char *func_name() const { return "lcase"; }
};

class Item_func_ucase :public Item_str_conv
{
public:
  Item_func_ucase(Item *item) :Item_str_conv(item) {}
  String *val_str(String *);
  const char *func_name() const { return "ucase"; }
};


class Item_func_left :public Item_str_func
{
public:
  Item_func_left(Item *a,Item *b) :Item_str_func(a,b) {}
  String *val_str(String *);
  void fix_length_and_dec();
  const char *func_name() const { return "left"; }
};


class Item_func_right :public Item_str_func
{
  String tmp_value;
public:
  Item_func_right(Item *a,Item *b) :Item_str_func(a,b) {}
  String *val_str(String *);
  void fix_length_and_dec();
  const char *func_name() const { return "right"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_substr :public Item_str_func
{
  String tmp_value;
public:
  Item_func_substr(Item *a,Item *b) :Item_str_func(a,b) {}
  Item_func_substr(Item *a,Item *b,Item *c) :Item_str_func(a,b,c) {}
  String *val_str(String *);
  void fix_length_and_dec();
  const char *func_name() const { return "substr"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_substr_index :public Item_str_func
{
  String tmp_value;
public:
  Item_func_substr_index(Item *a,Item *b,Item *c) :Item_str_func(a,b,c) {}
  String *val_str(String *);
  void fix_length_and_dec() { max_length= args[0]->max_length; }
  const char *func_name() const { return "substr_index"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_ltrim :public Item_str_func
{
  String tmp_value;
public:
  Item_func_ltrim(Item *a,Item *b) :Item_str_func(a,b) {}
  String *val_str(String *);
  void fix_length_and_dec() { max_length= args[0]->max_length; }
  const char *func_name() const { return "ltrim"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_rtrim :public Item_str_func
{
  String tmp_value;
public:
  Item_func_rtrim(Item *a,Item *b) :Item_str_func(a,b) {}
  String *val_str(String *);
  void fix_length_and_dec() { max_length= args[0]->max_length; }
  const char *func_name() const { return "rtrim"; }
  unsigned int size_of() { return sizeof(*this);}  
};

class Item_func_trim :public Item_str_func
{
  String tmp_value;
public:
  Item_func_trim(Item *a,Item *b) :Item_str_func(a,b) {}
  String *val_str(String *);
  void fix_length_and_dec() { max_length= args[0]->max_length; }
  const char *func_name() const { return "trim"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_password :public Item_str_func
{
  char tmp_value[17];
public:
  Item_func_password(Item *a) :Item_str_func(a) {}
  String *val_str(String *);
  void fix_length_and_dec() { max_length = 16; }
  const char *func_name() const { return "password"; }
  unsigned int size_of() { return sizeof(*this);}  
};

class Item_func_des_encrypt :public Item_str_func
{
  String tmp_value;
public:
  Item_func_des_encrypt(Item *a) :Item_str_func(a) {}
  Item_func_des_encrypt(Item *a, Item *b): Item_str_func(a,b) {}
  String *val_str(String *);
  void fix_length_and_dec()
  { maybe_null=1; max_length = args[0]->max_length+8; }
  const char *func_name() const { return "des_encrypt"; }
  unsigned int size_of() { return sizeof(*this);}  
};

class Item_func_des_decrypt :public Item_str_func
{
  String tmp_value;
public:
  Item_func_des_decrypt(Item *a) :Item_str_func(a) {}
  Item_func_des_decrypt(Item *a, Item *b): Item_str_func(a,b) {}
  String *val_str(String *);
  void fix_length_and_dec() { maybe_null=1; max_length = args[0]->max_length; }
  const char *func_name() const { return "des_decrypt"; }
  unsigned int size_of() { return sizeof(*this);}  
};

class Item_func_encrypt :public Item_str_func
{
  String tmp_value;
public:
  Item_func_encrypt(Item *a) :Item_str_func(a) {}
  Item_func_encrypt(Item *a, Item *b): Item_str_func(a,b) {}
  String *val_str(String *);
  void fix_length_and_dec() { maybe_null=1; max_length = 13; }
  unsigned int size_of() { return sizeof(*this);}  
};

#include "sql_crypt.h"

class Item_func_encode :public Item_str_func
{
 protected:
  SQL_CRYPT sql_crypt;
public:
  Item_func_encode(Item *a, char *seed):
    Item_str_func(a),sql_crypt(seed) {}
  String *val_str(String *);
  void fix_length_and_dec();
  unsigned int size_of() { return sizeof(*this);}  
};

class Item_func_decode :public Item_func_encode
{
public:
  Item_func_decode(Item *a, char *seed): Item_func_encode(a,seed) {}
  String *val_str(String *);
};


class Item_func_database :public Item_str_func
{
public:
  Item_func_database() {}
  String *val_str(String *);
  void fix_length_and_dec() { max_length= MAX_FIELD_NAME; }
  const char *func_name() const { return "database"; }
};

class Item_func_user :public Item_str_func
{
public:
  Item_func_user() {}
  String *val_str(String *);
  void fix_length_and_dec() { max_length= USERNAME_LENGTH+HOSTNAME_LENGTH+1; }
  const char *func_name() const { return "user"; }
};


class Item_func_soundex :public Item_str_func
{
  String tmp_value;
public:
  Item_func_soundex(Item *a) :Item_str_func(a) {}
  String *val_str(String *);
  void fix_length_and_dec();
  const char *func_name() const { return "soundex"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_elt :public Item_str_func
{
  Item *item;

public:
  Item_func_elt(Item *a,List<Item> &list) :Item_str_func(list),item(a) {}
  ~Item_func_elt() { delete item; }
  double val();
  longlong val_int();
  String *val_str(String *str);
  bool fix_fields(THD *thd,struct st_table_list *tlist)
  {
    return (item->fix_fields(thd,tlist) || Item_func::fix_fields(thd,tlist));
  }
  void split_sum_func(List<Item> &fields);
  void fix_length_and_dec();
  void update_used_tables();
  const char *func_name() const { return "elt"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_make_set :public Item_str_func
{
  Item *item;
  String tmp_str;

public:
  Item_func_make_set(Item *a,List<Item> &list) :Item_str_func(list),item(a) {}
  ~Item_func_make_set() { delete item; }
  String *val_str(String *str);
  bool fix_fields(THD *thd,struct st_table_list *tlist)
  {
    return (item->fix_fields(thd,tlist) || Item_func::fix_fields(thd,tlist));
  }
  void split_sum_func(List<Item> &fields);
  void fix_length_and_dec();
  void update_used_tables();
  const char *func_name() const { return "make_set"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_format :public Item_str_func
{
  String tmp_str;
public:
  Item_func_format(Item *org,int dec);
  String *val_str(String *);
  void fix_length_and_dec()
  {
    max_length=args[0]->max_length+(args[0]->max_length-args[0]->decimals)/3;
  }
  const char *func_name() const { return "format"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_char :public Item_str_func
{
public:
  Item_func_char(List<Item> &list) :Item_str_func(list) {}
  String *val_str(String *);
  void fix_length_and_dec() { maybe_null=0; max_length=arg_count; binary=0;}
  const char *func_name() const { return "char"; }
};


class Item_func_repeat :public Item_str_func
{
  String tmp_value;
public:
  Item_func_repeat(Item *arg1,Item *arg2) :Item_str_func(arg1,arg2) {}
  String *val_str(String *);
  void fix_length_and_dec();
  const char *func_name() const { return "repeat"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_rpad :public Item_str_func
{
  String tmp_value;
public:
  Item_func_rpad(Item *arg1,Item *arg2,Item *arg3)
    :Item_str_func(arg1,arg2,arg3) {}
  String *val_str(String *);
  void fix_length_and_dec();
  const char *func_name() const { return "rpad"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_lpad :public Item_str_func
{
  String tmp_value;
public:
  Item_func_lpad(Item *arg1,Item *arg2,Item *arg3)
    :Item_str_func(arg1,arg2,arg3) {}
  String *val_str(String *);
  void fix_length_and_dec();
  const char *func_name() const { return "lpad"; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_conv :public Item_str_func
{
public:
  Item_func_conv(Item *a,Item *b,Item *c) :Item_str_func(a,b,c) {}
  const char *func_name() const { return "conv"; }
  String *val_str(String *);
  void fix_length_and_dec() { decimals=0; max_length=64; }
};


class Item_func_hex :public Item_str_func
{
  String tmp_value;
public:
  Item_func_hex(Item *a) :Item_str_func(a) {}
  const char *func_name() const { return "hex"; }
  String *val_str(String *);
  void fix_length_and_dec() { decimals=0; max_length=args[0]->max_length*2; }
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_binary :public Item_str_func
{
public:
  Item_func_binary(Item *a) :Item_str_func(a) {}
  const char *func_name() const { return "binary"; }
  String *val_str(String *a)
  {
    String *tmp=args[0]->val_str(a);
    null_value=args[0]->null_value;
    return tmp;
   }
  void fix_length_and_dec() { binary=1; max_length=args[0]->max_length; }
  void print(String *str) { print_op(str); }
};


class Item_load_file :public Item_str_func
{
  String tmp_value;
public:
  Item_load_file(Item *a) :Item_str_func(a) {}
  String *val_str(String *);
  const char *func_name() const { return "load_file"; }
  void fix_length_and_dec()
  { binary=1; maybe_null=1; max_length=MAX_BLOB_WIDTH;}
  unsigned int size_of() { return sizeof(*this);}  
};


class Item_func_export_set: public Item_str_func
{
 public:
  Item_func_export_set(Item *a,Item *b,Item* c) :Item_str_func(a,b,c) {}
  Item_func_export_set(Item *a,Item *b,Item* c,Item* d) :Item_str_func(a,b,c,d) {}
  Item_func_export_set(Item *a,Item *b,Item* c,Item* d,Item* e) :Item_str_func(a,b,c,d,e) {}
  String  *val_str(String *str);
  void fix_length_and_dec();
  const char *func_name() const { return "export_set"; }
};

 class Item_func_inet_ntoa : public Item_str_func
{
public:
  Item_func_inet_ntoa(Item *a) :Item_str_func(a)
    {
    }
  String* val_str(String* str);
  const char *func_name() const { return "inet_ntoa"; }
  void fix_length_and_dec() { decimals = 0; max_length=3*8+7; }
};

class Item_func_quote :public Item_str_func
{
public:
  Item_func_quote(Item *a) :Item_str_func(a) {}
  const char *func_name() const { return "quote"; }
  String *val_str(String *);
  void fix_length_and_dec() { max_length= args[0]->max_length * 2 + 2; }
};
