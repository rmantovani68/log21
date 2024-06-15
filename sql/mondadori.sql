/*
* function fn_qtspe_stor(text,text);
* ritorna, per l'ordine $1, il numero di copie spedite dell'articolo $2
*/
drop function fn_qtspe_stor(text,text);
create function fn_qtspe_stor(text,text) RETURNS integer as '
DECLARE
	ordine ALIAS for $1;
	articolo ALIAS for $2;
BEGIN
	return sum(rpqtspe) from rig_prod_stor where ordprog=ordine and rpcdpro=articolo;
END;
' language 'plpgsql';


/*
* function fn_qtord_stor(text,text);
* ritorna, per l'ordine $1, il numero di copie ordinate relative all'articolo $2
*/
drop function fn_qtord_stor(text,text);
create function fn_qtord_stor(text,text) RETURNS integer as '
DECLARE
	ordine ALIAS for $1;
	articolo ALIAS for $2;
BEGIN
	return sum(raqtord) from ric_art_stor where ordprog=ordine and racdpro=articolo;
END;
' language 'plpgsql';


/*
* function fn_qtspe(text,text);
* ritorna, per l'ordine $1, il numero di copie spedite dell'articolo $2
*/
drop function fn_qtspe(text,text);
create function fn_qtspe(text,text) RETURNS integer as '
DECLARE
	ordine ALIAS for $1;
	articolo ALIAS for $2;
BEGIN
	return sum(rpqtspe) from rig_prod where ordprog=ordine and rpcdpro=articolo;
END;
' language 'plpgsql';


/*
* function fn_qtord(text,text);
* ritorna, per l'ordine $1, il numero di copie ordinate relative all'articolo $2
*/
drop function fn_qtord(text,text);
create function fn_qtord(text,text) RETURNS integer as '
DECLARE
	ordine ALIAS for $1;
	articolo ALIAS for $2;
BEGIN
	return sum(raqtord) from ric_art where ordprog=ordine and racdpro=articolo;
END;
' language 'plpgsql';


/*
* function fn_diff_sped(text)
* controlla per tutti gli articoli degli ordini ricevuti un eventuale differenza
* tra quantita' ordinata e spedita
*/
drop function fn_diff_sped(text);
create function fn_diff_sped(text) RETURNS INTEGER as '
DECLARE
	cedola ALIAS for $1;
	ordini RECORD;
	articoli RECORD;
	qtspe INTEGER;
	qtord INTEGER;
	diff INTEGER;
	nrecords INTEGER := 0;
	query_text VARCHAR;
BEGIN
	-- cancello il contenuto della relazione diff_sped dove andro a memorizzare le eventuali differenze riscontrate
	DROP TABLE diff_sped;

	CREATE TABLE diff_sped 
	(
		rodtbam text,
		ronmced text,
		ordprog text,
		rocdrid text,
		rodscli text,
		ronmrgh int2,
		racdpro text,
		raqtord int2,
		rpqtspe int2
	);

	query_text := ''SELECT * FROM ric_ord where trim(ronmced) = ''||''''''''||cedola||''''''''||'' order by ordprog;'';
	FOR ordini IN EXECUTE query_text LOOP
		RAISE NOTICE ''ordine %'',ordini.ordprog;
		FOR articoli IN SELECT * FROM ric_art where ordprog=ordini.ordprog order by racdpro LOOP
			qtspe := fn_qtspe(ordini.ordprog,articoli.racdpro);
			qtord := fn_qtord(ordini.ordprog,articoli.racdpro);
			--RAISE NOTICE ''  - articolo % - qtord % - qtspe %'',articoli.racdpro,qtord,qtspe;
			IF qtspe = NULL THEN
				RAISE NOTICE ''cedola % - data % - spedizione % - cliente % - rag. soc. % - righe % - articolo % - copie ordinate % - copie spedite 0 - diff %'',cedola,ordini.rodtbam,ordini.ordprog,ordini.rocdrid,ordini.rodscli,ordini.ronmrgh,articoli.racdpro,qtord,qtord;
				insert into diff_sped values (ordini.rodtbam,cedola,ordini.ordprog,ordini.rocdrid,ordini.rodscli,ordini.ronmrgh,articoli.racdpro,qtord,0);
				nrecords := nrecords + 1;
			END IF;
			IF qtord != qtspe THEN
				diff := qtord-qtspe;
				insert into diff_sped values (ordini.rodtbam,cedola,ordini.ordprog,ordini.rocdrid,ordini.rodscli,ordini.ronmrgh,articoli.racdpro,qtord,qtspe);
				RAISE NOTICE ''cedola % - data % - spedizione % - cliente % - rag. soc. % - righe % - articolo % - copie ordinate % - copie spedite 0 - diff %'',cedola,ordini.rodtbam,ordini.ordprog,ordini.rocdrid,ordini.rodscli,ordini.ronmrgh,articoli.racdpro,qtord,qtspe,diff;
				nrecords := nrecords + 1;
			END IF;
		END LOOP;
	END LOOP;
	RAISE NOTICE ''trovati % records'',nrecords;
	RETURN nrecords;
