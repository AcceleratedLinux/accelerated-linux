/* Internal type definitions for GDB.

   Copyright (C) 1992, 1993, 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001,
   2002, 2003, 2004, 2006, 2007, 2008 Free Software Foundation, Inc.

   Contributed by Cygnus Support, using pieces from other GDB modules.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#if !defined (GDBTYPES_H)
#define GDBTYPES_H 1

#include "hashtab.h"

/* Forward declarations for prototypes.  */
struct field;
struct block;

/* Some macros for char-based bitfields.  */

#define B_SET(a,x)	((a)[(x)>>3] |= (1 << ((x)&7)))
#define B_CLR(a,x)	((a)[(x)>>3] &= ~(1 << ((x)&7)))
#define B_TST(a,x)	((a)[(x)>>3] & (1 << ((x)&7)))
#define B_TYPE		unsigned char
#define	B_BYTES(x)	( 1 + ((x)>>3) )
#define	B_CLRALL(a,x)	memset ((a), 0, B_BYTES(x))

/* Different kinds of data types are distinguished by the `code' field.  */

enum type_code
  {
    TYPE_CODE_UNDEF,		/* Not used; catches errors */
    TYPE_CODE_PTR,		/* Pointer type */

    /* Array type with lower & upper bounds.

       Regardless of the language, GDB represents multidimensional
       array types the way C does: as arrays of arrays.  So an
       instance of a GDB array type T can always be seen as a series
       of instances of TYPE_TARGET_TYPE (T) laid out sequentially in
       memory.

       Row-major languages like C lay out multi-dimensional arrays so
       that incrementing the rightmost index in a subscripting
       expression results in the smallest change in the address of the
       element referred to.  Column-major languages like Fortran lay
       them out so that incrementing the leftmost index results in the
       smallest change.

       This means that, in column-major languages, working our way
       from type to target type corresponds to working through indices
       from right to left, not left to right.  */
    TYPE_CODE_ARRAY,

    TYPE_CODE_STRUCT,		/* C struct or Pascal record */
    TYPE_CODE_UNION,		/* C union or Pascal variant part */
    TYPE_CODE_ENUM,		/* Enumeration type */
    TYPE_CODE_FLAGS,		/* Bit flags type */
    TYPE_CODE_FUNC,		/* Function type */
    TYPE_CODE_INT,		/* Integer type */

    /* Floating type.  This is *NOT* a complex type.  Beware, there are parts
       of GDB which bogusly assume that TYPE_CODE_FLT can mean complex.  */
    TYPE_CODE_FLT,

    /* Void type.  The length field specifies the length (probably always
       one) which is used in pointer arithmetic involving pointers to
       this type, but actually dereferencing such a pointer is invalid;
       a void type has no length and no actual representation in memory
       or registers.  A pointer to a void type is a generic pointer.  */
    TYPE_CODE_VOID,

    TYPE_CODE_SET,		/* Pascal sets */
    TYPE_CODE_RANGE,		/* Range (integers within spec'd bounds) */

    /* A string type which is like an array of character but prints
       differently (at least for (the deleted) CHILL).  It does not
       contain a length field as Pascal strings (for many Pascals,
       anyway) do; if we want to deal with such strings, we should use
       a new type code.  */
    TYPE_CODE_STRING,

    /* String of bits; like TYPE_CODE_SET but prints differently (at
       least for (the deleted) CHILL).  */
    TYPE_CODE_BITSTRING,

    /* Unknown type.  The length field is valid if we were able to
       deduce that much about the type, or 0 if we don't even know that.  */
    TYPE_CODE_ERROR,

    /* C++ */
    TYPE_CODE_METHOD,		/* Method type */

    /* Pointer-to-member-function type.  This describes how to access a
       particular member function of a class (possibly a virtual
       member function).  The representation may vary between different
       C++ ABIs.  */
    TYPE_CODE_METHODPTR,

    /* Pointer-to-member type.  This is the offset within a class to some
       particular data member.  The only currently supported representation
       uses an unbiased offset, with -1 representing NULL; this is used
       by the Itanium C++ ABI (used by GCC on all platforms).  */
    TYPE_CODE_MEMBERPTR,

    TYPE_CODE_REF,		/* C++ Reference types */

    TYPE_CODE_CHAR,		/* *real* character type */

    /* Boolean type.  0 is false, 1 is true, and other values are non-boolean
       (e.g. FORTRAN "logical" used as unsigned int).  */
    TYPE_CODE_BOOL,

    /* Fortran */
    TYPE_CODE_COMPLEX,		/* Complex float */

    TYPE_CODE_TYPEDEF,
    TYPE_CODE_TEMPLATE,		/* C++ template */
    TYPE_CODE_TEMPLATE_ARG,	/* C++ template arg */

    TYPE_CODE_NAMESPACE,	/* C++ namespace.  */

    TYPE_CODE_DECFLOAT		/* Decimal floating point.  */
  };

/* For now allow source to use TYPE_CODE_CLASS for C++ classes, as an
   alias for TYPE_CODE_STRUCT.  This is for DWARF, which has a distinct
   "class" attribute.  Perhaps we should actually have a separate TYPE_CODE
   so that we can print "class" or "struct" depending on what the debug
   info said.  It's not clear we should bother.  */

#define TYPE_CODE_CLASS TYPE_CODE_STRUCT

/* Some bits for the type's flags word, and macros to test them. */

/* Unsigned integer type.  If this is not set for a TYPE_CODE_INT, the
   type is signed (unless TYPE_FLAG_NOSIGN (below) is set). */

#define TYPE_FLAG_UNSIGNED	(1 << 0)
#define TYPE_UNSIGNED(t)	(TYPE_FLAGS (t) & TYPE_FLAG_UNSIGNED)

/* No sign for this type.  In C++, "char", "signed char", and "unsigned
   char" are distinct types; so we need an extra flag to indicate the
   absence of a sign! */

#define TYPE_FLAG_NOSIGN	(1 << 1)
#define TYPE_NOSIGN(t)		(TYPE_FLAGS (t) & TYPE_FLAG_NOSIGN)

/* This appears in a type's flags word if it is a stub type (e.g., if
   someone referenced a type that wasn't defined in a source file
   via (struct sir_not_appearing_in_this_film *)).  */

#define TYPE_FLAG_STUB		(1 << 2)
#define TYPE_STUB(t)		(TYPE_FLAGS (t) & TYPE_FLAG_STUB)

/* The target type of this type is a stub type, and this type needs to
   be updated if it gets un-stubbed in check_typedef.
   Used for arrays and ranges, in which TYPE_LENGTH of the array/range
   gets set based on the TYPE_LENGTH of the target type.
   Also, set for TYPE_CODE_TYPEDEF. */

#define TYPE_FLAG_TARGET_STUB	(1 << 3)
#define TYPE_TARGET_STUB(t)	(TYPE_FLAGS (t) & TYPE_FLAG_TARGET_STUB)

/* Static type.  If this is set, the corresponding type had 
 * a static modifier.
 * Note: This may be unnecessary, since static data members
 * are indicated by other means (bitpos == -1)
 */

#define TYPE_FLAG_STATIC	(1 << 4)
#define TYPE_STATIC(t)		(TYPE_FLAGS (t) & TYPE_FLAG_STATIC)

/* Constant type.  If this is set, the corresponding type has a
 * const modifier.
 */

