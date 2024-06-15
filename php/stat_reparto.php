<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>statistica giornaliera di reparto resi</title>
</head>
<body background="cni-ril-sfondo.gif" bgcolor="#C0C0C0">
<?php
	include "picking.php";

	page_title("STATISTICA GIORNALIERA DI REPARTO RESI");

	$mesi[]="GENNAIO";
	$mesi[]="FEBBRAIO";
	$mesi[]="MARZO";
	$mesi[]="APRILE";
	$mesi[]="MAGGIO";
	$mesi[]="GIUGNO";
	$mesi[]="LUGLIO";
	$mesi[]="AGOSTO";
	$mesi[]="SETTEMBRE";
	$mesi[]="OTTOBRE";
	$mesi[]="NOVEMBRE";
	$mesi[]="DICEMBRE";


	if($CB_MESE=="" || $CB_ANNO==""){
		/*
		* Ricavo la data odierna
		*/
		$date = getdate();

		$month = date("m");
		$year = date("Y");
	} else {
		$month=$CB_MESE;
		$year=$CB_ANNO;
		$esegui_statistica=TRUE;
	}

	$Form = new HTML_Form ("stat_reparto.php");

	/*
	for($nIndex=2002;$nIndex<2020;$nIndex++){
		$Entries[]=$nIndex;
	}
	*/
	$item = array ("2002","2002");
	$Entries = array ($item);

	$Form->addSelect("CB_ANNO", "", $Entries, $year,"1");

	$Form->displaySelect("CB_ANNO", "", $Entries, $year,"1");

	//printf("<center><FORM method=\"get\" action=\"stat_reparto.php\" target=\"centrale\">");

	/*
	printf("<table><th><h3>ANNO</h3><th align=\"right\"><select name=\"CB_ANNO\" size=\"1\">");
	for($nIndex=2002;$nIndex<2020;$nIndex++){
		if($year==$nIndex){
			printf("<option selected value=$nIndex>$nIndex</option>");
		} else {
			printf("<option value=$nIndex>$nIndex</option>");
		}
	}
	printf("</select><tr><th><h3>MESE</h3></th><th align=\"right\"><select name=\"CB_MESE\" size=\"1\">");
	for($nIndex=0;$nIndex<=11;$nIndex++){
		if($month==$nIndex+1){
			printf("<option selected value=%d>%s</option>",$nIndex+1,$mesi[$nIndex]);
		} else {
			printf("<option value=%d>%s</option>",$nIndex+1,$mesi[$nIndex]);
		}
	}
	printf("</select></th><tr>");

	printf("<tr><th>VALORE MEDIO</th><th><input type=\"text\" name=\"VALORE_MEDIO\" size=\"15\" value=\"%s\">",$VALORE_MEDIO);

	printf("</select></th><tr><td></td><th align=\"right\"><INPUT type=\"submit\" value=\"ESEGUI STATISTICA\"></th></form></table></center>");
	$res = $db->query("select count(brcdsoc),sum(brnmclp),sum(brnmcpp),brcdsoc from bolle_reso where date_trunc('day',brtmstr)=date_trunc('day','now'::datetime) group by brcdsoc union select count(brcdsoc),sum(brnmclp),sum(brnmcpp),brcdsoc from bolle_storico where date_trunc('day',brtmstr)=date_trunc('day','now'::datetime) group by brcdsoc;");


	if($esegui_statistica){

		$res = $db->query("select count(brcdsoc),sum(brnmclp),sum(brnmcpp) from bolle_reso where date_trunc('month',brtmrcz)<timestamp('2002-04-01') and brfase='R' union select count(brcdsoc),sum(brnmclp),sum(brnmcpp) from bolle_storico where date_trunc('month',brtmrcz)<timestamp('2002-04-01') and brfase='R';");

		$Bolle=0;
		$Colli=0;
		$Prodotti=0;

		if($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
			$Bolle=$row[0];
			$Colli=$row[1];
			$Prodotti=$row[2];
		}
		


		$res->free();

		echo "GIACENZA INIZIALE :" . $CB_ANNO;
		if(DB::isError($res)){
			die($res->getMessage);
		}

		$res->free();
	}
	*/

	$db->disconnect();
?>

</body>
</html>
