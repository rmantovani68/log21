drop function collo_cppsrea(text,int);
CREATE FUNCTION collo_cppsrea(text,int) returns numeric(11,3) as 'select (cppsrea::float/1000::float)::numeric(11,3) from col_prod where ordprog=$1 and cpnmcol=$2' language 'sql';
drop function collo_cppspre(text,int);
CREATE FUNCTION collo_cppspre(text,int) returns numeric(5,3) as 'select (cppspre::float/1000::float)::numeric(5,3) from col_prod where ordprog=$1 and cpnmcol=$2' language 'sql';
drop function ordine_ropsrea(text);
CREATE FUNCTION ordine_ropsrea(text) returns numeric(11,3) as 'select (ropsrea::float/1000::float)::numeric(11,3) from ric_ord where ordprog=$1' language 'sql';
drop function ordine_ropspre(text);
CREATE FUNCTION ordine_ropspre(text) returns numeric(11,3) as 'select (ropspre::float/1000::float)::numeric(11,3) from ric_ord where ordprog=$1' language 'sql';
drop function prodotto_prpspre(text);
CREATE FUNCTION prodotto_prpspre(text) returns numeric(5,3) as 'select (prpesgr::float/1000::float)::numeric(5,3) from catalogo where prcdpro=$1' language 'sql';
drop function riga_rppspre(text,int,text);
CREATE FUNCTION riga_rppspre(text,int,text) returns numeric(5,3) as 'select (rpqtspe::float*prpesgr::float/1000::float)::numeric(5,3) from catalogo,rig_prod where ordprog=$1 and rpnmcol=$2 and prcdpro=rpcdpro and prcdpro=$3' language 'sql';
drop function linea_sdpspre(text);
CREATE FUNCTION linea_sdpspre(text) returns numeric(11,3) as 'select (sdpspre::float/1000::float)::numeric(11,3) from distribuzione_l where sdcdlin=$1' language 'sql';
drop function copie_articolo_in_ordine(text,text);
CREATE FUNCTION copie_articolo_in_ordine(text,text) returns bigint as 'select sum(rpqtspe) from rig_prod where ordprog=$1 and rpcdpro=$2' language 'sql';
drop function copie_articolo_in_ordine(text,text);
CREATE FUNCTION copie_articolo_in_ordine(text,text) returns bigint as 'select sum(rpqtspe) from rig_prod where ordprog=$1 and rpcdpro=$2' language 'sql';