# ----------------------------------------
# Cancellazione delle stape in coda
# ----------------------------------------
echo 'Cancellazione stampe in corso ...'
lpq -P lp   | while read a b c d e; do echo lprm -P lp   $c; lprm -P lp   $c;done 
lpq -P ser  | while read a b c d e; do echo lprm -P ser  $c; lprm -P ser  $c;done 
lpq -P ser2 | while read a b c d e; do echo lprm -P ser2 $c; lprm -P ser2 $c;done 
echo 'Cancellazione Stampe Terminata'
