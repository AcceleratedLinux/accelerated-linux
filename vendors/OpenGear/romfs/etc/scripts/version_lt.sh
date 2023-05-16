# sh

# Utility functions for comparing Opengear version strings

# Opengear version comparator: returns true iff version $1 < $2
# This should be the same as how netflash compares software
# version strings.
version_lt () {
	local v1=$1 v2=$2
	_numeralize_version v1
	_numeralize_version v2
	_version_lt_recursive $v1 $v2
}

# Recursively compares dot-separated integer components
# of a version string
_version_lt_recursive () {
     local n1=${1%%.*} n2=${2%%.*}
     if [ -n "$n1$n2" ] && [ "$n1" = "$n2" ]; then
         _version_lt_recursive ${1#*.} ${2#*.}
     else
     	 [ "$n1" -lt "$n2" ]
     fi
}

# Convert a version string of the form
#   [X[.Y[.Z[pN]]]]
# into a 5-dotted form
#   X.Y.Z.P.N form
# where p->P is one of:
#   p->0 b->1 (none)->2 u->3
#
# Omitted components are replaced with 0.
#
# eg 3.16.2p2 -> 3.16.2.0.2
#    2        -> 2.0.0.2.0
#
_numeralize_version () { # var
    local out
    case ${!1} in
	*.*.*p*) out=${!1%p*}.0.${!1#*p};;
	*.*.*b*) out=${!1%b*}.1.${!1#*b};;
	*.*.*u*) out=${!1%u*}.3.${!1#*u};;
	*.*.*)   out=${!1}.2.0;;
	*.*)     out=${!1}.0.2.0;;
	"")      out=0.0.0.2.0;;
    devbuild|branchbuild|CI-*) out=99.99.99.99.99;;
	*)       out=${!1}.0.0.2.0;;
    esac
    eval $1=\$out
}

# An awk function equivalent of version_lt
version_lt_awk='
    function version_lt(a,b) {
	a = _numeralize_version(a);
	b = _numeralize_version(b);
	while (int(a) == int(b)) {
	    if (!sub(/[^.]*\./, "", a)) break;
	    if (!sub(/[^.]*\./, "", b)) break;
	}
	return int(a) < int(b);
    }
    function _ndots(v) {
   	return gsub(/\./, "", v);
    }
    function _numeralize_version(v,    nd) {
	if (v == "devbuild" ||
	    v == "branchbuild" ||
	    v ~ /^CI-/)
	        v = "99.99.99.99.99";
	sub(/p/, ".0.", v)
	sub(/b/, ".1.", v)
	sub(/u/, ".3.", v)
	if (v == "") v = "0.0.0.2.0";
	nd = _ndots(v);
	if (nd == 0) v = v ".0.0.2.0";
	if (nd == 1) v = v   ".0.2.0";
	if (nd == 2) v = v     ".2.0";
        return v;
    }
'


# Self tests

_numeralize_version_self_test () {
	numeralize_eq () { local v=$1; _numeralize_version v; [ $v = $2 ]; }
	trap 'echo FAIL; exit 1' 0; set -ex 

	numeralize_eq ''	0.0.0.2.0
	numeralize_eq 1		1.0.0.2.0
	numeralize_eq 1.2	1.2.0.2.0
	numeralize_eq 1.2.3	1.2.3.2.0
	numeralize_eq 1.2.3p4	1.2.3.0.4
	numeralize_eq 1.2.3b4	1.2.3.1.4
	numeralize_eq 1.2.3u4	1.2.3.3.4
	numeralize_eq devbuild 99.99.99.99.99
	numeralize_eq branchbuild 99.99.99.99.99
	numeralize_eq CI-1234 99.99.99.99.99

	trap '' 0; echo OK
}

_version_lt_self_test () {
	not () { if "$@"; then false; else true; fi;  }
	trap 'echo FAIL; exit 1' 0; set -ex

	    version_lt 3.6.5     3.12.5
	not version_lt 3.12.5    3.6.5
	    version_lt 3.16.2p0  3.18
	not version_lt 3.18      3.16.2p0
	    version_lt 3.1.2p1   3.1.2p2
	    version_lt 3.1.2p4   3.1.2b3
	    version_lt 3.1.2b3   3.1.2
	    version_lt 3.1.2     3.1.2u0
	    version_lt 3.16.0    devbuild
	not version_lt CI-1234   3.18
	    version_lt 3.16.5u2  branchbuild

	trap '' 0; echo OK
}

_awk_version_lt_self_test () {
    version_lt () {
    	awk "$version_lt_awk
	       BEGIN { if (version_lt(\"$1\",\"$2\")) exit 0; else exit 1; }
	    "
    }
    _version_lt_self_test
}

_test_all () {
	_numeralize_version_self_test
	_version_lt_self_test
	_awk_version_lt_self_test
}

