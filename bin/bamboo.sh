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

	[ -d vendors/$_target ] || return 1

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
	branch_regexp='^[1-9][0-9]\.(2|5|8|11)\.[0-9]{1,3}_branch$|_sprint$'
	if [[ "${bamboo_repository_branch_name}" =~ $branch_regexp ]]; then
		type=candidate
	else
		type=development
	fi

	artifact_dir=/public/builds/DAL/${type}/${group}/${version}
	mkdir -p ${artifact_dir}

	artifacts_file=${artifact_dir}/unconfirmed_artifacts.txt
	touch ${artifacts_file}

	start=$(date -u -R)

	echo	"export BUILD_COMMIT=${bamboo_repository_revision_number}" >"$1"
	echo	"export BUILD_TYPE=${type}" >>"$1"
	echo	"export BUILD_GROUP=${group}" >>"$1"
	echo	"export BUILD_VERSION=${version}" >>"$1"
	echo	"export BUILD_ARTIFACT_DIR=${artifact_dir}" >>"$1"
	echo	"export BUILD_ARTIFACTS_FILE=${artifacts_file}" >>"$1"
	echo -e	"export BUILD_START_STRING=\"${start}\"" >>"$1"
	echo	"export NO_BUILD_INTO_TFTPBOOT=1" >>"$1"
	echo	"export ON_BUILD_MACHINE=1" >>"$1"

	if [ "${bamboo_MIGRATE_RELEASE_VERSION}" ]; then
		version=$(echo "$bamboo_MIGRATE_RELEASE_VERSION" | awk -F "-" '{print $1}')
		migrate_build_dir="/public/builds/DAL/release/$version"
		echo "export MIGRATE_BUILD_DIR=${migrate_build_dir}" >>"$1"
		echo "export MIGRATE_RELEASE_VERSION=${bamboo_MIGRATE_RELEASE_VERSION}" >>"$1"
	fi

	cp "$1" ${artifact_dir}

	exit 0
}

function build() {
	target=$(echo "${bamboo_shortJobName}" | tr ' ' '/')
	if ! valid_target $target; then
		echo "Skipping invalid target: $target."
		exit 0
	fi

	if grep --silent "^${target}$" ${BUILD_ARTIFACTS_FILE}; then
		echo "Previous artifacts found."
		exit 1
	fi

	if [ -z "${BUILD_COMMIT}" ]; then
		echo "No commit hash found."
		exit 1
	fi
	git checkout "${BUILD_COMMIT}"

	# enable static analysis by default (can disable from bamboo)
	export ANALYZE=1
	[ "$bamboo_ANALYZE" ] && ANALYZE="$bamboo_ANALYZE"

	make ${target}_default && make release
	ec=$?

	artifacts=${BUILD_ARTIFACT_DIR}/${target}
	mkdir -p ${artifacts}

	# do kcheck after the build so we have the Kconfig* files handy
	case "$target" in
	*Factory82*|*8200-kboot*)
		;;
	*)
		tools/kcheck/check 2>&1
		ec=$(($ec + $?))
		tools/kcheck/test > "${artifacts}/features.json" 2>/dev/null
		;;
	esac

	git diff --exit-code
	ec=$(($ec + $?))

	if [ ${ec} -eq 0 ]; then
		# copy the artifacts if successful
		cp -rat ${artifacts} release/*
		ec=$?
	fi

	if [ ${ec} -eq 0 ]; then
		echo "${target}" >> "${BUILD_ARTIFACTS_FILE}"
		ec=$?
	fi

	out=${artifacts}/result.html
	html_header "${out}"
	echo	"The exit code was ${ec}.<br/>" >>"${out}"
	echo -e "More information can be found on <a href=\"https://bamboo.digi.com/browse/${bamboo_buildResultKey}\">Bamboo</a>.<br/>" >>"${out}"
	html_footer "${out}"

	exit ${ec}
}

function report() {
	if [ -z "$1" ]; then
		usage
	fi

	html_header "$1"
	echo "Version: ${BUILD_VERSION}<br/>" >>"$1"
	echo "<ul>" >>"$1"
	for file in $(find ${BUILD_ARTIFACT_DIR} -name result.html); do
		dir=$(dirname $file)
		noname=${dir%/*/*}
		name=${dir#$noname}
		echo -e "<li><a href=\"http://eng.digi.com/${dir#/public/}\">${name#/}</a>" >>"$1"
	done
	echo "</ul>" >>"$1"
	html_footer "$1"

	exit 0
}

function ratify() {
	mv ${BUILD_ARTIFACT_DIR}/unconfirmed_artifacts.txt ${BUILD_ARTIFACT_DIR}/valid_artifacts.txt
	ec=$?

    exit ${ec}
}

function upload() {
	uploader="$1"
	instance="$2"

	descriptors=$(find $BUILD_ARTIFACT_DIR -name \*-rci_descriptors.tar.gz)
	if [ -z "$descriptors" ]; then
		echo "No descriptors found."
		exit 0
	fi
    options=""
	if [[ $string == *DEVONLY* ]]; then
    	options="$options -f"
    fi
	$uploader ${options} ${bamboo_RM_USERNAME} ${bamboo_RM_PASSWORD} $instance $descriptors
	ec=$?

    exit ${ec}
}

function usage() {
	echo "You're doing it wrong."
	exit 1
}

function signing_intermediate() {

	if [ "$BUILD_TYPE" == "candidate" ]; then
		# temporarily validate some artifacts for signing.
		cp ${BUILD_ARTIFACT_DIR}/unconfirmed_artifacts.txt ${BUILD_ARTIFACT_DIR}/valid_artifacts.txt
		python3 dal/prop/bin/sign-images.py "$BUILD_ARTIFACT_DIR"
		rc=$?
		rm -f ${BUILD_ARTIFACT_DIR}/valid_artifacts.txt
		exit $rc
	fi
	exit 0
}

function signing_final() {

	if [ "$BUILD_TYPE" == "candidate" ]; then
		python3 dal/prop/bin/sign-images.py "$BUILD_ARTIFACT_DIR"
		exit $?
	fi
	exit 0
}

#retry a command usage: retry "command" number_of_retries sleep_between_retries_in_seconds
function retry() {

	local cmd="$1"
	local max_tries="$2"
	local sleep_period="$3"

	counter=0
	until [ "$counter" -eq $max_tries ]
	do
		$cmd && break
		counter=$((counter+1))
		sleep $sleep_period
	done

	if [ "$counter" -eq $max_tries ]; then
		echo "Unable to excute $cmd"
		return 1
	fi
}

function docker_clean() {

	if [ -n "$1" ]; then

		#Copy docker_dal_build_tag file
		retry "cp /public/DAL/docker_dal_build_tag ." 5 3 || exit 1

		#Extract the tag from file
		tag=$(sed -n -e 's/buildtag=//p' < docker_dal_build_tag)

	else
		tag=${bamboo_docker_buildtag}
	fi

	if [ -n "$tag" ]; then

		#Check if docker image is available with tag
		doc_img=$(docker images | grep $tag)

		#Delete old images, required to free up memory.
		if [ -z "$doc_img" ]; then
			docker rmi $(docker images | grep 'dal_build')
		fi
	fi

	exit 0
}

case "$1" in
info)	info "$2"
		;;
build)	build
		;;
report)	report "$2"
		;;
ratify)	ratify
		;;
upload)	upload "$2" "$3"
		;;
signing_intermediate) signing_intermediate
		;;
signing_final) signing_final
		;;
docker_clean) docker_clean "$2"
		;;
*)		usage
		;;
esac
exit 1
