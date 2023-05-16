#!/bin/bash

# Runtime SSH configuration
: ${SECRET_KEY_PATH:="/run/secrets/private_key"}
key=""
if [ -f "$SECRET_KEY_PATH" ] \
        && mount | grep -Fqs "$(dirname "$SECRET_KEY_PATH")"
then
    # There is a key mounted as a Docker secret
    key="$SECRET_KEY_PATH"

    if [ 600 != "$(stat -c %a "$key")" ]
    then
        echo "SSH private key permissions are invalid" >&2
    fi
fi

if [ -f "$key" ]
then
    install -m 0700 -d ~/.ssh
    # Write the key path into SSH configuration
    cat >~/.ssh/config <<-EOF
	Host *
	    StrictHostKeyChecking no
	    IdentityFile $key
	EOF
fi

: ${DAL_INSTALL_PATH:=~/dal}
[ -d "$DAL_INSTALL_PATH" ] && cd "$DAL_INSTALL_PATH"

eval "$@"
