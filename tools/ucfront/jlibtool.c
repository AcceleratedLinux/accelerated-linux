/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2002 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>

#ifdef __EMX__
#  define SHELL_CMD  "sh"
#  define GEN_EXPORTS "emxexp"
#  define DEF2IMPLIB_CMD "emximp"
#  define SHARE_SW   "-Zdll -Zmtd"
#  define USE_OMF 1
#  define TRUNCATE_DLL_NAME
#  define DYNAMIC_LIB_EXT "dll"
#  define EXE_EXT ".exe"

#  if USE_OMF
     /* OMF is the native format under OS/2 */
#    define STATIC_LIB_EXT "lib"
#    define OBJECT_EXT     "obj"
#    define LIBRARIAN      "emxomfar"
#    define LIBRARIAN_OPTS "cr"
#  else
     /* but the alternative, a.out, can fork() which is sometimes necessary */
#    define STATIC_LIB_EXT "a"
#    define OBJECT_EXT     "o"
#    define LIBRARIAN      "ar"
#    define LIBRARIAN_OPTS "cr"
#  endif
#endif

#if defined(__APPLE__)
#  define SHELL_CMD  "/bin/sh"
#  define DYNAMIC_LIB_EXT "dylib"
#  define MODULE_LIB_EXT  "so"
#  define STATIC_LIB_EXT "a"
#  define OBJECT_EXT     "o"
#  define LIBRARIAN      "ar"
#  define LIBRARIAN_OPTS "cr"
/* man libtool(1) documents ranlib option of -c.  */
#  define RANLIB "ranlib"
#  define PIC_FLAG "-fPIC -fno-common"
#  define RPATH "-rpath"
#  define SHARED_OPTS "-dynamiclib"
#  define MODULE_OPTS "-bundle"
#  define DYNAMIC_LINK_OPTS "-flat_namespace -undefined suppress"
#  define dynamic_link_version_func darwin_dynamic_link_function
#  define DYNAMIC_INSTALL_NAME "-install_name"
//-install_name  /Users/jerenk/apache-2.0-cvs/lib/libapr.0.dylib -compatibility_version 1 -current_version 1.0
#endif

#if defined(__linux__)
#  define SHELL_CMD  "/bin/sh"
#  define DYNAMIC_LIB_EXT "so"
#  define MODULE_LIB_EXT  "so"
#  define STATIC_LIB_EXT "a"
#  define OBJECT_EXT     "o"
#  define LIBRARIAN      "ar"
#  define LIBRARIAN_OPTS "cr"
#  define RANLIB "ranlib"
#  define PIC_FLAG "-fPIC"
#  define RPATH "-rpath"
#  define DYNAMIC_LINK_OPTS "-shared"
#  define NO_UNDEFINED "-no-undefined"
#  define LINKER_FLAG_PREFIX "-Wl,"
#endif

#ifdef __EMX__
#include <process.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

/* We want to say we are libtool 1.4 for shlibtool compatibility. */
#define VERSION "1.4"

enum tool_mode_t {
    mUnknown,
    mCompile,
    mLink,
    mInstall,
};

enum output_t {
    otGeneral,
    otObject,
    otProgram,
    otLibrary,
    otStaticLibraryOnly,
    otDynamicLibraryOnly,
    otModule,
};

enum pic_mode_e {
    UNKNOWN,
    PREFER,
    AVOID,
};

typedef struct {
    const char **vals;
    int num; 
} count_chars;

typedef struct {
    const char *normal;
    const char *install;
} library_name;

typedef struct {
    count_chars *normal;
    count_chars *install;
    count_chars *dependencies;
} library_opts;

typedef struct {
    int silent;
    int debug;
    int shared;
    int export_all;
    int dry_run;
    enum pic_mode_e pic_mode;
    int export_dynamic;
    int no_undefined;
} options_t;

typedef struct {
    enum tool_mode_t mode;
    enum output_t output;
    options_t options;

    char *output_name;
    char *fake_output_name;
    char *basename;

    const char *install_path;
    const char *compiler;
    const char *program;
    count_chars *program_opts;

    count_chars *arglist;
    count_chars *tmp_dirs;
    count_chars *obj_files;
    count_chars *dep_rpaths;
    count_chars *rpaths;

    library_name static_name;
    library_name shared_name;
    library_name module_name;

    library_opts static_opts;
    library_opts shared_opts;

    const char *version_info;
} command_t;

void init_count_chars(count_chars *cc)
{
    cc->vals = (const char**)malloc(PATH_MAX);
    cc->num = 0;
}

void clear_count_chars(count_chars *cc)
{
    int i;
    for (i = 0; i < cc->num; i++) {
        cc->vals[i] = 0;
    }

    cc->num = 0;
}

void push_count_chars(count_chars *cc, const char *newval)
{
    cc->vals[cc->num++] = newval;
}

