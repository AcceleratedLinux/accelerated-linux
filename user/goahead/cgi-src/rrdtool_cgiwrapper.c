#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RRDTOOL_PATH	"/bin/rrdtool"

#define DD printf("%d\n", __LINE__); fflush(stdout);

typedef struct cgi_s {
  char    *name, *value;
} s_cgi;

int cgiDebugLevel = 0;
int cgiDebugStderr = 0;

char *cgiDecodeString (char *text)
{
    char *cp, *xp;

    for (cp=text,xp=text; *cp; cp++) {
	if (*cp == '%') {
	    if (strchr("0123456789ABCDEFabcdef", *(cp+1))
		&& strchr("0123456789ABCDEFabcdef", *(cp+2))) {
		if (islower((unsigned int)*(cp+1)))
		    *(cp+1) = toupper((unsigned int)*(cp+1));
		if (islower((unsigned int)*(cp+2)))
		    *(cp+2) = toupper((unsigned int)*(cp+2));
		*(xp) = (*(cp+1) >= 'A' ? *(cp+1) - 'A' + 10 : *(cp+1) - '0' ) * 16
		    + (*(cp+2) >= 'A' ? *(cp+2) - 'A' + 10 : *(cp+2) - '0');
		xp++;cp+=2;
	    }
	} else {
	    *(xp++) = *cp;
	}
    }
    memset(xp, 0, cp-xp);
    return text;
}

/*  cgiInit()
 *
 *  Read from stdin if no string is provided via CGI.  Variables that
 *  doesn't have a value associated with it doesn't get stored.
 */
s_cgi **cgiInit ()
{
    int length;
    char *line = NULL;
    int numargs;
    char *cp, *ip, *esp, *sptr;
    s_cgi **result;
    int i, k;
    char tmp[101];

    cp = getenv("REQUEST_METHOD");
    ip = getenv("CONTENT_LENGTH");

    if (cp && !strcmp(cp, "POST")) {
	if (ip) {
	    length = atoi(ip);
	    if ((line = (char *)malloc (length+2)) == NULL)
		return NULL;
	    fgets(line, length+1, stdin);
	} else
	    return NULL;
    } else if (cp && !strcmp(cp, "GET")) {
	esp = getenv("QUERY_STRING");
	if (esp && strlen(esp)) {
	    if ((line = (char *)malloc (strlen(esp)+2)) == NULL)
		return NULL;
	    sprintf (line, "%s", esp);
	} else
	    return NULL;
    } else {
	length = 0;
	printf ("(offline mode: enter name=value pairs on standard input)\n");
	for (cp = fgets(tmp, 100, stdin); cp != NULL;
	     cp = fgets(tmp, 100, stdin) ) {
	    if (strlen(tmp)) {
		length += strlen(tmp);
		if ((ip = (char *)malloc ((length+1) * sizeof(char))) == NULL)
		    return NULL;
		memset(ip,0, length);
		if (line) {
		    if (line[strlen(line)-1] == '\n')
			line[strlen(line)-1] = '&';
		    strcpy(ip, line);
		}
		ip = strcat(ip, tmp);
		if (line)
		    free (line);
		line = ip;
	    }
	}
	if (!line)
	    return NULL;
	if (line[strlen(line)-1] == '\n')
	    line[strlen(line)-1] = '\0';
    }

    /*
     *  From now on all cgi variables are stored in the variable line
     *  and look like  foo=bar&foobar=barfoo&foofoo=
     */

    if (cgiDebugLevel > 0) {
	if (cgiDebugStderr) {
	    fprintf (stderr, "Received cgi input: %s\n", line);
	} else {
	    printf ("<b>Received cgi input</b><br>\n<pre>\n--\n%s\n--\n</pre>\n\n", line);
	}
    }
    for (cp=line; *cp; cp++) {
	if (*cp == '+') {
	    *cp = ' ';
	}
    }
    if (strlen(line)) {
	for (numargs=1,cp=line; *cp; cp++) {
	    if (*cp == '&') numargs++;
	}
    } else {
	numargs = 0;
    }
    if (cgiDebugLevel > 0) {
	if (cgiDebugStderr) {
	    fprintf (stderr, "%d cgi variables found.\n", numargs);
	} else {
	    printf ("%d cgi variables found.<br>\n", numargs);
	}
    }
    if ((result = (s_cgi **)malloc((numargs+1) * sizeof(s_cgi *))) == NULL) {
	return NULL;
    }

    memset (result, 0, (numargs+1) * sizeof(s_cgi *));

    cp = line;
    i=0;
    while (*cp) {
	if ((ip = (char *)strchr(cp, '&')) != NULL) {
	    *ip = '\0';
	}else {
	    ip = cp + strlen(cp);
	}

	if ((esp=(char *)strchr(cp, '=')) == NULL) {
	    cp = ++ip;
	    continue;
	}

	if (!strlen(esp)) {
	    cp = ++ip;
	    continue;
	}

	if (i<numargs) {

	    for (k=0; k<i && (strncmp(result[k]->name,cp, esp-cp)); k++);
	    /* try to find out if there's already such a variable */
	    if (k == i) {	/* No such variable yet */
		if ((result[i] = (s_cgi *)malloc(sizeof(s_cgi))) == NULL)
		    return NULL;
		if ((result[i]->name = (char *)malloc((esp-cp+1) * sizeof(char))) == NULL)
		    return NULL;
		memset (result[i]->name, 0, esp-cp+1);
		strncpy(result[i]->name, cp, esp-cp);
		cp = ++esp;
		if ((result[i]->value = (char *)malloc((ip-esp+1) * sizeof(char))) == NULL)
		    return NULL;
		memset (result[i]->value, 0, ip-esp+1);
		strncpy(result[i]->value, cp, ip-esp);
		result[i]->value = cgiDecodeString(result[i]->value);
		if (cgiDebugLevel) {
		    if (cgiDebugStderr)
			fprintf (stderr, "%s: %s\n", result[i]->name, result[i]->value);
		    else
			printf ("<h3>Variable %s</h3>\n<pre>\n%s\n</pre>\n\n", result[i]->name, result[i]->value);
		}
		i++;
	    } else {	/* There is already such a name, suppose a mutiple field */
		if ((sptr = (char *)malloc((strlen(result[k]->value)+(ip-esp)+2)* sizeof(char))) == NULL)
		    return NULL;
		memset (sptr, 0, strlen(result[k]->value)+(ip-esp)+2);
		sprintf (sptr, "%s\n", result[k]->value);
		cp = ++esp;
		strncat(sptr, cp, ip-esp);
		free(result[k]->value);
		result[k]->value = sptr;
	    }
	}
	cp = ++ip;
    }
    return result;
}

