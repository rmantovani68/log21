#!/usr/bin/awk -f

#
#

{ 
# <object class="GtkLabel" id="lb_stato_linea">
    
	#printf "$0  : %s\n",$0
	#printf "$1  : %s\n",$1
	#printf "$2  : %s\n",$2
	#printf "$3  : %s\n",$3
    print $0
    if($1=="<object"){
        s = split($2, a, "=", seps)
        class = a[2]
        s = split($3, a, "=", seps)
        name = a[2]

        # printf("class : %s id : %s\n", class, name)
        sub("\"", "", name)
        sub("\"", "", name)
        sub("<", "", name)
        sub(">", "", name)
        printf( "                <property name=\"name\">%s</property>\n", name)

        # printf("s : %d\n", s)
        # i=0
        # for(str in a){
        #     printf("i : %d - str : %s - ", i, a[i+1])
        #     i=i+1
        # }
        # printf("\n");
    }

}
