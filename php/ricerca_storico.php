<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>Ricerca in storico</title>
</head>
<body background="cni-ril-sfondo.gif" bgcolor="#C0C0C0">

<?php
	include "picking.php";
	/* tabella */
	$tableStyle = "border=1 cellpadding=1 align=center cellspacing=2"; 
	$tabnavStyle = "border=0 cellpadding=1 align=center width=100% cellspacing=2"; 

	/* atributi tabella */
	$headerStyle = "bgcolor=#bbccbb  nowrap align=center"; 
	$rowStyle1   = "bgcolor=#999999  nowrap align=right font-style="; 
	$rowStyle2   = "bgcolor=#aaaaaa  nowrap align=right"; 
	$footerStyle = "bgcolor=#c0c0c0  nowrap align=right"; 



	page_title("RICERCA DATI DI STORICO");


	$db = DB::connect("pgsql://roberto:3zin@localhost/mondadori", true);
	if(DB::isError($db)) {
		die($db->getMessage);
	}

	if(!strlen($SPEDIZIONE)){
		center(true);

		$year=date("Y");
		$TEXT=sprintf("AM0000015%02d",$year-2000);


		$Form = new HTML_Form ("ricerca_storico.php","get","Spedizione");
		$Form->start();
		$Form->addText("SPEDIZIONE","SPEDIZIONE",$TEXT);
		$Form->addCheckbox("STORICO","STORICO","");
		$Form->addSubmit("submit", " OK ");
		$Form->end();
		$Form->display();

		center(false);
	}

	if(strlen($SPEDIZIONE)>11){
		$table  = new HTML_Table($tableStyle); 
		$table->setRowType(0,"TH"); 

		if($STORICO=="on"){
			$ordini=ric_ord_stor;
			$colli=col_prod_stor;
			$righe=rig_prod_stor;
		} else {
			$ordini=ric_ord;
			$colli=col_prod;
			$righe=rig_prod;
		}

		$szCmd=sprintf("select ronmbam, ronmcll, ropspre, ropsrea, data(rotmeva) , ora(rotmeva), rocdrid,rodscli , rococli , roincli , rolocli , roprcli , rocpcli,ronmced from %s where ordprog = '%s';",$ordini,$SPEDIZIONE);
		$res = $db->query($szCmd);

		if(DB::isError($res)){
			die($res->getMessage);
		}


		$title=sprintf("Dati Spedizione : %s",$SPEDIZIONE);
		$table->setCaption($title,$headerStyle);

		if($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
			$table->addRow(array("CODICE SPEDIZIONE",$SPEDIZIONE));
			$table->addRow(array("CLIENTE",          sprintf("%s %s",$row[6],$row[7])));
			$table->addRow(array("PRESSO P.V.",      $row[8]));
			$table->addRow(array("INDIRIZZO",        $row[9]));
			$table->addRow(array("LOCALITÀ",         $row[10]));
			$table->addRow(array("PROVINCIA",        $row[11]));
			$table->addRow(array("CAP",              $row[12]));
			$table->addRow(array("CODICE BAM",       $row[0]));
			$table->addRow(array("COLLI",            $row[1]));
			$table->addRow(array("PESO CALCOLATO",   $row[2]));
			$table->addRow(array("PESO REALE",       $row[3]));
			$table->addRow(array("DATA EVASIONE",    $row[4]));
			$table->addRow(array("ORA EVASIONE",     $row[5]));
			$table->addRow(array("CEDOLA",           $row[13]));

			$table->altRowAttributes(0,$rowStyle1,$rowStyle2); 
			$table->setColAttributes(0,"bgcolor=c0a0b0 align=left nowrap");
		}
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}


		$szCmd=sprintf("select ordprog from %s where ordprog<'%s' order by ordprog desc limit 1;",$ordini,$SPEDIZIONE);
		$res = $db->query($szCmd);
		if($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
			$prev=sprintf("<a href=\"ricerca_storico.php?SPEDIZIONE=%s\"><img src=stock_left_arrow.png border=0>%s</a>",$row[0],$row[0]); 
		} else {
			$prev="";
		}
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}

		$szCmd=sprintf("select ordprog from %s where ordprog>'%s' order by ordprog asc limit 1;",$ordini,$SPEDIZIONE);
		$res = $db->query($szCmd);
		if($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
			$next=sprintf("<a href=\"ricerca_storico.php?SPEDIZIONE=%s\">%s<img src=stock_right_arrow.png border=0></a>",$row[0],$row[0]); 
		} else {
			$next="";
		}
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}

		$tabnav = new HTML_Table($tabnavStyle); 
		$tabnav->addRow(array($prev,$next));
		$tabnav->setColAttributes(0,"align=left nowrap");
		$tabnav->setColAttributes(1,"align=right nowrap");
		$tabnav->display();


		$table->display();

		$szCmd=sprintf("select c.cpnmcol,c.cppspre,c.cppsrea, data(c.cptmeva), ora(c.cptmeva) from %s as c where c.ordprog = '%s' order by c.cpnmcol;",$colli,$SPEDIZIONE);
		$res = $db->query($szCmd);

		if(DB::isError($res)){
			die($res->getMessage);
		}

		newline();

		$colli = new HTML_Table($tableStyle); 

		$title=sprintf("COLLI");
		$colli->setCaption($title,$headerStyle);

		$colHeaders = array("COLLO", "PESO P.", "PESO R.","DATA EVASIONE","ORA EVASIONE"); 
		$colli->addRow($colHeaders,$headerStyle);

		while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
			$colli->addRow(array($row[0],$row[1],$row[2],$row[3],$row[4]));
		}
		$colli->altRowAttributes(1,$rowStyle1,$rowStyle2); 

		newline();
		$colli->display();


		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}

		newline();

		$szCmd=sprintf("select r.rpcdpro,p.prdstit,p.prdsaut,p.prpzcoe,r.rpqtspe,p.prpesgr*r.rpqtspe,r.rpnmcol from %s r,catalogo p where r.ordprog='%s' and r.rpcdpro=p.prcdpro order by r.rpnmcol;",$righe,$SPEDIZIONE);
		$res = $db->query($szCmd);

		if(DB::isError($res)){
			die($res->getMessage);
		}

		$righe = new HTML_Table($tableStyle); 

		$title=sprintf("RIGHE");
		$righe->setCaption($title,$headerStyle);
		$colHeaders = array("CODICE", "TITOLO", "AUTORE", "PREZZO", "COPIE","PESO","COLLO"); 
		$righe->addRow($colHeaders,$headerStyle);


		$collo=1;
		while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
			if($collo!=$row[6]){
				$righe->addRow(array(sprintf("COLLO NUMERO %2d",$collo),"","","","","",""),$headerStyle);
				$collo=$row[6];
			}
			$righe->addRow(array($row[0],$row[1],$row[2],$row[3],$row[4],$row[5],$row[6]),$rowStyle1);
		}
		/*
		$righe->altRowAttributes(1,$rowStyle1,$rowStyle2); 
		*/

		newline();

		$righe->display();

		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}
	}


	$db->disconnect();
?>

</body>
</html>