void insert_count_chars(count_chars *cc, const char *newval, int position)
{
    int i;

    for (i = cc->num; i > position; i--) {
        cc->vals[i] = cc->vals[i-1];
    }

    cc->vals[position] = newval;
    cc->num++;
}

void append_count_chars(count_chars *cc, count_chars *cctoadd)
{
    int i;
    for (i = 0; i < cctoadd->num; i++) {
        if (cctoadd->vals[i]) {
            push_count_chars(cc, cctoadd->vals[i]);
        }
    }
}

char *shell_esc(const char *str)
{
    char *cmd, *d;
    const char *s;
	int n, len;

	if (strpbrk(str, "*?["))
		return strdup(str);

	/* count the 's we need to escape */
    for (len = n = 0, s = str; *s; s++) {
        if (*s == '\'')
			n++;
		len++;
	}

    cmd = (char *) malloc(len + n * 3 + 2 /* ' at ends */ + 1 /*nul*/);
	if (!cmd)
		return NULL;

    d = cmd;

	*d++ = '\'';
    for (s = str; *s; s++) {
        if (*s == '\'') {
            *d++ = '\'';
            *d++ = '\\';
            *d++ = '\'';
        }
        *d++ = *s;
	}
	*d++ = '\'';
    *d = '\0';

    return cmd;
}

const char *flatten_count_chars(count_chars *cc)
{
    int i, size;
    char *newval;
	char *esc;

    size = 0;
    for (i = 0; i < cc->num; i++) {
        if (cc->vals[i]) {
			esc = shell_esc(cc->vals[i]);
            size += strlen(esc) + 1;
			free(esc);
        }
    }

    newval = (char*)malloc(size + 1);
    newval[0] = 0;
    newval[size] = 0;

    for (i = 0; i < cc->num; i++) {
        if (cc->vals[i]) {
			esc = shell_esc(cc->vals[i]);
            strcat(newval, esc);
            strcat(newval, " ");
			free(esc);
        }
    }

    return newval;
}


int external_spawn(command_t *cmd, const char *file, const char **argv)
{
    if (!cmd->options.silent) {
        const char **argument = argv;
        printf("Executing: ");
        while (*argument) {
            printf("%s ", *argument);
            argument++;
        }
        puts("");
    }

    if (cmd->options.dry_run) {
        return 0;
    }
#ifdef __EMX__
    return spawnvp(P_WAIT, file, argv);
#else
    {
        pid_t pid;
        pid = fork();
        if (pid == 0) {
            return execvp(argv[0], (char**)argv);
        }
        else {
            int statuscode;
            waitpid(pid, &statuscode, 0);
            if (WIFEXITED(statuscode)) {
                return WEXITSTATUS(statuscode);
            }
            return 0;
        }
    }
#endif
}

int run_command(command_t *cmd_data, count_chars *cc)
{
    const char *command;
    const char *spawn_args[4];
    count_chars tmpcc;

    init_count_chars(&tmpcc);

    if (cmd_data->program) {
        push_count_chars(&tmpcc, cmd_data->program);
    }

    append_count_chars(&tmpcc, cmd_data->program_opts);

    append_count_chars(&tmpcc, cc);

    command = flatten_count_chars(&tmpcc);

    spawn_args[0] = SHELL_CMD;
    spawn_args[1] = "-c";
    spawn_args[2] = command;
    spawn_args[3] = NULL;
    return external_spawn(cmd_data, spawn_args[0], (const char**)spawn_args);
}

int parse_long_opt(char *arg, command_t *cmd_data)
{
    char *equal_pos = strchr(arg, '=');
    char var[50];
    char value[500];

    if (equal_pos) {
        strncpy(var, arg, equal_pos - arg);
        var[equal_pos - arg] = 0;
        strcpy(value, equal_pos + 1);
    } else {
        strcpy(var, arg);
    }

    if (strcmp(var, "silent") == 0) {
        cmd_data->options.silent = 1;
    } else if (strcmp(var, "mode") == 0) {
        if (strcmp(value, "compile") == 0) {
            cmd_data->mode = mCompile;
            cmd_data->output = otObject;
        }

        if (strcmp(value, "link") == 0) {
            cmd_data->mode = mLink;
            cmd_data->output = otLibrary;
        }

        if (strcmp(value, "install") == 0) {
            cmd_data->mode = mInstall;
        }
    } else if (strcmp(var, "shared") == 0) {
        if (cmd_data->mode == mLink) {
            cmd_data->output = otDynamicLibraryOnly;
        }
        cmd_data->options.shared = 1;
    } else if (strcmp(var, "export-all") == 0) {
        cmd_data->options.export_all = 1;
    } else if (strcmp(var, "dry-run") == 0) {
        printf("Dry-run mode on!\n");
        cmd_data->options.dry_run = 1;
    } else if (strcmp(var, "version") == 0) {
        printf("Version " VERSION "\n");
    } else if (strcmp(var, "tag") == 0) {
        if (cmd_data->options.debug) {
            printf("Sorry.  Don't care about tags.\n");
        }
    } else if (strcmp(var, "help") == 0) {
        printf("Sorry.  No help available.\n");
    } else {
        return 0;
    }

    return 1;
}

