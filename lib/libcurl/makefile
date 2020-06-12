CONFOPTS :=	--enable-dependency-tracking \
		--disable-largefile \
		--enable-http \
		--enable-ftp \
		--disable-tftp \
		--disable-pop3 \
		--disable-imap \
		--disable-smtp \
		--disable-gopher \
		--enable-file \
		--disable-ldap \
		--disable-ldaps \
		--disable-rtsp \
		--disable-proxy \
		--disable-dict \
		--disable-telnet \
		--disable-manual \
  		--disable-libcurl-option \
		--enable-ipv6 \
		--enable-nonblocking \
		--disable-thread \
		--disable-ares \
		--enable-verbose \
		--disable-sspi \
		--disable-debug \
  		--disable-curldebug \
		--enable-crypto-auth \
  		--disable-ntlm-wb \
  		--disable-tls-srp \
		--enable-cookies \
		--with-ca-bundle=/etc/ssl/certs/ca-certificates.crt \
		--with-random=/dev/urandom \
		--with-libidn2

URL := https://curl.haxx.se/download/curl-7.69.0.tar.bz2
ROMFS = curl_romfs
MAKEVARS = 
include $(ROOTDIR)/tools/automake.inc

curl_romfs:
	$(ROMFSINST) -R -E CONFIG_LIB_LIBCURL_CURL /bin/curl
	$(ROMFSINST) -R -E CONFIG_LIB_LIBCURL_CURL_CONFIG /bin/curl-config

ifdef NOT_DEFINED
  --enable-debug          Enable debug build options
  --disable-debug         Disable debug build options
  --enable-curldebug      Enable curl debug memory tracking
  --disable-curldebug     Disable curl debug memory tracking
  --enable-shared[=PKGS]  build shared libraries [default=yes]
  --enable-static[=PKGS]  build static libraries [default=yes]
  --enable-http           Enable HTTP support
  --disable-http          Disable HTTP support
  --enable-ftp            Enable FTP support
  --disable-ftp           Disable FTP support
  --enable-file           Enable FILE support
  --disable-file          Disable FILE support
  --enable-ldap           Enable LDAP support
  --disable-ldap          Disable LDAP support
  --enable-ldaps          Enable LDAPS support
  --disable-ldaps         Disable LDAPS support
  --enable-rtsp           Enable RTSP support
  --disable-rtsp          Disable RTSP support
  --enable-proxy          Enable proxy support
  --disable-proxy         Disable proxy support
  --enable-dict           Enable DICT support
  --disable-dict          Disable DICT support
  --enable-telnet         Enable TELNET support
  --disable-telnet        Disable TELNET support
  --enable-tftp           Enable TFTP support
  --disable-tftp          Disable TFTP support
  --enable-pop3           Enable POP3 support
  --disable-pop3          Disable POP3 support
  --enable-imap           Enable IMAP support
  --disable-imap          Disable IMAP support
  --enable-smtp           Enable SMTP support
  --disable-smtp          Disable SMTP support
  --enable-gopher         Enable Gopher support
  --disable-gopher        Disable Gopher support
  --enable-manual         Enable built-in manual
  --disable-manual        Disable built-in manual
  --enable-libcurl-option Enable --libcurl C code generation support
  --disable-libcurl-option
                          Disable --libcurl C code generation support
  --enable-libgcc         use libgcc when linking
  --enable-ipv6           Enable ipv6 (with ipv4) support
  --disable-ipv6          Disable ipv6 support
  --enable-nonblocking    Enable non-blocking communications
  --disable-nonblocking   Disable non-blocking communications
  --enable-threaded-resolver
                          Enable threaded resolver
  --disable-threaded-resolver
                          Disable threaded resolver
  --enable-verbose        Enable verbose strings
  --disable-verbose       Disable verbose strings
  --enable-sspi           Enable SSPI
  --disable-sspi          Disable SSPI
  --enable-crypto-auth    Enable cryptographic authentication
  --disable-crypto-auth   Disable cryptographic authentication
  --enable-ntlm-wb[=FILE] Enable NTLM delegation to winbinds ntlm_auth
                          helper, where FILE is ntlm_auths absolute filename
                          (default: /usr/bin/ntlm_auth)
  --disable-ntlm-wb       Disable NTLM delegation to winbinds ntlm_auth
                          helper
  --enable-tls-srp        Enable TLS-SRP authentication
  --disable-tls-srp       Disable TLS-SRP authentication
  --enable-cookies        Enable cookies support
  --disable-cookies       Disable cookies support

endif
