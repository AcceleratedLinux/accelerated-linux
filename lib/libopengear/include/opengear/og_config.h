#ifndef _OG_CONFIG
#define _OG_CONFIG

#include <vendor/autoconf.h>
#include <config/autoconf.h>
#include <linux/autoconf.h>
/* Opengear Configuration Header, use this for feature detection */

#if defined(CONFIG_MACH_IM72xx) /* OpenGear/IM72xx, TrippLite/B098 and BlackBox/LES17xxA */
#define PRODUCT_IM72xx 1
#define HAVE_SFP 1
#define MAX_NUM_ETH 2
#define HAVE_USB 1
#define HAVE_WIFI_AP_UI 1
#define HAVE_SERIAL 1
#define HAVE_BRIDGING 1
#define HAVE_BONDING 1
#define HAVE_SD_CARD 1
#define HAVE_GIGABIT 1
#endif

#if defined(CONFIG_DEFAULTS_OPENGEAR_CM71XX)   || \
    defined(CONFIG_DEFAULTS_TRIPPLITE_B097)    || \
    defined(CONFIG_DEFAULTS_BLACKBOX_LES15XXA)
#define PRODUCT_CM71xx 1
#define MAX_NUM_ETH 2
#define HAVE_USB 1
#define HAVE_SERIAL 1
#define HAVE_BRIDGING 1
#define HAVE_BONDING 1
#define HAVE_SD_CARD 1
#define HAVE_GIGABIT 1
#endif

#if defined(CONFIG_DEFAULTS_OPENGEAR_CM7196)
#define PRODUCT_CM7196 1
#define MAX_NUM_ETH 2
#define HAVE_SFP 1
#define HAVE_INTERNAL_TEMP 1
#define HAVE_USB 1
#define HAVE_SERIAL 1
#define HAVE_BRIDGING 1
#define HAVE_BONDING 1
#define HAVE_GIGABIT 1
#endif

#if defined(CONFIG_DEFAULTS_OPENGEAR_ACM700X)    || \
    defined(CONFIG_DEFAULTS_OPENGEAR_ACM7004_5)  || \
    defined(CONFIG_DEFAULTS_TRIPPLITE_B093)      || \
    defined(CONFIG_DEFAULTS_BLACKBOX_LES160XA)
#define PRODUCT_ACM700x 1
#define MAX_NUM_ETH 2
#define HAVE_IOPORTS 1
#define HAVE_INTERNAL_TEMP 1
#define HAVE_USB 1
#define HAVE_SERIAL 1
#define HAVE_BRIDGING 1
#define HAVE_BONDING 1
#define HAVE_WIFI_AP_UI 1
#define HAVE_GIGABIT 1
#define HAVE_NAND_FLASH 1
#endif

#if defined (CONFIG_DEFAULTS_OPENGEAR_ACM7004_5)
#define PRODUCT_ACM7004_5 1
#endif

/* Otherwise default to VACM */
#if defined(CONFIG_X86)
#define PRODUCT_ACM700x
#define HAVE_INTERNAL_TEMP 1
#define HAVE_RS485 1
#define MAX_NUM_ETH 2
#define HAVE_USB 1
#define HAVE_SERIAL 1
#define HAVE_BRIDGING 1
#define HAVE_BONDING 1
#define WANT_FLASH_UPGRADE 1
#define HAVE_BROKEN_MULTICAST 1
#endif

/* Feature Definitions */
#ifdef HAVE_USB
/* 3G Support */

#ifdef CONFIG_USER_LIBQMI
#define HAVE_CELLMODEM 1
#endif /* CONFIG_USER_LIBQMI */

/* USB Storage */
#ifdef CONFIG_USB_STORAGE
#define HAVE_USB_STORAGE 1
/* FTPD */
#ifdef CONFIG_USER_VSFTPD_VSFTPD
#define HAVE_FTPD 1
#endif
/* TFTPD */
#ifdef CONFIG_USER_TFTP_HPA
#define HAVE_TFTPD 1
#endif

#endif /* CONFIG_USB_STORAGE */

/* WIFI */
#if defined(CONFIG_USER_IW) || defined(CONFIG_USER_WIRELESS_TOOLS_IWPRIV)
#define HAVE_WIFI 1

#if defined(CONFIG_USER_HOSTAPD)
#define HAVE_WIFI_AP 1
#endif

#endif /* CONFIG_USER IW || CONFIG_USER_WIRELESS_TOOLS_IWPRIV */
#endif /* HAVE_USB */