/* Return 1 if we eat it. */
int parse_short_opt(char *arg, command_t *cmd_data)
{
    if (strcmp(arg, "export-dynamic") == 0) {
        cmd_data->options.export_dynamic = 1;
        return 1;
    }

    if (strcmp(arg, "module") == 0) {
        cmd_data->output = otModule;
        return 1;
    }

    if (strcmp(arg, "Zexe") == 0) {
        return 1;
    }

    if (strcmp(arg, "avoid-version") == 0) {
        return 1;
    }

    if (strcmp(arg, "prefer-pic") == 0) {
        cmd_data->options.pic_mode = PREFER;
        return 1;
    }

    if (strcmp(arg, "prefer-non-pic") == 0) {
        cmd_data->options.pic_mode = AVOID;
        return 1;
    }

    if (strcmp(arg, "static") == 0) {
        /* Don't respect it for now. */
        return 1;
    }

    if (strcmp(arg, "no-undefined") == 0) {
        cmd_data->options.no_undefined = 1;
        return 1;
    }

    if (strcmp(arg, "release") == 0) {
        if (cmd_data->options.debug) {
            printf("Sorry. Don't care about release numbering\n");
        }
		return 2;
	}

    if (strcmp(arg, "dlopen") == 0) {
        if (cmd_data->options.debug) {
            printf("Sorry. Don't care about dlopen options\n");
        }
		return 2;
	}

    if (cmd_data->mode == mLink) {
        if (arg[0] == 'L' || arg[0] == 'l') {
            /* Hack... */
            arg--;
            push_count_chars(cmd_data->shared_opts.dependencies, arg);
            return 1;
        }
    }
    return 0;
}

char *truncate_dll_name(char *path)
{
    /* Cut DLL name down to 8 characters after removing any mod_ prefix */
    char *tmppath = strdup(path);
    char *newname = strrchr(tmppath, '/') + 1;
    char *ext = strrchr(tmppath, '.');
    int len;

    if (ext == NULL)
        return tmppath;

    len = ext - newname;

    if (strncmp(newname, "mod_", 4) == 0) {
        strcpy(newname, newname + 4);
        len -= 4;
    }

    if (len > 8) {
        strcpy(newname + 8, strchr(newname, '.'));
    }

    return tmppath;
}

long safe_strtol(const char *nptr, const char **endptr, int base)
{
    long rv;

    errno = 0;

    rv = strtol(nptr, (char**)endptr, 10);

    if (errno == ERANGE) {
        return 0;
    }

    return rv; 
}

/* version_info is in the form of MAJOR:MINOR:PATCH */
const char *darwin_dynamic_link_function(const char *version_info)
{
    char *newarg;
    long major, minor, patch;

    major = 0;
    minor = 0;
    patch = 0;

    if (version_info) {
        major = safe_strtol(version_info, &version_info, 10);

        if (version_info) {
            if (version_info[0] == ':') {
                version_info++;
            }

            minor = safe_strtol(version_info, &version_info, 10);

            if (version_info) {
                if (version_info[0] == ':') {
                    version_info++;
                }

                patch = safe_strtol(version_info, &version_info, 10);

            }
        }
    }

    /* Avoid -dylib_compatibility_version must be greater than zero errors. */
    if (major == 0) {
        major = 1;
    }
    newarg = (char*)malloc(100);
    snprintf(newarg, 99,
             "-compatibility_version %ld -current_version %ld.%ld",
             major, major, minor);

    return newarg;
}

/* genlib values
 * 0 - static
 * 1 - dynamic
 * 2 - module
 */
char *gen_library_name(const char *name, int genlib)
{
    char *newarg, *newext;
	char *cp;

    newarg = (char *)malloc(strlen(name) + 10);
	*newarg = '\0';

	cp = strrchr(name, '/');
	if (cp) {
		cp++;
		strcpy(newarg, name);
		newarg[cp - name] = '\0';
		name = cp;
	} else
		strcpy(newarg, ".libs/");

    if (genlib == 2 && strncmp(name, "lib", 3) == 0) {
        name += 3;
    }

    strcat(newarg, name);

    newext = strrchr(newarg, '.') + 1;

    switch (genlib) {
    case 0:
        strcpy(newext, STATIC_LIB_EXT);
        break;
    case 1:
        strcpy(newext, DYNAMIC_LIB_EXT);
        break;
    case 2:
        strcpy(newext, MODULE_LIB_EXT);
        break;
    }

    return newarg;
}

/* genlib values
 * 0 - static
 * 1 - dynamic
 * 2 - module
 */
char *gen_install_name(const char *name, int genlib)
{
    struct stat sb;
    char *newname;
    int rv;

    newname = gen_library_name(name, genlib);

    /* Check if it exists. If not, return NULL.  */
    rv = stat(newname, &sb);

    if (rv) {
        return NULL;
    }

    return newname;
}