#define TYPE_FLAG_CONST		(1 << 5)
#define TYPE_CONST(t)		(TYPE_INSTANCE_FLAGS (t) & TYPE_FLAG_CONST)

/* Volatile type.  If this is set, the corresponding type has a
 * volatile modifier.
 */

#define TYPE_FLAG_VOLATILE	(1 << 6)
#define TYPE_VOLATILE(t)	(TYPE_INSTANCE_FLAGS (t) & TYPE_FLAG_VOLATILE)


/* This is a function type which appears to have a prototype.  We need this
   for function calls in order to tell us if it's necessary to coerce the args,
   or to just do the standard conversions.  This is used with a short field. */

#define TYPE_FLAG_PROTOTYPED	(1 << 7)
#define TYPE_PROTOTYPED(t)	(TYPE_FLAGS (t) & TYPE_FLAG_PROTOTYPED)

/* This flag is used to indicate that processing for this type
   is incomplete.

   (Mostly intended for HP platforms, where class methods, for
   instance, can be encountered before their classes in the debug
   info; the incomplete type has to be marked so that the class and
   the method can be assigned correct types.) */

#define TYPE_FLAG_INCOMPLETE	(1 << 8)
#define TYPE_INCOMPLETE(t)	(TYPE_FLAGS (t) & TYPE_FLAG_INCOMPLETE)

/* Instruction-space delimited type.  This is for Harvard architectures
   which have separate instruction and data address spaces (and perhaps
   others).

   GDB usually defines a flat address space that is a superset of the
   architecture's two (or more) address spaces, but this is an extension
   of the architecture's model.

   If TYPE_FLAG_INST is set, an object of the corresponding type
   resides in instruction memory, even if its address (in the extended
   flat address space) does not reflect this.

   Similarly, if TYPE_FLAG_DATA is set, then an object of the 
   corresponding type resides in the data memory space, even if
   this is not indicated by its (flat address space) address.

   If neither flag is set, the default space for functions / methods
   is instruction space, and for data objects is data memory.  */

#define TYPE_FLAG_CODE_SPACE	(1 << 9)
#define TYPE_CODE_SPACE(t)	(TYPE_INSTANCE_FLAGS (t) & TYPE_FLAG_CODE_SPACE)

#define TYPE_FLAG_DATA_SPACE	(1 << 10)
#define TYPE_DATA_SPACE(t)	(TYPE_INSTANCE_FLAGS (t) & TYPE_FLAG_DATA_SPACE)

/* FIXME drow/2002-06-03:  Only used for methods, but applies as well
   to functions.  */

#define TYPE_FLAG_VARARGS	(1 << 11)
#define TYPE_VARARGS(t)		(TYPE_FLAGS (t) & TYPE_FLAG_VARARGS)

/* Identify a vector type.  Gcc is handling this by adding an extra
   attribute to the array type.  We slurp that in as a new flag of a
   type.  This is used only in dwarf2read.c.  */
#define TYPE_FLAG_VECTOR	(1 << 12)
#define TYPE_VECTOR(t)		(TYPE_FLAGS (t) & TYPE_FLAG_VECTOR)

/* Address class flags.  Some environments provide for pointers whose
   size is different from that of a normal pointer or address types
   where the bits are interpreted differently than normal addresses.  The
   TYPE_FLAG_ADDRESS_CLASS_n flags may be used in target specific
   ways to represent these different types of address classes.  */
#define TYPE_FLAG_ADDRESS_CLASS_1 (1 << 13)
#define TYPE_ADDRESS_CLASS_1(t) (TYPE_INSTANCE_FLAGS(t) \
                                 & TYPE_FLAG_ADDRESS_CLASS_1)
#define TYPE_FLAG_ADDRESS_CLASS_2 (1 << 14)
#define TYPE_ADDRESS_CLASS_2(t) (TYPE_INSTANCE_FLAGS(t) \
				 & TYPE_FLAG_ADDRESS_CLASS_2)
#define TYPE_FLAG_ADDRESS_CLASS_ALL (TYPE_FLAG_ADDRESS_CLASS_1 \
				     | TYPE_FLAG_ADDRESS_CLASS_2)
#define TYPE_ADDRESS_CLASS_ALL(t) (TYPE_INSTANCE_FLAGS(t) \
				   & TYPE_FLAG_ADDRESS_CLASS_ALL)

/* The debugging formats (especially STABS) do not contain enough information
   to represent all Ada types---especially those whose size depends on
   dynamic quantities.  Therefore, the GNAT Ada compiler includes
   extra information in the form of additional type definitions
   connected by naming conventions.  This flag indicates that the 
   type is an ordinary (unencoded) GDB type that has been created from 
   the necessary run-time information, and does not need further 
   interpretation. Optionally marks ordinary, fixed-size GDB type. */

#define TYPE_FLAG_FIXED_INSTANCE (1 << 15)

/* This debug target supports TYPE_STUB(t).  In the unsupported case we have to
   rely on NFIELDS to be zero etc., see TYPE_IS_OPAQUE ().
   TYPE_STUB(t) with !TYPE_STUB_SUPPORTED(t) may exist if we only guessed
   the TYPE_STUB(t) value (see dwarfread.c).  */

#define TYPE_FLAG_STUB_SUPPORTED (1 << 16)
#define TYPE_STUB_SUPPORTED(t)   (TYPE_FLAGS (t) & TYPE_FLAG_STUB_SUPPORTED)

/* Not textual.  By default, GDB treats all single byte integers as
   characters (or elements of strings) unless this flag is set.  */

#define TYPE_FLAG_NOTTEXT	(1 << 17)
#define TYPE_NOTTEXT(t)		(TYPE_FLAGS (t) & TYPE_FLAG_NOTTEXT)

/*  Array bound type.  */
enum array_bound_type
{
  BOUND_SIMPLE = 0,
  BOUND_BY_VALUE_IN_REG,
  BOUND_BY_REF_IN_REG,
  BOUND_BY_VALUE_ON_STACK,
  BOUND_BY_REF_ON_STACK,
  BOUND_CANNOT_BE_DETERMINED
};

/* This structure is space-critical.
   Its layout has been tweaked to reduce the space used.  */

struct main_type
{
  /* Code for kind of type */

  ENUM_BITFIELD(type_code) code : 8;

  /* Array bounds.  These fields appear at this location because
     they pack nicely here.  */

  ENUM_BITFIELD(array_bound_type) upper_bound_type : 4;
  ENUM_BITFIELD(array_bound_type) lower_bound_type : 4;

  /* Name of this type, or NULL if none.

     This is used for printing only, except by poorly designed C++ code.
     For looking up a name, look for a symbol in the VAR_DOMAIN.  */

  char *name;

  /* Tag name for this type, or NULL if none.  This means that the
     name of the type consists of a keyword followed by the tag name.
     Which keyword is determined by the type code ("struct" for
     TYPE_CODE_STRUCT, etc.).  As far as I know C/C++ are the only languages
     with this feature.

     This is used for printing only, except by poorly designed C++ code.
     For looking up a name, look for a symbol in the STRUCT_DOMAIN.
     One more legitimate use is that if TYPE_FLAG_STUB is set, this is
     the name to use to look for definitions in other files.  */

  char *tag_name;

  /* Every type is now associated with a particular objfile, and the
     type is allocated on the objfile_obstack for that objfile.  One problem
     however, is that there are times when gdb allocates new types while
     it is not in the process of reading symbols from a particular objfile.
     Fortunately, these happen when the type being created is a derived
     type of an existing type, such as in lookup_pointer_type().  So
     we can just allocate the new type using the same objfile as the
     existing type, but to do this we need a backpointer to the objfile
     from the existing type.  Yes this is somewhat ugly, but without
     major overhaul of the internal type system, it can't be avoided
     for now. */