char *cgiGetValue(s_cgi **parms, const char *var)
{
    int i;

    if (parms) {
	for (i=0;parms[i]; i++) {
	    if (!strcmp(var,parms[i]->name)) {
		if (cgiDebugLevel > 0) {
		    if (cgiDebugStderr) {
			fprintf (stderr, "%s found as %s\n", var, parms[i]->value);
		    } else {
			printf ("%s found as %s<br>\n", var, parms[i]->value);
		    }
		}
		return parms[i]->value;
	    }
	}
    }
    return NULL;
}

int isValidString(char *str)
{
    if(strchr(str, ';'))
        return 0;
    if(strchr(str, '\n'))
        return 0;
    if(strchr(str, '\r'))
        return 0;
    if(strchr(str, '`'))
        return 0;
    return 1;
}

int isFileExisted(char *file){
    FILE *fp = fopen(file, "r");
    if(fp){
      fclose(fp);
      return 1;
    }
    return 0;
}

int main(void)
{
    char cmd[1024];
    char *l, *lines;
    int x_p, offset;
    s_cgi **c;

    printf("Pragma: no-cache\nContent-type: text/plain\n\n");
    fflush(stdout);

    c = cgiInit();

    if(!c)
        return 0;

    if(!isFileExisted(RRDTOOL_PATH)){
        printf("no_rrdtool");
        return 0;
    }

    if(! (l = cgiGetValue(c, "w")) ){
        return 0;
    }
    x_p = atoi(l);

    if(! (l = cgiGetValue(c, "offset")) ){
        printf("no offset");
        return 0;
    }
    offset = atoi(l);

    if(! x_p  || !offset)
        return 0;

    if(! (l = cgiGetValue(c, "lines")) ){
        printf("no lines");
        return 0;
    }

    if(!  (lines = strdup(l))  )
        return 0;
    cgiDecodeString(lines);

    if(! isValidString(lines)){
        goto end;
    }

    snprintf(cmd, 1024, "%s graph /var/anything %s -w %d -s %d", RRDTOOL_PATH, lines, x_p, time(NULL)+offset );
    system(cmd);
    system("ls / > /var/wow");

    fflush(stdout);

end:
    free(lines);
    return 0;
}

