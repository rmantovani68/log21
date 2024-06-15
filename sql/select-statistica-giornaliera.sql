select 
	to_char(data,'YYYY-MM-DD') as data,
	cedola,
	sum(ordini) as ordini,
	sum(colli) as colli,
	sum(colli_ff) as colli_ff,
	sum(colli_p) as colli_p,
	sum(righe_colli) as righe,
	sum(copie_colli) as copie,
	sum(righe_colli_ff) as righe_ff,
	sum(copie_colli_ff) as copie_ff,
	sum(righe_colli_p) as righe_p,
	sum(copie_colli_p) as copie_p
from statistica_tmp group by data,cedola order by data,cedola::integer ;
