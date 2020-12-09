#ifndef NETFLASH_CHECK_H
#define NETFLASH_CHECK_H

#include <config/autoconf.h>

struct check_opt {
	int dochecksum;
	int doversion;
	int dohardwareversion;
	int doremoveversion;
	int dominimumcheck;
#ifdef CONFIG_USER_NETFLASH_SHA256
	int dosha256sum;
#endif
#ifdef CONFIG_USER_NETFLASH_HMACMD5
	char *hmacmd5key;
#endif
#ifdef CONFIG_USER_NETFLASH_CRYPTO_V3
	char *ca;
	char *crl;
#endif
	int doremovesig;
};

void update_chksum(unsigned char *data, int length);
void calc_chksum(void);
void chksum(void);
int check_version_info(int offset, int doversion, int dohardwareversion, int removeversion, int failifnoversion, int dominimumcheck);
void check(const struct check_opt *opt);

#ifdef CONFIG_USER_NETFLASH_VERIFY_FW_PRODUCT_INFO
void check_fw_product_info(void);
#endif

#endif