char *check_object_exists(command_t *cmd, const char *arg, int arglen)
{
    char *newarg, *ext;
    int pass, rv;

    newarg = (char *)malloc(arglen + 10);
    memcpy(newarg, arg, arglen);
    newarg[arglen] = 0;
    ext = newarg + arglen;

    pass = 0;

    do {
        struct stat sb;

        switch (pass) {
        case 0:
            strcpy(ext, OBJECT_EXT);
            break;
/*
        case 1:
            strcpy(ext, NO_PIC_EXT);
            break;
*/
        default:
            break;
        } 

        if (!cmd->options.silent) {
            printf("Checking: %s\n", newarg);
        }
        rv = stat(newarg, &sb);
		if (rv == -1 && strpbrk(newarg, "*?["))
			rv = 0; // assume globbing will work
    }
    while (rv != 0 && ++pass < 1);

    if (rv == 0) {
        if (pass == 1) {
            cmd->options.pic_mode = AVOID;
        }
        return newarg;
    }

    return NULL;
}

/* libdircheck values:
 * 0 - no .libs suffix
 * 1 - .libs suffix
 */
char *check_library_exists(command_t *cmd, const char *arg, int pathlen,
                           int libdircheck)
{
    char *newarg, *ext;
    int pass, rv, newpathlen;

    newarg = (char *)malloc(strlen(arg) + 10);
    strcpy(newarg, arg);
    newarg[pathlen] = 0;

    newpathlen = pathlen;
    if (libdircheck) {
        strcat(newarg, ".libs/");
        newpathlen += sizeof(".libs/") - 1;
    }

    strcpy(newarg+newpathlen, arg+pathlen);
    ext = strrchr(newarg, '.') + 1;

    pass = 0;

    do {
        struct stat sb;

        switch (pass) {
        case 0:
            if (cmd->options.pic_mode != AVOID || cmd->options.shared) {
                strcpy(ext, DYNAMIC_LIB_EXT);
                break;
            }
            pass = 1;
        case 1:
            strcpy(ext, STATIC_LIB_EXT);
            break;
        case 2:
            strcpy(ext, MODULE_LIB_EXT);
            break;
        case 3:
            strcpy(ext, OBJECT_EXT);
            break;
        default:
            break;
        } 

        if (!cmd->options.silent) {
            printf("Checking: %s\n", newarg);
        }
        rv = stat(newarg, &sb);
    }
    while (rv != 0 && ++pass < 4);

    if (rv == 0) {
        return newarg;
    }

    return NULL;
}

void add_linker_flag_prefix(count_chars *cc, const char *arg)
{
#ifndef LINKER_FLAG_PREFIX
    push_count_chars(cc, arg);
#else
    char *newarg;
    newarg = (char*)malloc(strlen(arg) + sizeof(LINKER_FLAG_PREFIX));
    strcpy(newarg, LINKER_FLAG_PREFIX);
    strcpy(newarg + sizeof(LINKER_FLAG_PREFIX) - 1, arg);
    push_count_chars(cc, newarg);
#endif
}

int parse_input_file_name(char *arg, command_t *cmd_data)
{
    char *ext = strrchr(arg, '.');
    char *name = strrchr(arg, '/');
    int pathlen;
    char *newarg;

    if (!ext) {
        return 0;
    }

    ext++;

    if (name == NULL) {
        name = strrchr(arg, '\\');

        if (name == NULL) {
            name = arg;
        } else {
            name++;
        }
    } else {
        name++;
    }

    pathlen = name - arg;

    if (strcmp(ext, "lo") == 0) {
        newarg = check_object_exists(cmd_data, arg, ext - arg);
        if (!newarg) {
            printf("Can not find suitable object file for %s\n", arg);
            exit(1);
        }
        if (cmd_data->mode != mLink) {
            push_count_chars(cmd_data->arglist, newarg);
        }
        else {
            push_count_chars(cmd_data->obj_files, newarg);
        }
        return 1;
    }

    if (strcmp(ext, "la") == 0) {
        switch (cmd_data->mode) {
        case mLink:
            /* Try the .libs dir first! */
            newarg = check_library_exists(cmd_data, arg, pathlen, 1);
            if (!newarg) {
                /* Try the normal dir next. */
                newarg = check_library_exists(cmd_data, arg, pathlen, 0);
                if (!newarg) {
                    printf("Can not find suitable library for %s\n", arg);
                    exit(1);
                }
            }

            if (cmd_data->mode != mLink) {
                push_count_chars(cmd_data->arglist, newarg);
            }
            else {
                push_count_chars(cmd_data->shared_opts.dependencies, newarg);
            }
            break;
        case mInstall:
            /* If we've already recorded a library to install, we're most
             * likely getting the .la file that we want to install as.
             * The problem is that we need to add it as the directory,
             * not the .la file itself.  Otherwise, we'll do odd things.
             */
            if (cmd_data->output == otLibrary) {
                arg[pathlen] = '\0';
                push_count_chars(cmd_data->arglist, arg);
            }
            else {
                cmd_data->output = otLibrary;
                cmd_data->output_name = arg;
                cmd_data->static_name.install = gen_install_name(arg, 0);
                cmd_data->shared_name.install = gen_install_name(arg, 1);
                cmd_data->module_name.install = gen_install_name(arg, 2);
            }
            break;
        default:
            break;
        }
        return 1;
    }

    if (strcmp(ext, "c") == 0) {
        /* If we don't already have an idea what our output name will be. */
        if (cmd_data->basename == NULL) {
            cmd_data->basename = (char *)malloc(strlen(arg) + 4);
            strcpy(cmd_data->basename, arg);
            strcpy(strrchr(cmd_data->basename, '.') + 1, "lo");

            cmd_data->fake_output_name = strrchr(cmd_data->basename, '/');
            if (cmd_data->fake_output_name) {
                cmd_data->fake_output_name++;
            }
            else {
                cmd_data->fake_output_name = cmd_data->basename;
            }
        }
    }

    return 0;
}

