/* Copyright (C) 1979-2002 MySQL AB
   
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

/* Pack isam file */

#ifndef USE_MY_FUNC
#define USE_MY_FUNC			/* We nead at least my_malloc */
#endif

#include "isamdef.h"
#include <queues.h>
#include <my_tree.h>
#include "mysys_err.h"
#ifdef MSDOS
#include <io.h>
#endif
#ifndef __GNU_LIBRARY__
#define __GNU_LIBRARY__			/* Skipp warnings in getopt.h */
#endif
#include <my_getopt.h>

#if INT_MAX > 32767
#define BITS_SAVED 32
#else
#define BITS_SAVED 16
#endif

#define IS_OFFSET ((uint) 32768)	/* Bit if offset or char in tree */
#define HEAD_LENGTH	32
#define ALLOWED_JOIN_DIFF	256	/* Diff allowed to join trees */

#define DATA_TMP_EXT		".TMD"
#define OLD_EXT			".OLD"
#define WRITE_COUNT		MY_HOW_OFTEN_TO_WRITE

#ifdef __WIN__
static double ulonglong2double(ulonglong value)
{
  longlong nr=(longlong) value;
  if (nr >= 0)
    return (double) nr;
  return (18446744073709551616.0 + (double) nr);
}

#if SIZEOF_OFF_T > 4
#define my_off_t2double(A) ulonglong2double(A)
#else
#define my_off_t2double(A) ((double) (A))
#endif /* SIZEOF_OFF_T > 4 */
#endif

struct st_file_buffer {
  File file;
  char *buffer,*pos,*end;
  my_off_t pos_in_file;
  int bits;
  uint bytes;
};

struct st_huff_tree;
struct st_huff_element;

typedef struct st_huff_counts {
  uint	field_length,max_zero_fill;
  uint	pack_type;
  uint	max_end_space,max_pre_space,length_bits,min_space;
  enum en_fieldtype field_type;
  struct st_huff_tree *tree;		/* Tree for field */
  my_off_t counts[256];
  my_off_t end_space[8];
  my_off_t pre_space[8];
  my_off_t tot_end_space,tot_pre_space,zero_fields,empty_fields,bytes_packed;
  TREE	int_tree;
  byte *tree_buff;
  byte *tree_pos;
} HUFF_COUNTS;

typedef struct st_huff_element HUFF_ELEMENT;

struct st_huff_element {
  my_off_t count;
  union un_element {
    struct st_nod {
      HUFF_ELEMENT *left,*right;
    } nod;
    struct st_leaf {
      HUFF_ELEMENT *null;
      uint	element_nr;		/* Number of element */
    } leaf;
  } a;
};


typedef struct st_huff_tree {
  HUFF_ELEMENT *root,*element_buffer;
  HUFF_COUNTS *counts;
  uint tree_number;
  uint elements;
  my_off_t bytes_packed;
  uint tree_pack_length;
  uint min_chr,max_chr,char_bits,offset_bits,max_offset,height;
  ulong *code;
  uchar *code_len;
} HUFF_TREE;


typedef struct st_isam_mrg {
  N_INFO **file,**current,**end;
  uint count;
  uint	min_pack_length;		/* Theese is used by packed data */
  uint	max_pack_length;
  uint	ref_length;
  my_off_t records;
} MRG_INFO;


extern int main(int argc,char * *argv);
static void get_options(int *argc,char ***argv);
static N_INFO *open_isam_file(char *name,int mode);
static bool open_isam_files(MRG_INFO *mrg,char **names,uint count);
static int compress(MRG_INFO *file,char *join_name);
static HUFF_COUNTS *init_huff_count(N_INFO *info,my_off_t records);
static void free_counts_and_tree_and_queue(HUFF_TREE *huff_trees,
					   uint trees,
					   HUFF_COUNTS *huff_counts,
					   uint fields);
static int compare_tree(const uchar *s,const uchar *t);
static int get_statistic(MRG_INFO *mrg,HUFF_COUNTS *huff_counts);
static void check_counts(HUFF_COUNTS *huff_counts,uint trees,
			 my_off_t records);
static int test_space_compress(HUFF_COUNTS *huff_counts,my_off_t records,
			       uint max_space_length,my_off_t *space_counts,
			       my_off_t tot_space_count,
			       enum en_fieldtype field_type);
static HUFF_TREE* make_huff_trees(HUFF_COUNTS *huff_counts,uint trees);
static int make_huff_tree(HUFF_TREE *tree,HUFF_COUNTS *huff_counts);
static int compare_huff_elements(void *not_used, byte *a,byte *b);
static int save_counts_in_queue(byte *key,element_count count,
				    HUFF_TREE *tree);
static my_off_t calc_packed_length(HUFF_COUNTS *huff_counts,uint flag);
static uint join_same_trees(HUFF_COUNTS *huff_counts,uint trees);
static int make_huff_decode_table(HUFF_TREE *huff_tree,uint trees);
static void make_traverse_code_tree(HUFF_TREE *huff_tree,
				    HUFF_ELEMENT *element,uint size,
				    ulong code);
static int write_header(MRG_INFO *isam_file, uint header_length,uint trees,
			my_off_t tot_elements,my_off_t filelength);
static void write_field_info(HUFF_COUNTS *counts, uint fields,uint trees);
static my_off_t write_huff_tree(HUFF_TREE *huff_tree,uint trees);
static uint *make_offset_code_tree(HUFF_TREE *huff_tree,
				       HUFF_ELEMENT *element,
				       uint *offset);
static uint max_bit(uint value);
static int compress_isam_file(MRG_INFO *file,HUFF_COUNTS *huff_counts);
static char *make_new_name(char *new_name,char *old_name);
static char *make_old_name(char *new_name,char *old_name);
static void init_file_buffer(File file,pbool read_buffer);
static int flush_buffer(uint neaded_length);
static void end_file_buffer(void);
static void write_bits(ulong value,uint bits);
static void flush_bits(void);
static void save_integer(byte *pos,uint pack_length,my_off_t value);
static void save_state(N_INFO *isam_file,MRG_INFO *mrg,my_off_t new_length,
		       ulong crc);
static int save_state_mrg(File file,MRG_INFO *isam_file,my_off_t new_length,
			  ulong crc);
static int mrg_close(MRG_INFO *mrg);
static int mrg_rrnd(MRG_INFO *info,byte *buf);
static void mrg_reset(MRG_INFO *mrg);


static int error_on_write=0,test_only=0,verbose=0,silent=0,
	   write_loop=0,force_pack=0,isamchk_neaded=0;
