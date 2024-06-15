drop table if exists righe_tmp;
create table righe_tmp (
	ronmced text,
	ordprog text,
	rpnmcol integer,
	rpcdpro text,
	rpqtspe integer,
	rptmpre timestamp
);

insert into righe_tmp (ronmced,ordprog,rpnmcol,rpcdpro,rpqtspe,rptmpre) select
	r.ronmced,r.ordprog,rpnmcol,rpcdpro,rpqtspe,coalesce(rptmpre, cptmeva) as rptmpre
from
	ric_ord r,col_prod c,rig_prod p
where
	c.ordprog in
	(select distinct ordprog from col_prod where cpstato in ('P','T','E','H')) and
	cptpfor not in ('F','P') and
	r.ordprog = c.ordprog and p.ordprog=c.ordprog and p.rpnmcol=c.cpnmcol 
;

