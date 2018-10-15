#if HAVE_CONFIG_H
#include "clamav-config.h"
#endif

#include <stdio.h>

#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <string.h>
#include <check.h>
#include "../libclamav/clamav.h"
#include "../libclamav/others.h"
#include "../libclamav/matcher.h"
#include "../libclamav/version.h"
#include "../libclamav/dsig.h"
#include "../libclamav/sha256.h"
#include "checks.h"

/* extern void cl_free(struct cl_engine *engine); */
START_TEST (test_cl_free)
/*
    struct cl_engine *engine = NULL;
    cl_free(NULL);
*/
END_TEST

/* extern struct cl_engine *cl_dup(struct cl_engine *engine); */
START_TEST (test_cl_dup)
    /*
    struct cl_engine *engine;
    fail_unless(NULL == cl_dup(NULL), "cl_dup null pointer");
    */
END_TEST

/* extern int cl_build(struct cl_engine *engine); */
START_TEST (test_cl_build)
    /*
    struct cl_engine *engine;
    fail_unless(CL_ENULLARG == cl_build(NULL), "cl_build null pointer");
    engine = calloc(sizeof(struct cl_engine),1);
    fail_unless(engine, "cl_build calloc");
    fail_unless(CL_ENULLARG == cl_build(engine), "cl_build(engine) with null ->root");
    */
/*    engine->root = cli_calloc(CL_TARGET_TABLE_SIZE, sizeof(struct cli_matcher *)); */
END_TEST

/* extern void cl_debug(void); */
START_TEST (test_cl_debug)
{
    int old_status = cli_debug_flag;
    cli_debug_flag = 0;
    cl_debug();
    fail_unless(1 == cli_debug_flag, "cl_debug failed to set cli_debug_flag");

    cli_debug_flag = 1;
    cl_debug();
    fail_unless(1 == cli_debug_flag, "cl_debug failed when flag was already set");
    cli_debug_flag = old_status;
}
END_TEST

/* extern const char *cl_retdbdir(void); */
START_TEST (test_cl_retdbdir)
    fail_unless(!strcmp(DATADIR, cl_retdbdir()), "cl_retdbdir");
END_TEST

#ifndef REPO_VERSION
#define REPO_VERSION VERSION
#endif

/* extern const char *cl_retver(void); */
START_TEST (test_cl_retver)
{
    const char *ver = cl_retver();
    fail_unless(!strcmp(REPO_VERSION""VERSION_SUFFIX, ver),"cl_retver");
    fail_unless(strcspn(ver,"012345789") < strlen(ver),
		    "cl_retver must have a number");
}
END_TEST

/* extern void cl_cvdfree(struct cl_cvd *cvd); */
START_TEST (test_cl_cvdfree)
/*
    struct cl_cvd *cvd1, *cvd2;

    cvd1 = malloc(sizeof(struct cl_cvd));
    fail_unless(cvd1, "cvd malloc");
    cl_cvdfree(cvd1);

    cvd2 = malloc(sizeof(struct cl_cvd));
    cvd2->time = malloc(1);
    cvd2->md5 = malloc(1);
    cvd2->dsig= malloc(1);
    cvd2->builder = malloc(1);
    fail_unless(cvd2, "cvd malloc");
    fail_unless(cvd2->time, "cvd malloc");
    fail_unless(cvd2->md5, "cvd malloc");
    fail_unless(cvd2->dsig, "cvd malloc");
    fail_unless(cvd2->builder, "cvd malloc");
    cl_cvdfree(cvd2);
    cl_cvdfree(NULL);
*/
END_TEST

