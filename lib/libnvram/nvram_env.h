#define ENV_BLK_SIZE 0x1000

#ifdef CONFIG_DUAL_IMAGE
#define ENV_UBOOT_SIZE 0x1000
#endif

extern		void  nvram_printenv(int index, int argc, char *argv[]);
extern char const *nvram_getenv  (int index, unsigned char *name);
extern		int   nvram_setenv  (int index, int argc, char *argv[]);
extern		int   nvram_buff_setenv  (int index, int argc, char *argv[]);
extern 		int   do_nvram_commit(int index);
extern unsigned	long  crc32	 (unsigned long, const unsigned char *, unsigned);

