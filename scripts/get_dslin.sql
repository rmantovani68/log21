with
query1 as (select (select elcdslin from ttlv where elccdsoc='AM' and elccdmag='0000015' and elctpspe='56' and elccdlin='00' and elccdsca='**' and elccdssc='**') as res1),
query2 as (select (select elcdslin from ttlv where elccdsoc='AM' and elccdmag='0000015' and elctpspe='56' and elccdlin='00' and elccdsca='00' and elccdssc='00') as res2),
query3 as (select (select elcdstsp from ttts where elccdsoc='AM' and elccdmag='0000015' and elctpspe='56') as res3)

select coalesce(res1,
	(select coalesce(res2,
		(select coalesce(res3,
		'ND'
		) from query3)
	) from query2)
) from query1;

;
