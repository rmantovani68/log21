-- backup anno di storico 2002
drop table ric_ord_stor_anno_2002;
drop table col_prod_stor_anno_2002;
drop table rig_prod_stor_anno_2002;
drop table ric_art_stor_anno_2002;
drop table ric_note_stor_anno_2002;

create table ric_ord_stor_anno_2002 as select * from ric_ord_stor where false;
create table col_prod_stor_anno_2002 as select * from col_prod_stor where false;
create table rig_prod_stor_anno_2002 as select * from rig_prod_stor where false;
create table ric_art_stor_anno_2002 as select * from ric_art_stor where false;
create table ric_note_stor_anno_2002 as select * from ric_note_stor where false;


\pset footer off
\pset tuples_only on
\o backup_dati_2002.txt
select 'insert into ric_ord_stor_anno_2002 select * from ric_ord_stor where ordprog='''||ordprog||''';' from tmp_ric_ord_stor_anno_2002;
select 'delete from ric_ord_stor where ordprog='''||ordprog||''';' from tmp_ric_ord_stor_anno_2002;

select 'insert into ric_art_stor_anno_2002 select * from ric_art_stor where ordprog='''||ordprog||''';' from tmp_ric_ord_stor_anno_2002;
select 'delete from ric_art_stor where ordprog='''||ordprog||''';' from tmp_ric_ord_stor_anno_2002;

select 'insert into ric_note_stor_anno_2002 select * from ric_note_stor where ordprog='''||ordprog||''';' from tmp_ric_ord_stor_anno_2002;
select 'delete from ric_note_stor where ordprog='''||ordprog||''';' from tmp_ric_ord_stor_anno_2002;

select 'insert into rig_prod_stor_anno_2002 select * from rig_prod_stor where ordprog='''||ordprog||''';' from tmp_ric_ord_stor_anno_2002;
select 'delete from rig_prod_stor where ordprog='''||ordprog||''';' from tmp_ric_ord_stor_anno_2002;

select 'insert into col_prod_stor_anno_2002 select * from col_prod_stor where ordprog='''||ordprog||''';' from tmp_ric_ord_stor_anno_2002;
select 'delete from col_prod_stor where ordprog='''||ordprog||''';' from tmp_ric_ord_stor_anno_2002;