int parse_output_file_name(char *arg, command_t *cmd_data)
{
    char *name = strrchr(arg, '/');
    char *ext = strrchr(arg, '.');
    char *newarg = NULL;
    int pathlen;

    cmd_data->fake_output_name = arg;

    if (name) {
        name++;
    }
    else {
        name = strrchr(arg, '\\');

        if (name == NULL) {
            name = arg;
        }
        else {
            name++;
        }
    }

    if (ext) {
        ext++;
        pathlen = name - arg;

        if (strcmp(ext, "la") == 0) {
            assert(cmd_data->mode == mLink);

            cmd_data->basename = arg;
            cmd_data->static_name.normal = gen_library_name(arg, 0);
            cmd_data->shared_name.normal = gen_library_name(arg, 1);
            cmd_data->module_name.normal = gen_library_name(arg, 2);
            cmd_data->static_name.install = gen_install_name(arg, 0);
            cmd_data->shared_name.install = gen_install_name(arg, 1);
            cmd_data->module_name.install = gen_install_name(arg, 2);

#ifdef TRUNCATE_DLL_NAME
            if (shared) {
              arg = truncate_dll_name(arg);
            }
#endif

            cmd_data->output_name = arg;
            return 1;
        }

        if (strcmp(ext, "lo") == 0) {
            cmd_data->basename = arg;
            cmd_data->output = otObject;
            newarg = (char *)malloc(strlen(arg) + 2);
            strcpy(newarg, arg);
            ext = strrchr(newarg, '.') + 1;
            strcpy(ext, OBJECT_EXT);
            cmd_data->output_name = newarg;
            return 1;
        }
    }

    cmd_data->basename = arg;
    cmd_data->output = otProgram;
    newarg = (char *)malloc(strlen(arg) + 5);
    strcpy(newarg, arg);
#ifdef EXE_EXT
    strcat(newarg, EXE_EXT);
#endif
    cmd_data->output_name = newarg;
    return 1;
}

/* returns just a file's name without path or extension */
char *nameof(char *fullpath)
{
    char buffer[1024];
    char *ext;
    char *name = strrchr(fullpath, '/');

    if (name == NULL) {
        name = strrchr(fullpath, '\\');
    }

    if (name == NULL) {
        name = fullpath;
    } else {
        name++;
    }

    strcpy(buffer, name);
    ext = strrchr(buffer, '.');

    if (ext) {
        *ext = 0;
        return strdup(buffer);
    }

    return name;
}

void parse_args(int argc, char *argv[], command_t *cmd_data)
{
    int a;
    char *arg;
    int argused;

    for (a=1; a < argc; a++) {
        arg = argv[a];
        argused = 1;

        if (arg[0] == '-') {
            if (arg[1] == '-') {
                argused = parse_long_opt(arg + 2, cmd_data);
            }
            else {
                argused = parse_short_opt(arg + 1, cmd_data);
            }

            /* We haven't done anything with it yet, try some of the
             * more complicated short opts... */
            if (argused == 0 && a + 1 < argc) {
                if (arg[1] == 'o' && !arg[2]) {
                    arg = argv[++a];
                    argused = parse_output_file_name(arg, cmd_data);
                } else if (strcmp(arg+1, "MT") == 0) {
                    /* Not sure but pass it through */
					if (cmd_data->options.debug) {
						printf("Adding: %s\n", arg);
					}
					push_count_chars(cmd_data->arglist, arg);
					a++; /* skip to filename */
					argused = 0;
                } else if (strcmp(arg+1, "rpath") == 0) {
                    /* Aha, we should try to link both! */
                    cmd_data->install_path = argv[++a];
                    argused = 1;
                } else if (strcmp(arg+1, "version-info") == 0) {
                    /* Store for later deciphering */
                    cmd_data->version_info = argv[++a];
                    argused = 1;
                }
            }
        } else {
            argused = parse_input_file_name(arg, cmd_data);
        }

        if (!argused) {
            if (cmd_data->options.debug) {
                printf("Adding: %s\n", arg);
            }
            push_count_chars(cmd_data->arglist, arg);
        }
		if (argused > 1)
			a += (argused - 1);
    }

}

