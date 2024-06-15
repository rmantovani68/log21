drop table if exists statistica_tmp;
create table statistica_tmp (
	data timestamp,
	cedola text,
	ordini integer default 0,
	ordini_ff integer default 0,
	ordini_p integer default 0,
	colli integer default 0,
	colli_ff integer default 0,
	colli_p integer default 0,
	righe_colli integer default 0,
	righe_colli_ff integer default 0,
	righe_colli_p integer default 0,
	copie_colli integer default 0,
	copie_colli_ff integer default 0,
	copie_colli_p integer default 0,
	righe_prelevate integer default 0,
	righe_prelevate_ff integer default 0,
	righe_prelevate_p integer default 0,
	copie_prelevate integer default 0,
	copie_prelevate_ff integer default 0,
	copie_prelevate_p integer default 0
);

drop table if exists date_series;
create table date_series (
	data timestamp 
);

insert into date_series (data)
select data from  generate_series('2018-01-01',date('now'),'1 day') as data;

-- inserisco tutte le date
insert into statistica_tmp (data,cedola)
select 
	date(data) as data,
	ronmced as cedola
from date_series d  left outer join ric_ord_stor on (date(rotmeva) = date(data))
;

-- inserisco i dati dei colli evasi/spediti divisi per cedola e tipo : fuori formato (FF)/bancali (P)
insert into statistica_tmp (data,cedola,ordini)
select 
	date(data) as data,
	ronmced as cedola, 
	count(*)
from ric_ord_stor r, date_series d
 where 
	date(rotmeva) = date(data) and 
 	r.rostato in ('E','H') 
group by data,ronmced
order by data,ronmced
;

-- inserisco i dati dei colli normali (non F o P) in linea/evasi/spediti divisi per cedola e tipo : fuori formato (FF)/bancali (P)
-- colli normali (non F o P)
insert into statistica_tmp (data,cedola,colli)
select 
	date(data) as data,
	r.ronmced as cedola, 
	count(*)
from ric_ord_stor r, col_prod_stor c , date_series d 
where 
	c.ordprog in 
	(select distinct ordprog from col_prod_stor where cpstato in ('E','H')) and
	cptpfor not in ('F','P') and r.ordprog = c.ordprog and 
	date(cptmeva) = date(data)
group by data,ronmced
order by data,ronmced
;

-- colli F
insert into statistica_tmp (data,cedola,colli_ff)
select 
	date(data) as data,
	r.ronmced as cedola, 
	count(*)
from ric_ord_stor r, col_prod_stor c, date_series
where 
	c.ordprog in 
	(select distinct ordprog from col_prod_stor where cpstato in ('E','H')) and
	cptpfor in ('F') and r.ordprog = c.ordprog and 
	date(cptmeva) = date(data)
group by data,ronmced
order by data,ronmced
;

-- colli P
insert into statistica_tmp (data,cedola,colli_p)
select 
	date(data) as data,
	r.ronmced as cedola, 
	count(*)
from ric_ord_stor r, col_prod_stor c, date_series
where 
	c.ordprog in 
	(select distinct ordprog from col_prod_stor where cpstato in ('E','H')) and
	cptpfor in ('P') and r.ordprog = c.ordprog and 
	date(cptmeva) = date(data)
group by data,ronmced
order by data,ronmced
;



-- inserisco i dati delle righe dei colli evasi/spediti divisi per cedola e tipo : fuori formato (FF)/bancali (P)

-- colli normali (non F o P)
insert into statistica_tmp (data,cedola,righe_colli,copie_colli)
select 
	date(data) as data,
	r.ronmced as cedola, 
	sum(cpnmrgh) as righe_colli ,
	sum(cpnmcpe) as copie_colli 
from 
	col_prod_stor c,ric_ord_stor r , date_series
where 
	c.ordprog in 
		(select distinct ordprog from col_prod_stor where cpstato in ('E','H')) and 
	cptpfor not in ('F','P') and r.ordprog = c.ordprog and 
	date(cptmeva) = date(data)
group by data,ronmced
order by data,ronmced
;
 
-- colli normali (non F o P)
insert into statistica_tmp (data,cedola,righe_prelevate,copie_prelevate) 
select
	date(data) as data,
	r.ronmced as cedola, 
	count(*)     as righe_prelevate ,
	sum(rpqtspe) as copie_prelevate 
from 
	col_prod_stor c,ric_ord_stor r , rig_prod_stor p, date_series
where 
	r.ordprog in (select ordprog from ric_ord_stor where rostato in ('P','T','E','H')) and 
	cptpfor not in ('F','P') and r.ordprog = c.ordprog and p.ordprog=c.ordprog and p.rpnmcol = c.cpnmcol 
	and date(rptmpre) = date(data)
	group by data,ronmced
	order by data,ronmced
;

-- colli FF 
insert into statistica_tmp (data,cedola,righe_colli_ff,copie_colli_ff,righe_prelevate_ff,copie_prelevate_ff) 
select 
	date(data) as data,
	r.ronmced as cedola, 
	sum(cpnmrgh) as righe_colli_ff,
	sum(cpnmcpe) as copie_colli_ff,
	-- le righe dei colli in oggetto sono sicuramente prelevate
	sum(cpnmrgh) as righe_prelevate_ff ,
	sum(cpnmcpe) as copie_prelevate_ff 
from 
	col_prod_stor c,ric_ord_stor r , date_series
where 
	c.ordprog in 
		(select distinct ordprog from col_prod_stor where cpstato in ('E','H')) and 
	cptpfor='F' and r.ordprog = c.ordprog and 
	date(cptmeva) = date(data)
group by data,ronmced
order by data,ronmced
;

-- colli P 
insert into statistica_tmp (data,cedola,righe_colli_p,copie_colli_p,righe_prelevate_p,copie_prelevate_p) 
select 
	date(data) as data,
	r.ronmced as cedola, 
	sum(cpnmrgh) as righe_colli_p,
	sum(cpnmcpe) as copie_colli_p,
	-- le righe dei colli in oggetto sono sicuramente prelevate
	sum(cpnmrgh) as righe_prelevate_p ,
	sum(cpnmcpe) as copie_prelevate_p 
from 
	col_prod_stor c,ric_ord_stor r , date_series
where 
	c.ordprog in 
		(select distinct ordprog from col_prod_stor where cpstato in ('E','H')) and 
	cptpfor='P' and r.ordprog = c.ordprog and 
	date(cptmeva) = date(data)
group by data,ronmced
order by data,ronmced
;