  struct objfile *objfile;

  /* For a pointer type, describes the type of object pointed to.
     For an array type, describes the type of the elements.
     For a function or method type, describes the type of the return value.
     For a range type, describes the type of the full range.
     For a complex type, describes the type of each coordinate.
     Unused otherwise.  */

  struct type *target_type;

  /* Flags about this type.  */

  int flags;

  /* Number of fields described for this type */

  short nfields;

  /* Field number of the virtual function table pointer in
     VPTR_BASETYPE.  If -1, we were unable to find the virtual
     function table pointer in initial symbol reading, and
     get_vptr_fieldno should be called to find it if possible.
     get_vptr_fieldno will update this field if possible.
     Otherwise the value is left at -1.

     Unused if this type does not have virtual functions.  */

  short vptr_fieldno;

  /* For structure and union types, a description of each field.
     For set and pascal array types, there is one "field",
     whose type is the domain type of the set or array.
     For range types, there are two "fields",
     the minimum and maximum values (both inclusive).
     For enum types, each possible value is described by one "field".
     For a function or method type, a "field" for each parameter.
     For C++ classes, there is one field for each base class (if it is
     a derived class) plus one field for each class data member.  Member
     functions are recorded elsewhere.

     Using a pointer to a separate array of fields
     allows all types to have the same size, which is useful
     because we can allocate the space for a type before
     we know what to put in it.  */

  struct field
  {
    union field_location
    {
      /* Position of this field, counting in bits from start of
	 containing structure.
	 For gdbarch_bits_big_endian=1 targets, it is the bit offset to the MSB.
	 For gdbarch_bits_big_endian=0 targets, it is the bit offset to the LSB.
	 For a range bound or enum value, this is the value itself. */

      int bitpos;

      /* For a static field, if TYPE_FIELD_STATIC_HAS_ADDR then physaddr
	 is the location (in the target) of the static field.
	 Otherwise, physname is the mangled label of the static field. */

      CORE_ADDR physaddr;
      char *physname;
    }
    loc;

    /* For a function or member type, this is 1 if the argument is marked
       artificial.  Artificial arguments should not be shown to the
       user.  */
    unsigned int artificial : 1;

    /* This flag is zero for non-static fields, 1 for fields whose location
       is specified by the label loc.physname, and 2 for fields whose location
       is specified by loc.physaddr.  */

    unsigned int static_kind : 2;

    /* Size of this field, in bits, or zero if not packed.
       For an unpacked field, the field's type's length
       says how many bytes the field occupies.  */

    unsigned int bitsize : 29;

    /* In a struct or union type, type of this field.
       In a function or member type, type of this argument.
       In an array type, the domain-type of the array.  */

    struct type *type;

    /* Name of field, value or argument.
       NULL for range bounds, array domains, and member function
       arguments.  */

    char *name;

  } *fields;

  /* For types with virtual functions (TYPE_CODE_STRUCT), VPTR_BASETYPE
     is the base class which defined the virtual function table pointer.  

     For types that are pointer to member types (TYPE_CODE_METHODPTR,
     TYPE_CODE_MEMBERPTR), VPTR_BASETYPE is the type that this pointer
     is a member of.

     For method types (TYPE_CODE_METHOD), VPTR_BASETYPE is the aggregate
     type that contains the method.

     Unused otherwise.  */

  struct type *vptr_basetype;

  /* Slot to point to additional language-specific fields of this type.  */

  union type_specific
  {
    /* CPLUS_STUFF is for TYPE_CODE_STRUCT.  It is initialized to point to
       cplus_struct_default, a default static instance of a struct
       cplus_struct_type. */

    struct cplus_struct_type *cplus_stuff;

    /* FLOATFORMAT is for TYPE_CODE_FLT.  It is a pointer to two
       floatformat objects that describe the floating-point value
       that resides within the type.  The first is for big endian
       targets and the second is for little endian targets.  */

    const struct floatformat **floatformat;
  } type_specific;
};

/* A ``struct type'' describes a particular instance of a type, with
   some particular qualification.  */
struct type
{
  /* Type that is a pointer to this type.
     NULL if no such pointer-to type is known yet.
     The debugger may add the address of such a type
     if it has to construct one later.  */

  struct type *pointer_type;

  /* C++: also need a reference type.  */

  struct type *reference_type;

  /* Variant chain.  This points to a type that differs from this one only
     in qualifiers and length.  Currently, the possible qualifiers are
     const, volatile, code-space, data-space, and address class.  The
     length may differ only when one of the address class flags are set.
     The variants are linked in a circular ring and share MAIN_TYPE.  */
  struct type *chain;

  /* Flags specific to this instance of the type, indicating where
     on the ring we are.  */
  int instance_flags;

  /* Length of storage for a value of this type.  This is what
     sizeof(type) would return; use it for address arithmetic,
     memory reads and writes, etc.  This size includes padding.  For
     example, an i386 extended-precision floating point value really
     only occupies ten bytes, but most ABI's declare its size to be
     12 bytes, to preserve alignment.  A `struct type' representing
     such a floating-point type would have a `length' value of 12,
     even though the last two bytes are unused.

     There's a bit of a host/target mess here, if you're concerned
     about machines whose bytes aren't eight bits long, or who don't
     have byte-addressed memory.  Various places pass this to memcpy
     and such, meaning it must be in units of host bytes.  Various
     other places expect they can calculate addresses by adding it
     and such, meaning it must be in units of target bytes.  For
     some DSP targets, in which HOST_CHAR_BIT will (presumably) be 8
     and TARGET_CHAR_BIT will be (say) 32, this is a problem.

     One fix would be to make this field in bits (requiring that it
     always be a multiple of HOST_CHAR_BIT and TARGET_CHAR_BIT) ---
     the other choice would be to make it consistently in units of
     HOST_CHAR_BIT.  However, this would still fail to address
     machines based on a ternary or decimal representation.  */
  
  unsigned length;

  /* Core type, shared by a group of qualified types.  */
  struct main_type *main_type;
};

#define	NULL_TYPE ((struct type *) 0)

/* C++ language-specific information for TYPE_CODE_STRUCT and TYPE_CODE_UNION
   nodes.  */