int explode_static_lib(const char *lib, command_t *cmd_data)
{
    char tmpdir[1024];
    char savewd[1024];
    char cmd[1024];
    const char *name;
    DIR *dir;
    struct dirent *entry;

    /* Bah! */
    if (cmd_data->options.dry_run) {
        return 0;
    }

    strcpy(tmpdir, lib);
    strcat(tmpdir, ".exploded");

    mkdir(tmpdir, 0);
    push_count_chars(cmd_data->tmp_dirs, strdup(tmpdir));
    getcwd(savewd, sizeof(savewd));

    if (chdir(tmpdir) != 0)
        return 1;

    strcpy(cmd, LIBRARIAN " x ");
    name = strrchr(lib, '/');

    if (name) {
        name++;
    } else {
        name = lib;
    }

    strcat(cmd, "../");
    strcat(cmd, name);
    system(cmd);
    chdir(savewd);
    dir = opendir(tmpdir);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            strcpy(cmd, tmpdir);
            strcat(cmd, "/");
            strcat(cmd, entry->d_name);
            push_count_chars(cmd_data->arglist, strdup(cmd));
        }
    }

    closedir(dir);
    return 0;
}

#ifdef GEN_EXPORTS
void generate_def_file(command_t *cmd_data)
{
    char def_file[1024];
    char implib_file[1024];
    char *ext;
    FILE *hDef;
    char *export_args[1024];
    int num_export_args = 0;
    char *cmd;
    int cmd_size = 0;
    int a;

    if (cmd_data->output_name) {
        strcpy(def_file, cmd_data->output_name);
        strcat(def_file, ".def");
        hDef = fopen(def_file, "w");

        if (hDef != NULL) {
            fprintf(hDef, "LIBRARY '%s' INITINSTANCE\n", nameof(cmd_data->output_name));
            fprintf(hDef, "DATA NONSHARED\n");
            fprintf(hDef, "EXPORTS\n");
            fclose(hDef);

            for (a = 0; a < cmd_data->num_obj_files; a++) {
                cmd_size += strlen(cmd_data->obj_files[a]) + 1;
            }

            cmd_size += strlen(GEN_EXPORTS) + strlen(def_file) + 3;
            cmd = (char *)malloc(cmd_size);
            strcpy(cmd, GEN_EXPORTS);

            for (a=0; a < cmd_data->num_obj_files; a++) {
                strcat(cmd, " ");
                strcat(cmd, cmd_data->obj_files[a] );
            }

            strcat(cmd, ">>");
            strcat(cmd, def_file);
            puts(cmd);
            export_args[num_export_args++] = SHELL_CMD;
            export_args[num_export_args++] = "-c";
            export_args[num_export_args++] = cmd;
            export_args[num_export_args++] = NULL;
            external_spawn(cmd_data, export_args[0], (const char**)export_args);
            cmd_data->arglist[cmd_data->num_args++] = strdup(def_file);

            /* Now make an import library for the dll */
            num_export_args = 0;
            export_args[num_export_args++] = DEF2IMPLIB_CMD;
            export_args[num_export_args++] = "-o";

            strcpy(implib_file, ".libs/");
            strcat(implib_file, cmd_data->basename);
            ext = strrchr(implib_file, '.');

            if (ext)
                *ext = 0;

            strcat(implib_file, ".");
            strcat(implib_file, STATIC_LIB_EXT);

            export_args[num_export_args++] = implib_file;
            export_args[num_export_args++] = def_file;
            export_args[num_export_args++] = NULL;
            external_spawn(cmd_data, export_args[0], (const char**)export_args);

        }
    }
}
#endif

const char* expand_path(const char *relpath)
{
    char foo[PATH_MAX], *newpath;

    getcwd(foo, PATH_MAX-1);
    newpath = (char*)malloc(strlen(foo)+strlen(relpath)+2);
    strcat(newpath, foo);
    strcat(newpath, "/");
    strcat(newpath, relpath);
    return newpath;
}

