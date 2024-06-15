--
-- rm 15-03-2018
-- Statistica colli scartati per cedola
--

select  
	cedola ,
	sum(colli_totali) as colli_totali ,
	sum(colli_scartati) as colli_scartati, 
	to_char(sum(colli_scartati)*100/sum(colli_totali),'99.99')||' %' as percentuale 
from 
		(
--		select cpnmced as cedola,0 as colli_totali,count(*) as colli_scartati  from colli_scartati group by cpnmced 
--		union 
--		select ronmced as cedola,count(*) as colli_totali,0 as colli_scartati from col_prod_stor c,ric_ord_stor o where  c.ordprog=o.ordprog group by cedola
--		union
--		select ronmced as cedola,count(*) as colli_totali,0 as colli_scartati from col_prod c,ric_ord o where  c.ordprog=o.ordprog group by cedola
--		union
--		select 'totali' as cedola,count(*) as colli_totali,0 as colli_scartati from col_prod c,ric_ord o where  c.ordprog=o.ordprog

			select cpnmced as cedola,0 as colli_totali,count(*) as colli_scartati  from colli_scartati group by cpnmced
			union
			select ronmced as cedola,count(*) as colli_totali,0 as colli_scartati from col_prod_stor c,ric_ord_stor o where  c.ordprog=o.ordprog group by cedola
			union
			select ronmced as cedola,count(*) as colli_totali,0 as colli_scartati from col_prod c,ric_ord o where  c.ordprog=o.ordprog group by cedola
			union
			select 'totali' as cedola,0 as colli_totali,count(*) as colli_scartati  from colli_scartati 
			union
			select 'totali' as cedola,count(*) as colli_totali,0 as colli_scartati from col_prod_stor c,ric_ord_stor o where  c.ordprog=o.ordprog 
			union
			select 'totali' as cedola,count(*) as colli_totali,0 as colli_scartati from col_prod c,ric_ord o where  c.ordprog=o.ordprog 
		) as dati 
group by cedola 
order by lpad(cedola,10,' ')
;
