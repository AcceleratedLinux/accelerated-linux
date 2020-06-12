#!/bin/sh
#
# A tool to simplify Makefiles that need to put something
# into the ROMFS
#
# Copyright (C) David McCullough, 2002,2003
#
#############################################################################

# Provide a default PATH setting to avoid potential problems...
PATH="/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:$PATH"
FAKEROOT_SCRIPT="${ROOTDIR}/tools/fakeroot-build.sh"

usage()
{
cat << !EOF >&2
$0: [options] [src] dst
    -v          : output actions performed.
    -V          : output actions not performed.
    -e env-var  : only take action if env-var is set to "y".
    -E env-var  : only take action if env-var is not set to "y".
    -o option   : only take action if option is set to "y".
    -O option   : only take action if option is not set to "y".
    -c          : process with cpp+cflags
    -p perms    : chmod style permissions for dst.
    -w usr:grp  : chown style ownership for dst.
    -C caps     : setcap style capabilities for dst.
    -m          : modify file permissions/ownership/capabilities without copy
    -d          : make dst directory if it doesn't exist
    -S          : don't strip after installing
    -a text     : append text to dst.
    -A pattern  : only append text if pattern doesn't exist in file
    -l link     : dst is a hard link to 'link'.
    -s sym-link : dst is a sym-link to 'sym-link'.
    -M          : install kernel module into dst subdir of module dir
    -r          : root directory of install (default ROMFSDIR)
    -R          : remove dst
    -f          : do not follow symlinks

    if "src" is not provided,  basename is run on dst to determine the
    source in the current directory.

    multiple -e and -o options are ANDed together.  To achieve an OR affect
    use a single -e/-o with 1 or more y/n/"" chars in the condition.

    if src is a directory,  everything in it is copied recursively to dst
    with special files removed (currently CVS and Subversion dirs).
!EOF
	exit 1
}

#############################################################################

setperm()
{
	rc=0
	# Always start with write access for the user so that files can be
	# updated/overwritten during make romfs
	chmod u+w ${ROMFSDIR}${dst}
	if [ "$perm" ]
	then
		[ "$v" ] && echo "chmod ${perm} ${ROMFSDIR}${dst}"
		chmod ${perm} ${ROMFSDIR}${dst}
		rc=$?
	fi
	return $rc
}

setcaps()
{
	# Can't arbitrarily change capabilities in non-root build. Write the
	# commands into the script that runs under fakeroot before the final
	# romfs is built.
	if [ "$caps" ]
	then
		cmd="setcap ${caps} ${ROMFSDIR}${dst}"
		sed -i'' "\?^setcap .* ${ROMFSDIR}${dst}\$?d" "${FAKEROOT_SCRIPT}"
		[ "$v" ] && echo "defer to fakeroot: $cmd"
		echo "$cmd" >> "${FAKEROOT_SCRIPT}"
		# setcap changes the owner to the build user, so force to root if not
		# a custom ownership.
		[ -z "$owner" ] && owner="root:root"
	fi
}

setowner()
{
	# Can't arbitrarily change user/group as non-root build. Write the commands
	# into the script that runs under fakeroot before the final romfs is built.
	if [ "$owner" ]
	then
		cmd="chown ${owner} ${ROMFSDIR}${dst}"
		sed -i'' "\?^chown .* ${ROMFSDIR}${dst}\$?d" "${FAKEROOT_SCRIPT}" 2>/dev/null
		[ "$v" ] && echo "defer to fakeroot: $cmd"
		echo "$cmd" >> "${FAKEROOT_SCRIPT}"
	fi
}


setattrs()
{
	setcaps
	setowner
	setperm
}

#############################################################################

