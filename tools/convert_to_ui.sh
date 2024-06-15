#!/bin/bash

# convert_to_ui.sh
# converte una lista di campi ricavata da \t in psql in un xml da inserire in una grid di gtk-builder
# usage convert_to_ui.sh <file> <packing position>
#

c=0
packing=$2
if [ a"$packing" = a ];then
    packing=0
fi
for i in `cat $1 | grep -o '^ [a-z]*| [a-z]*'`; do 
    if [ $c -eq  0 ];then 
        campo=$i
        c=1; 
        print=0
    elif [ $c -eq  1 ];then 
        c=2;
        print=0
    elif [ $c -eq  2 ];then 
        tipo=$i
        c=0; 
        print=1
    fi

    # echo "$tipo"
    if [ $print -eq 1 ];then
        echo '        <!-- name : '$campo' type : '$tipo' -->'
        echo '        <child>'
        echo '          <object class="GtkLabel" id="lb_'$campo'">'
        echo '            <property name="name">lb_'$campo'</property>'
        echo '            <property name="visible">True</property>'
        echo '            <property name="can-focus">False</property>'
        echo '            <property name="xpad">5</property>'
        echo '            <property name="ypad">0</property>'
        echo '            <property name="label" translatable="yes">campo '$campo'</property>'
        echo '            <property name="justify">center</property>'
        echo '            <property name="width-chars">13</property>'
        echo '            <property name="xalign">0</property>'
        echo '            <property name="yalign">0.5</property>'
        echo '          </object>'
        echo '          <packing>'
        echo '            <property name="left-attach">0</property>'
        echo '            <property name="top-attach">'$packing'</property>'
        echo '          </packing>'
        echo '        </child>'
        if [ "$tipo" = "text" ];then
            echo '        <child>'
            echo '          <object class="GtkEntry" id="entry_'$campo'">'
            echo '            <property name="name">entry_'$campo'</property>'
            echo '            <property name="visible">True</property>'
            echo '            <property name="can-focus">True</property>'
            echo '            <property name="invisible-char">●</property>'
            echo '            <property name="width-chars">10</property>'
            echo '          </object>'
            echo '          <packing>'
            echo '            <property name="left-attach">1</property>'
            echo '            <property name="top-attach">'$packing'</property>'
            echo '          </packing>'
            echo '        </child>'
        fi
        if [ "$tipo" = "integer" ];then
            echo '        <child>'
            echo '          <object class="GtkSpinButton" id="sb_'$campo'">'
            echo '            <property name="name">sb_'$campo'</property>'
            echo '            <property name="visible">True</property>'
            echo '            <property name="can-focus">True</property>'
            echo '            <property name="text" translatable="yes">0</property>'
            echo '            <property name="climb-rate">1</property>'
            echo '          </object>'
            echo '          <packing>'
            echo '            <property name="left-attach">1</property>'
            echo '            <property name="top-attach">'$packing'</property>'
            echo '          </packing>'
            echo '        </child>'
        fi
        if [ "$tipo" = "real" ];then
            echo '        <child>'
            echo '          <object class="GtkSpinButton" id="sb_'$campo'">'
            echo '            <property name="name">sb_'$campo'</property>'
            echo '            <property name="visible">True</property>'
            echo '            <property name="can-focus">True</property>'
            echo '            <property name="text" translatable="yes">0</property>'
            echo '            <property name="climb-rate">1</property>'
            echo '          </object>'
            echo '          <packing>'
            echo '            <property name="left-attach">1</property>'
            echo '            <property name="top-attach">'$packing'</property>'
            echo '          </packing>'
            echo '        </child>'
        fi
        packing=$((packing+1))
    fi
done
