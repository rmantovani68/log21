--
-- esempio di lista linee di distribuzione su richiesta eg
--

with distr as ( 
select 
rocdlin, rodslin, r.ordprog , count(c.ordprog) as NRM, 0 as FFT , 0 as P, ronmcll
from ric_ord r , col_prod c 
where r.ordprog=c.ordprog and c.cptpfor not in ('P','F') 
group by rocdlin,rodslin, r.ordprog,ronmcll
union
select 
rocdlin,rodslin,  r.ordprog , 0 as NRM, count(c.ordprog) as FFT, 0 as P, ronmcll
from ric_ord r , col_prod c 
where r.ordprog=c.ordprog and c.cptpfor in ('F') 
group by rocdlin,rodslin, r.ordprog,ronmcll 
union
select 
rocdlin,rodslin,  r.ordprog , 0 as NRM, 0 as FFT, count(c.ordprog) as P, ronmcll
from ric_ord r , col_prod c 
where r.ordprog=c.ordprog and c.cptpfor in ('P') 
group by rocdlin,rodslin, r.ordprog,ronmcll 
)
select 
rocdlin,rodslin, count(ordprog) as ordini, sum(nrm) as NRM, sum(FFT) as FFT, sum(P) as P , sum(ronmcll) as "TOT" 
from distr group by rocdlin,rodslin;
