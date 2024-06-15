<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
  <title>Informazioni Generali</title>
  <meta name="author" content="Roberto Mantovani">
  <meta name="description" content="Informazioni generali sulla cedola in lavorazione">
	<style type="text/css"> body { font-family: "Courier New", monospace; } </style>
</head>
<body background="cni-ril-sfondo.gif" text="#000000" bgcolor="#c0c0c0" link="#0000ee" vlink="#551a8b" alink="#0000ee">

<?php
	include "picking.php";


	if($CEDOLA==0){
		page_title("INSERIRE NUMERO CEDOLA");
		center(true);

		$Form = new HTML_Form ("general.php","get","Numero Cedola");
		$Form->start();
		$Form->addText("CEDOLA","CEDOLA",$CEDOLA);
		$Form->addCheckbox("DO_STORICO","STORICO","");
		$Form->addCheckbox("DO_LISTA_ARTICOLI","LISTA ARTICOLI","");
		$Form->addCheckbox("DO_DISTRIBUZIONE","LISTA DI DISTRIBUZIONE","");
		$Form->addCheckbox("DO_STATISTICA_EVASIONE","STATISTICA DI EVASIONE","");
		$Form->addCheckbox("DO_GRAFICO","GRAFICO EVASIONE ORARIA","");
		$Form->addSubmit("submit", " OK ");
		$Form->end();
		$Form->display();

		center(false);
	} else {
		center(true);

		$db = DB::connect("pgsql://roberto:3zin@localhost/mondadori", true);
		if(DB::isError($db)) {
			die($db->getMessage);
		}

		if($DO_STORICO=="on"){
			$ordini=ric_ord_stor;
			$colprod=col_prod_stor;
			$rigprod=rig_prod_stor;
			$ricart=ric_art_stor;
		} else {
			$ordini=ric_ord;
			$colprod=col_prod;
			$rigprod=rig_prod;
			$ricart=ric_art;
		}

		/* tabella */
		$tableStyle = "border=0 cellpadding=0"; 

		$table_articoli  = new HTML_Table($tableStyle); 
		$table_totali = new HTML_Table($tableStyle); 
		$table_distribuzione = new HTML_Table($tableStyle); 
		$table_evasione = new HTML_Table($tableStyle); 
		$table_generale = new HTML_Table($tableStyle); 

		/* atributi tabella */
		$headerStyle = "bgcolor=#aaccbb nowrap"; 
		$rowStyle1   = "bgcolor=#999999 nowrap"; 
		$rowStyle2   = "bgcolor=#aaaaaa nowrap"; 
		$footerStyle = "bgcolor=#c0c0c0 nowrap"; 
		$table_articoli->setRowType(0,"TH"); 
		$table_distribuzione->setRowType(0,"TH"); 
		$table_evasione->setRowType(0,"TH"); 

		$res = $db->query("drop table bal_art_tmp;"); 
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}

		$res = $db->query("create table bal_art_tmp (
			bacdpro char(12),
			bacdubi char(12),
			bacdset int2,
			banmrgh int4,
			banmcpe int4,
			bacdflg char(1),
			baswffo int2,
			baqtffo int2,
			baqtpcf int2,
			baqtpcp int2,
			bafcpcf char(1),
			bastato char(1),
			batpubi char(1),
			badstit char(40),
			balargh int4,
			balungh int4,
			baaltez int4,
			bapesgr int4
		);");
		if(DB::isError($res)){
			die($res->getMessage);
		}
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}

		$query=sprintf("insert into bal_art_tmp (bacdpro,banmrgh,banmcpe) select r.racdpro,count(r.ordprog),sum(r.raqtord) from %s r,%s o where o.ronmced='%d' and r.ordprog=o.ordprog group by racdpro;",$ricart,$ordini,$CEDOLA);

		$res = $db->query($query);
		if(DB::isError($res)){
			die($res->getMessage);
		}

		$bOK=TRUE;
		if($db->affectedRows()==0){
			$bOK=FALSE;
		}

		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}
		if($bOK==FALSE){
			if($DO_STORICO=="on"){
				$errore=sprintf("CEDOLA %d non presente in storico",$CEDOLA);
			} else {
				$errore=sprintf("CEDOLA %d non presente",$CEDOLA);
			}
			page_title($errore);
			die("");
		}


		/* ricavo i dati da catalogo e ubicazioni */
		$res = $db->query("select bacdpro from bal_art_tmp;");
		while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){

			$bOK=TRUE;

			if($DO_STORICO!="on"){

				$query=sprintf("select ubicazione,settore from ubicazioni where codprod='%s';", $row[0]);


				$resUbi=$db->query($query);
				if($rowUbi = $resUbi->fetchRow(DB_FETCHMODE_ORDERED)){
					$szCDUBI=$rowUbi[0];
					$nCDSET=$rowUbi[1];
				} else {
					$bOK=FALSE;
				}
			} else {
				$szCDUBI="";
				$nCDSET="";
			}

			if($bOK){
				$query=sprintf("select prswffo,prqtffo,prqtpcf,prfcpcf,prdstit,prlargh,prlungh,praltez,prpesgr from catalogo where prcdpro='%s';", $row[0]);


				$resCat=$db->query($query);

				if($rowCat = $resCat->fetchRow(DB_FETCHMODE_ORDERED)){
					$szUpdate=sprintf("update bal_art_tmp set baswffo=%d, baqtffo=%d, baqtpcf=%d, bafcpcf='%s', badstit=%s, balargh=%d,balungh=%d,baaltez=%d,bapesgr=%d,bacdubi='%s', bacdset=%d  where bacdpro='%s';",
						$rowCat[0],                           /* SWFFO */
						$rowCat[1],                           /* QTFFO */
						$rowCat[2],                           /* QTPCF */
						$rowCat[3],                           /* FCPCF */
						$db->quote($rowCat[4]),               /* DSTIT */
						$rowCat[5],                           /* LUNGH */
						$rowCat[6],                           /* LARGH */
						$rowCat[7],                           /* ALTEZ */
						$rowCat[8],                           /* PESGR */
						$szCDUBI,                             /* CDUBI */
						$nCDSET,                              /* CDSET */
						$row[0]);

					
					$resUpdate=$db->query($szUpdate);
					if(!DB::isError($resUpdate) && $resUpdate!=DB_OK){
						$resUpdate->free();
					}
				}
			}
			if(!DB::isError($resCat) && $resCat!=DB_OK){
				$resCat->free();
			}
			if(!DB::isError($resUbi) && $resUbi!=DB_OK){
				$resUbi->free();
			}
		}
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}

		if($DO_STORICO=="on"){
			$colHeaders = array("CODICE","TITOLO","LUNGH.","LARGH.","ALT.","VOLUME (CC)","PESO (gr)","RIGHE","COPIE","FF","Q.FF","Q.PCF","F.PCF");
		} else {
			$colHeaders = array("CODICE","TITOLO","LUNGH.","LARGH.","ALT.","VOLUME (CC)","PESO (gr)","UBICAZIONE","SETTORE","RIGHE","COPIE","FF","Q.FF","Q.PCF","F.PCF");
		}
		$table_articoli->addRow($colHeaders, $headerStyle);

 
 		$query = sprintf("select bacdpro , badstit , balungh , balargh , baaltez , ((balargh::float8*baaltez::float8*balungh::float8)/1000)::int4 ,bapesgr , fila(bacdubi)||'.'||colonna(bacdubi)||'.'||piano(bacdubi) , bacdset , banmrgh , banmcpe , bacdflg , baswffo , baqtffo , baqtpcf , bafcpcf , bastato from bal_art_tmp order by bacdpro;");
		$res = $db->query($query);


		if(DB::isError($res)){
			die($res->getMessage);
		}


		$tot_spedizioni=0;
		$tot_articoli=0;
		$tot_righe=0;
		$tot_copie=0;
		$tot_ff=0;
		$tot_pcf=0;

		while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
			if($DO_STORICO=="on"){
				$table_articoli->addRow(array( $row[0], $row[1], $row[2], $row[3], $row[4], $row[5], $row[6], $row[7], $row[10], $row[11], $row[12], $row[13], $row[14]));
			} else {
				$table_articoli->addRow(array( $row[0], $row[1], $row[2], $row[3], $row[4], $row[5], $row[6], $row[7], $row[8], $row[9], $row[10], $row[11], $row[12], $row[13], $row[14]));
			}
			$tot_righe+=$row[9];
			$tot_copie+=$row[10];
			$tot_ff+=($row[12])?1:0;
			$tot_pcf+=($row[14])?1:0;
			$tot_articoli++;
		}
		$table_articoli->altRowAttributes(1,$rowStyle1,$rowStyle2); 

		/* allineamento a destra campi numerici */
		if($DO_STORICO=="on"){
			$table_articoli->updateColAttributes(2,"align=right");
			$table_articoli->updateColAttributes(3,"align=right");
			$table_articoli->updateColAttributes(4,"align=right");
			$table_articoli->updateColAttributes(5,"align=right");
			$table_articoli->updateColAttributes(6,"align=right");
			$table_articoli->updateColAttributes(8,"align=right");
			$table_articoli->updateColAttributes(9,"align=right");
			$table_articoli->updateColAttributes(10,"align=right");
			$table_articoli->updateColAttributes(11,"align=right");
		} else {
			$table_articoli->updateColAttributes(2,"align=right");
			$table_articoli->updateColAttributes(3,"align=right");
			$table_articoli->updateColAttributes(4,"align=right");
			$table_articoli->updateColAttributes(5,"align=right");
			$table_articoli->updateColAttributes(6,"align=right");
			$table_articoli->updateColAttributes(8,"align=right");
			$table_articoli->updateColAttributes(9,"align=right");
			$table_articoli->updateColAttributes(10,"align=right");
			$table_articoli->updateColAttributes(11,"align=right");
			$table_articoli->updateColAttributes(12,"align=right");
			$table_articoli->updateColAttributes(13,"align=right");
		}
		
		$table_totali->addRow(array("TOTALE ARTICOLI",$tot_articoli));
		$table_totali->addRow(array("TOTALE RIGHE", $tot_righe));
		$table_totali->addRow(array("TOTALE COPIE" ,$tot_copie));
		$table_totali->addRow(array("PRODOTTI FUORI FORMATO",$tot_ff));
		$table_totali->addRow(array("PRODOTTI PRECONFEZIONATI",($tot_pcf==0)?"0":$tot_pcf));
		$table_totali->updateColAttributes(0,$rowStyle1,"TH");
		$table_totali->updateColAttributes(1,$rowStyle2,"TH");
		$table_totali->updateColAttributes(1,"align=right");
    /* $table_totali->updateRowAttributes(0,$headerStyle); */
						

		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}

		$titolo=sprintf("INFORMAZIONI GENERALI CEDOLA NUMERO %d",$CEDOLA);
		page_title($titolo);
		newline();


		newline();
		if($DO_DISTRIBUZIONE && do_distribuzione_cedola($db,$CEDOLA)){
			$query=sprintf("select sdtpspe ,sdcdlin ,sddslin ,sdnmort ,sdnmclt ,sdnmrgh ,sdnmcpe ,sdpspre::float/1000 ,sdnmori ,sdnmcli ,sdnmore ,sdnmcle ,sdnmorx ,sdnmclx ,sdnmord ,sdnmcld ,sdnmors ,sdnmcls from sel_dist_tmp order by sdtpspe,sdcdlin;");
			$colHeaders = array( "TIPO","LINEA","DESCRIZIONE","ORDINI","COLLI","RIGHE","COPIE","PESO (Kg)","LANCIATI","COLLI","EVASI","COLLI","XAB","COLLI","DIST","COLLI","HOST","COLLI");
			$res = $db->query($query);

			$table_distribuzione->addRow($colHeaders, $headerStyle);
			while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
				$table_distribuzione->addRow(array( $row[0], $row[1], $row[2], $row[3], $row[4], $row[5], $row[6], $row[7], $row[8], $row[9], $row[10], $row[11], $row[12], $row[13], $row[14], $row[15], $row[16]));
			}
			if(!DB::isError($res) && $res!=DB_OK){
				$res->free();
			}
			$table_distribuzione->updateColAttributes(3,"align=right");
			$table_distribuzione->updateColAttributes(4,"align=right");
			$table_distribuzione->updateColAttributes(5,"align=right");
			$table_distribuzione->updateColAttributes(6,"align=right");
			$table_distribuzione->updateColAttributes(7,"align=right");
			$table_distribuzione->updateColAttributes(8,"align=right");
			$table_distribuzione->updateColAttributes(9,"align=right");
			$table_distribuzione->updateColAttributes(10,"align=right");
			$table_distribuzione->updateColAttributes(11,"align=right");
			$table_distribuzione->updateColAttributes(12,"align=right");
			$table_distribuzione->updateColAttributes(13,"align=right");
			$table_distribuzione->updateColAttributes(14,"align=right");
			$table_distribuzione->updateColAttributes(15,"align=right");
			$table_distribuzione->updateColAttributes(16,"align=right");
			$table_distribuzione->altRowAttributes(1,$rowStyle1,$rowStyle2); 
		}

		do_statistica_evasione($db,$CEDOLA);

		if($DO_STATISTICA_EVASIONE=="on" ){

			$colHeaders = array( "DATA","ORA","COLLI","RIGHE","COPIE");
			$table_evasione->addRow($colHeaders, $headerStyle);
			$query=sprintf("select sedteva,setmeva,senmcll,senmrgh,senmcpe from stat_eva_tmp order by sedteva,setmeva;");
			$res = $db->query($query);
			while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
				$table_evasione->addRow(array( $row[0], $row[1], $row[2], $row[3], $row[4]));
			}
			if(!DB::isError($res) && $res!=DB_OK){
				$res->free();
			}
			$table_evasione->updateColAttributes(2,"align=right");
			$table_evasione->updateColAttributes(3,"align=right");
			$table_evasione->updateColAttributes(4,"align=right");
			$table_evasione->altRowAttributes(1,$rowStyle1,$rowStyle2); 
		}

		if($DO_GRAFICO){
			do_chart("evasione_oraria_colli");
			do_chart("evasione_oraria_righe");
			do_chart("evasione_oraria_copie");
			do_chart("evasione_giornaliera_colli");
			do_chart("evasione_giornaliera_righe");
			do_chart("evasione_giornaliera_copie");
		}

		$table_generale->addRow(array("TOTALI CEDOLA"));
		$table_generale->addRow(array($table_totali->toHtml()));

		if($DO_LISTA_ARTICOLI=="on"){
			$table_generale->addRow(array("LISTA ARTICOLI"));
			$table_generale->addRow(array($table_articoli->toHtml()));
		}
		if($DO_DISTRIBUZIONE=="on"){
			$table_generale->addRow(array("LISTA DI DISTRIBUZIONE"));
			$table_generale->addRow(array($table_distribuzione->toHtml()));
		}
		if($DO_STATISTICA_EVASIONE=="on"){



			$table_generale->addRow(array("STATISTICA DI EVASIONE"));
			$table_generale->addRow(array($table_evasione->toHtml()));
		}
		$table_generale->display();
		center(false);

		$db->disconnect();
	}
?>


</body>
</html>
