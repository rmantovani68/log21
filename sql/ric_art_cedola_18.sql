drop table ric_art_cedola_18;
create table ric_art_cedola_18 as select * from ric_art_stor where ordprog in (select ordprog from ric_ord_cedola_18_2004);