struct cplus_struct_type
  {
    /* Number of base classes this type derives from.  The baseclasses are
       stored in the first N_BASECLASSES fields (i.e. the `fields' field of
       the struct type).  I think only the `type' field of such a field has
       any meaning.  */

    short n_baseclasses;

    /* Number of methods with unique names.  All overloaded methods with
       the same name count only once. */

    short nfn_fields;

    /* Number of methods described for this type, not including the
       methods that it derives from.  */

    short nfn_fields_total;

    /* The "declared_type" field contains a code saying how the
       user really declared this type, e.g., "class s", "union s",
       "struct s".
       The 3 above things come out from the C++ compiler looking like classes, 
       but we keep track of the real declaration so we can give
       the correct information on "ptype". (Note: TEMPLATE may not
       belong in this list...)  */

#define DECLARED_TYPE_CLASS 0
#define DECLARED_TYPE_UNION 1
#define DECLARED_TYPE_STRUCT 2
#define DECLARED_TYPE_TEMPLATE 3
    short declared_type;	/* One of the above codes */

    /* For derived classes, the number of base classes is given by n_baseclasses
       and virtual_field_bits is a bit vector containing one bit per base class.
       If the base class is virtual, the corresponding bit will be set.
       I.E, given:

       class A{};
       class B{};
       class C : public B, public virtual A {};

       B is a baseclass of C; A is a virtual baseclass for C.
       This is a C++ 2.0 language feature. */

    B_TYPE *virtual_field_bits;

    /* For classes with private fields, the number of fields is given by
       nfields and private_field_bits is a bit vector containing one bit
       per field.
       If the field is private, the corresponding bit will be set. */

    B_TYPE *private_field_bits;

    /* For classes with protected fields, the number of fields is given by
       nfields and protected_field_bits is a bit vector containing one bit
       per field.
       If the field is private, the corresponding bit will be set. */

    B_TYPE *protected_field_bits;

    /* for classes with fields to be ignored, either this is optimized out
       or this field has length 0 */

    B_TYPE *ignore_field_bits;

    /* For classes, structures, and unions, a description of each field,
       which consists of an overloaded name, followed by the types of
       arguments that the method expects, and then the name after it
       has been renamed to make it distinct.

       fn_fieldlists points to an array of nfn_fields of these. */

    struct fn_fieldlist
      {

	/* The overloaded name.  */

	char *name;

	/* The number of methods with this name.  */

	int length;

	/* The list of methods.  */

	struct fn_field
	  {

	    /* If is_stub is clear, this is the mangled name which we can
	       look up to find the address of the method (FIXME: it would
	       be cleaner to have a pointer to the struct symbol here
	       instead).  */

	    /* If is_stub is set, this is the portion of the mangled
	       name which specifies the arguments.  For example, "ii",
	       if there are two int arguments, or "" if there are no
	       arguments.  See gdb_mangle_name for the conversion from this
	       format to the one used if is_stub is clear.  */

	    char *physname;

	    /* The function type for the method.
	       (This comment used to say "The return value of the method",
	       but that's wrong. The function type 
	       is expected here, i.e. something with TYPE_CODE_FUNC,
	       and *not* the return-value type). */

	    struct type *type;

	    /* For virtual functions.
	       First baseclass that defines this virtual function.   */

	    struct type *fcontext;

	    /* Attributes. */

	    unsigned int is_const:1;
	    unsigned int is_volatile:1;
	    unsigned int is_private:1;
	    unsigned int is_protected:1;
	    unsigned int is_public:1;
	    unsigned int is_abstract:1;
	    unsigned int is_static:1;
	    unsigned int is_final:1;
	    unsigned int is_synchronized:1;
	    unsigned int is_native:1;
	    unsigned int is_artificial:1;

	    /* A stub method only has some fields valid (but they are enough
	       to reconstruct the rest of the fields).  */
	    unsigned int is_stub:1;

	    /* C++ method that is inlined */
	    unsigned int is_inlined:1;

	    /* Unused.  */
	    unsigned int dummy:3;

	    /* Index into that baseclass's virtual function table,
	       minus 2; else if static: VOFFSET_STATIC; else: 0.  */

	    unsigned int voffset:16;

#define VOFFSET_STATIC 1

	  }
	 *fn_fields;

      }
     *fn_fieldlists;

    /* If this "struct type" describes a template, then it 
     * has arguments. "template_args" points to an array of
     * template arg descriptors, of length "ntemplate_args".
     * The only real information in each of these template arg descriptors
     * is a name. "type" will typically just point to a "struct type" with
     * the placeholder TYPE_CODE_TEMPLATE_ARG type.
     */
    short ntemplate_args;
    struct template_arg
      {
	char *name;
	struct type *type;
      }
     *template_args;

    /* If this "struct type" describes a template, it has a list
     * of instantiations. "instantiations" is a pointer to an array
     * of type's, one representing each instantiation. There
     * are "ninstantiations" elements in this array.
     */
    short ninstantiations;
    struct type **instantiations;

    /* Pointer to information about enclosing scope, if this is a
     * local type.  If it is not a local type, this is NULL
     */
    struct local_type_info
      {
	char *file;
	int line;
      }
     *localtype_ptr;
  };

/* Struct used in computing virtual base list */
struct vbase
  {
    struct type *vbasetype;	/* pointer to virtual base */
    struct vbase *next;		/* next in chain */
  };

/* Struct used for ranking a function for overload resolution */
struct badness_vector
  {
    int length;
    int *rank;
  };

/* The default value of TYPE_CPLUS_SPECIFIC(T) points to the
   this shared static structure. */

extern const struct cplus_struct_type cplus_struct_default;

extern void allocate_cplus_struct_type (struct type *);

#define INIT_CPLUS_SPECIFIC(type) \
  (TYPE_CPLUS_SPECIFIC(type)=(struct cplus_struct_type*)&cplus_struct_default)
#define ALLOCATE_CPLUS_STRUCT_TYPE(type) allocate_cplus_struct_type (type)
#define HAVE_CPLUS_STRUCT(type) \
  (TYPE_CPLUS_SPECIFIC(type) != &cplus_struct_default)

#define TYPE_INSTANCE_FLAGS(thistype) (thistype)->instance_flags
#define TYPE_MAIN_TYPE(thistype) (thistype)->main_type
#define TYPE_NAME(thistype) TYPE_MAIN_TYPE(thistype)->name
#define TYPE_TAG_NAME(type) TYPE_MAIN_TYPE(type)->tag_name
#define TYPE_TARGET_TYPE(thistype) TYPE_MAIN_TYPE(thistype)->target_type
#define TYPE_POINTER_TYPE(thistype) (thistype)->pointer_type
#define TYPE_REFERENCE_TYPE(thistype) (thistype)->reference_type
#define TYPE_CHAIN(thistype) (thistype)->chain
/* Note that if thistype is a TYPEDEF type, you have to call check_typedef.
   But check_typedef does set the TYPE_LENGTH of the TYPEDEF type,
   so you only have to call check_typedef once.  Since allocate_value
   calls check_typedef, TYPE_LENGTH (VALUE_TYPE (X)) is safe.  */
#define TYPE_LENGTH(thistype) (thistype)->length
#define TYPE_OBJFILE(thistype) TYPE_MAIN_TYPE(thistype)->objfile
#define TYPE_FLAGS(thistype) TYPE_MAIN_TYPE(thistype)->flags
/* Note that TYPE_CODE can be TYPE_CODE_TYPEDEF, so if you want the real
   type, you need to do TYPE_CODE (check_type (this_type)). */
#define TYPE_CODE(thistype) TYPE_MAIN_TYPE(thistype)->code
#define TYPE_NFIELDS(thistype) TYPE_MAIN_TYPE(thistype)->nfields
#define TYPE_FIELDS(thistype) TYPE_MAIN_TYPE(thistype)->fields
#define TYPE_TEMPLATE_ARGS(thistype) TYPE_CPLUS_SPECIFIC(thistype)->template_args
#define TYPE_INSTANTIATIONS(thistype) TYPE_CPLUS_SPECIFIC(thistype)->instantiations

#define TYPE_INDEX_TYPE(type) TYPE_FIELD_TYPE (type, 0)
#define TYPE_LOW_BOUND(range_type) TYPE_FIELD_BITPOS (range_type, 0)
#define TYPE_HIGH_BOUND(range_type) TYPE_FIELD_BITPOS (range_type, 1)

