<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>statistica di produttivita' giornaliera</title>
</head>
<body background="cni-ril-sfondo.gif" bgcolor="#C0C0C0">

<?php
	include "picking.php";

	page_title("STATISTICA DI PROCESSO GIORNALIERA");

	$res = $db->query("select count(brcdsoc),sum(brnmclp),sum(brnmcpp),brcdsoc from bolle_reso where date_trunc('day',brtmstr)=date_trunc('day','now'::datetime) group by brcdsoc union select count(brcdsoc),sum(brnmclp),sum(brnmcpp),brcdsoc from bolle_storico where date_trunc('day',brtmstr)=date_trunc('day','now'::datetime) group by brcdsoc;");
	if(DB::isError($res)){
		die($res->getMessage);
	}

	$nBolle=0;
	$nColli=0;
	$nProdotti=0;
	
	newline();
	table_title("BOLLE DI RESO SUDDIVISE PER SOCIETA");

	$colHeaders = array("SOCIETA'", "BOLLE", "COLLI","PRODOTTI"); 
	$table->addRow($colHeaders, $headerStyle);

	while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
		$table->addRow(array($row[3],$row[0],$row[1],$row[2]));
		$nBolle   += $row[0];
		$nColli   += $row[1];
		$nProdotti+= $row[2];
	}
	$table->addRow(array("TOTALE",$nBolle,$nColli,$nProdotti), $footerStyle);
	$table->altRowAttributes(1,$rowStyle1,$rowStyle2); 

	center(TRUE);
	newline();
	$table->display();
	center(FALSE);

	$res->free();

	$db->disconnect();
?>

</body>
</html>