/* extern int cl_statfree(struct cl_stat *dbstat); */
START_TEST (test_cl_statfree)
/*
    struct cl_stat *stat;
    fail_unless(CL_ENULLARG == cl_statfree(NULL), "cl_statfree(NULL)");
    
    stat = malloc(sizeof(struct cl_stat));
    fail_unless(NULL != stat, "malloc");
    fail_unless(CL_SUCCESS == cl_statfree(stat), "cl_statfree(empty_struct)");
    
    stat = malloc(sizeof(struct cl_stat));
    fail_unless(NULL != stat, "malloc");
    stat->stattab = strdup("test");
    fail_unless(NULL != stat->stattab, "strdup");
    fail_unless(CL_SUCCESS == cl_statfree(stat), "cl_statfree(stat with stattab)");

    stat = malloc(sizeof(struct cl_stat));
    fail_unless(NULL != stat, "malloc");
    stat->stattab = NULL;
    fail_unless(CL_SUCCESS == cl_statfree(stat), "cl_statfree(stat with stattab) set to NULL");
*/
END_TEST

/* extern unsigned int cl_retflevel(void); */
START_TEST (test_cl_retflevel)
END_TEST    

/* extern struct cl_cvd *cl_cvdhead(const char *file); */
START_TEST (test_cl_cvdhead)
/*
    fail_unless(NULL == cl_cvdhead(NULL), "cl_cvdhead(null)");
    fail_unless(NULL == cl_cvdhead("input/cl_cvdhead/1.txt"), "cl_cvdhead(515 byte file, all nulls)");
*/
    /* the data read from the file is passed to cl_cvdparse, test cases for that are separate */
END_TEST

/* extern struct cl_cvd *cl_cvdparse(const char *head); */
START_TEST (test_cl_cvdparse)
END_TEST

/* int cl_scandesc(int desc, const char **virname, unsigned long int *scanned, const struct cl_engine *engine, const struct cl_limits *limits, unsigned int options) */
START_TEST (test_cl_scandesc)
END_TEST

/* int cl_scanfile(const char *filename, const char **virname, unsigned long int *scanned, const struct cl_engine *engine, const struct cl_limits *limits, unsigned int options) */
START_TEST (test_cl_scanfile)
END_TEST

/* int cl_load(const char *path, struct cl_engine **engine, unsigned int *signo, unsigned int options) */
START_TEST (test_cl_load)
END_TEST

/* int cl_cvdverify(const char *file) */
START_TEST (test_cl_cvdverify)
END_TEST

/* int cl_statinidir(const char *dirname, struct cl_stat *dbstat) */
START_TEST (test_cl_statinidir)
END_TEST

/* int cl_statchkdir(const struct cl_stat *dbstat) */
START_TEST (test_cl_statchkdir)
END_TEST

/* void cl_settempdir(const char *dir, short leavetemps) */
START_TEST (test_cl_settempdir)
END_TEST

/* const char *cl_strerror(int clerror) */
START_TEST (test_cl_strerror)
END_TEST

static Suite *test_cl_suite(void)
{
    Suite *s = suite_create("cl_api");
    TCase *tc_cl = tcase_create("cl_dup");

    suite_add_tcase (s, tc_cl);
    tcase_add_test(tc_cl, test_cl_free);
    tcase_add_test(tc_cl, test_cl_dup);
    tcase_add_test(tc_cl, test_cl_build);
    tcase_add_test(tc_cl, test_cl_debug);
    tcase_add_test(tc_cl, test_cl_retdbdir);
    tcase_add_test(tc_cl, test_cl_retver);
    tcase_add_test(tc_cl, test_cl_cvdfree);
    tcase_add_test(tc_cl, test_cl_statfree);
    tcase_add_test(tc_cl, test_cl_retflevel);
    tcase_add_test(tc_cl, test_cl_cvdhead);
    tcase_add_test(tc_cl, test_cl_cvdparse);
    tcase_add_test(tc_cl, test_cl_scandesc);
    tcase_add_test(tc_cl, test_cl_scanfile);
    tcase_add_test(tc_cl, test_cl_load);
    tcase_add_test(tc_cl, test_cl_cvdverify);
    tcase_add_test(tc_cl, test_cl_statinidir);
    tcase_add_test(tc_cl, test_cl_statchkdir);
    tcase_add_test(tc_cl, test_cl_settempdir);
    tcase_add_test(tc_cl, test_cl_strerror);

    return s;
}

static uint8_t le_data[4] = {0x67,0x45,0x23,0x01};
static int32_t le_expected[4] = { 0x01234567, 0x67012345, 0x45670123, 0x23456701};
uint8_t *data = NULL;
uint8_t *data2 = NULL;
#define DATA_REP 100