/* Moto-specific stuff for FORTRAN arrays */

#define TYPE_ARRAY_UPPER_BOUND_TYPE(thistype) \
	TYPE_MAIN_TYPE(thistype)->upper_bound_type
#define TYPE_ARRAY_LOWER_BOUND_TYPE(thistype) \
	TYPE_MAIN_TYPE(thistype)->lower_bound_type

#define TYPE_ARRAY_UPPER_BOUND_VALUE(arraytype) \
   (TYPE_FIELD_BITPOS((TYPE_FIELD_TYPE((arraytype),0)),1))

#define TYPE_ARRAY_LOWER_BOUND_VALUE(arraytype) \
   (TYPE_FIELD_BITPOS((TYPE_FIELD_TYPE((arraytype),0)),0))

/* C++ */

#define TYPE_VPTR_BASETYPE(thistype) TYPE_MAIN_TYPE(thistype)->vptr_basetype
#define TYPE_DOMAIN_TYPE(thistype) TYPE_MAIN_TYPE(thistype)->vptr_basetype
#define TYPE_VPTR_FIELDNO(thistype) TYPE_MAIN_TYPE(thistype)->vptr_fieldno
#define TYPE_FN_FIELDS(thistype) TYPE_CPLUS_SPECIFIC(thistype)->fn_fields
#define TYPE_NFN_FIELDS(thistype) TYPE_CPLUS_SPECIFIC(thistype)->nfn_fields
#define TYPE_NFN_FIELDS_TOTAL(thistype) TYPE_CPLUS_SPECIFIC(thistype)->nfn_fields_total
#define TYPE_NTEMPLATE_ARGS(thistype) TYPE_CPLUS_SPECIFIC(thistype)->ntemplate_args
#define TYPE_NINSTANTIATIONS(thistype) TYPE_CPLUS_SPECIFIC(thistype)->ninstantiations
#define TYPE_DECLARED_TYPE(thistype) TYPE_CPLUS_SPECIFIC(thistype)->declared_type
#define	TYPE_TYPE_SPECIFIC(thistype) TYPE_MAIN_TYPE(thistype)->type_specific
#define TYPE_CPLUS_SPECIFIC(thistype) TYPE_MAIN_TYPE(thistype)->type_specific.cplus_stuff
#define TYPE_FLOATFORMAT(thistype) TYPE_MAIN_TYPE(thistype)->type_specific.floatformat
#define TYPE_BASECLASS(thistype,index) TYPE_MAIN_TYPE(thistype)->fields[index].type
#define TYPE_N_BASECLASSES(thistype) TYPE_CPLUS_SPECIFIC(thistype)->n_baseclasses
#define TYPE_BASECLASS_NAME(thistype,index) TYPE_MAIN_TYPE(thistype)->fields[index].name
#define TYPE_BASECLASS_BITPOS(thistype,index) TYPE_FIELD_BITPOS(thistype,index)
#define BASETYPE_VIA_PUBLIC(thistype, index) \
  ((!TYPE_FIELD_PRIVATE(thistype, index)) && (!TYPE_FIELD_PROTECTED(thistype, index)))

#define BASETYPE_VIA_VIRTUAL(thistype, index) \
  (TYPE_CPLUS_SPECIFIC(thistype)->virtual_field_bits == NULL ? 0 \
    : B_TST(TYPE_CPLUS_SPECIFIC(thistype)->virtual_field_bits, (index)))

#define FIELD_TYPE(thisfld) ((thisfld).type)
#define FIELD_NAME(thisfld) ((thisfld).name)
#define FIELD_BITPOS(thisfld) ((thisfld).loc.bitpos)
#define FIELD_ARTIFICIAL(thisfld) ((thisfld).artificial)
#define FIELD_BITSIZE(thisfld) ((thisfld).bitsize)
#define FIELD_STATIC_KIND(thisfld) ((thisfld).static_kind)
#define FIELD_PHYSNAME(thisfld) ((thisfld).loc.physname)
#define FIELD_PHYSADDR(thisfld) ((thisfld).loc.physaddr)
#define SET_FIELD_PHYSNAME(thisfld, name) \
  ((thisfld).static_kind = 1, FIELD_PHYSNAME(thisfld) = (name))
#define SET_FIELD_PHYSADDR(thisfld, name) \
  ((thisfld).static_kind = 2, FIELD_PHYSADDR(thisfld) = (name))
#define TYPE_FIELD(thistype, n) TYPE_MAIN_TYPE(thistype)->fields[n]
#define TYPE_FIELD_TYPE(thistype, n) FIELD_TYPE(TYPE_FIELD(thistype, n))
#define TYPE_FIELD_NAME(thistype, n) FIELD_NAME(TYPE_FIELD(thistype, n))
#define TYPE_FIELD_BITPOS(thistype, n) FIELD_BITPOS(TYPE_FIELD(thistype,n))
#define TYPE_FIELD_ARTIFICIAL(thistype, n) FIELD_ARTIFICIAL(TYPE_FIELD(thistype,n))
#define TYPE_FIELD_BITSIZE(thistype, n) FIELD_BITSIZE(TYPE_FIELD(thistype,n))
#define TYPE_FIELD_PACKED(thistype, n) (FIELD_BITSIZE(TYPE_FIELD(thistype,n))!=0)
#define TYPE_TEMPLATE_ARG(thistype, n) TYPE_CPLUS_SPECIFIC(thistype)->template_args[n]
#define TYPE_INSTANTIATION(thistype, n) TYPE_CPLUS_SPECIFIC(thistype)->instantiations[n]

#define TYPE_FIELD_PRIVATE_BITS(thistype) \
  TYPE_CPLUS_SPECIFIC(thistype)->private_field_bits
#define TYPE_FIELD_PROTECTED_BITS(thistype) \
  TYPE_CPLUS_SPECIFIC(thistype)->protected_field_bits
#define TYPE_FIELD_IGNORE_BITS(thistype) \
  TYPE_CPLUS_SPECIFIC(thistype)->ignore_field_bits
#define TYPE_FIELD_VIRTUAL_BITS(thistype) \
  TYPE_CPLUS_SPECIFIC(thistype)->virtual_field_bits
#define SET_TYPE_FIELD_PRIVATE(thistype, n) \
  B_SET (TYPE_CPLUS_SPECIFIC(thistype)->private_field_bits, (n))
#define SET_TYPE_FIELD_PROTECTED(thistype, n) \
  B_SET (TYPE_CPLUS_SPECIFIC(thistype)->protected_field_bits, (n))
#define SET_TYPE_FIELD_IGNORE(thistype, n) \
  B_SET (TYPE_CPLUS_SPECIFIC(thistype)->ignore_field_bits, (n))
#define SET_TYPE_FIELD_VIRTUAL(thistype, n) \
  B_SET (TYPE_CPLUS_SPECIFIC(thistype)->virtual_field_bits, (n))
#define TYPE_FIELD_PRIVATE(thistype, n) \
  (TYPE_CPLUS_SPECIFIC(thistype)->private_field_bits == NULL ? 0 \
    : B_TST(TYPE_CPLUS_SPECIFIC(thistype)->private_field_bits, (n)))
#define TYPE_FIELD_PROTECTED(thistype, n) \
  (TYPE_CPLUS_SPECIFIC(thistype)->protected_field_bits == NULL ? 0 \
    : B_TST(TYPE_CPLUS_SPECIFIC(thistype)->protected_field_bits, (n)))
