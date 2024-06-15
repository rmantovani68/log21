drop table stat_tmp;
create table stat_tmp as
select 
	anno(rotmrcz) as anno,
	ronmced::integer as cedola,
	rocdlin as linea,
	count(*) as ordini,
	sum(ronmcll) as colli,
	sum(ronmrgh) as righe,
	sum(ronmcpe) as copie,
	trunc(sum(ropspre)::numeric/1000::numeric,2)  as peso 
	from ric_ord_stor 
	group by 
	anno(rotmrcz),
	ronmced::integer,
	rocdlin
	order by 
	1,
	2	
	;

select anno,cedola,linea,ordini,colli,righe,copie,peso 
from stat_tmp order by 1,2,3;

select anno,cedola,count(linea) as linee,sum(ordini) as ordini,sum(colli) as colli,sum(righe) as righe,sum(copie) as copie,sum(peso) as peso 
from stat_tmp group by anno,cedola order by 1,2;