static void data_setup(void)
{
        uint8_t *p;
        size_t i;

	data = malloc(sizeof(le_data)*DATA_REP);
	data2 = malloc(sizeof(le_data)*DATA_REP);
	fail_unless(!!data, "unable to allocate memory for fixture");
        fail_unless(!!data2, "unable to allocate memory for fixture");
        p = data;
        /* make multiple copies of le_data, we need to run readint tests in a loop, so we need
         * to give it some data to run it on */
        for(i=0; i<DATA_REP;i++) {
                memcpy(p, le_data, sizeof(le_data));
                p += sizeof(le_data);
        }
        memset(data2, 0, DATA_REP*sizeof(le_data));
}

static void data_teardown(void)
{
        free(data);
	free(data2);
}

#ifdef CHECK_HAVE_LOOPS
/* test reading with different alignments, _i is parameter from tcase_add_loop_test */
START_TEST (test_cli_readint16)
{
    size_t j;
    int16_t value;
    /* read 2 bytes apart, start is not always aligned*/
    for(j=_i;j <= DATA_REP*sizeof(le_data)-2;j += 2) {
        value = le_expected[j&3];
        fail_unless(cli_readint16(&data[j]) == value, "(1) data read must be little endian");
    }
    /* read 2 bytes apart, always aligned*/
    for(j=0;j <= DATA_REP*sizeof(le_data)-2;j += 2) {
        value = le_expected[j&3];
        fail_unless(cli_readint16(&data[j]) == value, "(2) data read must be little endian");
    }
}
END_TEST

/* test reading with different alignments, _i is parameter from tcase_add_loop_test */
START_TEST (test_cli_readint32)
{
    size_t j;
    int32_t value = le_expected[_i&3];
    /* read 4 bytes apart, start is not always aligned*/
    for(j=_i;j < DATA_REP*sizeof(le_data)-4;j += 4) {
        fail_unless(cli_readint32(&data[j]) == value, "(1) data read must be little endian");
    }
    value = le_expected[0];
    /* read 4 bytes apart, always aligned*/
    for(j=0;j < DATA_REP*sizeof(le_data)-4;j += 4) {
        fail_unless(cli_readint32(&data[j]) == value, "(2) data read must be little endian");
    }
}
END_TEST

/* test writing with different alignments, _i is parameter from tcase_add_loop_test */
START_TEST (test_cli_writeint32)
{
    size_t j;
    /* write 4 bytes apart, start is not always aligned*/
    for(j=_i;j < DATA_REP*sizeof(le_data) - 4;j += 4) {
        cli_writeint32(&data2[j], 0x12345678);
    }
    for(j=_i;j < DATA_REP*sizeof(le_data) - 4;j += 4) {
        fail_unless(cli_readint32(&data2[j]) == 0x12345678, "write/read mismatch");
    }
    /* write 4 bytes apart, always aligned*/
    for(j=0;j < DATA_REP*sizeof(le_data) - 4;j += 4) {
        cli_writeint32(&data2[j], 0x12345678);
    }
    for(j=0;j < DATA_REP*sizeof(le_data) - 4;j += 4) {
        fail_unless(cli_readint32(&data2[j]) == 0x12345678, "write/read mismatch");
    }
}
END_TEST