#define TYPE_FIELD_IGNORE(thistype, n) \
  (TYPE_CPLUS_SPECIFIC(thistype)->ignore_field_bits == NULL ? 0 \
    : B_TST(TYPE_CPLUS_SPECIFIC(thistype)->ignore_field_bits, (n)))
#define TYPE_FIELD_VIRTUAL(thistype, n) \
  (TYPE_CPLUS_SPECIFIC(thistype)->virtual_field_bits == NULL ? 0 \
    : B_TST(TYPE_CPLUS_SPECIFIC(thistype)->virtual_field_bits, (n)))

#define TYPE_FIELD_STATIC(thistype, n) (TYPE_MAIN_TYPE (thistype)->fields[n].static_kind != 0)
#define TYPE_FIELD_STATIC_KIND(thistype, n) TYPE_MAIN_TYPE (thistype)->fields[n].static_kind
#define TYPE_FIELD_STATIC_HAS_ADDR(thistype, n) (TYPE_MAIN_TYPE (thistype)->fields[n].static_kind == 2)
#define TYPE_FIELD_STATIC_PHYSNAME(thistype, n) FIELD_PHYSNAME(TYPE_FIELD(thistype, n))
#define TYPE_FIELD_STATIC_PHYSADDR(thistype, n) FIELD_PHYSADDR(TYPE_FIELD(thistype, n))

#define TYPE_FN_FIELDLISTS(thistype) TYPE_CPLUS_SPECIFIC(thistype)->fn_fieldlists
#define TYPE_FN_FIELDLIST(thistype, n) TYPE_CPLUS_SPECIFIC(thistype)->fn_fieldlists[n]
#define TYPE_FN_FIELDLIST1(thistype, n) TYPE_CPLUS_SPECIFIC(thistype)->fn_fieldlists[n].fn_fields
#define TYPE_FN_FIELDLIST_NAME(thistype, n) TYPE_CPLUS_SPECIFIC(thistype)->fn_fieldlists[n].name
#define TYPE_FN_FIELDLIST_LENGTH(thistype, n) TYPE_CPLUS_SPECIFIC(thistype)->fn_fieldlists[n].length

#define TYPE_FN_FIELD(thisfn, n) (thisfn)[n]
#define TYPE_FN_FIELD_PHYSNAME(thisfn, n) (thisfn)[n].physname
#define TYPE_FN_FIELD_TYPE(thisfn, n) (thisfn)[n].type
#define TYPE_FN_FIELD_ARGS(thisfn, n) TYPE_FIELDS ((thisfn)[n].type)
#define TYPE_FN_FIELD_CONST(thisfn, n) ((thisfn)[n].is_const)
#define TYPE_FN_FIELD_VOLATILE(thisfn, n) ((thisfn)[n].is_volatile)
#define TYPE_FN_FIELD_PRIVATE(thisfn, n) ((thisfn)[n].is_private)
#define TYPE_FN_FIELD_PROTECTED(thisfn, n) ((thisfn)[n].is_protected)
#define TYPE_FN_FIELD_PUBLIC(thisfn, n) ((thisfn)[n].is_public)
#define TYPE_FN_FIELD_STATIC(thisfn, n) ((thisfn)[n].is_static)
#define TYPE_FN_FIELD_FINAL(thisfn, n) ((thisfn)[n].is_final)
#define TYPE_FN_FIELD_SYNCHRONIZED(thisfn, n) ((thisfn)[n].is_synchronized)
#define TYPE_FN_FIELD_NATIVE(thisfn, n) ((thisfn)[n].is_native)
#define TYPE_FN_FIELD_ARTIFICIAL(thisfn, n) ((thisfn)[n].is_artificial)
#define TYPE_FN_FIELD_ABSTRACT(thisfn, n) ((thisfn)[n].is_abstract)
#define TYPE_FN_FIELD_STUB(thisfn, n) ((thisfn)[n].is_stub)
#define TYPE_FN_FIELD_INLINED(thisfn, n) ((thisfn)[n].is_inlined)
#define TYPE_FN_FIELD_FCONTEXT(thisfn, n) ((thisfn)[n].fcontext)
#define TYPE_FN_FIELD_VOFFSET(thisfn, n) ((thisfn)[n].voffset-2)
#define TYPE_FN_FIELD_VIRTUAL_P(thisfn, n) ((thisfn)[n].voffset > 1)
#define TYPE_FN_FIELD_STATIC_P(thisfn, n) ((thisfn)[n].voffset == VOFFSET_STATIC)

#define TYPE_LOCALTYPE_PTR(thistype) (TYPE_CPLUS_SPECIFIC(thistype)->localtype_ptr)
#define TYPE_LOCALTYPE_FILE(thistype) (TYPE_CPLUS_SPECIFIC(thistype)->localtype_ptr->file)
#define TYPE_LOCALTYPE_LINE(thistype) (TYPE_CPLUS_SPECIFIC(thistype)->localtype_ptr->line)

#define TYPE_IS_OPAQUE(thistype) (((TYPE_CODE (thistype) == TYPE_CODE_STRUCT) ||        \
                                   (TYPE_CODE (thistype) == TYPE_CODE_UNION))        && \
                                  (TYPE_NFIELDS (thistype) == 0)                     && \
                                  (TYPE_CPLUS_SPECIFIC (thistype) && (TYPE_NFN_FIELDS (thistype) == 0)) && \
                                  (TYPE_STUB (thistype) || !TYPE_STUB_SUPPORTED (thistype)))

struct builtin_type
{
  /* Address/pointer types.  */

  /* `pointer to data' type.  Some target platforms use an implicitly
     {sign,zero} -extended 32-bit ABI pointer on a 64-bit ISA.  */
  struct type *builtin_data_ptr;

  /* `pointer to function (returning void)' type.  Harvard
     architectures mean that ABI function and code pointers are not
     interconvertible.  Similarly, since ANSI, C standards have
     explicitly said that pointers to functions and pointers to data
     are not interconvertible --- that is, you can't cast a function
     pointer to void * and back, and expect to get the same value.
     However, all function pointer types are interconvertible, so void
     (*) () can server as a generic function pointer.  */
  struct type *builtin_func_ptr;

  /* The target CPU's address type.  This is the ISA address size.  */
  struct type *builtin_core_addr;


  /* Types used for symbols with no debug information.  */
  struct type *nodebug_text_symbol;
  struct type *nodebug_data_symbol;
  struct type *nodebug_unknown_symbol;
  struct type *nodebug_tls_symbol;


  /* Integral types.  */

  /* We use these for the '/c' print format, because c_char is just a
     one-byte integral type, which languages less laid back than C
     will print as ... well, a one-byte integral type.  */
  struct type *builtin_true_char;
  struct type *builtin_true_unsigned_char;

  /* Implicit size/sign (based on the the architecture's ABI).  */
  struct type *builtin_void;
  struct type *builtin_char;
  struct type *builtin_short;
  struct type *builtin_int;
  struct type *builtin_long;
  struct type *builtin_signed_char;
  struct type *builtin_unsigned_char;
  struct type *builtin_unsigned_short;
  struct type *builtin_unsigned_int;
  struct type *builtin_unsigned_long;
  struct type *builtin_float;
  struct type *builtin_double;
  struct type *builtin_long_double;
  struct type *builtin_complex;
  struct type *builtin_double_complex;
  struct type *builtin_string;
  struct type *builtin_bool;
  struct type *builtin_long_long;
  struct type *builtin_unsigned_long_long;
  struct type *builtin_decfloat;
  struct type *builtin_decdouble;
  struct type *builtin_declong;
};

