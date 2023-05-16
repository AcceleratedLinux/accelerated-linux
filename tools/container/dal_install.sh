#!/bin/bash

set -e

# Download DAL source repository
: ${DAL_INSTALL_PATH:=~/dal}
: ${DAL_CLONE_URL:="ssh://git@stash.digi.com/acl/accelerated-linux.git"}
: ${DAL_BRANCH:="master"}
if ! [ -d "$DAL_INSTALL_PATH"/.git ]
then
    git clone "$DAL_CLONE_URL" -b "$DAL_BRANCH" "$DAL_INSTALL_PATH"
fi

# Set up for cross-gdb
# FIXME This won't persist unless we manually commit an image from it
if ! [ -f ~/.gdbinit ] || ! grep -F -q romfs ~/.gdbinit
then
    cat >~/.gdbinit <<-EOF
	set sysroot "$DAL_INSTALL_PATH"/romfs
	EOF
fi
