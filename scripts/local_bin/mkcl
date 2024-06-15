#!/bin/bash

awk '
BEGIN { start = 0;
	print "\n/* FILE GENERATO AUTOMATICAMENTE A PARTIRE DA '$1' */"
	print "/* NON MODIFICARE MANUALMENTE */"
	print "\n\nstatic const char * pa_cb_names[] = {"
}
$0 == "/* LISTA DEI CODICI DI CALLBACK */" { start = 1; next; }
$0 == "/* FINE DELLA LISTA DEI CODICI DI CALLBACK */" { start = 0; next; }
start { print "\"" substr($1,4) "\","; next; }
END { print "};\n"; }' $1

