-- esempio di insert dati da storico 
--
drop table roced;

create table roced as select ordprog from ric_ord_stor where ronmced in ('1720','1730');
insert into ric_ord select * from ric_ord_stor where ordprog in (select ordprog from roced);
insert into ric_art select * from ric_art_stor where ordprog in (select ordprog from roced);
insert into col_prod select * from col_prod_stor where ordprog in (select ordprog from roced);
insert into rig_prod select * from rig_prod_stor where ordprog in (select ordprog from roced);
