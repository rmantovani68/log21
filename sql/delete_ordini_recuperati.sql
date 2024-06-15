
delete from  ric_ord  where ordprog in (select ordprog from sel_ord);
delete from  ric_art  where ordprog in (select ordprog from sel_ord);
delete from  col_prod where ordprog in (select ordprog from sel_ord);
delete from  rig_prod where ordprog in (select ordprog from sel_ord);


