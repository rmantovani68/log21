-- recupera ordini da storico

drop table sel_ord;
create table sel_ord as select ordprog from ric_ord_stor where ordprog in
(
	select ordprog from ric_ord_stor where 
		( rocdlin in ('SGM') and ronmced='31' ) or 
		( rocdlin in ('SGM') and  ronmced='0' and  to_char(rotmrcz,'YYYY-MM-DD')='2019-09-24')
);

-- select count(*) from sel_ord;

-- select ronmced,rocdlin,rotmrcz from ric_ord_stor where ordprog in (select ordprog from sel_ord);

-- inserimento in linea
insert into ric_ord select * from ric_ord_stor where ordprog in (select ordprog from sel_ord);
insert into ric_art select * from ric_art_stor where ordprog in (select ordprog from sel_ord);
insert into col_prod select * from col_prod_stor where ordprog in (select ordprog from sel_ord);
insert into rig_prod select * from rig_prod_stor where ordprog in (select ordprog from sel_ord);

-- successiva cancellazione
-- delete from ric_ord where ordprog in (select ordprog from sel_ord);
-- delete from ric_art where ordprog in (select ordprog from sel_ord);
-- delete from col_prod where ordprog in (select ordprog from sel_ord);
-- delete from rig_prod where ordprog in (select ordprog from sel_ord);