void link_fixup(command_t *c)
{
    /* If we were passed an -rpath directive, we need to build
     * shared objects too.  Otherwise, we should only create static
     * libraries.
     */
    if (!c->install_path && (c->output == otDynamicLibraryOnly ||
        c->output == otModule || c->output == otLibrary)) {
        c->output = otStaticLibraryOnly;
    }

    if (c->output == otDynamicLibraryOnly ||
        c->output == otModule ||
        c->output == otLibrary) {

        push_count_chars(c->shared_opts.normal, "-o");
        if (c->output == otModule) {
            push_count_chars(c->shared_opts.normal, c->module_name.normal);
        }
        else {
            char *tmp;
            push_count_chars(c->shared_opts.normal, c->shared_name.normal);
#ifdef DYNAMIC_INSTALL_NAME
            push_count_chars(c->shared_opts.normal, DYNAMIC_INSTALL_NAME);

            tmp = (char*)malloc(PATH_MAX);
            strcat(tmp, c->install_path);
            strcat(tmp, strrchr(c->shared_name.normal, '/'));
            push_count_chars(c->shared_opts.normal, tmp);
#endif
        }

        append_count_chars(c->shared_opts.normal, c->obj_files);
        append_count_chars(c->shared_opts.normal, c->shared_opts.dependencies);

        if (c->options.export_all) {
#ifdef GEN_EXPORTS
            generate_def_file(c);
#endif
        }
    }

    if (c->output == otLibrary || c->output == otStaticLibraryOnly) {
        push_count_chars(c->static_opts.normal, "-o");
        push_count_chars(c->static_opts.normal, c->output_name);
    }

    if (c->output == otProgram) {
        if (c->output_name) {
            push_count_chars(c->arglist, "-o");
            push_count_chars(c->arglist, c->output_name);
            append_count_chars(c->arglist, c->obj_files);
            append_count_chars(c->arglist, c->shared_opts.dependencies);
#if 0 // exes are not shared :-) def DYNAMIC_LINK_OPTS
            if (c->options.pic_mode != AVOID) {
                push_count_chars(c->arglist, DYNAMIC_LINK_OPTS);
            }
#endif
        }
    }

#ifdef NO_UNDEFINED
    if (c->options.no_undefined) {
        add_linker_flag_prefix(c->arglist, NO_UNDEFINED);
    }
#endif
}

void post_parse_fixup(command_t *cmd_data)
{
    switch (cmd_data->mode)
    {
    case mCompile:
#ifdef PIC_FLAG
        if (cmd_data->options.pic_mode != AVOID) {
            push_count_chars(cmd_data->arglist, PIC_FLAG);
        }
#endif
        if (cmd_data->output_name) {
            push_count_chars(cmd_data->arglist, "-o");
            push_count_chars(cmd_data->arglist, cmd_data->output_name);
        }
        break;
    case mLink:
        link_fixup(cmd_data);
        break;
    case mInstall:
        if (cmd_data->output == otLibrary) {
            link_fixup(cmd_data);
        }
    default:
        break;
    }

#if USE_OMF
    if (cmd_data->output == otObject ||
        cmd_data->output == otProgram ||
        cmd_data->output == otLibrary ||
        cmd_data->output == otDynamicLibraryOnly) {
        push_count_chars(cmd_data->arglist, "-Zomf");
    }
#endif

    if (cmd_data->options.shared &&
            (cmd_data->output == otObject ||
             cmd_data->output == otLibrary ||
             cmd_data->output == otDynamicLibraryOnly)) {
#ifdef SHARE_SW
        push_count_chars(cmd_data->arglist, SHARE_SW);
#endif
    }
}

