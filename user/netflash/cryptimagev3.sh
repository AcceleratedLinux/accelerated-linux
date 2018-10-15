#!/bin/bash

if [ $# -ne 3 ]; then
	echo "Usage: $0 image key cert"
	exit 1
fi

# FIXME: check key matches cert?

hdr_magic=d5e8b29c
hdr_flags=00
echo ${hdr_magic}${hdr_flags} | xxd -r -p >> ${1}

cert=$(openssl x509 -inform PEM -outform DER -in ${3} | xxd -p)
certlen=$(echo $cert | xxd -r -p | wc -c)
echo 01$(printf %08x $certlen)${cert} | xxd -r -p >> ${1}

sig=$(openssl dgst -sha256 -sign ${2} -binary ${1} | xxd -p)
siglen=$(echo $sig | xxd -r -p | wc -c)
echo 02$(printf %08x $siglen)${sig} | xxd -r -p >> ${1}

len=$((5 + 5 + $certlen + 5 + $siglen + 1))
echo 00$(printf %08x $len)8f7d66ab | xxd -r -p >> ${1}
