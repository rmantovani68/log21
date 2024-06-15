CREATE LANGUAGE plpgsql;

DROP FUNCTION if exists fn_insert_trigger() CASCADE;
CREATE FUNCTION fn_insert_trigger() RETURNS trigger  AS $fn_insert_trigger$
BEGIN
	-- RAISE NOTICE 'Eseguo : %', TG_NAME;
	NEW.insert_user:=current_user;
	NEW.insert_time:=current_timestamp;
	RETURN NEW;
END;
$fn_insert_trigger$  LANGUAGE plpgsql;

COMMENT ON FUNCTION fn_insert_trigger() IS 'funzione fn_insert_trigger() - richiamata da trigger - valorizza i campi insert_user e insert_time';


DROP FUNCTION if exists fn_update_trigger() CASCADE;
CREATE FUNCTION fn_update_trigger() RETURNS trigger  AS $fn_update_trigger$
BEGIN
	-- RAISE NOTICE 'Eseguo : %', TG_NAME;
	NEW.update_user:=current_user;
	NEW.update_time:=current_timestamp;
	RETURN NEW;
END;
$fn_update_trigger$  LANGUAGE plpgsql;

COMMENT ON FUNCTION fn_update_trigger() IS 'funzione fn_update_trigger() - richiamata da trigger - valorizza i campi update_user e update_time';
