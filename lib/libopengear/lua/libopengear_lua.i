%module "libopengear_lua"
%{
#include <opengear/libinfod.h>
%}

%include <typemaps.i>

/* use the default ** typemaps for char ** */
%apply SWIGTYPE** OUTPUT { char ** ret };

/* but define an OUTPUT one to return a string not userdata */
%typemap(argout) char ** OUTPUT
%{ lua_pushstring(L, *$1); SWIG_arg++; %}

/* lua_push(l)tring makes a copy, so we need to free the
 * returned char * to avoid a leak */
%typemap(freearg) char ** OUTPUT
%{ if ($result != 0) free(*$1); %};
/* Because the above checks $result on an early error path,
 * we need to initialise it */
%typemap(arginit) char ** OUTPUT
%{ $result = 0; %}

/* Apply char ** OUTPUT to char ** ret */
%apply char ** OUTPUT { char ** ret };

%include "../include/opengear/libinfod.h"