static my_bool backup, opt_wait;
static int tmpfile_createflag=O_RDWR | O_TRUNC | O_EXCL;
static uint tree_buff_length=8196-MALLOC_OVERHEAD,force_pack_ref_length;
static char tmp_dir[FN_REFLEN]={0},*join_table;
static my_off_t intervall_length;
static ulong  crc;
static struct st_file_buffer file_buffer;
static QUEUE queue;
static HUFF_COUNTS *global_count;
static char zero_string[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const char *load_default_groups[]= { "pack_isam",0 };

	/* The main program */

int main(int argc, char **argv)
{
  int error,ok;
  MRG_INFO merge;
  MY_INIT(argv[0]);

  load_defaults("my",load_default_groups,&argc,&argv);
  get_options(&argc,&argv);

  error=ok=isamchk_neaded=0;
  if (join_table)
  {						/* Join files into one */
    if (open_isam_files(&merge,argv,(uint) argc) ||
	compress(&merge,join_table))
      error=1;
  }
  else while (argc--)
  {
    N_INFO *isam_file;
    if (!(isam_file=open_isam_file(*argv++,O_RDWR)))
      error=1;
    else
    {
      merge.file= &isam_file;
      merge.current=0;
      merge.count=1;
      if (compress(&merge,0))
	error=1;
      else
	ok=1;
    }
  }
  if (ok && isamchk_neaded && !silent)
    puts("Remember to run isamchk -rq on compressed databases");
  VOID(fflush(stdout)); VOID(fflush(stderr));
  my_end(verbose ? MY_CHECK_ERROR | MY_GIVE_INFO : MY_CHECK_ERROR);
  exit(error ? 2 : 0);
#ifndef _lint
  return 0;					/* No compiler warning */
#endif
}


static struct my_option my_long_options[] =
{
  {"backup", 'b', "Make a backup of the table as table_name.OLD",
   (gptr*) &backup, (gptr*) &backup, 0, GET_BOOL, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"debug", '#', "Output debug log. Often this is 'd:t:o,filename'",
   0, 0, 0, GET_STR, OPT_ARG, 0, 0, 0, 0, 0, 0},
  {"force", 'f',
   "Force packing of table even if it's gets bigger or tempfile exists.",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"join", 'j',
   "Join all given tables into 'new_table_name'. All tables MUST have the identical layout.",
   (gptr*) &join_table, (gptr*) &join_table, 0, GET_STR, REQUIRED_ARG, 0, 0,
   0, 0, 0, 0},
  {"help", '?', "Display this help and exit.",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0},
  {"packlength", 'p', "Force storage size of recordlength (1, 2 or 3)",
   (gptr*) &force_pack_ref_length, (gptr*) &force_pack_ref_length, 0,
   GET_UINT, REQUIRED_ARG, 0, 0, 0, 0, 0, 0},
  {"silent", 's', "Be more silent.",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0 ,0},
  {"tmpdir", 'T', "Use temporary directory to store temporary table",
   (gptr*) &tmp_dir, (gptr*) &tmp_dir, 0, GET_STR, REQUIRED_ARG, 0, 0, 0, 0,
   0, 0},
  {"test", 't', "Don't pack table, only test packing it",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0 ,0},
  {"verbose", 'v', "Write info about progress and packing result",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0 ,0},
  {"version", 'V', "output version information and exit",
   0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0 ,0},
  {"wait", 'w', "Wait and retry if table is in use", (gptr*) &opt_wait,
   (gptr*) &opt_wait, 0, GET_BOOL, NO_ARG, 0, 0, 0, 0, 0, 0},
  { 0, 0, 0, 0, 0, 0, GET_NO_ARG, NO_ARG, 0, 0, 0, 0, 0, 0}
};


static void print_version(void)
{
  printf("%s Ver 5.10 for %s on %s\n", my_progname, SYSTEM_TYPE, MACHINE_TYPE);
}

static void usage(void)
{
  print_version();
  puts("Copyright (C) 2002 MySQL AB");
  puts("This software comes with ABSOLUTELY NO WARRANTY. This is free software,");
  puts("and you are welcome to modify and redistribute it under the GPL license\n");

  puts("Pack a ISAM-table to take much smaller space");
  puts("Keys are not updated, so you must run isamchk -rq on any table");
  puts("that has keys after you have compressed it");
  puts("You should give the .ISM file as the filename argument");

  printf("\nUsage: %s [OPTIONS] filename...\n", my_progname);
  my_print_help(my_long_options);
  print_defaults("my", load_default_groups);
  my_print_variables(my_long_options);
}


static my_bool
get_one_option(int optid, const struct my_option *opt __attribute__((unused)),
	       char *argument)
{
  uint length;

  switch(optid) {
  case 'f':
    force_pack= 1;
    tmpfile_createflag= O_RDWR | O_TRUNC;
    break;
  case 'p':
    if (force_pack_ref_length > 3)
      force_pack_ref_length= 0;
    break;
  case 's':
    write_loop= verbose= 0;
    silent= 1;
    break;
  case 't':
    test_only= verbose= 1;
    break;
  case 'T':
    length=(uint) (strmov(tmp_dir, argument) - tmp_dir);
    if (length != dirname_length(tmp_dir))
    {
      tmp_dir[length]= FN_LIBCHAR;
      tmp_dir[length + 1]= 0;
    }
    break;
  case 'v':
    verbose= 1;
    silent= 0;
    break;
  case '#':
    DBUG_PUSH(argument ? argument : "d:t:o");
    break;
  case 'V': print_version(); exit(0);
  case 'I':
  case '?':
    usage();
    exit(0);
  }
  return 0;
}

	/* reads options */
	/* Initiates DEBUG - but no debugging here ! */

static void get_options(int *argc, char ***argv)
{
  int ho_error;

  if ((ho_error=handle_options(argc, argv, my_long_options, get_one_option)))
    exit(ho_error);

  my_progname= argv[0][0];
  if (isatty(fileno(stdout)))
    write_loop=1;

  if (!*argc)
  {
    usage();
    exit(1);
  }
  if (join_table)
  {
    backup=0;					/* Not needed */
    tmp_dir[0]=0;
  }
  return;
}


static N_INFO *open_isam_file(char *name,int mode)
{
  N_INFO *isam_file;
  ISAM_SHARE *share;
  DBUG_ENTER("open_isam_file");

  if (!(isam_file=nisam_open(name,mode,(opt_wait ? HA_OPEN_WAIT_IF_LOCKED :
				     HA_OPEN_ABORT_IF_LOCKED))))
  {
    VOID(fprintf(stderr,"%s gave error %d on open\n",name,my_errno));
    DBUG_RETURN(0);
  }
  share=isam_file->s;
  if (share->base.blobs)
  {
    VOID(fprintf(stderr,"%s has blobs, can't pack it\n",name));
    VOID(nisam_close(isam_file));
    DBUG_RETURN(0);
  }
  if (share->base.options & HA_OPTION_COMPRESS_RECORD && !join_table)
  {
    if (!force_pack)
    {
      VOID(fprintf(stderr,"%s is already compressed\n",name));
      VOID(nisam_close(isam_file));
      DBUG_RETURN(0);
    }
    if (verbose)
      puts("Recompressing already compressed table");
    share->base.options&= ~HA_OPTION_READ_ONLY_DATA; /* We are modifing it */
  }
  if (! force_pack && share->state.records != 0 &&
      (share->state.records <= 1 ||
       share->state.data_file_length < 1024) && ! join_table)
  {
    VOID(fprintf(stderr,"%s is too small to compress\n",name));
    VOID(nisam_close(isam_file));
    DBUG_RETURN(0);
  }
  VOID(nisam_lock_database(isam_file,F_WRLCK));
  DBUG_RETURN(isam_file);
}


static bool open_isam_files(MRG_INFO *mrg,char **names,uint count)
{
  uint i,j;
  mrg->count=0;
  mrg->current=0;
  mrg->file=(N_INFO**) my_malloc(sizeof(N_INFO*)*count,MYF(MY_FAE));
  for (i=0; i < count ; i++)
  {
    if (!(mrg->file[i]=open_isam_file(names[i],O_RDONLY)))
      goto error;
  }
  /* Check that files are identical */
  for (j=0 ; j < count-1 ; j++)
  {
    N_RECINFO *m1,*m2,*end;
    if (mrg->file[j]->s->base.reclength != mrg->file[j+1]->s->base.reclength ||
	mrg->file[j]->s->base.fields != mrg->file[j+1]->s->base.fields)
      goto diff_file;
    m1=mrg->file[j]->s->rec;
    end=m1+mrg->file[j]->s->base.fields;
    m2=mrg->file[j+1]->s->rec;
    for ( ; m1 != end ; m1++,m2++)
    {
      if ((m1->base.type != m2->base.type && ! force_pack) ||
	  m1->base.length != m2->base.length)
	goto diff_file;
    }
  }
  mrg->count=count;
  return 0;

 diff_file:
  fprintf(stderr,"%s: Tables '%s' and '%s' are not identical\n",
	  my_progname,names[j],names[j+1]);
 error:
  while (i--)
    nisam_close(mrg->file[i]);
  return 1;
}


static int compress(MRG_INFO *mrg,char *result_table)
{
  int error;
  File new_file,join_isam_file;
  N_INFO *isam_file;
  ISAM_SHARE *share;
  char org_name[FN_REFLEN],new_name[FN_REFLEN],temp_name[FN_REFLEN];
  uint i,header_length,fields,trees,used_trees;
  my_off_t old_length,new_length,tot_elements;
  HUFF_COUNTS *huff_counts;
  HUFF_TREE *huff_trees;
  DBUG_ENTER("compress");

  isam_file=mrg->file[0];			/* Take this as an example */
  share=isam_file->s;
  new_file=join_isam_file= -1;
  trees=fields=0;
  huff_trees=0;
  huff_counts=0;

  /* Create temporary or join file */

  if (backup)
    VOID(fn_format(org_name,isam_file->filename,"",N_NAME_DEXT,2));
  else
    VOID(fn_format(org_name,isam_file->filename,"",N_NAME_DEXT,2+4+16));
  if (!test_only && result_table)
  {
    /* Make a new indexfile based on first file in list */
    uint length;
    char *buff;
    strmov(org_name,result_table);		/* Fix error messages */
    VOID(fn_format(new_name,result_table,"",N_NAME_IEXT,2));
    if ((join_isam_file=my_create(new_name,0,tmpfile_createflag,MYF(MY_WME)))
	< 0)
      goto err;
    length=share->base.keystart;
    if (!(buff=my_malloc(length,MYF(MY_WME))))
      goto err;
    if (my_pread(share->kfile,buff,length,0L,MYF(MY_WME | MY_NABP)) ||
	my_write(join_isam_file,buff,length,
		 MYF(MY_WME | MY_NABP | MY_WAIT_IF_FULL)))
    {
      my_free(buff,MYF(0));
      goto err;
    }
    my_free(buff,MYF(0));
    VOID(fn_format(new_name,result_table,"",N_NAME_DEXT,2));
  }
  else if (!tmp_dir[0])
    VOID(make_new_name(new_name,org_name));
  else
    VOID(fn_format(new_name,org_name,tmp_dir,DATA_TMP_EXT,1+2+4));
  if (!test_only &&
      (new_file=my_create(new_name,0,tmpfile_createflag,MYF(MY_WME))) < 0)
    goto err;

  /* Start calculating statistics */

  mrg->records=0;
  for (i=0 ; i < mrg->count ; i++)
    mrg->records+=mrg->file[i]->s->state.records;
  if (write_loop || verbose)
  {
    printf("Compressing %s: (%lu records)\n",
	   result_table ? new_name : org_name,(ulong) mrg->records);
  }
  trees=fields=share->base.fields;
  huff_counts=init_huff_count(isam_file,mrg->records);
  QUICK_SAFEMALLOC;
  if (write_loop || verbose)
    printf("- Calculating statistics\n");
  if (get_statistic(mrg,huff_counts))
    goto err;
  NORMAL_SAFEMALLOC;
  old_length=0;
  for (i=0; i < mrg->count ; i++)
    old_length+= (mrg->file[i]->s->state.data_file_length -
		  mrg->file[i]->s->state.empty);

  if (init_queue(&queue,256,0,0,compare_huff_elements,0))
    goto err;
  check_counts(huff_counts,fields,mrg->records);
  huff_trees=make_huff_trees(huff_counts,trees);
  if ((int) (used_trees=join_same_trees(huff_counts,trees)) < 0)
    goto err;
  if (make_huff_decode_table(huff_trees,fields))
    goto err;

  init_file_buffer(new_file,0);
  file_buffer.pos_in_file=HEAD_LENGTH;
  if (! test_only)
    VOID(my_seek(new_file,file_buffer.pos_in_file,MY_SEEK_SET,MYF(0)));

  write_field_info(huff_counts,fields,used_trees);
  if (!(tot_elements=write_huff_tree(huff_trees,trees)))
    goto err;
  header_length=(uint) file_buffer.pos_in_file+
    (uint) (file_buffer.pos-file_buffer.buffer);

  /* Compress file */
  if (write_loop || verbose)
    printf("- Compressing file\n");
  error=compress_isam_file(mrg,huff_counts);
  new_length=file_buffer.pos_in_file;
  if (!error && !test_only)
  {
    char buff[MEMMAP_EXTRA_MARGIN];		/* End marginal for memmap */
    bzero(buff,sizeof(buff));
    error=my_write(file_buffer.file,buff,sizeof(buff),
		   MYF(MY_WME | MY_NABP | MY_WAIT_IF_FULL)) != 0;
  }
  if (!error)
    error=write_header(mrg,header_length,used_trees,tot_elements,
		       new_length);
  end_file_buffer();

  if (verbose && mrg->records)
    printf("Min record length: %6d   Max length: %6d   Mean total length: %6lu\n",
	   mrg->min_pack_length,mrg->max_pack_length,
	   (ulong) (new_length/mrg->records));

  if (!test_only)
  {
    error|=my_close(new_file,MYF(MY_WME));
    if (!result_table)
    {
      error|=my_close(isam_file->dfile,MYF(MY_WME));
      isam_file->dfile= -1;		/* Tell nisam_close file is closed */
    }
  }

  free_counts_and_tree_and_queue(huff_trees,trees,huff_counts,fields);
  if (! test_only && ! error)
  {
    if (result_table)
    {
      error=save_state_mrg(join_isam_file,mrg,new_length,crc);
    }
    else
    {
      if (backup)
      {
	if (my_rename(org_name,make_old_name(temp_name,isam_file->filename),
		      MYF(MY_WME)))
	  error=1;
	else
	{
	  if (tmp_dir[0])
	  {
	    if (!(error=my_copy(new_name,org_name,MYF(MY_WME))))
	      VOID(my_delete(new_name,MYF(MY_WME)));
	  }
	  else
	    error=my_rename(new_name,org_name,MYF(MY_WME));
	  if (!error)
	    VOID(my_copystat(temp_name,org_name,MYF(MY_COPYTIME)));
	}
      }
      else
      {
	if (tmp_dir[0])
	{

	  if (!(error=my_copy(new_name,org_name,
			      MYF(MY_WME | MY_HOLD_ORIGINAL_MODES
				  | MY_COPYTIME))))
	    VOID(my_delete(new_name,MYF(MY_WME)));
	}
	else
	  error=my_redel(org_name,new_name,MYF(MY_WME | MY_COPYTIME));
      }
      if (! error)
	save_state(isam_file,mrg,new_length,crc);
    }
  }
  error|=mrg_close(mrg);
  if (join_isam_file >= 0)
    error|=my_close(join_isam_file,MYF(MY_WME));
  if (error)
  {
    VOID(fprintf(stderr,"Aborting: %s is not compressed\n",org_name));
    DBUG_RETURN(-1);
  }
  if (write_loop || verbose)
  {
    if (old_length)
      printf("%.4g%%     \n",
	     my_off_t2double(old_length-new_length)*100.0/
	     my_off_t2double(old_length));
    else
      puts("Empty file saved in compressed format");
  }
  DBUG_RETURN(0);

 err:
  free_counts_and_tree_and_queue(huff_trees,trees,huff_counts,fields);
  if (new_file >= 0)
    VOID(my_close(new_file,MYF(0)));
  if (join_isam_file >= 0)
    VOID(my_close(join_isam_file,MYF(0)));
  mrg_close(mrg);
  VOID(fprintf(stderr,"Aborted: %s is not compressed\n",org_name));
  DBUG_RETURN(-1);
}

	/* Init a huff_count-struct for each field and init it */

static HUFF_COUNTS *init_huff_count(N_INFO *info,my_off_t records)
{
  reg2 uint i;
  reg1 HUFF_COUNTS *count;
  if ((count = (HUFF_COUNTS*) my_malloc(info->s->base.fields*sizeof(HUFF_COUNTS),
					MYF(MY_ZEROFILL | MY_WME))))
  {
    for (i=0 ; i < info->s->base.fields ; i++)
    {
      enum en_fieldtype type;
      count[i].field_length=info->s->rec[i].base.length;
      type= count[i].field_type= (enum en_fieldtype) info->s->rec[i].base.type;
      if (type == FIELD_INTERVALL ||
	  type == FIELD_CONSTANT ||
	  type == FIELD_ZERO)
	type = FIELD_NORMAL;
      if (count[i].field_length <= 8 &&
	  (type == FIELD_NORMAL ||
	   type == FIELD_SKIP_ZERO))
	count[i].max_zero_fill= count[i].field_length;
      init_tree(&count[i].int_tree,0,0,-1,(qsort_cmp2) compare_tree,0,NULL,NULL);
      if (records)
	count[i].tree_pos=count[i].tree_buff =
	  my_malloc(count[i].field_length > 1 ? tree_buff_length : 2,
		    MYF(MY_WME));
    }
  }
  return count;
}


	/* Free memory used by counts and trees */

static void free_counts_and_tree_and_queue(HUFF_TREE *huff_trees, uint trees, HUFF_COUNTS *huff_counts, uint fields)
{
  register uint i;

  if (huff_trees)
  {
    for (i=0 ; i < trees ; i++)
    {
      if (huff_trees[i].element_buffer)
	my_free((gptr) huff_trees[i].element_buffer,MYF(0));
      if (huff_trees[i].code)
	my_free((gptr) huff_trees[i].code,MYF(0));
    }
    my_free((gptr) huff_trees,MYF(0));
  }
  if (huff_counts)
  {
    for (i=0 ; i < fields ; i++)
    {
      if (huff_counts[i].tree_buff)
      {
	my_free((gptr) huff_counts[i].tree_buff,MYF(0));
	delete_tree(&huff_counts[i].int_tree);
      }
    }
    my_free((gptr) huff_counts,MYF(0));
  }
  delete_queue(&queue);		/* This is safe to free */
  return;
}

	/* Read through old file and gather some statistics */

static int get_statistic(MRG_INFO *mrg,HUFF_COUNTS *huff_counts)
{
  int error;
  uint length,reclength;
  byte *record,*pos,*next_pos,*end_pos,*start_pos;
  my_off_t record_count;
  HUFF_COUNTS *count,*end_count;
  TREE_ELEMENT *element;
  DBUG_ENTER("get_statistic");

  reclength=mrg->file[0]->s->base.reclength;
  record=(byte*) my_alloca(reclength);
  end_count=huff_counts+mrg->file[0]->s->base.fields;
  record_count=crc=0;

  mrg_reset(mrg);
  while ((error=mrg_rrnd(mrg,record)) >= 0)
  {
    if (! error)
    {
      crc^=checksum(record,reclength);
      for (pos=record,count=huff_counts ;
	   count < end_count ;
	   count++,
	   pos=next_pos)
      {
	next_pos=end_pos=(start_pos=pos)+count->field_length;

	/* Put value in tree if there is room for it */
	if (count->tree_buff)
	{
	  global_count=count;
	  if (!(element=tree_insert(&count->int_tree,pos,0)) ||
	      ((element->count == 1 &&
	       count->tree_buff + tree_buff_length <
	       count->tree_pos + count->field_length) ||
	       (count->field_length == 1 &&
		count->int_tree.elements_in_tree > 1)))
	  {
	    delete_tree(&count->int_tree);
	    my_free(count->tree_buff,MYF(0));
	    count->tree_buff=0;
	  }
	  else
	  {
	    if (element->count == 1)
	    {				/* New element */
	      memcpy(count->tree_pos,pos,(size_t) count->field_length);
	      tree_set_pointer(element,count->tree_pos);
	      count->tree_pos+=count->field_length;
	    }
	  }
	}

	/* Save character counters and space-counts and zero-field-counts */
	if (count->field_type == FIELD_NORMAL ||
	    count->field_type == FIELD_SKIP_ENDSPACE)
	{
	  for ( ; end_pos > pos ; end_pos--)
	    if (end_pos[-1] != ' ')
	      break;
	  if (end_pos == pos)
	  {
	    count->empty_fields++;
	    count->max_zero_fill=0;
	    continue;
	  }
	  length= (uint) (next_pos-end_pos);
	  count->tot_end_space+=length;
	  if (length < 8)
	    count->end_space[length]++;
	  if (count->max_end_space < length)
	    count->max_end_space = length;
	}
	if (count->field_type == FIELD_NORMAL ||
	    count->field_type == FIELD_SKIP_PRESPACE)
	{
	  for (pos=start_pos; pos < end_pos ; pos++)
	    if (pos[0] != ' ')
	      break;
	  if (end_pos == pos)
	  {
	    count->empty_fields++;
	    count->max_zero_fill=0;
	    continue;
	  }
	  length= (uint) (pos-start_pos);
	  count->tot_pre_space+=length;
	  if (length < 8)
	    count->pre_space[length]++;
	  if (count->max_pre_space < length)
	    count->max_pre_space = length;
	}
	if (count->field_length <= 8 &&
	    (count->field_type == FIELD_NORMAL ||
	     count->field_type == FIELD_SKIP_ZERO))
	{
	  uint i;
	  if (!memcmp((byte*) start_pos,zero_string,count->field_length))
	  {
	    count->zero_fields++;
	    continue;
	  }
#ifdef BYTE_ORDER_HIGH_FIRST
	  for (i =0 ; i < count->max_zero_fill && ! start_pos[i] ; i++) ;
	  if (i < count->max_zero_fill)
	    count->max_zero_fill=i;
#else
	  for (i =0 ; i < count->max_zero_fill && ! end_pos[-1 - (int) i] ; i++) ;
	  if (i < count->max_zero_fill)
	    count->max_zero_fill=i;
#endif
	}
	for (pos=start_pos ; pos < end_pos ; pos++)
	  count->counts[(uchar) *pos]++;
      }
      record_count++;
      if (write_loop && record_count % WRITE_COUNT == 0)
      {
	printf("%lu\r",(ulong) record_count); VOID(fflush(stdout));
      }
    }
  }
  if (write_loop)
  {
    printf("            \r"); VOID(fflush(stdout));
  }
  mrg->records=record_count;
  my_afree((gptr) record);
  DBUG_RETURN(0);
}

static int compare_huff_elements(void *not_used __attribute__((unused)),
				 byte *a, byte *b)
{
  return *((my_off_t*) a) < *((my_off_t*) b) ? -1 :
    (*((my_off_t*) a) == *((my_off_t*) b)  ? 0 : 1);
}

	/* Check each tree if we should use pre-space-compress, end-space-
	   compress, empty-field-compress or zero-field-compress */

static void check_counts(HUFF_COUNTS *huff_counts, uint trees, my_off_t records)
{
  uint space_fields,fill_zero_fields,field_count[(int) FIELD_ZERO+1];
  my_off_t old_length,new_length,length;
  DBUG_ENTER("check_counts");

  bzero((gptr) field_count,sizeof(field_count));
  space_fields=fill_zero_fields=0;

  for (; trees-- ; huff_counts++)
  {
    huff_counts->field_type=FIELD_NORMAL;
    huff_counts->pack_type=0;

    if (huff_counts->zero_fields || ! records)
    {
      my_off_t old_space_count;
      if (huff_counts->zero_fields == records)
      {
	huff_counts->field_type= FIELD_ZERO;
	huff_counts->bytes_packed=0;
	huff_counts->counts[0]=0;
	goto found_pack;
      }
      old_space_count=huff_counts->counts[' '];
      huff_counts->counts[' ']+=huff_counts->tot_end_space+
	huff_counts->tot_pre_space +
	  huff_counts->empty_fields * huff_counts->field_length;
      old_length=calc_packed_length(huff_counts,0)+records/8;
      length=huff_counts->zero_fields*huff_counts->field_length;
      huff_counts->counts[0]+=length;
      new_length=calc_packed_length(huff_counts,0);
      if (old_length < new_length && huff_counts->field_length > 1)
      {
	huff_counts->field_type=FIELD_SKIP_ZERO;
	huff_counts->counts[0]-=length;
	huff_counts->bytes_packed=old_length- records/8;
	goto found_pack;
      }
      huff_counts->counts[' ']=old_space_count;
    }
    huff_counts->bytes_packed=calc_packed_length(huff_counts,0);
    if (huff_counts->empty_fields)
    {
      if (huff_counts->field_length > 2 &&
	  huff_counts->empty_fields + (records - huff_counts->empty_fields)*
	  (1+max_bit(max(huff_counts->max_pre_space,
			 huff_counts->max_end_space))) <
	  records * max_bit(huff_counts->field_length))
      {
	huff_counts->pack_type |= PACK_TYPE_SPACE_FIELDS;
      }
      else
      {
	length=huff_counts->empty_fields*huff_counts->field_length;
	if (huff_counts->tot_end_space || ! huff_counts->tot_pre_space)
	{
	  huff_counts->tot_end_space+=length;
	  huff_counts->max_end_space=huff_counts->field_length;
	  if (huff_counts->field_length < 8)
	    huff_counts->end_space[huff_counts->field_length]+=
	      huff_counts->empty_fields;
	}
	else
	{
	  huff_counts->tot_pre_space+=length;
	  huff_counts->max_pre_space=huff_counts->field_length;
	  if (huff_counts->field_length < 8)
	    huff_counts->pre_space[huff_counts->field_length]+=
	      huff_counts->empty_fields;
	}
      }
    }
    if (huff_counts->tot_end_space)
    {
      huff_counts->counts[' ']+=huff_counts->tot_pre_space;
      if (test_space_compress(huff_counts,records,huff_counts->max_end_space,
			      huff_counts->end_space,
			      huff_counts->tot_end_space,FIELD_SKIP_ENDSPACE))
	goto found_pack;
      huff_counts->counts[' ']-=huff_counts->tot_pre_space;
    }
    if (huff_counts->tot_pre_space)
    {
      if (test_space_compress(huff_counts,records,huff_counts->max_pre_space,
			      huff_counts->pre_space,
			      huff_counts->tot_pre_space,FIELD_SKIP_PRESPACE))
	goto found_pack;
    }

  found_pack:			/* Found field-packing */

    /* Test if we can use zero-fill */

    if (huff_counts->max_zero_fill &&
	(huff_counts->field_type == FIELD_NORMAL ||
	 huff_counts->field_type == FIELD_SKIP_ZERO))
    {
      huff_counts->counts[0]-=huff_counts->max_zero_fill*
	(huff_counts->field_type == FIELD_SKIP_ZERO ?
	 records - huff_counts->zero_fields : records);
      huff_counts->pack_type|=PACK_TYPE_ZERO_FILL;
      huff_counts->bytes_packed=calc_packed_length(huff_counts,0);
    }

    /* Test if intervall-field is better */

    if (huff_counts->tree_buff)
    {
      HUFF_TREE tree;

      tree.element_buffer=0;
      if (!make_huff_tree(&tree,huff_counts) &&
	  tree.bytes_packed+tree.tree_pack_length < huff_counts->bytes_packed)
      {
	if (tree.elements == 1)
	  huff_counts->field_type=FIELD_CONSTANT;
	else
	  huff_counts->field_type=FIELD_INTERVALL;
	huff_counts->pack_type=0;
      }
      else
      {
	my_free((gptr) huff_counts->tree_buff,MYF(0));
	delete_tree(&huff_counts->int_tree);
	huff_counts->tree_buff=0;
      }
      if (tree.element_buffer)
	my_free((gptr) tree.element_buffer,MYF(0));
    }
    if (huff_counts->pack_type & PACK_TYPE_SPACE_FIELDS)
      space_fields++;
    if (huff_counts->pack_type & PACK_TYPE_ZERO_FILL)
      fill_zero_fields++;
    field_count[huff_counts->field_type]++;
  }
  if (verbose)
    printf("\nnormal:    %3d  empty-space:     %3d  empty-zero:       %3d  empty-fill: %3d\npre-space: %3d  end-space:       %3d  table-lookup:     %3d  zero:       %3d\n",
	   field_count[FIELD_NORMAL],space_fields,
	   field_count[FIELD_SKIP_ZERO],fill_zero_fields,
	   field_count[FIELD_SKIP_PRESPACE],
	   field_count[FIELD_SKIP_ENDSPACE],
	   field_count[FIELD_INTERVALL],
	   field_count[FIELD_ZERO]);
  DBUG_VOID_RETURN;
}

	/* Test if we can use space-compression and empty-field-compression */

static int
test_space_compress(HUFF_COUNTS *huff_counts, my_off_t records,
		    uint max_space_length, my_off_t *space_counts,
		    my_off_t tot_space_count, enum en_fieldtype field_type)
{
  int min_pos;
  uint length_bits,i;
  my_off_t space_count,min_space_count,min_pack,new_length,skipp;

  length_bits=max_bit(max_space_length);

		/* Default no end_space-packing */
  space_count=huff_counts->counts[(uint) ' '];
  min_space_count= (huff_counts->counts[(uint) ' ']+= tot_space_count);
  min_pack=calc_packed_length(huff_counts,0);
  min_pos= -2;
  huff_counts->counts[(uint) ' ']=space_count;

	/* Test with allways space-count */
  new_length=huff_counts->bytes_packed+length_bits*records/8;
  if (new_length+1 < min_pack)
  {
    min_pos= -1;
    min_pack=new_length;
    min_space_count=space_count;
  }
	/* Test with length-flag */
  for (skipp=0L, i=0 ; i < 8 ; i++)
  {
    if (space_counts[i])
    {
      if (i)
	huff_counts->counts[(uint) ' ']+=space_counts[i];
      skipp+=huff_counts->pre_space[i];
      new_length=calc_packed_length(huff_counts,0)+
	(records+(records-skipp)*(1+length_bits))/8;
      if (new_length < min_pack)
      {
	min_pos=(int) i;
	min_pack=new_length;
	min_space_count=huff_counts->counts[(uint) ' '];
      }
    }
  }

  huff_counts->counts[(uint) ' ']=min_space_count;
  huff_counts->bytes_packed=min_pack;
  switch (min_pos) {
  case -2:
    return(0);				/* No space-compress */
  case -1:				/* Always space-count */
    huff_counts->field_type=field_type;
    huff_counts->min_space=0;
    huff_counts->length_bits=max_bit(max_space_length);
    break;
  default:
    huff_counts->field_type=field_type;
    huff_counts->min_space=(uint) min_pos;
    huff_counts->pack_type|=PACK_TYPE_SELECTED;
    huff_counts->length_bits=max_bit(max_space_length);
    break;
  }
  return(1);				/* Using space-compress */
}


	/* Make a huff_tree of each huff_count */

static HUFF_TREE* make_huff_trees(HUFF_COUNTS *huff_counts, uint trees)
{
  uint tree;
  HUFF_TREE *huff_tree;
  DBUG_ENTER("make_huff_trees");

  if (!(huff_tree=(HUFF_TREE*) my_malloc(trees*sizeof(HUFF_TREE),
					 MYF(MY_WME | MY_ZEROFILL))))
    DBUG_RETURN(0);

  for (tree=0 ; tree < trees ; tree++)
  {
    if (make_huff_tree(huff_tree+tree,huff_counts+tree))
    {
      while (tree--)
	my_free((gptr) huff_tree[tree].element_buffer,MYF(0));
      my_free((gptr) huff_tree,MYF(0));
      DBUG_RETURN(0);
    }
  }
  DBUG_RETURN(huff_tree);
}

	/* Update huff_tree according to huff_counts->counts or
	   huff_counts->tree_buff */

static int make_huff_tree(HUFF_TREE *huff_tree, HUFF_COUNTS *huff_counts)
{
  uint i,found,bits_packed,first,last;
  my_off_t bytes_packed;
  HUFF_ELEMENT *a,*b,*new;

  first=last=0;
  if (huff_counts->tree_buff)
  {
    found= (uint) (huff_counts->tree_pos - huff_counts->tree_buff) /
      huff_counts->field_length;
    first=0; last=found-1;
  }
  else
  {
    for (i=found=0 ; i < 256 ; i++)
    {
      if (huff_counts->counts[i])
      {
	if (! found++)
	  first=i;
	last=i;
      }
    }
    if (found < 2)
      found=2;
  }

  if (queue.max_elements < found)
  {
    delete_queue(&queue);
    if (init_queue(&queue,found,0,0,compare_huff_elements,0))
      return -1;
  }

  if (!huff_tree->element_buffer)
  {
    if (!(huff_tree->element_buffer=
	 (HUFF_ELEMENT*) my_malloc(found*2*sizeof(HUFF_ELEMENT),MYF(MY_WME))))
      return 1;
  }
  else
  {
    HUFF_ELEMENT *temp;
    if (!(temp=
	  (HUFF_ELEMENT*) my_realloc((gptr) huff_tree->element_buffer,
				     found*2*sizeof(HUFF_ELEMENT),
				     MYF(MY_WME))))
      return 1;
    huff_tree->element_buffer=temp;
  }

  huff_counts->tree=huff_tree;
  huff_tree->counts=huff_counts;
  huff_tree->min_chr=first;
  huff_tree->max_chr=last;
  huff_tree->char_bits=max_bit(last-first);
  huff_tree->offset_bits=max_bit(found-1)+1;

  if (huff_counts->tree_buff)
  {
    huff_tree->elements=0;
    tree_walk(&huff_counts->int_tree,
	      (int (*)(void*, element_count,void*)) save_counts_in_queue,
	      (gptr) huff_tree, left_root_right);
    huff_tree->tree_pack_length=(1+15+16+5+5+
				 (huff_tree->char_bits+1)*found+
				 (huff_tree->offset_bits+1)*
				 (found-2)+7)/8 +
				   (uint) (huff_tree->counts->tree_pos-
					   huff_tree->counts->tree_buff);
  }
  else
  {
    huff_tree->elements=found;
    huff_tree->tree_pack_length=(9+9+5+5+
				 (huff_tree->char_bits+1)*found+
				 (huff_tree->offset_bits+1)*
				 (found-2)+7)/8;

    for (i=first, found=0 ; i <= last ; i++)
    {
      if (huff_counts->counts[i])
      {
	new=huff_tree->element_buffer+(found++);
	new->count=huff_counts->counts[i];
	new->a.leaf.null=0;
	new->a.leaf.element_nr=i;
	queue.root[found]=(byte*) new;
      }
    }
    while (found < 2)
    {			/* Our huff_trees request at least 2 elements */
      new=huff_tree->element_buffer+(found++);
      new->count=0;
      new->a.leaf.null=0;
      if (last)
	new->a.leaf.element_nr=huff_tree->min_chr=last-1;
      else
	new->a.leaf.element_nr=huff_tree->max_chr=last+1;
      queue.root[found]=(byte*) new;
    }
  }
  queue.elements=found;

  for (i=found/2 ; i > 0 ; i--)
    _downheap(&queue,i);
  bytes_packed=0; bits_packed=0;
  for (i=1 ; i < found ; i++)
  {
    a=(HUFF_ELEMENT*) queue_remove(&queue,0);
    b=(HUFF_ELEMENT*) queue.root[1];
    new=huff_tree->element_buffer+found+i;
    new->count=a->count+b->count;
    bits_packed+=(uint) (new->count & 7);
    bytes_packed+=new->count/8;
    new->a.nod.left=a;			/* lesser in left  */
    new->a.nod.right=b;
    queue.root[1]=(byte*) new;
    queue_replaced(&queue);
  }
  huff_tree->root=(HUFF_ELEMENT*) queue.root[1];
  huff_tree->bytes_packed=bytes_packed+(bits_packed+7)/8;
  return 0;
}

static int compare_tree(register const uchar *s, register const uchar *t)
{
  uint length;
  for (length=global_count->field_length; length-- ;)
    if (*s++ != *t++)
      return (int) s[-1] - (int) t[-1];
  return 0;
}

	/* Used by make_huff_tree to save intervall-counts in queue */

static int save_counts_in_queue(byte *key, element_count count, HUFF_TREE *tree)
{
  HUFF_ELEMENT *new;

  new=tree->element_buffer+(tree->elements++);
  new->count=count;
  new->a.leaf.null=0;
  new->a.leaf.element_nr= (uint) (key- tree->counts->tree_buff) /
    tree->counts->field_length;
  queue.root[tree->elements]=(byte*) new;
  return 0;
}


	/* Calculate length of file if given counts should be used */
	/* Its actually a faster version of make_huff_tree */

static my_off_t calc_packed_length(HUFF_COUNTS *huff_counts, uint add_tree_lenght)
{
  uint i,found,bits_packed,first,last;
  my_off_t bytes_packed;
  HUFF_ELEMENT element_buffer[256];
  DBUG_ENTER("calc_packed_length");

  first=last=0;
  for (i=found=0 ; i < 256 ; i++)
  {
    if (huff_counts->counts[i])
    {
      if (! found++)
	first=i;
      last=i;
      queue.root[found]=(byte*) &huff_counts->counts[i];
    }
  }
  if (!found)
    DBUG_RETURN(0);			/* Empty tree */
  if (found < 2)
    queue.root[++found]=(byte*) &huff_counts->counts[last ? 0 : 1];

  queue.elements=found;

  bytes_packed=0; bits_packed=0;
  if (add_tree_lenght)
    bytes_packed=(8+9+5+5+(max_bit(last-first)+1)*found+
		  (max_bit(found-1)+1+1)*(found-2) +7)/8;
  for (i=(found+1)/2 ; i > 0 ; i--)
    _downheap(&queue,i);
  for (i=0 ; i < found-1 ; i++)
  {
    HUFF_ELEMENT *a,*b,*new;
    a=(HUFF_ELEMENT*) queue_remove(&queue,0);
    b=(HUFF_ELEMENT*) queue.root[1];
    new=element_buffer+i;
    new->count=a->count+b->count;
    bits_packed+=(uint) (new->count & 7);
    bytes_packed+=new->count/8;
    queue.root[1]=(byte*) new;
    queue_replaced(&queue);
  }
  DBUG_RETURN(bytes_packed+(bits_packed+7)/8);
}


	/* Remove trees that don't give any compression */

static uint join_same_trees(HUFF_COUNTS *huff_counts, uint trees)
{
  uint k,tree_number;
  HUFF_COUNTS count,*i,*j,*last_count;

  last_count=huff_counts+trees;
  for (tree_number=0, i=huff_counts ; i < last_count ; i++)
  {
    if (!i->tree->tree_number)
    {
      i->tree->tree_number= ++tree_number;
      if (i->tree_buff)
	continue;			/* Don't join intervall */
      for (j=i+1 ; j < last_count ; j++)
      {
	if (! j->tree->tree_number && ! j->tree_buff)
	{
	  for (k=0 ; k < 256 ; k++)
	    count.counts[k]=i->counts[k]+j->counts[k];
	  if (calc_packed_length(&count,1) <=
	      i->tree->bytes_packed + j->tree->bytes_packed+
	      i->tree->tree_pack_length+j->tree->tree_pack_length+
	      ALLOWED_JOIN_DIFF)
	  {
	    memcpy((byte*) i->counts,(byte*) count.counts,
		   sizeof(count.counts[0])*256);
	    my_free((gptr) j->tree->element_buffer,MYF(0));
	    j->tree->element_buffer=0;
	    j->tree=i->tree;
	    bmove((byte*) i->counts,(byte*) count.counts,
		  sizeof(count.counts[0])*256);
	    if (make_huff_tree(i->tree,i))
	      return (uint) -1;
	  }
	}
      }
    }
  }
  if (verbose)
    printf("Original trees:  %d  After join: %d\n",trees,tree_number);
  return tree_number;			/* Return trees left */
}


	/* Fill in huff_tree decode tables */

static int make_huff_decode_table(HUFF_TREE *huff_tree, uint trees)
{
  uint elements;
  for ( ; trees-- ; huff_tree++)
  {
    if (huff_tree->tree_number > 0)
    {
      elements=huff_tree->counts->tree_buff ? huff_tree->elements : 256;
      if (!(huff_tree->code =
	    (ulong*) my_malloc(elements*
			       (sizeof(ulong)+sizeof(uchar)),
			       MYF(MY_WME | MY_ZEROFILL))))
	return 1;
      huff_tree->code_len=(uchar*) (huff_tree->code+elements);
      make_traverse_code_tree(huff_tree,huff_tree->root,32,0);
    }
  }
  return 0;
}


static void make_traverse_code_tree(HUFF_TREE *huff_tree, HUFF_ELEMENT *element,
				    uint size, ulong code)
{
  uint chr;
  if (!element->a.leaf.null)
  {
    chr=element->a.leaf.element_nr;
    huff_tree->code_len[chr]=(uchar) (32-size);
    huff_tree->code[chr]=    (code >> size);
    if (huff_tree->height < 32-size)
      huff_tree->height= 32-size;
  }
  else
  {
    size--;
    make_traverse_code_tree(huff_tree,element->a.nod.left,size,code);
    make_traverse_code_tree(huff_tree,element->a.nod.right,size,
			    code+((ulong) 1L << size));
  }
  return;
}


	/* Write header to new packed data file */

static int write_header(MRG_INFO *mrg,uint head_length,uint trees,
			my_off_t tot_elements,my_off_t filelength)
{
  byte *buff=file_buffer.pos;

  bzero(buff,HEAD_LENGTH);
  memcpy(buff,nisam_pack_file_magic,4);
  int4store(buff+4,head_length);
  int4store(buff+8, mrg->min_pack_length);
  int4store(buff+12,mrg->max_pack_length);
  int4store(buff+16,tot_elements);
  int4store(buff+20,intervall_length);
  int2store(buff+24,trees);
  buff[26]=(char) mrg->ref_length;
	/* Save record pointer length */
  buff[27]= (uchar) (filelength >= (1L << 24) ? 4 :
		     filelength >= (1L << 16) ? 3 : 2);
  if (test_only)
    return 0;
  VOID(my_seek(file_buffer.file,0L,MY_SEEK_SET,MYF(0)));
  return my_write(file_buffer.file,file_buffer.pos,HEAD_LENGTH,
		  MYF(MY_WME | MY_NABP | MY_WAIT_IF_FULL)) != 0;
}

	/* Write fieldinfo to new packed file */

static void write_field_info(HUFF_COUNTS *counts, uint fields, uint trees)
{
  reg1 uint i;
  uint huff_tree_bits;
  huff_tree_bits=max_bit(trees ? trees-1 : 0);

  for (i=0 ; i++ < fields ; counts++)
  {
    write_bits((ulong) (int) counts->field_type,4);
    write_bits(counts->pack_type,4);
    if (counts->pack_type & PACK_TYPE_ZERO_FILL)
      write_bits(counts->max_zero_fill,4);
    else
      write_bits(counts->length_bits,4);
    write_bits((ulong) counts->tree->tree_number-1,huff_tree_bits);
  }
  flush_bits();
  return;
}

	/* Write all huff_trees to new datafile. Return tot count of
	   elements in all trees
	   Returns 0 on error */

static my_off_t write_huff_tree(HUFF_TREE *huff_tree, uint trees)
{
  uint i,int_length;
  uint *packed_tree,*offset,length;
  my_off_t elements;

  for (i=length=0 ; i < trees ; i++)
    if (huff_tree[i].tree_number > 0 && huff_tree[i].elements > length)
      length=huff_tree[i].elements;
  if (!(packed_tree=(uint*) my_alloca(sizeof(uint)*length*2)))
  {
    my_error(EE_OUTOFMEMORY,MYF(ME_BELL),sizeof(uint)*length*2);
    return 0;
  }

  intervall_length=0;
  for (elements=0; trees-- ; huff_tree++)
  {
    if (huff_tree->tree_number == 0)
      continue;				/* Deleted tree */
    elements+=huff_tree->elements;
    huff_tree->max_offset=2;
    if (huff_tree->elements <= 1)
      offset=packed_tree;
    else
      offset=make_offset_code_tree(huff_tree,huff_tree->root,packed_tree);
    huff_tree->offset_bits=max_bit(huff_tree->max_offset);
    if (huff_tree->max_offset >= IS_OFFSET)
    {				/* This should be impossible */
      VOID(fprintf(stderr,"Tree offset got too big: %d, aborted\n",
	      huff_tree->max_offset));
      my_afree((gptr) packed_tree);
      return 0;
    }

#ifdef EXTRA_DBUG
    printf("pos: %d  elements: %d  tree-elements: %d  char_bits: %d\n",
	   (uint) (file_buffer.pos-file_buffer.buffer),
	   huff_tree->elements,  (offset-packed_tree),huff_tree->char_bits);
#endif
    if (!huff_tree->counts->tree_buff)
    {
      write_bits(0,1);
      write_bits(huff_tree->min_chr,8);
      write_bits(huff_tree->elements,9);
      write_bits(huff_tree->char_bits,5);
      write_bits(huff_tree->offset_bits,5);
      int_length=0;
    }
    else
    {
      int_length=(uint) (huff_tree->counts->tree_pos -
			 huff_tree->counts->tree_buff);
      write_bits(1,1);
      write_bits(huff_tree->elements,15);
      write_bits(int_length,16);
      write_bits(huff_tree->char_bits,5);
      write_bits(huff_tree->offset_bits,5);
      intervall_length+=int_length;
    }
    length=(uint) (offset-packed_tree);
    if (length != huff_tree->elements*2-2)
      printf("error: Huff-tree-length: %d != calc_length: %d\n",
	     length,huff_tree->elements*2-2);

    for (i=0 ; i < length ; i++)
    {
      if (packed_tree[i] & IS_OFFSET)
	write_bits(packed_tree[i] - IS_OFFSET+ ((ulong) 1L << huff_tree->offset_bits),
		   huff_tree->offset_bits+1);
      else
	write_bits(packed_tree[i]-huff_tree->min_chr,huff_tree->char_bits+1);
    }
    flush_bits();
    if (huff_tree->counts->tree_buff)
    {
      for (i=0 ; i < int_length ; i++)
	write_bits((uint) (uchar) huff_tree->counts->tree_buff[i],8);
    }
    flush_bits();
  }
  my_afree((gptr) packed_tree);
  return elements;
}


static uint *make_offset_code_tree(HUFF_TREE *huff_tree, HUFF_ELEMENT *element,
				   uint *offset)
{
  uint *prev_offset;

  prev_offset= offset;
  if (!element->a.nod.left->a.leaf.null)
  {
    offset[0] =(uint) element->a.nod.left->a.leaf.element_nr;
    offset+=2;
  }
  else
  {
    prev_offset[0]= IS_OFFSET+2;
    offset=make_offset_code_tree(huff_tree,element->a.nod.left,offset+2);
  }
  if (!element->a.nod.right->a.leaf.null)
  {
    prev_offset[1]=element->a.nod.right->a.leaf.element_nr;
    return offset;
  }
  else
  {
    uint temp=(uint) (offset-prev_offset-1);
    prev_offset[1]= IS_OFFSET+ temp;
    if (huff_tree->max_offset < temp)
      huff_tree->max_offset = temp;
    return make_offset_code_tree(huff_tree,element->a.nod.right,offset);
  }
}

	/* Get number of bits neaded to represent value */

static uint max_bit(register uint value)
{
  reg2 uint power=1;

  while ((value>>=1))
    power++;
  return (power);
}


static int compress_isam_file(MRG_INFO *mrg, HUFF_COUNTS *huff_counts)
{
  int error;
  uint i,max_calc_length,pack_ref_length,min_record_length,max_record_length,
  intervall,field_length;
  my_off_t record_count,max_allowed_length;
  ulong length;
  byte *record,*pos,*end_pos,*record_pos,*start_pos;
  HUFF_COUNTS *count,*end_count;
  HUFF_TREE *tree;
  N_INFO *isam_file=mrg->file[0];
  DBUG_ENTER("compress_isam_file");

  if (!(record=(byte*) my_alloca(isam_file->s->base.reclength)))
    return -1;
  end_count=huff_counts+isam_file->s->base.fields;
  min_record_length= (uint) ~0;
  max_record_length=0;

  for (i=max_calc_length=0 ; i < isam_file->s->base.fields ; i++)
  {
    if (!(huff_counts[i].pack_type & PACK_TYPE_ZERO_FILL))
      huff_counts[i].max_zero_fill=0;
    if (huff_counts[i].field_type == FIELD_CONSTANT ||
	huff_counts[i].field_type == FIELD_ZERO)
      continue;
    if (huff_counts[i].field_type == FIELD_INTERVALL)
      max_calc_length+=huff_counts[i].tree->height;
    else
      max_calc_length+=
	(huff_counts[i].field_length - huff_counts[i].max_zero_fill)*
	  huff_counts[i].tree->height+huff_counts[i].length_bits;
  }
  max_calc_length/=8;
  if (max_calc_length <= 255)
    pack_ref_length=1;
  else if (max_calc_length <= 65535)
    pack_ref_length=2;
  else
    pack_ref_length=3;
  if (force_pack_ref_length)
    pack_ref_length=force_pack_ref_length;
  max_allowed_length= 1L << (pack_ref_length*8);
  record_count=0;

  mrg_reset(mrg);
  while ((error=mrg_rrnd(mrg,record)) >= 0)
  {
    if (! error)
    {
      if (flush_buffer(max_calc_length+pack_ref_length))
	break;
      record_pos=file_buffer.pos;
      file_buffer.pos+=pack_ref_length;
      for (start_pos=record, count= huff_counts; count < end_count ; count++)
      {
	end_pos=start_pos+(field_length=count->field_length);
	tree=count->tree;

	if (count->pack_type & PACK_TYPE_SPACE_FIELDS)
	{
	  for (pos=start_pos ; *pos == ' ' && pos < end_pos; pos++) ;
	  if (pos == end_pos)
	  {
	    write_bits(1,1);
	    start_pos=end_pos;
	    continue;
	  }
	  write_bits(0,1);
	}

#ifdef BYTE_ORDER_HIGH_FIRST
	start_pos+=count->max_zero_fill;
#else
	end_pos-=count->max_zero_fill;
#endif
	field_length-=count->max_zero_fill;

	switch(count->field_type) {
	case FIELD_SKIP_ZERO:
	  if (!memcmp((byte*) start_pos,zero_string,field_length))
	  {
	    write_bits(1,1);
	    start_pos=end_pos;
	    break;
	  }
	  write_bits(0,1);
	  /* Fall through */
	case FIELD_NORMAL:
	  for ( ; start_pos < end_pos ; start_pos++)
	    write_bits(tree->code[(uchar) *start_pos],
		       (uint) tree->code_len[(uchar) *start_pos]);
	  break;
	case FIELD_SKIP_ENDSPACE:
	  for (pos=end_pos ; pos > start_pos && pos[-1] == ' ' ; pos--) ;
	  length=(uint) (end_pos-pos);
	  if (count->pack_type & PACK_TYPE_SELECTED)
	  {
	    if (length > count->min_space)
	    {
	      write_bits(1,1);
	      write_bits(length,count->length_bits);
	    }
	    else
	    {
	      write_bits(0,1);
	      pos=end_pos;
	    }
	  }
	  else
	    write_bits(length,count->length_bits);
	  for ( ; start_pos < pos ; start_pos++)
	    write_bits(tree->code[(uchar) *start_pos],
		       (uint) tree->code_len[(uchar) *start_pos]);
	  start_pos=end_pos;
	  break;
	case FIELD_SKIP_PRESPACE:
	  for (pos=start_pos ; pos < end_pos && pos[0] == ' ' ; pos++) ;
	  length=(uint) (pos-start_pos);
	  if (count->pack_type & PACK_TYPE_SELECTED)
	  {
	    if (length > count->min_space)
	    {
	      write_bits(1,1);
	      write_bits(length,count->length_bits);
	    }
	    else
	    {
	      pos=start_pos;
	      write_bits(0,1);
	    }
	  }
	  else
	    write_bits(length,count->length_bits);
	  for (start_pos=pos ; start_pos < end_pos ; start_pos++)
	    write_bits(tree->code[(uchar) *start_pos],
		       (uint) tree->code_len[(uchar) *start_pos]);
	  break;
	case FIELD_CONSTANT:
	case FIELD_ZERO:
	  start_pos=end_pos;
	  break;
	case FIELD_INTERVALL:
	  global_count=count;
	  pos=(byte*) tree_search(&count->int_tree,start_pos);
	  intervall=(uint) (pos - count->tree_buff)/field_length;
	  write_bits(tree->code[intervall],(uint) tree->code_len[intervall]);
	  start_pos=end_pos;
	  break;
	case FIELD_BLOB:
	  VOID(fprintf(stderr,"Can't pack files with blobs. Aborting\n"));
	  DBUG_RETURN(1);
	case FIELD_LAST:
	case FIELD_VARCHAR:
	case FIELD_CHECK:
	  abort();				/* Impossible */
	}
#ifndef BYTE_ORDER_HIGH_FIRST
	start_pos+=count->max_zero_fill;
#endif
      }
      flush_bits();
      length=(ulong) (file_buffer.pos-record_pos)-pack_ref_length;
      save_integer(record_pos,pack_ref_length,length);
      if (length < (ulong) min_record_length)
	min_record_length=(uint) length;
      if (length > (ulong) max_record_length)
      {
	max_record_length=(uint) length;
	if (max_record_length >= max_allowed_length)
	{
	  fprintf(stderr,
		  "Error: Found record with packed-length: %d, max is: %lu\n",
		  max_record_length, (ulong) max_allowed_length);
	  error=1;
	  break;
	}
      }
      if (write_loop && ++record_count % WRITE_COUNT == 0)
      {
	printf("%lu\r",(ulong) record_count); VOID(fflush(stdout));
      }
    }
    else if (my_errno != HA_ERR_RECORD_DELETED)
      break;
  }
  if (error < 0)
  {
    error=0;
    if (my_errno != HA_ERR_END_OF_FILE)
    {
      fprintf(stderr,"%s: Got error %d reading records\n",my_progname,my_errno);
      error= 1;
    }
  }

  my_afree((gptr) record);
  mrg->ref_length=pack_ref_length;
  mrg->min_pack_length=max_record_length ? min_record_length : 0;
  mrg->max_pack_length=max_record_length;
  if (verbose && max_record_length &&
      max_record_length < max_allowed_length/256)
    printf("Record-length is %d bytes, could have been %d bytes\nYou can change this by using -p=%d next time you pack this file\n",
	   pack_ref_length,
	   max_record_length/256+1,
	   max_record_length/256+1);
  DBUG_RETURN(error || error_on_write || flush_buffer((uint) ~0));
}


static char *make_new_name(char *new_name, char *old_name)
{
  return fn_format(new_name,old_name,"",DATA_TMP_EXT,2+4);
}

static char *make_old_name(char *new_name, char *old_name)
{
  return fn_format(new_name,old_name,"",OLD_EXT,2+4);
}

	/* rutines for bit writing buffer */

static void init_file_buffer(File file, pbool read_buffer)
{
  file_buffer.file=file;
  file_buffer.buffer=my_malloc(ALIGN_SIZE(RECORD_CACHE_SIZE),MYF(MY_WME));
  file_buffer.end=file_buffer.buffer+ALIGN_SIZE(RECORD_CACHE_SIZE)-4;
  file_buffer.pos_in_file=0;
  error_on_write=0;
  if (read_buffer)
  {

    file_buffer.pos=file_buffer.end;
    file_buffer.bits=0;
  }
  else
  {
    file_buffer.pos=file_buffer.buffer;
    file_buffer.bits=BITS_SAVED;
  }
  file_buffer.bytes=0;
}


static int flush_buffer(uint neaded_length)
{
  uint length;
  if ((uint) (file_buffer.end - file_buffer.pos) > neaded_length)
    return 0;
  length=(uint) (file_buffer.pos-file_buffer.buffer);
  file_buffer.pos=file_buffer.buffer;
  file_buffer.pos_in_file+=length;
  if (test_only)
    return 0;
  return (error_on_write|=test(my_write(file_buffer.file,file_buffer.buffer,
					length,
					MYF(MY_WME | MY_NABP |
					    MY_WAIT_IF_FULL))));
}

static void end_file_buffer(void)
{
  my_free((gptr) file_buffer.buffer,MYF(0));
}

	/* output `bits` low bits of `value' */

static void write_bits (register ulong value, register uint bits)
{
  if ((file_buffer.bits-=(int) bits) >= 0)
  {
    file_buffer.bytes|=value << file_buffer.bits;
  }
  else
  {
    reg3 uint byte_buff;
    bits= (uint) -file_buffer.bits;
    byte_buff=file_buffer.bytes | (uint) (value >> bits);
#if BITS_SAVED == 32
    *file_buffer.pos++= (byte) (byte_buff >> 24) ;
    *file_buffer.pos++= (byte) (byte_buff >> 16) ;
#endif
    *file_buffer.pos++= (byte) (byte_buff >> 8) ;
    *file_buffer.pos++= (byte) byte_buff;

    value&=((ulong) 1L << bits)-1;
#if BITS_SAVED == 16
    if (bits >= sizeof(uint))
    {
      bits-=8;
      *file_buffer.pos++= (uchar) (value >> bits);
      value&= ((ulong) 1L << bits)-1;
      if (bits >= sizeof(uint))
      {
	bits-=8;
	*file_buffer.pos++= (uchar) (value >> bits);
	value&= ((ulong) 1L << bits)-1;
      }
    }
#endif
    if (file_buffer.pos >= file_buffer.end)
      VOID(flush_buffer((uint) ~0));
    file_buffer.bits=(int) (BITS_SAVED - bits);
    file_buffer.bytes=(uint) (value << (BITS_SAVED - bits));
  }
  return;
}

	/* Flush bits in bit_buffer to buffer */

static void flush_bits (void)
{
  uint bits,byte_buff;

  bits=(file_buffer.bits) & ~7;
  byte_buff = file_buffer.bytes >> bits;
  bits=BITS_SAVED - bits;
  while (bits > 0)
  {
    bits-=8;
    *file_buffer.pos++= (byte) (uchar) (byte_buff >> bits) ;
  }
  file_buffer.bits=BITS_SAVED;
  file_buffer.bytes=0;
  return;
}

	/* Store long in 1,2,3,4 or 5 bytes */

static void save_integer(byte *pos, uint pack_length, my_off_t value)
{
  switch (pack_length) {
  case 5: int5store(pos,(ulonglong) value); break;
  default: int4store(pos,(ulong) value); break;
  case 3: int3store(pos,(ulong) value);  break;
  case 2: int2store(pos,(uint) value); break;
  case 1: pos[0]= (byte) (uchar) value; break;
  }
  return;
}


/****************************************************************************
** functions to handle the joined files
****************************************************************************/

static void save_state(N_INFO *isam_file,MRG_INFO *mrg,my_off_t new_length,
		       ulong crc)
{
  ISAM_SHARE *share=isam_file->s;
  uint options=uint2korr(share->state.header.options);
  DBUG_ENTER("save_state");

  options|= HA_OPTION_COMPRESS_RECORD | HA_OPTION_READ_ONLY_DATA;
  int2store(share->state.header.options,options);

  share->state.data_file_length=(ulong) new_length;
  share->state.del=share->state.empty=0;
  share->state.dellink= (ulong) NI_POS_ERROR;
  share->state.splitt=(ulong) mrg->records;
  share->state.version=(ulong) time((time_t*) 0);
  share->state.keys=0;
  share->state.key_file_length=share->base.keystart;

  isam_file->update|=(HA_STATE_CHANGED | HA_STATE_ROW_CHANGED);
  isam_file->this_uniq=crc;		/* Save crc here */
  share->changed=1;			/* Force write of header */
  VOID(my_chsize(share->kfile, share->state.key_file_length, 0,
		 MYF(0)));
  if (share->state.keys != share->base.keys)
    isamchk_neaded=1;
  DBUG_VOID_RETURN;
}


static int save_state_mrg(File file,MRG_INFO *mrg,my_off_t new_length,
			  ulong crc)
{
  N_STATE_INFO state;
  N_INFO *isam_file=mrg->file[0];
  uint options;
  DBUG_ENTER("save_state_mrg");

  memcpy(&state,&isam_file->s->state,sizeof(state));
  options= (uint2korr(state.header.options) | HA_OPTION_COMPRESS_RECORD |
	    HA_OPTION_READ_ONLY_DATA);
  int2store(state.header.options,options);
  state.data_file_length=(ulong) new_length;
  state.del=state.empty=0;
  state.dellink= (ulong) NI_POS_ERROR;
  state.records=state.splitt=(ulong) mrg->records;
  state.version=(ulong) time((time_t*) 0);
  state.keys=0;
  state.key_file_length=isam_file->s->base.keystart;
  state.uniq=crc;
  if (state.keys != isam_file->s->base.keys)
    isamchk_neaded=1;
  DBUG_RETURN (my_pwrite(file,(char*) &state.header,
			 isam_file->s->state_length,0L,
			 MYF(MY_NABP | MY_WME)) != 0);
}


/* reset for mrg_rrnd */

static void mrg_reset(MRG_INFO *mrg)
{
  if (mrg->current)
  {
    nisam_extra(*mrg->current,HA_EXTRA_NO_CACHE);
    mrg->current=0;
  }
}

static int mrg_rrnd(MRG_INFO *info,byte *buf)
{
  int error;
  N_INFO *isam_info;
  my_off_t filepos;

  if (!info->current)
  {
    isam_info= *(info->current=info->file);
    info->end=info->current+info->count;
    nisam_extra(isam_info,HA_EXTRA_RESET);
    nisam_extra(isam_info,HA_EXTRA_CACHE);
    filepos=isam_info->s->pack.header_length;
  }
  else
  {
    isam_info= *info->current;
    filepos= isam_info->nextpos;
  }

  for (;;)
  {
    isam_info->update&= (HA_STATE_CHANGED | HA_STATE_ROW_CHANGED);
    if ((error=(*isam_info->s->read_rnd)(isam_info,(byte*) buf,
					 (ulong) filepos, 1)) >= 0 ||
	my_errno != HA_ERR_END_OF_FILE)
      return (error);
    nisam_extra(isam_info,HA_EXTRA_NO_CACHE);
    if (info->current+1 == info->end)
      return(-1);
    info->current++;
    isam_info= *info->current;
    filepos=isam_info->s->pack.header_length;
    nisam_extra(isam_info,HA_EXTRA_RESET);
    nisam_extra(isam_info,HA_EXTRA_CACHE);
  }
}


static int mrg_close(MRG_INFO *mrg)
{
  uint i;
  int error=0;
  for (i=0 ; i < mrg->count ; i++)
    error|=nisam_close(mrg->file[i]);
  return error;
}