static struct dsig_test {
    const char *md5;
    const char *dsig;
    int result;
} dsig_tests [] = {
    {"ae307614434715274c60854c931a26de", "60uhCFmiN48J8r6c7coBv9Q1mehAWEGh6GPYA+60VhQcuXfb0iV1O+sCEyMiRXt/iYF6vXtPXHVd6DiuZ4Gfrry7sVQqNTt3o1/KwU1rc0l5FHgX/nC99fdr/fjaFtinMtRnUXHLeu0j8e6HK+7JLBpD37fZ60GC9YY86EclYGe", 
	CL_SUCCESS},
    {"96b7feb3b2a863846438809fe481906f", "Zh5gmf09Zfj6V4gmRKu/NURzhFiE9VloI7w1G33BgDdGSs0Xhscx6sjPUpFSCPsjOalyS4L8q7RS+NdGvNCsLymiIH6RYItlOZsygFhcGuH4jt15KAaAkvEg2TwmqR8z41nUaMlZ0c8q1MXYCLvQJyFARsfzIxS3PAoN2Y3HPoe",
	CL_SUCCESS},
    {"ae307614434715274c60854c931a26de", "Zh5gmf09Zfj6V4gmRKu/NURzhFiE9VloI7w1G33BgDdGSs0Xhscx6sjPUpFSCPsjOalyS4L8q7RS+NdGvNCsLymiIH6RYItlOZsygFhcGuH4jt15KAaAkvEg2TwmqR8z41nUaMlZ0c8q1MXYCLvQJyFARsfzIxS3PAoN2Y3HPoe",
	CL_EVERIFY},
    {"96b7feb3b2a863846438809fe481906f", "60uhCFmiN48J8r6c7coBv9Q1mehAWEGh6GPYA+60VhQcuXfb0iV1O+sCEyMiRXt/iYF6vXtPXHVd6DiuZ4Gfrry7sVQqNTt3o1/KwU1rc0l5FHgX/nC99fdr/fjaFtinMtRnUXHLeu0j8e6HK+7JLBpD37fZ60GC9YY86EclYGe",
	CL_EVERIFY},
    {"ae307614434715274060854c931a26de", "60uhCFmiN48J8r6c7coBv9Q1mehAWEGh6GPYA+60VhQcuXfb0iV1O+sCEyMiRXt/iYF6vXtPXHVd6DiuZ4Gfrry7sVQqNTt3o1/KwU1rc0l5FHgX/nC99fdr/fjaFtinMtRnUXHLeu0j8e6HK+7JLBpD37fZ60GC9YY86EclYGe",
	CL_EVERIFY},
    {"ae307614434715274c60854c931a26de", "60uhCFmiN48J8r6c7coBv9Q1mehAWEGh6GPYA+60VhQcuXfb0iV1O+sCEyMiRXt/iYF6vXtPXHVd6DiuZ4Gfrry7sVQqNTt3o1/KwU1rc0l5FHgX/nC99fdr/fjaatinMtRnUXHLeu0j8e6HK+7JLBpD37fZ60GC9YY86EclYGe",
	CL_EVERIFY},
    {"96b7feb3b2a863846438809fe481906f", "Zh5gmf09Zfj6V4gmRKu/NURzhFiE9VloI7w1G33BgDdGSs0Xhscx6sjPUpFSCPsjOalyS4L8q7RS+NdGvNCsLymiIH6RYItlOZsygFhcGuH4jt15KAaAkvEg2TwmqR8z41nUaMlZ0c8q1MYYCLvQJyFARsfzIxS3PAoN2Y3HPoe",
	CL_EVERIFY},
    {"ge307614434715274c60854c931a26dee","60uhCFmiN48J8r6c7coBv9Q1mehAWEGh6GPYA+60VhQcuXfb0iV1O+sCEyMiRXt/iYF6vXtPXHVd6DiuZ4Gfrry7sVQqNTt3o1/KwU1rc0l5FHgX/nC99fdr/fjaFtinMtRnUXHLeu0j8e6HK+7JLBpD37fZ60GC9YY86EclYGe",
	CL_EVERIFY},
    {"ae307614434715274c60854c931a26de", "60uhCFmiN48J8r6c7coBv9Q1mehAWEGh6GPYA+60VhQcuXfb0iV1O+sCEyMiRXt/iYF6vXtPXHVd6DiuZ4Gfrry7sVQqNTt3o1/KwU1rc0l5FHgX/nC99fdr/fjaFtinMtRnUXHLeu0j8e6HK+7JLBpD37fZ60GC9YY86EclYGee", 
	CL_EVERIFY},
    {"ae307614434715274c60854c931a26de", "60uhCFmiN48J8r6c7coBv9Q1mehAWEGh6GPYA+", 
	CL_EVERIFY}
};

static const size_t dsig_tests_cnt = sizeof(dsig_tests)/sizeof(dsig_tests[0]);

