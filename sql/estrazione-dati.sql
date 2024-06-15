-- estraggo i colli tipo 1

drop  table estrazione_dati_cedole_colli_1;
create table estrazione_dati_cedole_colli_1 
(
	anno integer,
	cedola integer,
	F1 integer
);

insert into estrazione_dati_cedole_colli_1 ( anno , cedola , F1 ) 
select 
		anno(rotmrcz)::integer ,
		ronmced::integer ,
		count(*)
	from ric_ord_stor r, col_prod_stor c
	where anno(rotmrcz)::integer in (2014,2015) and c.ordprog=r.ordprog and cptpfor='1'
	group by anno(rotmrcz),ronmced 
	order by anno(rotmrcz),ronmced
;
-- estraggo i colli tipo 2

drop  table estrazione_dati_cedole_colli_2;
create table estrazione_dati_cedole_colli_2 
(
	anno integer,
	cedola integer,
	F2 integer
);

insert into estrazione_dati_cedole_colli_2 ( anno , cedola , F2 ) 
select 
		anno(rotmrcz)::integer ,
		ronmced::integer ,
		count(*)
	from ric_ord_stor r, col_prod_stor c
	where anno(rotmrcz)::integer in (2014,2015) and c.ordprog=r.ordprog and cptpfor='2'
	group by anno(rotmrcz),ronmced 
	order by anno(rotmrcz),ronmced
;
-- estraggo i colli tipo 3

drop  table estrazione_dati_cedole_colli_3;
create table estrazione_dati_cedole_colli_3 
(
	anno integer,
	cedola integer,
	F3 integer
);

insert into estrazione_dati_cedole_colli_3 ( anno , cedola , F3 ) 
select 
		anno(rotmrcz)::integer ,
		ronmced::integer ,
		count(*)
	from ric_ord_stor r, col_prod_stor c
	where anno(rotmrcz)::integer in (2014,2015) and c.ordprog=r.ordprog and cptpfor='3'
	group by anno(rotmrcz),ronmced 
	order by anno(rotmrcz),ronmced
;
-- estraggo i colli tipo F

drop  table estrazione_dati_cedole_colli_F;
create table estrazione_dati_cedole_colli_F 
(
	anno integer,
	cedola integer,
	FF integer
);

insert into estrazione_dati_cedole_colli_F ( anno , cedola , FF ) 
select 
		anno(rotmrcz)::integer ,
		ronmced::integer ,
		count(*)
	from ric_ord_stor r, col_prod_stor c
	where anno(rotmrcz)::integer in (2014,2015) and c.ordprog=r.ordprog and cptpfor='F'
	group by anno(rotmrcz),ronmced 
	order by anno(rotmrcz),ronmced
;
-- estraggo i colli tipo C

drop  table estrazione_dati_cedole_colli_C;
create table estrazione_dati_cedole_colli_C 
(
	anno integer,
	cedola integer,
	FC integer
);

insert into estrazione_dati_cedole_colli_C ( anno , cedola , FC ) 
select 
		anno(rotmrcz)::integer ,
		ronmced::integer ,
		count(*)
	from ric_ord_stor r, col_prod_stor c
	where anno(rotmrcz)::integer in (2014,2015) and c.ordprog=r.ordprog and cptpfor='C'
	group by anno(rotmrcz),ronmced 
	order by anno(rotmrcz),ronmced
;
-- estraggo i colli tipo P

drop  table estrazione_dati_cedole_colli_P;
create table estrazione_dati_cedole_colli_P 
(
	anno integer,
	cedola integer,
	FP integer
);

insert into estrazione_dati_cedole_colli_P ( anno , cedola , FP ) 
select 
		anno(rotmrcz)::integer ,
		ronmced::integer ,
		count(*)
	from ric_ord_stor r, col_prod_stor c
	where anno(rotmrcz)::integer in (2014,2015) and c.ordprog=r.ordprog and cptpfor='P'
	group by anno(rotmrcz),ronmced 
	order by anno(rotmrcz),ronmced
;

-- estraggo i titoli

drop  table estrazione_dati_cedole_titoli;
create table estrazione_dati_cedole_titoli 
(
	anno integer,
	cedola integer,
	titoli integer
);

insert into estrazione_dati_cedole_titoli ( anno , cedola , titoli ) 
select 
		anno(rotmrcz)::integer ,
		ronmced::integer ,
		count(distinct(racdpro))
	from ric_ord_stor r, ric_art_stor a
	where anno(rotmrcz)::integer in (2014,2015) and a.ordprog=r.ordprog
	group by anno(rotmrcz),ronmced 
	order by anno(rotmrcz),ronmced
;

-- estraggo le linee
drop  table estrazione_dati_cedole_linee;
create table estrazione_dati_cedole_linee 
(
	anno integer,
	cedola integer,
	linee integer
);

insert into estrazione_dati_cedole_linee ( anno , cedola , linee ) 
select 
		anno(rotmrcz)::integer ,
		ronmced::integer ,
		count(distinct(rocdlin))
	from ric_ord_stor
	where anno(rotmrcz)::integer in (2014,2015)
	group by anno(rotmrcz),ronmced 
	order by anno(rotmrcz),ronmced
;


drop  table estrazione_dati_cedole;
create table estrazione_dati_cedole 
(
 anno integer,
 cedola integer,
 ordini integer,
 colli integer,
 copie  integer,
 righe integer,
 titoli integer,
 linee integer,
 F1 integer,
 F2 integer,
 F3 integer,
 FF integer,
 FP integer,
 FC integer
);

insert into estrazione_dati_cedole ( anno , cedola , ordini , colli , copie  , righe ) 
select 
		anno(rotmrcz)::integer ,
		ronmced::integer ,
		count(*),
		sum(ronmcll) ,
		sum(ronmrgh) ,
		sum(ronmcpe) 
	from ric_ord_stor 
	where anno(rotmrcz)::integer in (2014,2015) 
	group by anno(rotmrcz),ronmced 
	order by anno(rotmrcz),ronmced
;

-- ci metto dentro il numero titoli
update estrazione_dati_cedole e set titoli = t.titoli from estrazione_dati_cedole_titoli t where e.anno=t.anno and e.cedola=t.cedola;

-- ci metto dentro il numero linee
update estrazione_dati_cedole e set linee = t.linee from estrazione_dati_cedole_linee t where e.anno=t.anno and e.cedola=t.cedola;

-- ci metto dentro i colli tipo 3
update estrazione_dati_cedole e set F3 = t.F3 from estrazione_dati_cedole_colli_3 t where e.anno=t.anno and e.cedola=t.cedola;

-- ci metto dentro i colli tipo 2
update estrazione_dati_cedole e set F2 = t.F2 from estrazione_dati_cedole_colli_2 t where e.anno=t.anno and e.cedola=t.cedola;

-- ci metto dentro i colli tipo 1
update estrazione_dati_cedole e set F1 = t.F1 from estrazione_dati_cedole_colli_1 t where e.anno=t.anno and e.cedola=t.cedola;

-- ci metto dentro i colli tipo F
update estrazione_dati_cedole e set FF = t.FF from estrazione_dati_cedole_colli_F t where e.anno=t.anno and e.cedola=t.cedola;

-- ci metto dentro i colli tipo C
update estrazione_dati_cedole e set FC = t.FC from estrazione_dati_cedole_colli_C t where e.anno=t.anno and e.cedola=t.cedola;

-- ci metto dentro i colli tipo P
update estrazione_dati_cedole e set FP = t.FP from estrazione_dati_cedole_colli_P t where e.anno=t.anno and e.cedola=t.cedola;

