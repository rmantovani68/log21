select
	date('now') as data,
	r.ronmced as cedola, 
	count(*)     as righe_prelevate ,
	sum(rpqtspe) as copie_prelevate 
from 
	col_prod c,ric_ord r , rig_prod p
where 
	r.ordprog in (select ordprog from ric_ord where rostato in ('P','T')) and 
	cptpfor not in ('F','P') and r.ordprog = c.ordprog and p.ordprog=c.ordprog and p.rpnmcol = c.cpnmcol 
	and rptmpre > date('now') and rptmpre < date('now') + interval '1 day'
	group by data,ronmced
;

