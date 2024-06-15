select 
	r.ronmced,c.cppsrea,c.cptmeva,s.cppsrea,s.cptmpes,s.cppsrea-c.cppsrea,date_part('seconds',c.cptmeva-s.cptmpes),abs(datediff('seconds',c.cptmeva,s.cptmpes))
from 
	colli_scartati s , 
	(select ordprog,cpnmcol,cptmeva,cppsrea from col_prod  union select ordprog,cpnmcol,cptmeva,cppsrea from col_prod_stor) c,
	(select ordprog,ronmced from ric_ord  union select ordprog,ronmced from ric_ord_stor) r
where c.ordprog=r.ordprog and c.ordprog=s.ordprog and c.cpnmcol=s.cpnmcol and abs(s.cppsrea-c.cppsrea)>100
order by ronmced::integer;
