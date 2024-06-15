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
