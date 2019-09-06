#!/usr/bin/env bash

function html_header() {
	echo "<html><head><title>Build results</title></head><body>" >"$1"
}

function html_footer() {
	echo "</body></html>" >>"$1"
}

# filter out which targets are valid for this build, bamboo
# always builds a superset of all targets at this time.  Branches can
# override valid targets with a BUILD_TARGETS file at the top
# level.  User can override further with a custom BUILD_TARGETS
# variable when building manual builds.

function valid_target() {
	_target="$1"

	[ -z "${bamboo_BUILD_TARGETS}" -a -f BUILD_TARGETS ] &&
		bamboo_BUILD_TARGETS="$(tr '[\n]' '[ ]' < BUILD_TARGETS)"

	# sanitise variables to prevent simple user typos
	bamboo_BUILD_TARGETS=$(echo -n "${bamboo_BUILD_TARGETS}" | sed -e 's/^[ ]*//g' -e 's/[ 	]$//')

	if [ -n "${bamboo_BUILD_TARGETS}" ]; then
		if ! echo " ${bamboo_BUILD_TARGETS} " | grep -q " ${_target} "; then
			# this target is not a valid build
			return 1
		fi
	fi

	return 0
}

function info() {
	if [ -z "$1" ]; then 
		usage
	fi

	version=$(ON_BUILD_MACHINE=1 bin/version)
	group=$(expr match "$version" '\([0-9]*\.[0-9]*\)')
	branch_regexp='^[1-9][0-9]\.(2|5|8|11)\.[0-9]{1,3}_branch$'
	if [[ "${bamboo_repository_branch_name}" =~ $branch_regexp ]]; then
		type=candidate
	else
		type=development
	fi
	artifacts=/public/builds/DAL/${type}/${group}/${version}

	start=$(date -u -R)
	
	echo	"export BUILD_COMMIT=${bamboo_repository_revision_number}" >"$1"
	echo	"export BUILD_TYPE=${type}" >>"$1"
	echo	"export BUILD_GROUP=${group}" >>"$1"
	echo	"export BUILD_VERSION=${version}" >>"$1"
	echo	"export BUILD_ARTIFACT_DIR=${artifacts}" >>"$1"
	echo -e	"export BUILD_START_STRING=\"${start}\"" >>"$1"
	echo	"export NO_BUILD_INTO_TFTPBOOT=1" >>"$1"
	echo	"export ON_BUILD_MACHINE=1" >>"$1"
	
	mkdir -p ${artifacts}
	cp "$1" ${artifacts}
	touch "${artifacts}/valid_artifacts.txt"

	exit 0
}

function build() {
	if [ -z "$1" ]; then 
		usage
	fi
	. "$1"
	
	if [ -z "${BUILD_COMMIT}" ]; then 
		echo "No commit hash found."
		exit 1
	fi
	git checkout "${BUILD_COMMIT}"

	target=$(echo "${bamboo_shortJobName}" | tr ' ' '/')
	if ! valid_target $target; then
		exit 0
	fi

	make ${target}_default && make release
	ec=$?
	
	# do kcheck after the build so we have the Kconfig* files handy
	case "$target" in
	*Factory82*|*8200-kboot*)
		;;
	*)
		tools/kcheck/check 2>&1
		ec=$(($ec + $?))
		;;
	esac

	git diff --exit-code
	ec=$(($ec + $?))
	
	artifacts=${BUILD_ARTIFACT_DIR}/${target}
	mkdir -p ${artifacts}
	
	if [ ${ec} -eq 0 ]; then
		# copy the artifacts if successful
		cp -rat ${artifacts} release/*
		ec=$?
	fi

	if [ ${ec} -eq 0 ]; then
		echo ${target} >> ${BUILD_ARTIFACT_DIR}/valid_artifacts.txt
	fi

	out=${artifacts}/result.html
	html_header "${out}"
	echo	"The exit code was ${ec}.<br/>" >>"${out}"
	echo -e "More information can be found on <a href=\"https://bamboo.digi.com/browse/${bamboo_buildResultKey}\">Bamboo</a>.<br/>" >>"${out}"
	html_footer "${out}"
	
	exit ${ec}
}

function report() {
	if [ -z "$1" -o -z "$2" ]; then 
		usage
	fi
	. "$1"

	html_header "$2"
	echo "Version: ${BUILD_VERSION}<br/>" >>"$2"
	echo "<ul>" >>"$2"
	for file in $(find ${BUILD_ARTIFACT_DIR} -name result.html); do
		dir=$(dirname $file)
		noname=${dir%/*/*}
		name=${dir#$noname}
		echo -e "<li><a href=\"http://eng.digi.com/${dir#/public/}\">${name#/}</a>" >>"$2"
	done
	echo "</ul>" >>"$2"
	html_footer "$2"
	
	exit 0
}

function usage() {
	echo "You're doing it wrong."
	exit 1
}

case "$1" in
info)	info "$2"
		;;
build)	build "$2"
		;;
report)	report "$2" "$3"
		;;
*)		usage
		;;
esac
exit 1