START_TEST (test_cli_dsig)
{
    fail_unless(cli_versig(dsig_tests[_i].md5, dsig_tests[_i].dsig) == dsig_tests[_i].result,
		"digital signature verification test failed");
}
END_TEST

static uint8_t tv1[3] = {
  0x61, 0x62, 0x63
};

static uint8_t tv2[56] = {
  0x61, 0x62, 0x63, 0x64, 0x62, 0x63, 0x64, 0x65,
  0x63, 0x64, 0x65, 0x66, 0x64, 0x65, 0x66, 0x67,
  0x65, 0x66, 0x67, 0x68, 0x66, 0x67, 0x68, 0x69,
  0x67, 0x68, 0x69, 0x6a, 0x68, 0x69, 0x6a, 0x6b,
  0x69, 0x6a, 0x6b, 0x6c, 0x6a, 0x6b, 0x6c, 0x6d,
  0x6b, 0x6c, 0x6d, 0x6e, 0x6c, 0x6d, 0x6e, 0x6f,
  0x6d, 0x6e, 0x6f, 0x70, 0x6e, 0x6f, 0x70, 0x71
};

static uint8_t tv3[112] = {
  0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
  0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
  0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a,
  0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
  0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c,
  0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d,
  0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e,
  0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
  0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
  0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71,
  0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72,
  0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73,
  0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74,
  0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75
};

static uint8_t res256[3][SHA256_HASH_SIZE] = {
  { 0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde,
    0x5d, 0xae, 0x22, 0x23, 0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
    0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad },
  { 0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8, 0xe5, 0xc0, 0x26, 0x93,
    0x0c, 0x3e, 0x60, 0x39, 0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
    0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1 },
  { 0xcd, 0xc7, 0x6e, 0x5c, 0x99, 0x14, 0xfb, 0x92, 0x81, 0xa1, 0xc7, 0xe2,
    0x84, 0xd7, 0x3e, 0x67, 0xf1, 0x80, 0x9a, 0x48, 0xa4, 0x97, 0x20, 0x0e,
    0x04, 0x6d, 0x39, 0xcc, 0xc7, 0x11, 0x2c, 0xd0 }
};

START_TEST (test_sha256)
{
    SHA256_CTX sha256;
    uint8_t hsha256[SHA256_HASH_SIZE];
    uint8_t buf[1000];
    int i;

    memset (buf, 0x61, sizeof (buf));

    sha256_init (&sha256);
    sha256_update (&sha256, tv1, sizeof (tv1));
    sha256_final (&sha256, hsha256);
    fail_unless(!memcmp (hsha256, res256[0], sizeof (hsha256)), "sha256 test vector #1 failed");

    sha256_init (&sha256);
    sha256_update (&sha256, tv2, sizeof (tv2));
    sha256_final (&sha256, hsha256);
    fail_unless(!memcmp (hsha256, res256[1], sizeof (hsha256)), "sha256 test vector #2 failed");

    sha256_init (&sha256);
    for (i = 0; i < 1000; i++)
	sha256_update (&sha256, buf, sizeof (buf));
    sha256_final (&sha256, hsha256);
    fail_unless(!memcmp (hsha256, res256[2], sizeof (hsha256)), "sha256 test vector #3 failed");
}
END_TEST

static Suite *test_cli_suite(void)
{
    Suite *s = suite_create("cli");
    TCase *tc_cli_others = tcase_create("byteorder_macros");
    TCase *tc_cli_dsig = tcase_create("digital signatures");

    suite_add_tcase (s, tc_cli_others);
    tcase_add_checked_fixture (tc_cli_others, data_setup, data_teardown);
    tcase_add_loop_test(tc_cli_others, test_cli_readint32, 0, 16);
    tcase_add_loop_test(tc_cli_others, test_cli_readint16, 0, 16);
    tcase_add_loop_test(tc_cli_others, test_cli_writeint32, 0, 16);

    suite_add_tcase (s, tc_cli_dsig);
    tcase_add_loop_test(tc_cli_dsig, test_cli_dsig, 0, dsig_tests_cnt);
    tcase_add_test(tc_cli_dsig, test_sha256);

    return s;
}
#endif /* CHECK_HAVE_LOOPS */

