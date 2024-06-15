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


-- inserisco i dati dei colli evasi/spediti divisi per cedola e tipo : fuori formato (FF)/bancali (P)
insert into statistica_tmp (data,cedola,ordini)
select 
	date('now') as data,
	r.ronmced as cedola, 
	count(*)
from ric_ord r
where 
	rostato in ('E','H') and
	date(rotmeva) = date('now')
group by data,ronmced;

-- inserisco i dati dei colli evasi/spediti divisi per cedola e tipo : fuori formato (FF)/bancali (P)
-- colli normali (non F o P)
insert into statistica_tmp (data,cedola,colli)
select 
	date('now') as data,
	r.ronmced as cedola, 
	count(*)
from ric_ord r, col_prod c
where 
	c.ordprog in 
	(select distinct ordprog from col_prod where cpstato in ('E','H')) and
	cptpfor not in ('F','P') and r.ordprog = c.ordprog and 
	date(cptmeva) = date('now')
group by data,ronmced;

-- colli F
insert into statistica_tmp (data,cedola,colli_ff)
select 
	date('now') as data,
	r.ronmced as cedola, 
	count(*)
from ric_ord r, col_prod c
where 
	c.ordprog in 
	(select distinct ordprog from col_prod where cpstato in ('E','H')) and
	cptpfor in ('F') and r.ordprog = c.ordprog and 
	date(cptmeva) = date('now')
group by data,ronmced;

-- colli P
insert into statistica_tmp (data,cedola,colli_p)
select 
	date('now') as data,
	r.ronmced as cedola, 
	count(*)
from ric_ord r, col_prod c
where 
	c.ordprog in 
	(select distinct ordprog from col_prod where cpstato in ('E','H')) and
	cptpfor in ('P') and r.ordprog = c.ordprog and 
	date(cptmeva) = date('now')
group by data,ronmced;



-- inserisco i dati delle righe dei colli evasi/spediti divisi per cedola e tipo : fuori formato (FF)/bancali (P)
-- colli normali (non F o P)
insert into statistica_tmp (data,cedola,righe_colli,copie_colli,righe_prelevate,copie_prelevate) 
select 
	date('now') as data,
	r.ronmced as cedola, 
	sum(cpnmrgh) as righe_colli ,
	sum(cpnmcpe) as copie_colli ,
	-- le righe dei colli in oggetto sono sicuramente prelevate
	sum(cpnmrgh) as righe_prelevate ,
	sum(cpnmcpe) as copie_prelevate 
from 
	col_prod c,ric_ord r 
where 
	c.ordprog in 
		(select distinct ordprog from col_prod where cpstato in ('E','H')) and 
	cptpfor not in ('F','P') and r.ordprog = c.ordprog and 
	cptmeva > date('now') and cptmeva < date('now') + interval '1 day'
group by data,ronmced;

-- colli FF 
insert into statistica_tmp (data,cedola,righe_colli_ff,copie_colli_ff,righe_prelevate_ff,copie_prelevate_ff) 
select 
	date('now') as data,
	r.ronmced as cedola, 
	sum(cpnmrgh) as righe_colli_ff,
	sum(cpnmcpe) as copie_colli_ff,
	-- le righe dei colli in oggetto sono sicuramente prelevate
	sum(cpnmrgh) as righe_prelevate_ff ,
	sum(cpnmcpe) as copie_prelevate_ff 
from 
	col_prod c,ric_ord r 
where 
	c.ordprog in 
		(select distinct ordprog from col_prod where cpstato in ('E','H')) and 
	cptpfor='F' and r.ordprog = c.ordprog and 
	cptmeva > date('now') and cptmeva < date('now') + interval '1 day'
group by data,ronmced
order by data;

-- colli P 
insert into statistica_tmp (data,cedola,righe_colli_p,copie_colli_p,righe_prelevate_p,copie_prelevate_p) 
select 
	date('now') as data,
	r.ronmced as cedola, 
	sum(cpnmrgh) as righe_colli_p,
	sum(cpnmcpe) as copie_colli_p,
	-- le righe dei colli in oggetto sono sicuramente prelevate
	sum(cpnmrgh) as righe_prelevate_p ,
	sum(cpnmcpe) as copie_prelevate_p 
from 
	col_prod c,ric_ord r 
where 
	c.ordprog in 
		(select distinct ordprog from col_prod where cpstato in ('E','H')) and 
	cptpfor='P' and r.ordprog = c.ordprog and 
	cptmeva > date('now') and cptmeva < date('now') + interval '1 day'
group by data,ronmced
order by data;


--
-- a questo punto mi mancano le righe prelevate di ordini in stato  P o T
--

-- colli normali (non F o P)
insert into statistica_tmp (data,cedola,righe_prelevate,copie_prelevate) 
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
-- colli FF (F)
insert into statistica_tmp (data,cedola,righe_prelevate_ff,copie_prelevate_ff) 
select
	date('now') as data,
	r.ronmced as cedola, 
	count(*)     as righe_prelevate_ff ,
	sum(rpqtspe) as copie_prelevate_ff 
from 
	col_prod c,ric_ord r , rig_prod p
where 
	r.ordprog in (select ordprog from ric_ord where rostato in ('P','T')) and 
	cptpfor in ('F') and r.ordprog = c.ordprog and p.ordprog=c.ordprog and p.rpnmcol = c.cpnmcol 
	and rptmpre > date('now') and rptmpre < date('now') + interval '1 day'
	group by data,ronmced
;
-- colli bancale (P)
insert into statistica_tmp (data,cedola,righe_prelevate_p,copie_prelevate_p) 
select
	date('now') as data,
	r.ronmced as cedola, 
	count(*)     as righe_prelevate_p ,
	sum(rpqtspe) as copie_prelevate_p 
from 
	col_prod c,ric_ord r , rig_prod p
where 
	r.ordprog in (select ordprog from ric_ord where rostato in ('P','T')) and 
	cptpfor in ('P') and r.ordprog = c.ordprog and p.ordprog=c.ordprog and p.rpnmcol = c.cpnmcol 
	and rptmpre > date('now') and rptmpre < date('now') + interval '1 day'
	group by data,ronmced
;