/* Software Features */
#ifdef CONFIG_LIB_OPENSSL_FIPS
#define OPENGEAR_FIPS 1
#endif

#ifdef CONFIG_USER_NUT
#define HAVE_NUT 1
#define HAVE_CGI_CONFIGURABLE_NUT 1
#endif

#ifdef CONFIG_USER_NUT_WITH_OTHERS
#define ALL_NUT_DRIVERS 1
#endif

#ifdef CONFIG_USER_PAM_KRB5
#define HAVE_KRB5 1
#endif

#ifdef CONFIG_PROP_POWERALERT
#define HAVE_POWERALERT 1
#endif

#ifdef CONFIG_PROP_EMD_EMD
#define HAVE_EMD 1

#ifdef CONFIG_PROP_EMD_KS8692
#define HAVE_EMD_KS8692 1
#endif

#ifdef CONFIG_PROP_EMD_EXTERNAL
#define HAVE_EMD_EXTERNAL 1
#endif

#endif /* CONFIG_PROP_EMD_EMD */

#ifdef CONFIG_USER_DHCP_ISC_SERVER_DHCPD
#define HAVE_DHCPD 1
#endif

#ifdef CONFIG_USER_DNSMASQ2_DNSMASQ2
#define HAVE_DNSMASQ 1
#endif

#ifdef CONFIG_USER_ZIP_ZIP
#define HAVE_ZIP 1
#endif

#ifdef CONFIG_USER_PPTPD_PPTPD
#define HAVE_PPTPD 1
#endif

#ifdef CONFIG_USER_PPPD_WITH_RADIUS
#define HAVE_PPPD_WITH_RADIUS 1
#endif

#ifdef CONFIG_USER_OPENSWAN
#define HAVE_IPSEC 1
#endif

#ifdef CONFIG_USER_OPENVPN_OPENVPN
#define HAVE_OPENVPN 1
#endif

#ifdef CONFIG_USER_BUSYBOX_FEATURE_SHADOWPASSWDS
#define HAVE_SHADOW 1
#endif

#ifdef CONFIG_PROP_PSMON
#define HAVE_PSMON 1
#endif

#ifdef CONFIG_PROP_RPCD
#define HAVE_RPCD 1
#endif

#ifdef CONFIG_USER_CURL_CURL
#define HAVE_CURL 1
#endif

#ifdef CONFIG_USER_EZIPUPDATE_EZIPUPDATE
#define DYNDNS 1
#define HAVE_DYNDNS 1
#endif

#ifdef CONFIG_USER_LLDPD
#define HAVE_LLDP 1
#endif

/* SNMPD */
#if defined(CONFIG_USER_NETSNMP_AGENT_SNMPD) || defined(CONFIG_USER_NETSNMP_NETSNMP) || defined(CONFIG_USER_NETSNMP_SNMPD) \
	|| defined(CONFIG_USER_SNMPD_SNMPD)
#define HAVE_SNMPD 1
#define HAVE_CGI_CONFIGURABLE_SNMPD 1
#endif

#ifdef CONFIG_PROP_CGI_RECOVERY
#define RECOVERY_IMAGE 1
#endif

/* Services/features on the box: allow them to be CGI-configurable */
#ifdef CONFIG_USER_TELNETD_TELNETD
#define HAVE_CGI_CONFIGURABLE_TELNETD 1
#endif

#ifdef CONFIG_PROP_NAGIOS
#define HAVE_CGI_CONFIGURABLE_NAGIOS 1
#endif

#define HAVE_CGI_CONFIGURABLE_SERIAL_PORTBASES 1

/* TrippLite do not get Portshare Encryption */
#if !defined(CONFIG_DEFAULTS_TRIPPLITE)
#define HAVE_CGI_PORTSHARE_ENCRYPTION 1
#endif

// Cherokee Webserver
#ifdef CONFIG_USER_CHEROKEE
#define HAVE_CHEROKEE
#endif

// Rebrand
#ifdef CONFIG_DEFAULTS_BLACKBOX
#define HAVE_REBRAND_BLACKBOX
#endif

#ifdef CONFIG_DEFAULTS_TRIPPLITE
#define HAVE_REBRAND_TRIPPLITE
#endif

#if defined(HAVE_REBRAND_BLACKBOX) || \
    defined(HAVE_REBRAND_TRIPPLITE)
#define CMS_LABEL "VCMS"
#else
#define CMS_LABEL "Lighthouse"
#endif

#endif /* _OG_CONFIG */