file_copy()
{
	rc=0
	[ "$ACL_LICENSE" ] ||
		ACL_LICENSE="$($ROOTDIR/bin/license-detect.sh `pwd`)"
	[ "$ACL_PKG" ] ||
		ACL_PKG="$(echo ${PWD} | sed -r 's?^/.*/(user|lib)/??')"
	if [ -d "${src}" ]
	then
		[ "$v" ] && echo "CopyDir ${src} ${ROMFSDIR}${dst}"
		(
			cd ${src} || return 1
			echo "$src"
			find * -type f 2> /dev/null | grep -E -v '/CVS|/\.svn' | while read t; do
				echo "`pwd`/${t} ${ROMFSDIR}${dst}/${t}"
			done >> $IMAGEDIR/romfs-inst.log
			echo "${ACL_PKG}:${ACL_LICENSE:-UNKNOWN}:${ACL_URL}:${dst}" >> $IMAGEDIR/license.log
			VV=
			[ "$v" ] && VV=v
			find . -print | grep -E -v '/CVS|/\.svn' | cpio --quiet -p${VV}dum${follow} ${ROMFSDIR}${dst}
			rc=$?
			# And make sure these files are still writable
			find . -print | grep -E -v '/CVS|/\.svn' | ( cd ${ROMFSDIR}${dst}; xargs chmod u+w )
			setattrs ${ROMFSDIR}${dst}
			find . -type f | grep -E -v '/CVS|/\.svn|\.ko$' | while read t; do
				if [ -n "$strip" ]; then
					${STRIPTOOL} ${ROMFSDIR}${dst}/$t 2>/dev/null
					${STRIPTOOL} -R .comment -R .note ${ROMFSDIR}${dst}/$t 2>/dev/null
				fi
			done
		)
	else
		if [ -d ${ROMFSDIR}${dst} ]; then
			dstfile=${ROMFSDIR}${dst}/`basename ${src}`
		else
			dstfile=${ROMFSDIR}${dst}
		fi
		rm -f ${dstfile}
		[ "$v" ] && echo "cp ${src} ${dstfile}"
		if [ ! -d "$IMAGEDIR" ]
		then
			mkdir -p $IMAGEDIR
		fi
		case "$src" in
			/*) echo "${src} ${dstfile}" ;;
			*)  echo "`pwd`/${src} ${dstfile}" ;;
		esac >> $IMAGEDIR/romfs-inst.log
		echo "${ACL_PKG}:${ACL_LICENSE:-UNKNOWN}:${ACL_URL}:${dstfile#${ROMFSDIR}}" >> $IMAGEDIR/license.log
		cp ${src} ${dstfile}
		rc=$?
		if [ $rc -eq 0 ]; then
			if [ -n "$strip" ]; then
				chmod u+w ${dstfile}
				${STRIPTOOL} ${dstfile} 2>/dev/null
				${STRIPTOOL} -R .comment -R .note ${dstfile} 2>/dev/null
			fi
			setattrs ${dstfile}
			rc=$?
		fi
	fi
	return $rc
}

#############################################################################

file_append()
{
	touch ${ROMFSDIR}${dst} || return 1
	if [ -z "${pattern}" ] && grep -F "${src}" ${ROMFSDIR}${dst} > /dev/null
	then
		[ "$V" ] && echo "File entry already installed."
	elif [ "${pattern}" ] && egrep "${pattern}" ${ROMFSDIR}${dst} > /dev/null
	then
		[ "$V" ] && echo "File pattern already installed."
	else
		[ "$v" ] && echo "Installing entry into ${ROMFSDIR}${dst}."
		if [ -s ${ROMFSDIR}${dst} ] ; then
			# if file lacks a trailing new line, add it before appending the text
			if [ $(tail -n1 ${ROMFSDIR}${dst} | tr -d '\n' | wc -c) = $(tail -n1 ${ROMFSDIR}${dst} | wc -c) ] ; then
				echo "" >> ${ROMFSDIR}${dst} || return 1
			fi
		fi
		echo "${src}" >> ${ROMFSDIR}${dst} || return 1
	fi
	setattrs ${ROMFSDIR}${dst}
	return $?
}

#############################################################################

file_modify()
{
	[ -f ${ROMFSDIR}${dst} ] || return 1
	setattrs ${ROMFSDIR}${dst}
	return $?
}

#############################################################################

hard_link()
{
	rm -f ${ROMFSDIR}${dst}
	[ "$v" ] && echo "ln ${src} ${ROMFSDIR}${dst}"
	ln ${ROMFSDIR}${src} ${ROMFSDIR}${dst}
	return $?
}

#############################################################################

sym_link()
{
	rm -f ${ROMFSDIR}${dst}
	[ "$v" ] && echo "ln -s ${src} ${ROMFSDIR}${dst}"
	ln -sf ${src} ${ROMFSDIR}${dst}
	return $?
}

#############################################################################

cpp_file()
{
	if [ -d ${ROMFSDIR}${dst} ]; then
		dstfile=${ROMFSDIR}${dst}/`basename ${src}`
	else
		dstfile=${ROMFSDIR}${dst}
	fi
	rm -f ${dstfile}
	[ "$v" ] && echo "${CROSS_COMPILE}cpp ${CFLAGS} -P < ${src} > ${dstfile}"
	${CROSS_COMPILE}cpp ${CFLAGS} -P < ${src} > ${dstfile}
	return $?
}

#############################################################################

rm_file()
{
	# if ROMFSDIR is not set, play it safe
	[ "${ROMFSDIR}" ] || return 1
	if [ -d "${ROMFSDIR}${dst}" ]; then
		echo "rm -rf ${ROMFSDIR}${dst}"
		rm -rf "${ROMFSDIR}${dst}"
	else
		echo "rm -f ${ROMFSDIR}${dst}"
		rm -f "${ROMFSDIR}${dst}"
	fi
	return $?
}

#############################################################################
#
# main program entry point
#

v=
V=
option=y
noption=
pattern=
perm=
func=file_copy
mdir=
src=
dst=
strip=1
kernmod=
r=
follow=L
caps=
owner=

while getopts 'VdRfSMvcme:E:o:O:A:p:a:l:s:r:C:w:' opt "$@"
do
	case "$opt" in
	v) v="1";                           ;;
	V) V="1";                           ;;
	d) mdir="1";                        ;;
	f) follow=;                         ;;
	S) strip=;							;;
	M) kernmod="1";                     ;;
	o) option="$OPTARG";                ;;
	O) noption="$OPTARG";               ;;
	e) eval option=\"\$$OPTARG\";       ;;
	E) eval noption=\"\$$OPTARG\";      ;;
	p) perm="$OPTARG";                  ;;
	a) src="$OPTARG"; func=file_append; ;;
	A) pattern="$OPTARG";               ;;
	l) src="$OPTARG"; func=hard_link;   ;;
	s) src="$OPTARG"; func=sym_link;    ;;
	r) ROMFSDIR="$OPTARG"; r=1;         ;;
	c) func=cpp_file;                   ;;
	R) func=rm_file;                    ;;
	C) caps="$OPTARG";                  ;;
	w) owner="$OPTARG";                 ;;
	m) func=file_modify;                ;;

	*)  break ;;
	esac
#
#	process option here to get an ANDing effect
#
	case "$option" in
	*[mMyY]*) # this gives OR effect, ie., nYn
		;;
	*)
		[ "$V" ] && echo "Condition not satisfied."
		exit 0
		;;
	esac

#
#	process negative options here to get an ANDing effect
#
	case "${noption:-n}" in
	*[nN]*) # this gives OR effect, ie., yNy
		;;
	*)
		[ "$V" ] && echo "Condition not satisfied."
		exit 0
		;;
	esac
done

if [ -z "$ROMFSDIR" -a -z "$r" ]
then
	echo "ROMFSDIR is not set" >&2
	usage
	exit 1
fi

if [ -z "$STRIPTOOL" ]
then
	STRIPTOOL=strip
fi	

shift `expr $OPTIND - 1`

case $# in
1)
	dst="$1"
	if [ -z "$src" ]
	then
		src="`basename $dst`"
	fi
	;;
2)
	if [ ! -z "$src" ]
	then
		echo "Source file already provided" >&2
		exit 1
	fi
	src="$1"
	dst="$2"
	;;
*)
	usage
	;;
esac

if [ -n "$kernmod" ]; then
	strip=
	kerndir=${ROOTDIR}/${LINUXDIR}
	# could prob take from UTS headers as well ...
	kernver=$(cat ${kerndir}/include/config/kernel.release)
	dst="/lib/modules/${kernver}/${dst}"
fi

if [ "$mdir" -a ! -d "`dirname ${ROMFSDIR}${dst}`/." ]
then
	mkdir -p "`dirname ${ROMFSDIR}${dst}`/." || exit 1
fi

$func || exit 1

exit 0

#############################################################################