int run_mode(command_t *cmd_data)
{
    int rv;
    count_chars *cctemp;

    cctemp = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cctemp);

    switch (cmd_data->mode)
    {
    case mCompile:
        rv = run_command(cmd_data, cmd_data->arglist);
        if (rv) {
            return rv;
        }
        break;
    case mInstall:
        /* Well, we'll assume it's a file going to a directory... */
        /* For brain-dead install-sh based scripts, we have to repeat
         * the command N-times.  install-sh should die.
         */
        if (!cmd_data->output_name) {
            rv = run_command(cmd_data, cmd_data->arglist);
            if (rv) {
                return rv;
            }
        }
        if (cmd_data->output_name) {
            append_count_chars(cctemp, cmd_data->arglist);
            insert_count_chars(cctemp,
                               cmd_data->output_name,
                               cctemp->num - 1);
            rv = run_command(cmd_data, cctemp);
            if (rv) {
                return rv;
            }
            clear_count_chars(cctemp);
        }
        if (cmd_data->static_name.install) {
            append_count_chars(cctemp, cmd_data->arglist);
            insert_count_chars(cctemp,
                               cmd_data->static_name.install,
                               cctemp->num - 1);
            rv = run_command(cmd_data, cctemp);
            if (rv) {
                return rv;
            }
            clear_count_chars(cctemp);
        }
        if (cmd_data->shared_name.install) {
            append_count_chars(cctemp, cmd_data->arglist);
            insert_count_chars(cctemp,
                               cmd_data->shared_name.install,
                               cctemp->num - 1);
            rv = run_command(cmd_data, cctemp);
            if (rv) {
                return rv;
            }
            clear_count_chars(cctemp);
        }
        if (cmd_data->module_name.install) {
            append_count_chars(cctemp, cmd_data->arglist);
            insert_count_chars(cctemp,
                               cmd_data->module_name.install,
                               cctemp->num - 1);
            rv = run_command(cmd_data, cctemp);
            if (rv) {
                return rv;
            }
            clear_count_chars(cctemp);
        }
        break;
    case mLink:
        if (!cmd_data->options.dry_run) {
            /* Check first to see if the dir already exists! */
            mode_t old_umask;

            old_umask = umask(0);
            umask(old_umask);

            mkdir(".libs", ~old_umask);
        }

        if (cmd_data->output == otStaticLibraryOnly ||
            cmd_data->output == otLibrary) {
#ifdef RANLIB
            const char *lib_args[3];
#endif
            /* Removes compiler! */
            cmd_data->program = LIBRARIAN;
            push_count_chars(cmd_data->program_opts, LIBRARIAN_OPTS);
            push_count_chars(cmd_data->program_opts,
                             cmd_data->static_name.normal);

            rv = run_command(cmd_data, cmd_data->obj_files);
            if (rv) {
                return rv;
            }

#ifdef RANLIB
            lib_args[0] = RANLIB;
            lib_args[1] = cmd_data->static_name.normal;
            lib_args[2] = NULL;
            external_spawn(cmd_data, RANLIB, lib_args);
#endif
            if (!cmd_data->options.dry_run) {
                //link(
            }
        }

        if (cmd_data->output == otDynamicLibraryOnly ||
            cmd_data->output == otModule ||
            cmd_data->output == otLibrary) {
            cmd_data->program = NULL;
            clear_count_chars(cmd_data->program_opts);

            append_count_chars(cmd_data->program_opts, cmd_data->arglist);
            if (cmd_data->output != otModule) {
#ifdef SHARED_OPTS
                push_count_chars(cmd_data->program_opts, SHARED_OPTS);
#endif
#ifdef dynamic_link_version_func
                push_count_chars(cmd_data->program_opts,
                             dynamic_link_version_func(cmd_data->version_info));
#endif
            }
            if (cmd_data->output == otModule) {
#ifdef MODULE_OPTS
                push_count_chars(cmd_data->program_opts, MODULE_OPTS);
#endif
            }
#ifdef DYNAMIC_LINK_OPTS
            if (cmd_data->options.pic_mode != AVOID) {
                push_count_chars(cmd_data->program_opts,
                                 DYNAMIC_LINK_OPTS);
            }
#endif
            rv = run_command(cmd_data, cmd_data->shared_opts.normal);
            if (rv) {
                return rv;
            }
        }
        if (cmd_data->output == otProgram) {
            rv = run_command(cmd_data, cmd_data->arglist);
            if (rv) {
                return rv;
            }
        }
        break;
    default:
        break;
    } 

    return 0;
}

void cleanup_tmp_dir(const char *dirname)
{
    DIR *dir;
    struct dirent *entry;
    char fullname[1024];

    dir = opendir(dirname);

    if (dir == NULL)
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            strcpy(fullname, dirname);
            strcat(fullname, "/");
            strcat(fullname, entry->d_name);
            remove(fullname);
        }
    }

    rmdir(dirname);
}

void cleanup_tmp_dirs(command_t *cmd_data)
{
    int d;

    for (d = 0; d < cmd_data->tmp_dirs->num; d++) {
        cleanup_tmp_dir(cmd_data->tmp_dirs->vals[d]);
    }
}

int ensure_fake_uptodate(command_t *cmd_data)
{
    /* FIXME: could do the stat/touch here, but nah... */
    const char *touch_args[3];

    if (cmd_data->mode == mInstall) {
        return 0;
    }

    touch_args[0] = "touch";
    touch_args[1] = cmd_data->fake_output_name;
    touch_args[2] = NULL;
    return external_spawn(cmd_data, "touch", touch_args);
}

int main(int argc, char *argv[])
{
    int rc;
    command_t cmd_data;
    char *v;

    memset(&cmd_data, 0, sizeof(cmd_data));

    cmd_data.options.pic_mode = UNKNOWN;

    v = getenv("V");
    if (v) {
        cmd_data.options.debug = atoi(v);
    }

    cmd_data.program_opts = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.program_opts);
    cmd_data.arglist = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.arglist);
    cmd_data.tmp_dirs = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.tmp_dirs);
    cmd_data.obj_files = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.obj_files);
    cmd_data.dep_rpaths = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.dep_rpaths);
    cmd_data.rpaths = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.rpaths);
    cmd_data.static_opts.normal = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.static_opts.normal);
    cmd_data.shared_opts.normal = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.shared_opts.normal);
    cmd_data.shared_opts.dependencies = (count_chars*)malloc(sizeof(count_chars));
    init_count_chars(cmd_data.shared_opts.dependencies);

    cmd_data.mode = mUnknown;
    cmd_data.output = otGeneral;

    parse_args(argc, argv, &cmd_data);
    post_parse_fixup(&cmd_data);

    if (cmd_data.mode == mUnknown) {
        exit(0);
    }

    rc = run_mode(&cmd_data);

    if (!rc) {
       ensure_fake_uptodate(&cmd_data);
    }

    cleanup_tmp_dirs(&cmd_data);
    return rc;
}
