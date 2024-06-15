
drop table tmp_ric_ord_stor_anno_2001;
create table tmp_ric_ord_stor_anno_2001 as select ordprog from ric_ord_stor where anno(rotmrcz)='2001';
drop table tmp_ric_ord_stor_anno_2002;
create table tmp_ric_ord_stor_anno_2002 as select ordprog from ric_ord_stor where anno(rotmrcz)='2002';
drop table tmp_ric_ord_stor_anno_2003;
create table tmp_ric_ord_stor_anno_2003 as select ordprog from ric_ord_stor where anno(rotmrcz)='2003';
drop table tmp_ric_ord_stor_anno_2004;
create table tmp_ric_ord_stor_anno_2004 as select ordprog from ric_ord_stor where anno(rotmrcz)='2004';