/* Return the type table for the specified architecture.  */
extern const struct builtin_type *builtin_type (struct gdbarch *gdbarch);

/* Compatibility macros to access types for the current architecture.  */
#define builtin_type_void_data_ptr \
	(builtin_type (current_gdbarch)->builtin_data_ptr)
#define builtin_type_void_func_ptr \
	(builtin_type (current_gdbarch)->builtin_func_ptr)
#define builtin_type_CORE_ADDR \
	(builtin_type (current_gdbarch)->builtin_core_addr)
#define builtin_type_true_char \
	(builtin_type (current_gdbarch)->builtin_true_char)
#define builtin_type_void \
	(builtin_type (current_gdbarch)->builtin_void)
#define builtin_type_char \
	(builtin_type (current_gdbarch)->builtin_char)
#define builtin_type_short \
	(builtin_type (current_gdbarch)->builtin_short)
#define builtin_type_int \
	(builtin_type (current_gdbarch)->builtin_int)
#define builtin_type_long \
	(builtin_type (current_gdbarch)->builtin_long)
#define builtin_type_signed_char \
	(builtin_type (current_gdbarch)->builtin_signed_char)
#define builtin_type_unsigned_char \
	(builtin_type (current_gdbarch)->builtin_unsigned_char)
#define builtin_type_unsigned_short \
	(builtin_type (current_gdbarch)->builtin_unsigned_short)
#define builtin_type_unsigned_int \
	(builtin_type (current_gdbarch)->builtin_unsigned_int)
#define builtin_type_unsigned_long \
	(builtin_type (current_gdbarch)->builtin_unsigned_long)
#define builtin_type_float \
	(builtin_type (current_gdbarch)->builtin_float)
#define builtin_type_double \
	(builtin_type (current_gdbarch)->builtin_double)
#define builtin_type_long_double \
	(builtin_type (current_gdbarch)->builtin_long_double)
#define builtin_type_complex \
	(builtin_type (current_gdbarch)->builtin_complex)
#define builtin_type_double_complex \
	(builtin_type (current_gdbarch)->builtin_double_complex)
#define builtin_type_string \
	(builtin_type (current_gdbarch)->builtin_string)
#define builtin_type_bool \
	(builtin_type (current_gdbarch)->builtin_bool)
#define builtin_type_long_long \
	(builtin_type (current_gdbarch)->builtin_long_long)
#define builtin_type_unsigned_long_long \
	(builtin_type (current_gdbarch)->builtin_unsigned_long_long)

 
/* Explicit sizes - see C9X <intypes.h> for naming scheme.  The "int0"
   is for when an architecture needs to describe a register that has
   no size.  */
extern struct type *builtin_type_int0;
extern struct type *builtin_type_int8;
extern struct type *builtin_type_uint8;
extern struct type *builtin_type_int16;
extern struct type *builtin_type_uint16;
extern struct type *builtin_type_int32;
extern struct type *builtin_type_uint32;
extern struct type *builtin_type_int64;
extern struct type *builtin_type_uint64;
extern struct type *builtin_type_int128;
extern struct type *builtin_type_uint128;

/* Explicit floating-point formats.  See "floatformat.h".  */
extern const struct floatformat *floatformats_ieee_single[BFD_ENDIAN_UNKNOWN];
extern const struct floatformat *floatformats_ieee_double[BFD_ENDIAN_UNKNOWN];
extern const struct floatformat *floatformats_ieee_double_littlebyte_bigword[BFD_ENDIAN_UNKNOWN];
extern const struct floatformat *floatformats_i387_ext[BFD_ENDIAN_UNKNOWN];
extern const struct floatformat *floatformats_m68881_ext[BFD_ENDIAN_UNKNOWN];
extern const struct floatformat *floatformats_arm_ext[BFD_ENDIAN_UNKNOWN];
extern const struct floatformat *floatformats_ia64_spill[BFD_ENDIAN_UNKNOWN];
extern const struct floatformat *floatformats_ia64_quad[BFD_ENDIAN_UNKNOWN];
extern const struct floatformat *floatformats_vax_f[BFD_ENDIAN_UNKNOWN];
extern const struct floatformat *floatformats_vax_d[BFD_ENDIAN_UNKNOWN];
extern const struct floatformat *floatformats_ibm_long_double[BFD_ENDIAN_UNKNOWN];

extern struct type *builtin_type_ieee_single;
extern struct type *builtin_type_ieee_double;
extern struct type *builtin_type_i387_ext;
extern struct type *builtin_type_m68881_ext;
extern struct type *builtin_type_arm_ext;
extern struct type *builtin_type_ia64_spill;
extern struct type *builtin_type_ia64_quad;

/* This type represents a type that was unrecognized in symbol
   read-in.  */

extern struct type *builtin_type_error;


/* Modula-2 types */

struct builtin_m2_type
{
  struct type *builtin_char;
  struct type *builtin_int;
  struct type *builtin_card;
  struct type *builtin_real;
  struct type *builtin_bool;
};

/* Return the Modula-2 type table for the specified architecture.  */
extern const struct builtin_m2_type *builtin_m2_type (struct gdbarch *gdbarch);

/* Compatibility macros to access types for the current architecture.  */
#define builtin_type_m2_char \
	(builtin_m2_type (current_gdbarch)->builtin_char)
#define builtin_type_m2_int \
	(builtin_m2_type (current_gdbarch)->builtin_int)
#define builtin_type_m2_card \
	(builtin_m2_type (current_gdbarch)->builtin_card)
#define builtin_type_m2_real \
	(builtin_m2_type (current_gdbarch)->builtin_real)
#define builtin_type_m2_bool \
	(builtin_m2_type (current_gdbarch)->builtin_bool)


/* Fortran (F77) types */

struct builtin_f_type
{
  struct type *builtin_character;
  struct type *builtin_integer;
  struct type *builtin_integer_s2;
  struct type *builtin_logical;
  struct type *builtin_logical_s1;
  struct type *builtin_logical_s2;
  struct type *builtin_real;
  struct type *builtin_real_s8;
  struct type *builtin_real_s16;
  struct type *builtin_complex_s8;
  struct type *builtin_complex_s16;
  struct type *builtin_complex_s32;
  struct type *builtin_void;
};

/* Return the Fortran type table for the specified architecture.  */
extern const struct builtin_f_type *builtin_f_type (struct gdbarch *gdbarch);

/* Compatibility macros to access types for the current architecture.  */
#define builtin_type_f_character \
	(builtin_f_type (current_gdbarch)->builtin_character)
#define builtin_type_f_integer \
	(builtin_f_type (current_gdbarch)->builtin_integer)
#define builtin_type_f_integer_s2 \
	(builtin_f_type (current_gdbarch)->builtin_integer_s2)
#define builtin_type_f_logical \
	(builtin_f_type (current_gdbarch)->builtin_logical)
#define builtin_type_f_logical_s1 \
	(builtin_f_type (current_gdbarch)->builtin_logical_s1)
#define builtin_type_f_logical_s2 \
	(builtin_f_type (current_gdbarch)->builtin_logical_s2)
#define builtin_type_f_real \
	(builtin_f_type (current_gdbarch)->builtin_real)
#define builtin_type_f_real_s8 \
	(builtin_f_type (current_gdbarch)->builtin_real_s8)
