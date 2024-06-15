<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
  <title>Monitoraggio settori</title>
  <meta name="author" content="Roberto Mantovani">
  <meta name="description" content="Monitoraggio settori">
	<style type="text/css"> body { font-family: "Courier New", monospace; } </style>
</head>
<body background="cni-ril-sfondo.gif" text="#000000" bgcolor="#c0c0c0" link="#0000ee" vlink="#551a8b" alink="#0000ee">

<?php
	include "picking.php";

	$righe=4;
	$colonne=3;

	center(true);

	$db = DB::connect("pgsql://roberto:3zin@localhost/mondadori", true);
	if(DB::isError($db)) {
		die($db->getMessage);
	}

	$tableStyle = "border=0 cellpadding=0"; 
	$table_settori  = new HTML_Table($tableStyle); 
	$table_settore = new HTML_Table($tableStyle); 


	$query=sprintf("select sum(r.rpqtspe),r.rpstato,u.settore from rig_prod as r, ubicazioni as u where r.rpcdubi=u.ubicazione group by u.settore,r.rpstato order by u.settore,r.rpstato;");
	$res = $db->query($query);

	if(DB::isError($res)){
		die($res->getMessage);
	}
	$row=0;
	$col=0;
	while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
		for($col=0;$col<$colonne
	}
	if(!DB::isError($res) && $res!=DB_OK){
		$res->free();
	}


	$table_settori->addRow($colHeaders, $headerStyle);


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
?>


</body>
</html>