END;
' language 'plpgsql';

/*
* function fn_diff_sped_stor(text)
* controlla per tutti gli articoli degli ordini ricevuti un eventuale differenza
* tra quantita' ordinata e spedita
*/
drop function fn_diff_sped_stor(text);
create function fn_diff_sped_stor(text) RETURNS INTEGER as '
DECLARE
	cedola ALIAS for $1;
	ordini RECORD;
	articoli RECORD;
	qtspe INTEGER;
	qtord INTEGER;
	diff INTEGER;
	nrecords INTEGER := 0;
	query_text VARCHAR;
BEGIN
	-- cancello il contenuto della relazione diff_sped dove andro a memorizzare le eventuali differenze riscontrate
	TRUNCATE TABLE diff_sped;
	query_text := ''SELECT * FROM ric_ord_stor where trim(ronmced) = ''||''''''''||cedola||''''''''||'' order by ordprog;'';
	FOR ordini IN EXECUTE query_text LOOP
		FOR articoli IN SELECT * FROM ric_art_stor where ordprog=ordini.ordprog order by racdpro LOOP
			qtspe := fn_qtspe_stor(ordini.ordprog,articoli.racdpro);
			qtord := fn_qtord_stor(ordini.ordprog,articoli.racdpro);
			IF qtspe = NULL THEN
				RAISE NOTICE ''cedola % - data % - spedizione % - cliente % - rag. soc. % - righe % - articolo % - copie ordinate % - copie spedite 0 - diff %'',cedola,ordini.rodtbam,ordini.ordprog,ordini.rocdrid,ordini.rodscli,ordini.ronmrgh,articoli.racdpro,qtord,qtord;
				insert into diff_sped values (ordini.rodtbam,cedola,ordini.ordprog,ordini.rocdrid,ordini.rodscli,ordini.ronmrgh,articoli.racdpro,qtord,0);
				nrecords := nrecords + 1;
			END IF;
			IF qtord != qtspe THEN
				diff := qtord-qtspe;
				insert into diff_sped values (ordini.rodtbam,cedola,ordini.ordprog,ordini.rocdrid,ordini.rodscli,ordini.ronmrgh,articoli.racdpro,qtord,qtspe);
				RAISE NOTICE ''cedola % - data % - spedizione % - cliente % - rag. soc. % - righe % - articolo % - copie ordinate % - copie spedite % - diff %'',cedola,ordini.rodtbam,ordini.ordprog,ordini.rocdrid,ordini.rodscli,ordini.ronmrgh,articoli.racdpro,qtord,qtspe,diff;
				nrecords := nrecords + 1;
			END IF;
		END LOOP;
	END LOOP;
	RAISE NOTICE ''trovati % records'',nrecords;
	RETURN nrecords;
END;
' language 'plpgsql';

/*
* function fn_recall_cedola(text)
* inserisce negli archivi di ricezione i dati relativi alla cedola passatagli
*/
drop function fn_recall_cedola(text);
create function fn_recall_cedola(text) RETURNS INTEGER as '
DECLARE
	cedola ALIAS for $1;
	ordini RECORD;
	nrecords INTEGER := 0;
	query_text VARCHAR;
	righe_affected int;
	note_affected int;
BEGIN
	TRUNCATE TABLE rig_prod;
	TRUNCATE TABLE col_prod;
	TRUNCATE TABLE ric_ord;
	TRUNCATE TABLE ric_art;
	TRUNCATE TABLE ric_note;
	query_text := ''SELECT ordprog FROM ric_ord_stor where trim(ronmced) = ''||''''''''||cedola||''''''''||'' order by ordprog;'';
	FOR ordini IN EXECUTE query_text LOOP
		INSERT INTO ric_ord select * FROM ric_ord_stor WHERE ORDPROG=ordini.ordprog;
		INSERT INTO ric_art select * FROM ric_art_stor WHERE ORDPROG=ordini.ordprog;
		GET DIAGNOSTICS righe_affected = ROW_COUNT;
		INSERT INTO ric_note select * FROM ric_note_stor WHERE ORDPROG=ordini.ordprog;
		GET DIAGNOSTICS note_affected = ROW_COUNT;
		RAISE NOTICE ''RECALL ORDINE % - % righe - % note'',ordini.ordprog,righe_affected,note_affected;
		nrecords := nrecords + 1;
	END LOOP;
	RAISE NOTICE ''inseriti % records'',nrecords;
	RETURN nrecords;
END;
' language 'plpgsql';
