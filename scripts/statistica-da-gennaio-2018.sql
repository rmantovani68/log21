drop table if exists statistica_tmp;
create table statistica_tmp (
	data timestamp,
	cedola text,
	ordini integer,
	ordini_ff integer,
	colli integer,
	colli_ff integer,
	righe integer,
	righe_ff integer,
	copie integer,
	copie_ff integer
);
insert into statistica_tmp (data,cedola,ordini,colli,righe,copie) 
select 
	data as data,
	r.ronmced as cedola, 
	count(distinct(r.ordprog)) as ordini,
	count(*) as colli,
	sum(cpnmrgh) as righe ,
	sum(cpnmcpe) as copie 
from 
	date,col_prod_stor c,ric_ord_stor r 
where 
	c.ordprog in 
		(select distinct ordprog from col_prod_stor where cpstato in ('E','H')) and 
	cptpfor!='F' and r.ordprog = c.ordprog and 
	cptmeva > data and cptmeva < data+'1 day' 
group by data,ronmced;

insert into statistica_tmp (data,cedola,ordini_ff,colli_ff,righe_ff,copie_ff) 
select 
	data as data,
	r.ronmced as cedola, 
	count(distinct(r.ordprog)) as ordini_ff,
	count(*) as colli_ff,
	sum(cpnmrgh) as righe_ff,
	sum(cpnmcpe) as copie_ff
from 
	date,col_prod_stor c,ric_ord_stor r 
where 
	c.ordprog in 
		(select distinct ordprog from col_prod_stor where cpstato in ('E','H')) and 
	cptpfor='F' and r.ordprog = c.ordprog and 
	cptmeva > data and cptmeva < data+'1 day' 
group by data,ronmced
order by data;

select 
	to_char(data,'YYY-MM-DD') as data,
	cedola,
	sum(ordini) as ordini,
	sum(colli) as colli,
	sum(righe) as righe,
	sum(copie) as copie,
	sum(ordini_ff) as ordini_ff ,
	sum(colli_ff) as colli_ff,
	sum(righe_ff) as righe_ff,
	sum(copie_ff) as copie_ff
from statistica_tmp group by data,cedola order by data,cedola ;
