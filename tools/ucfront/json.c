
/* Primitive JSON parsing helpers */

#include <string.h>
#include "json.h"

/* Tests if a character is a JSON token delimiter */
#define ISDELIM(c) ((c) < 0x80 && strchr("[]{},:", c))

/* Returns value of a hexadecimal digit */
static int
hexval(int ch)
{
	return (ch & 15) + ((ch >> 6) & 1) * 9;
}

/*
 * Skips whitespace and the next JSON token.
 *
 * Returns one of these tokens:
 *    '[', ']', '{', '}', ':', ', ', '\"', EOF or JSON_WORD.
 *
 * - When JSON_WORD is returned, an identifier is stored as UTF-8 in buf[].
 *   This is the case when the token is "true", "false" or "null".
 * - When '\"' is returned, a JSON string is stored as UTF-8 in buf[].
 *
 * In both cases, buf[] is nul-terminated, and may be truncated
 * if it would otheriwse overrun.
 */
int
json_read_token(FILE *f, char *buf, size_t bufsz)
{
	/* Macros for appending to buf[bufsz] without overrun */
#define ADDC(c) do { if (bufsz > 1) { *buf++=(c); bufsz--; } } while (0)
#define ADD0()  do { if (bufsz) *buf = 0; } while (0)

	int c = getc(f);
	while (c != EOF && c < ' ')
		c = getc(f);
	if (c == EOF)
		return EOF;
	if (ISDELIM(c))
		return c;
	if (c != '"') {
		/* An unquoted JSON_WORD is going to be something
		 * true, false or null. */
		do {
			ADDC(c);
			c = getc(f);
		} while (c != EOF && c > ' ' && !ISDELIM(c));
		if (ISDELIM(c))
			ungetc(c, f);
		ADD0();
		return JSON_WORD;
	}

	int esc = 0;
	int surrogate = 0;
	while ((c = getc(f)) != EOF) {
		if (esc) {
			/* Decode JSON string escapes into UTF-8 */
			esc = 0;
			switch (c) {
			case 'b': c = '\b'; break;
			case 'r': c = '\r'; break;
			case 't': c = '\t'; break;
			case 'n': c = '\n'; break;
			case 'f': c = '\f'; break;
			case 'u': c = hexval(getc(f)) << 12;
				  c|= hexval(getc(f)) <<  8;
				  c|= hexval(getc(f)) <<  4;
				  c|= hexval(getc(f)) <<  0;
				  if ((c & ~0x3ff) == 0xd800) {
					surrogate = c & 0x3ff;
					continue;
				  } else if ((c & ~0x3ff) == 0xdc00) {
					c &= 0x3ff;
					c |= surrogate << 18;
					c |= 0x100000;
					surrogate = 0;
				  }
				  if (!(c & ~0x7f))
					ADDC(c);
				  else if (!(c & ~0x7ff)) {
					ADDC(0xc0 | (c >> 6 & 0x1f));
					ADDC(0x80 | (c >> 0 & 0x1f));
				  } else if (!(c & ~0xfff)) {
					ADDC(0xe0 | (c >>12 & 0x0f));
					ADDC(0x80 | (c >> 6 & 0x1f));
					ADDC(0x80 | (c >> 0 & 0x1f));
				  } else {
					ADDC(0xf0 | (c >>18 & 0x07));
					ADDC(0x80 | (c >>12 & 0x1f));
					ADDC(0x80 | (c >> 6 & 0x1f));
					ADDC(0x80 | (c >> 0 & 0x1f));
				  }
				  continue;
			}
		} else if (c == '\\') {
			esc = 1;
			continue;
		} else if (c == '"')
			break;
		ADDC(c);
	}
	ADD0();
	return '"';
#undef ADD0
#undef ADDC
}

/*
 * Encodes the UTF-8 string s as a quoted JSON string into buf[].
 * This is used for emitting JSON.
 */
char *
json_str(const char *s, char *buf, size_t bufsz)
{
	char *ret = buf;
#define ADDC(c) do { if (bufsz > 1) { *buf++ = (c); bufsz--; } } while (0)
	ADDC('"');
	for (; *s; s++) {
		switch (*s) {
		case '"': /* fallthough */
		case '\\': ADDC('\\'); ADDC(*s); break;
		case '\r': ADDC('\\'); ADDC('r'); break;
		case '\n': ADDC('\\'); ADDC('n'); break;
		case '\t': ADDC('\\'); ADDC('t'); break;
		case '\b': ADDC('\\'); ADDC('b'); break;
		case '\f': ADDC('\\'); ADDC('f'); break;
		default:
			if (*s < 0x20) {
				static const char hex[] = "0123456789abcdef";
				ADDC('\\');
				ADDC('u');
				ADDC('0');
				ADDC('0');
				ADDC(hex[((*s) >> 4) & 0xf]);
				ADDC(hex[((*s) >> 0) & 0xf]);
			} else
				ADDC(*s);
		}
	}
	ADDC('"');
	if (bufsz)
		*buf = '\0';
	return ret;
#undef ADDC
}

/*
 * Reads from a JSON file, skipping over the next JSON value.
 * This will skip the next string, number, identifer, balanced array or object.
 * It doesn't do a "proper" JSON parse. It only looks for balanced [] and {}
 * and doesn't care about placement of commas and colons.
 * Returns 1 on success.
 * Returns 0 on bad JSON (e.g. unexpected EOF, unbalanced ", [] or {}).
 */
int
json_skip_value(FILE *f)
{
	int tok;

	tok = json_read_token(f, NULL, 0);
	if (tok != '[' && tok != '{')
		return tok == JSON_WORD || tok == '"';

	int depth = 1;
	while (depth) {
		tok = json_read_token(f, NULL, 0);
		if (tok == '{' || tok == '[')
			depth++;
		else if (tok == ']' || tok == '}')
			depth--;
		else if (!(tok == '"' || tok == JSON_WORD ||
		           tok == ':' || tok == ','))
			return 0;
	}
	return 1;
}
