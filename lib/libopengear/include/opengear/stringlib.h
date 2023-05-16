#ifndef _OPENGEAR_STRINGLIB_H_
#define _OPENGEAR_STRINGLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

/* This function will replace any instances of old_str in str with rep_str
 * str is assumed to be a malloc'ed string, and will be resized with realloc 
 * if rep_str is larger than old_str. A pointer to str will be returned.
 */
char *strrepl(char *str, char *old_str, char *rep_str);

/* This function will append src to dest, separated with string delim.
 * The original dest string will be realloced (or alloced if null)to fit the new characters 
 * and returned
 */
char *strappend(char *dest, const char *src, const char *delim);

/* 
 * This function will convert a c string to upper case.
 * It makes a copy of the string first 
 */
char *strtoupper(const char *orig);

/* 
 * This function will convert a c string to lower case.
 * It makes a copy of the string first 
 */
char *strtolower(const char *orig);

/*
 * Trims whitespace from the start and end of the given string. Copies the string
 * to a new block of memory (at *out), capped at len. (Doesn't allocate the memory,
 * so make sure you allocate enough at *out). returns the length of the trimmed
 * string, including the null. I think.
 */
size_t trimwhitespace(char *out, size_t len, const char *str);

/*
 * Trims whitespace from the start and end of the given string. Trims in-place,
 * returning a pointer to the trimmed string (within the given str). (Doesn't
 * allocate any memory, so make sure you keep a copy of the original pointer to
 * free if needed!)
 */
char *trimwhitespace_inplace(char *str);

#ifdef __cplusplus
}
#endif

#endif /* _OPENGEAR_STRINGLIB_H_ */
