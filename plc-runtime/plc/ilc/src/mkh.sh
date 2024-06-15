#!/bin/bash

awk '
BEGIN	{

	base="'"$1"'";
	hpriv=base "_p.h";
	hprot=base "_c.h";
	src=base ".c";

	print "/* QUESTO FILE E` STATO GENERATO IN MODO AUTOMATICO A PARTIRE DA '$1_c.h' */";
	print "/* NON APPORTARE MODIFICHE QUI, MA SOLO IN " hprot " */";

	while (getline line < hprot) {
		print line;
	}

	print "/* QUESTO FILE E` STATO GENERATO IN MODO AUTOMATICO A PARTIRE DA '$1_c.h' */" > hpriv;
	print "/* NON APPORTARE MODIFICHE QUI, MA SOLO IN " src " */" >>hpriv;

	start = 0; fase = 0;
}
/^\/\* FUNZIONI \*\//	{ start = 1; next; }
start == 0 { next; }
/^#ifdef _NO_PROTO/ { fase = 1; linea[0] = $0; next; }
{
	if (fase == 1) {
		gsub("\\(.*","();");
		linea[fase] = $0;
		++fase;
		static = $1;
	}
	else if (fase == 2) {
		linea[fase] = $0;
		++fase;
	}
	else if (fase == 3) {
		gsub("\\)$",");");
		linea[fase] = $0;
		++fase;
	}
	else if (fase == 4) {
		linea[fase] = $0;
		for (i = 0; i < 5; ++i) {
			if (static == "static")
				print linea[i] >> hpriv;
			else
				print linea[i];
		}
		fase = 0;
	}
}
' $1.c > $1.h



