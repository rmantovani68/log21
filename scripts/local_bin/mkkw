#!/bin/bash

sort -u $1 | awk '
BEGIN { n = 0; }
$1 == "" || $1 ~ "^#.*" { next; }
{
	kt[n] = $1;
	if ($2 ~ "^id")
		kf[n] = 1;
	else
		kf[n] = 0;
	n = n+1;
}
END {
	print "\n/* FILE GENERATO AUTOMATICAMENTE A PARTIRE DA '$1' */"
	print "/* NON MODIFICARE MANUALMENTE */"
	print "\n\nstatic const SCNKW kwAll[] = {"
	for (i = 0; i < n; ++i) {
		print "{ \"" kt[i] "\"," kf[i] " },";
	}
	print "};\nenum kw_t {";
	for (i = 0; i < n; ++i) {
		print "KT_" kt[i] ",";
	}
	print "};\n";
}'

