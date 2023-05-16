#!/bin/bash

__CCFG=/tmp/.ccfg.$$

trap "[[ -f $__CCFG ]] && rm $__CCFG" EXIT

config_get() { # field
	if [[ ! -f $__CCFG ]]; then
		config -g config > $__CCFG
	fi
	if [[ "$1" ]]; then
		grep "$1" $__CCFG | cut -f2- -d' '
	fi
}
