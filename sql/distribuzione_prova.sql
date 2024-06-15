--
-- prova di lista di distribuzione da utilizzare in ep - callbacks.c
--

--
--  Column  |  Type   | Modifiers 
-- ---------+---------+-----------
--  sdnmced | text    | 
--  sdtpspe | text    | 
--  sdcdlin | text    | 
--  sdcdve2 | text    | 
--  sddslin | text    | 
--  sddsvet | text    | 
--  sdpspre | integer | default 0
--  sdnmrgh | integer | default 0
--  sdnmcpe | integer | default 0
--  sdnmort | integer | default 0
--  sdnmclt | integer | default 0
--  sdnmori | integer | default 0
--  sdnmcli | integer | default 0
--  sdnmore | integer | default 0
--  sdnmcle | integer | default 0
--  sdnmors | integer | default 0
--  sdnmcls | integer | default 0
--  sdnmorx | integer | default 0
--  sdnmclx | integer | default 0
--  sdnmord | integer | default 0
--  sdnmcld | integer | default 0
--  




with distr as ( 
-- ordini totali 
select 
	ronmced,
	rocdlin, 
	rodslin, 
	ordprog,
	ropspre, 
	ronmrgh, 
	ronmcpe, 
	1 as ordtot , 
	0 as orddaf,
	0 as ordinp,
	0 as ordeva,
	0 as ordspd,
	ronmcll as ronmclt,
	0 as ronmcld,
	0 as ronmcli,
	0 as ronmcle,
	0 as ronmcls
from ric_ord  
group by ronmced,rocdlin,rodslin, ropspre,ronmrgh,ronmcpe,ordprog,ronmcll

union
-- ordini da fare 
select 
	ronmced,
	rocdlin, 
	rodslin, 
	ordprog,
	ropspre, 
	ronmrgh, 
	ronmcpe, 
	0 as ordtot , 
	1 as orddaf,
	0 as ordinp,
	0 as ordeva,
	0 as ordspd,
	0 as ronmclt,
	0 as ronmcld,
	0 as ronmcli,
	0 as ronmcle,
	0 as ronmcls
from ric_ord 
where rostato in (' ','R')
group by ronmced,rocdlin,rodslin, ropspre,ronmrgh,ronmcpe,ordprog,ronmcll

union
-- ordini in prelievo
select 
	ronmced,
	rocdlin, 
	rodslin, 
	ordprog,
	ropspre, 
	ronmrgh, 
	ronmcpe, 
	0 as ordtot , 
	0 as orddaf,
	1 as ordinp,
	0 as ordeva,
	0 as ordspd,
	0 as ronmclt,
	0 as ronmcld,
	0 as ronmcli,
	0 as ronmcle,
	0 as ronmcls
from ric_ord 
where rostato in ('P','T')
group by ronmced,rocdlin,rodslin, ropspre,ronmrgh,ronmcpe,ordprog,ronmcll

union
-- ordini evasi
select 
	ronmced,
	rocdlin, 
	rodslin, 
	ordprog,
	ropspre, 
	ronmrgh, 
	ronmcpe, 
	0 as ordtot , 
	0 as orddaf,
	0 as ordinp,
	1 as ordeva,
	0 as ordspd,
	0 as ronmclt,
	0 as ronmcld,
	0 as ronmcli,
	0 as ronmcle,
	0 as ronmcls
from ric_ord 
where rostato in ('E')
group by ronmced,rocdlin,rodslin, ropspre,ronmrgh,ronmcpe,ordprog,ronmcll

union
-- ordini spediti
select 
	ronmced,
	rocdlin, 
	rodslin, 
	ordprog,
	ropspre, 
	ronmrgh, 
	ronmcpe, 
	0 as ordtot , 
	0 as orddaf,
	0 as ordinp,
	1 as ordeva,
	0 as ordspd,
	0 as ronmclt,
	0 as ronmcld,
	0 as ronmcli,
	0 as ronmcle,
	0 as ronmcls
from ric_ord 
where rostato in ('H')
group by ronmced,rocdlin,rodslin, ropspre,ronmrgh,ronmcpe,ordprog,ronmcll

union
-- colli da fare
select 
	ronmced,
	rocdlin, 
	rodslin, 
	r.ordprog,
	ropspre, 
	ronmrgh, 
	ronmcpe, 
	0 as ordtot , 
	0 as orddaf,
	0 as ordinp,
	0 as ordeva,
	0 as ordspd,
	0 as ronmclt,
	count(c.ordprog) as ronmcld,
	0 as ronmcli,
	0 as ronmcle,
	0 as ronmcls
from ric_ord r, col_prod c
where r.ordprog=c.ordprog and cpstato in ('A')
group by ronmced,rocdlin,rodslin, ropspre,ronmrgh,ronmcpe,r.ordprog,ronmcll

union
-- colli in prelievo
select 
	ronmced,
	rocdlin, 
	rodslin, 
	r.ordprog,
	ropspre, 
	ronmrgh, 
	ronmcpe, 
	0 as ordtot , 
	0 as orddaf,
	0 as ordinp,
	0 as ordeva,
	0 as ordspd,
	0 as ronmclt,
	0 as ronmcld,
	count(c.ordprog) as ronmcli,
	0 as ronmcle,
	0 as ronmcls
from ric_ord r, col_prod c
where r.ordprog=c.ordprog and cpstato in ('A','C')
group by ronmced,rocdlin,rodslin, ropspre,ronmrgh,ronmcpe,r.ordprog,ronmcll
union
-- colli evasi
select 
	ronmced,
	rocdlin, 
	rodslin, 
	r.ordprog,
	ropspre, 
	ronmrgh, 
	ronmcpe, 
	0 as ordtot , 
	0 as orddaf,
	0 as ordinp,
	0 as ordeva,
	0 as ordspd,
	0 as ronmclt,
	0 as ronmcld,
	0 as ronmcli,
	count(c.ordprog) as ronmcle,
	0 as ronmcls
from ric_ord r, col_prod c
where r.ordprog=c.ordprog and cpstato in ('E')
group by ronmced,rocdlin,rodslin, ropspre,ronmrgh,ronmcpe,r.ordprog,ronmcll

)
select 
	ronmced,rocdlin,rodslin, 
	sum(ordtot) as ordtot, 
	sum(orddaf) as orddaf, 
	sum(ordinp) as ordinp, 
	sum(ordeva) as ordeva, 
	sum(ordspd) as ordspd, 

	sum(ronmclt) as ronmclt, 
	sum(ronmcld) as ronmcld, 
	sum(ronmcli) as ronmcli, 
	sum(ronmcle) as ronmcle, 
	sum(ronmcls) as ronmcls 
from distr group by ronmced,rocdlin,rodslin;