#define builtin_type_f_real_s16 \
	(builtin_f_type (current_gdbarch)->builtin_real_s16)
#define builtin_type_f_complex_s8 \
	(builtin_f_type (current_gdbarch)->builtin_complex_s8)
#define builtin_type_f_complex_s16 \
	(builtin_f_type (current_gdbarch)->builtin_complex_s16)
#define builtin_type_f_complex_s32 \
	(builtin_f_type (current_gdbarch)->builtin_complex_s32)
#define builtin_type_f_void \
	(builtin_f_type (current_gdbarch)->builtin_void)


/* RTTI for C++ */
/* extern struct type *builtin_type_cxx_typeinfo; */

/* Maximum and minimum values of built-in types */

#define	MAX_OF_TYPE(t)	\
   (TYPE_UNSIGNED(t) ? UMAX_OF_SIZE(TYPE_LENGTH(t)) \
    : MAX_OF_SIZE(TYPE_LENGTH(t)))

#define MIN_OF_TYPE(t)	\
   (TYPE_UNSIGNED(t) ? UMIN_OF_SIZE(TYPE_LENGTH(t)) \
    : MIN_OF_SIZE(TYPE_LENGTH(t)))

/* Allocate space for storing data associated with a particular type.
   We ensure that the space is allocated using the same mechanism that
   was used to allocate the space for the type structure itself.  I.E.
   if the type is on an objfile's objfile_obstack, then the space for data
   associated with that type will also be allocated on the objfile_obstack.
   If the type is not associated with any particular objfile (such as
   builtin types), then the data space will be allocated with xmalloc,
   the same as for the type structure. */

#define TYPE_ALLOC(t,size)  \
   (TYPE_OBJFILE (t) != NULL  \
    ? obstack_alloc (&TYPE_OBJFILE (t) -> objfile_obstack, size) \
    : xmalloc (size))

#define TYPE_ZALLOC(t,size)  \
   (TYPE_OBJFILE (t) != NULL  \
    ? memset (obstack_alloc (&TYPE_OBJFILE (t)->objfile_obstack, size),  \
	      0, size)  \
    : xzalloc (size))

extern struct type *alloc_type (struct objfile *);

extern struct type *init_type (enum type_code, int, int, char *,
			       struct objfile *);

/* Helper functions to construct a struct or record type.  An
   initially empty type is created using init_composite_type().
   Fields are then added using append_struct_type_field().  A union
   type has its size set to the largest field.  A struct type has each
   field packed against the previous.  */

extern struct type *init_composite_type (char *name, enum type_code code);
extern void append_composite_type_field (struct type *t, char *name,
					 struct type *field);

/* Helper functions to construct a bit flags type.  An initially empty
   type is created using init_flag_type().  Flags are then added using
   append_flag_type_flag().  */
extern struct type *init_flags_type (char *name, int length);
extern void append_flags_type_flag (struct type *type, int bitpos, char *name);

extern void make_vector_type (struct type *array_type);
extern struct type *init_vector_type (struct type *elt_type, int n);

extern struct type *lookup_reference_type (struct type *);

extern struct type *make_reference_type (struct type *, struct type **);

extern struct type *make_cv_type (int, int, struct type *, struct type **);

extern void replace_type (struct type *, struct type *);

extern int address_space_name_to_int (char *);

extern const char *address_space_int_to_name (int);

extern struct type *make_type_with_address_space (struct type *type, 
						  int space_identifier);

extern struct type *lookup_memberptr_type (struct type *, struct type *);

extern struct type *lookup_methodptr_type (struct type *);

extern void smash_to_method_type (struct type *type, struct type *domain,
				  struct type *to_type, struct field *args,
				  int nargs, int varargs);

extern void smash_to_memberptr_type (struct type *, struct type *,
				     struct type *);

extern struct type *allocate_stub_method (struct type *);

extern char *type_name_no_tag (const struct type *);

extern struct type *lookup_struct_elt_type (struct type *, char *, int);

extern struct type *make_pointer_type (struct type *, struct type **);

extern struct type *lookup_pointer_type (struct type *);

extern struct type *make_function_type (struct type *, struct type **);

extern struct type *lookup_function_type (struct type *);

extern struct type *create_range_type (struct type *, struct type *, int,
				       int);

extern struct type *create_array_type (struct type *, struct type *,
				       struct type *);

extern struct type *create_string_type (struct type *, struct type *);

extern struct type *create_set_type (struct type *, struct type *);

extern struct type *lookup_unsigned_typename (char *);

extern struct type *lookup_signed_typename (char *);

extern struct type *check_typedef (struct type *);

#define CHECK_TYPEDEF(TYPE) (TYPE) = check_typedef (TYPE)

extern void check_stub_method_group (struct type *, int);

extern char *gdb_mangle_name (struct type *, int, int);

extern struct type *lookup_typename (char *, struct block *, int);

extern struct type *lookup_template_type (char *, struct type *,
					  struct block *);

extern int get_vptr_fieldno (struct type *, struct type **);

extern int get_destructor_fn_field (struct type *, int *, int *);

extern int get_discrete_bounds (struct type *, LONGEST *, LONGEST *);

extern int is_ancestor (struct type *, struct type *);

/* Overload resolution */

#define LENGTH_MATCH(bv) ((bv)->rank[0])

/* Badness if parameter list length doesn't match arg list length */
#define LENGTH_MISMATCH_BADNESS      100
/* Dummy badness value for nonexistent parameter positions */
#define TOO_FEW_PARAMS_BADNESS       100
/* Badness if no conversion among types */
#define INCOMPATIBLE_TYPE_BADNESS    100

/* Badness of integral promotion */
#define INTEGER_PROMOTION_BADNESS      1
/* Badness of floating promotion */
#define FLOAT_PROMOTION_BADNESS        1
/* Badness of integral conversion */
#define INTEGER_CONVERSION_BADNESS     2
/* Badness of floating conversion */
#define FLOAT_CONVERSION_BADNESS       2
/* Badness of integer<->floating conversions */
#define INT_FLOAT_CONVERSION_BADNESS   2
/* Badness of converting to a boolean */
#define BOOLEAN_CONVERSION_BADNESS     2
/* Badness of pointer conversion */
#define POINTER_CONVERSION_BADNESS     2
/* Badness of conversion of pointer to void pointer */
#define VOID_PTR_CONVERSION_BADNESS    2
/* Badness of converting derived to base class */
#define BASE_CONVERSION_BADNESS        2
/* Badness of converting from non-reference to reference */
#define REFERENCE_CONVERSION_BADNESS   2

/* Non-standard conversions allowed by the debugger */
/* Converting a pointer to an int is usually OK */
#define NS_POINTER_CONVERSION_BADNESS 10


extern int compare_badness (struct badness_vector *, struct badness_vector *);

extern struct badness_vector *rank_function (struct type **, int,
					     struct type **, int);

extern int rank_one_type (struct type *, struct type *);

extern void recursive_dump_type (struct type *, int);

/* printcmd.c */

extern void print_scalar_formatted (const void *, struct type *, int, int,
				    struct ui_file *);

extern int can_dereference (struct type *);

extern int is_integral_type (struct type *);

extern void maintenance_print_type (char *, int);

extern htab_t create_copied_types_hash (struct objfile *objfile);

extern struct type *copy_type_recursive (struct objfile *objfile,
					 struct type *type,
					 htab_t copied_types);

#endif /* GDBTYPES_H */
