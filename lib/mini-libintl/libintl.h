#ifndef _LIBINTL_H 
#define _LIBINTL_H 1 

#include <features.h> 

#if defined(__UCLIBC__) && !defined(__UCLIBC_HAS_GETTEXT_AWARENESS__) 
/* Stubs for gettext/locale bullshit... */ 
#undef gettext 
#undef dgettext 
#undef dcgettext 
#undef ngettext 
#undef dngettext 
#undef dcngettext 
#undef textdomain 
#undef bindtextdomain 
#undef bind_textdomain_codeset 
/* this fucker seems to be installed in /usr/include/locale.h */ 
/* #undef setlocale */ 
//#undef _ 
//#undef N_ 

#define gettext(String) (String) 
#define dgettext(Domain, String) (String) 
#define dcgettext(Domain, String, Type) (String) 
#define ngettext(Singular, Plural, Count) ((Count) == 1 ? (const char *) (Singular) : (const char *) (Plural)) 
#define dngettext(Domain, Singular, Plural, Count) ((Count) == 1 ?  (const char *) (Singular) : (const char *) (Plural)) 
#define dcngettext(Domain, Singular, Plural, Count, Category) ((Count) == 1 ? (const char *) (Singular) : (const char *) (Plural)) 
#define dgettext(Domain, String) (String) 
#define dcgettext(Domain, String, Type) (String) 

#ifndef _LOCALE_H 
/* #define setlocale(Category, Locale) ((char *)NULL) */ 
#endif 

#define bindtextdomain(Domain, Directory) (Domain) 
#define bind_textdomain_codeset(Domain, Codeset) 
#define textdomain(String) (String)

//#define _(String) (String) 
//#define N_(String) (String) 

#endif /* GETTEXT */ 
#endif /* _LIBINTL_H */
