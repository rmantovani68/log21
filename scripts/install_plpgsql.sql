-- psql -f install_plgsql.sql
--
-- installazione di PL/pgsql per PostgreSQL
--

-- da psql dichiaro
CREATE FUNCTION plpgsql_call_handler () RETURNS OPAQUE AS
'/usr/lib/pgsql/plpgsql.so' LANGUAGE 'C';

-- e creo 
CREATE TRUSTED PROCEDURAL LANGUAGE 'plpgsql'
HANDLER plpgsql_call_handler
LANCOMPILER 'PL/pgsql';

-- N.B. -> definendo il linguaggio nel database template1, in tutti i databases 
-- definiti successivamente sara' automaticamente installato
-- per un regression test vedi /usr/lib/pgsql/test/regress/sql/plpgsql.sql
