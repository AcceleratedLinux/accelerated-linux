trap 'echo; confirm "*** Control-C pressed, abort" && exit 1;' INT

function logit {
	msg="$1"   
	echo "$msg"                      
	logger -t "$(basename $0)" "$msg"
}

function confirm {                                       
	msg="$1"                                         
	while true; do
		read -p "$msg [y/n]? " -n 1 ; echo
		[[ "$REPLY" == y || "$REPLY" == Y ]] && return 0
		[[ "$REPLY" == n || "$REPLY" == N ]] && return 1
	done
}  

function get_pass {
	msg="$1"
	var="$2"
	pass=""
	while [[ -z "$pass" ]]; do
		read -s -p "$msg: " pass1
		echo ''
		read -s -p "$msg (confirm): " pass2
		echo ''
		[[ "$pass1" == "$pass2" ]] && pass="$pass1"
	done
	eval "$var=\""$pass\"""
}

function hit_ui {
	data="$1"
	sid=$( printf %04x $RANDOM; printf %04x $RANDOM )
	
	if [[ -d /var/run/.sessions ]]; then    
		sessiondir=/var/run/.sessions
	else
		sessiondir=/var/run         
	fi
	
	sessionfile=${sessiondir}/session-$sid
	> $sessionfile ; chown root:admin $sessionfile ; chmod 660 $sessionfile
	echo -e "username = root\nexpiry_time = 2147483647\nauth_pid = 1\nauth_state = 7\nsession_len = 20\nautorefresh = true" \
		> $sessionfile
		
	html=$( curl \
		--cookie OgSessionId=$sid \
		--data "$data" \
		http://127.0.0.1/ )

	rm -f $sessionfile
	
	echo "$html" 2>&1 | grep error-message && return 1
	return 0
}