void errmsg_expected(void)
{
	fputs("cli_errmsg() expected here\n", stderr);
}

int open_testfile(const char *name)
{
	int fd;
	const char * srcdir = getenv("srcdir");
	char *str;

	if(!srcdir) {
		/* when run from automake srcdir is set, but if run manually then not */
		srcdir = SRCDIR;
	}

	str = cli_malloc(strlen(name)+strlen(srcdir)+2);
	fail_unless(!!str, "cli_malloc");
	sprintf(str, "%s/%s", srcdir, name);

	fd = open(str, O_RDONLY);
	fail_unless_fmt(fd >= 0, "open() failed: %s", str);
	free(str);
	return fd;
}

void diff_file_mem(int fd, const char *ref, size_t len)
{
	char c1,c2;
	size_t p, reflen = len;
	char *buf = cli_malloc(len);

	fail_unless_fmt(!!buf, "unable to malloc buffer: %d", len);
	p = read(fd, buf, len);
	fail_unless_fmt(p == len,  "file is smaller: %lu, expected: %lu", p, len);
	p = 0;
	while(len > 0) {
		c1 = ref[p];
		c2 = buf[p];
		if(c1 != c2)
			break;
		p++;
		len--;
	}
	if (len > 0)
		fail_unless_fmt(c1 == c2, "file contents mismatch at byte: %lu, was: %c, expected: %c", p, c2, c1);
	free(buf);
	p = lseek(fd, 0, SEEK_END);
        fail_unless_fmt(p == reflen, "trailing garbage, file size: %ld, expected: %ld", p, reflen);
	close(fd);
}

void diff_files(int fd, int ref_fd)
{
	char *ref;
	ssize_t nread;
	off_t siz = lseek(ref_fd, 0, SEEK_END);
	fail_unless_fmt(siz != -1, "lseek failed");

	ref = cli_malloc(siz);
	fail_unless_fmt(!!ref, "unable to malloc buffer: %d", siz);

	fail_unless_fmt(lseek(ref_fd, 0, SEEK_SET) == 0,"lseek failed");
	nread = read(ref_fd, ref, siz);
        fail_unless_fmt(nread == siz, "short read, expected: %ld, was: %ld", siz, nread);
	close(ref_fd);
	diff_file_mem(fd, ref, siz);
	free(ref);
}

#ifdef USE_MPOOL
static mpool_t *pool;
#else
static void *pool;
#endif
struct cli_dconf *dconf;

void dconf_setup(void)
{
	pool = NULL;
	dconf = NULL;
#ifdef USE_MPOOL
	pool = mpool_create();
	fail_unless(!!pool, "unable to create pool");
#endif
	dconf = cli_mpool_dconf_init(pool);
	fail_unless(!!dconf, "failed to init dconf");
}

void dconf_teardown(void)
{
	mpool_free(pool, dconf);
#ifdef USE_MPOOL
	if (pool)
		mpool_destroy(pool);
#endif
}


int main(void)
{
    int nf;
    Suite *s = test_cl_suite();
    SRunner *sr = srunner_create(s);
#ifdef CHECK_HAVE_LOOPS
    srunner_add_suite(sr, test_cli_suite());
#else
    printf("*** Warning ***: your check version is too old,\nseveral important tests will not execute\n");
#endif
    srunner_add_suite(sr, test_jsnorm_suite());
    srunner_add_suite(sr, test_str_suite());
    srunner_add_suite(sr, test_regex_suite());
    srunner_add_suite(sr, test_disasm_suite());
    srunner_add_suite(sr, test_uniq_suite());
    srunner_add_suite(sr, test_matchers_suite());
    srunner_add_suite(sr, test_htmlnorm_suite());

    srunner_set_log(sr, "test.log");
    if(freopen("test-stderr.log","w+",stderr) == NULL) {
	    fputs("Unable to redirect stderr!\n",stderr);
    }
    cl_debug();

    srunner_run_all(sr, CK_NORMAL);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
