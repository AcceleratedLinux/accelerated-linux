#! /bin/sh

#called by [update-check.sh]
#This scripts cleans the output of nslookup, converting a domain into an 
#IP address without extraneous information

nslookup_result=`nslookup "$1"`
results_start=`echo -e "$nslookup_result" | grep -n "$1" | cut -f1 -d: | tail -n 1`
results=`echo -e "$nslookup_result" | sed -n $results_start,100p | grep -o -m 1 '[[:digit:]]\{1,3\}\.[[:digit:]]\{1,3\}\.[[:digit:]]\{1,3\}\.[[:digit:]]\{1,3\}'`

if echo "$results" | grep '[[:digit:]]\{1,3\}\.[[:digit:]]\{1,3\}\.[[:digit:]]\{1,3\}\.[[:digit:]]\{1,3\}' > /dev/null; then
  echo "$results"
else
  exit 1
fi

#END
