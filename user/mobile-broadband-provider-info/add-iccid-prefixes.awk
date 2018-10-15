
BEGIN {
	FS=","
	while ((getline < "country-dialing-codes.txt") > 0) {
		gsub("-.*$","",$NF)
		country[tolower($2)] = $NF
		# print $1 " (" tolower($2) ") " country[tolower($2)] > "/dev/stderr"
	}
}


{
	# print country[tolower($1)] " ********* " $0 > "/dev/stderr"
	gsub("[0-9][0-9]* ", "89" country[tolower($1)] "&", $4)
	for (i = 1; i <= NF; i++)
		printf("%s,", $i)
	print ""
	# print country[tolower($1)] " --------- " $0 > "/dev/stderr"
}

