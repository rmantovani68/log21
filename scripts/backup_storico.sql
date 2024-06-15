-- backup anno di storico 2001
drop table tmp_ric_ord_stor_anno_2001;
drop table ric_ord_stor_anno_2001;
drop table col_prod_stor_anno_2001;
drop table rig_prod_stor_anno_2001;
drop table ric_art_stor_anno_2001;
drop table ric_note_stor_anno_2001;

create table tmp_ric_ord_stor_anno_2001 as select ordprog from ric_ord_stor where anno(rotmrcz)='2001';
create table ric_ord_stor_anno_2001 as select * from ric_ord_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2001);
create table col_prod_stor_anno_2001 as select * from col_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2001);
create table rig_prod_stor_anno_2001 as select * from rig_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2001);
create table ric_art_stor_anno_2001 as select * from ric_art_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2001);
create table ric_note_stor_anno_2001 as select * from ric_note_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2001);

delete from ric_ord_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2001);
delete from col_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2001);
delete from rig_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2001);
delete from ric_art_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2001);
delete from ric_note_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2001);


-- backup anno di storico 2002

drop table tmp_ric_ord_stor_anno_2002;
drop table ric_ord_stor_anno_2002;
drop table col_prod_stor_anno_2002;
drop table rig_prod_stor_anno_2002;
drop table ric_art_stor_anno_2002;
drop table ric_note_stor_anno_2002;

create table tmp_ric_ord_stor_anno_2002 as select ordprog from ric_ord_stor where anno(rotmrcz)='2002';
create table ric_ord_stor_anno_2002 as select * from ric_ord_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2002);
create table col_prod_stor_anno_2002 as select * from col_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2002);
create table rig_prod_stor_anno_2002 as select * from rig_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2002);
create table ric_art_stor_anno_2002 as select * from ric_art_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2002);
create table ric_note_stor_anno_2002 as select * from ric_note_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2002);

delete from ric_ord_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2002);
delete from col_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2002);
delete from rig_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2002);
delete from ric_art_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2002);
delete from ric_note_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2002);

-- backup anno di storico 2003


drop table tmp_ric_ord_stor_anno_2003;
drop table ric_ord_stor_anno_2003;
drop table col_prod_stor_anno_2003;
drop table rig_prod_stor_anno_2003;
drop table ric_art_stor_anno_2003;
drop table ric_note_stor_anno_2003;

create table tmp_ric_ord_stor_anno_2003 as select ordprog from ric_ord_stor where anno(rotmrcz)='2003';
create table ric_ord_stor_anno_2003 as select * from ric_ord_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2003);
create table col_prod_stor_anno_2003 as select * from col_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2003);
create table rig_prod_stor_anno_2003 as select * from rig_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2003);
create table ric_art_stor_anno_2003 as select * from ric_art_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2003);
create table ric_note_stor_anno_2003 as select * from ric_note_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2003);

delete from ric_ord_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2003);
delete from col_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2003);
delete from rig_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2003);
delete from ric_art_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2003);
delete from ric_note_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2003);
-- backup anno di storico 2004

drop table tmp_ric_ord_stor_anno_2004;
drop table ric_ord_stor_anno_2004;
drop table col_prod_stor_anno_2004;
drop table rig_prod_stor_anno_2004;
drop table ric_art_stor_anno_2004;
drop table ric_note_stor_anno_2004;

create table tmp_ric_ord_stor_anno_2004 as select ordprog from ric_ord_stor where anno(rotmrcz)='2004';
create table ric_ord_stor_anno_2004 as select * from ric_ord_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2004);
create table col_prod_stor_anno_2004 as select * from col_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2004);
create table rig_prod_stor_anno_2004 as select * from rig_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2004);
create table ric_art_stor_anno_2004 as select * from ric_art_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2004);
create table ric_note_stor_anno_2004 as select * from ric_note_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2004);

delete from ric_ord_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2004);
delete from col_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2004);
delete from rig_prod_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2004);
delete from ric_art_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2004);
delete from ric_note_stor where ordprog in (select ordprog from tmp_ric_ord_stor_anno_2004);

-- fine backup

vacuum;

