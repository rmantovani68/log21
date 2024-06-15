<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>controllo di analisi volumetrica</title>
</head>
<body background="cni-ril-sfondo.gif" bgcolor="#C0C0C0">

<?php
	include "picking.php";

	page_title("CONTROLLO ANALISI VOLUMETRICA");

	if(!strlen($CEDOLA)){
		center(true);

		newline();
		newline();

		table_title("Per eseguire il controllo e' necessario avere eseguito l'analisi volumetrica su tutti gli ordini della spedizione");

		newline();

		$Form = new HTML_Form ("check_volum.php","get","Spedizione");
		$Form->start();
		$Form->addText("CEDOLA","CEDOLA",$CEDOLA);
		$Form->addSubmit("submit", "VERIFICA");
		$Form->end();
		$Form->display();

	} else {
		$db = DB::connect("pgsql://roberto:3zin@localhost/mondadori", true);
		if(DB::isError($db)) {
			die($db->getMessage);
		}

		$szCmd=sprintf("select fn_diff_sped('%s');",$CEDOLA);
		$res = $db->query($szCmd);

		if(DB::isError($res)){
			die($res->getMessage);
		}

		newline();

		/* controllo l'esattezza dell'analisi volumetrica a seconda del contenuto della tabella diff_sped */
		$szCmd=sprintf("select * from diff_sped;");
		$res = $db->query($szCmd);

		if(DB::isError($res)){
			die($res->getMessage);
		}

		if($res->numRows()>0){
			table_title("ATTENZIONE : sono state riscontrate le seguenti incogruenze nell'analisi volumetrica");

			newline();

			$colHeaders = array("DATA","CEDOLA","SPEDIZIONE","CLIENTE","RAGIONE SOCIALE","RIGHE","ARTICOLO","QTA ORDINATA","QTA SPEDITA","DIFFERENZA");
			$table->addRow($colHeaders, $headerStyle,"TH");

			$Colonne=$res->numCols();
			while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
				for($i=0;$i<$Colonne;$i++){
					$array[$i]=$row[$i];
				}
				$table->addRow($array);
			}
			$table->display();

		} else {
			$szTitle=sprintf("CONTROLLO CEDOLA %s TERMINATO - NON E' STATA RISCONTRATA ALCUNA ANOMALIA",$CEDOLA);
			table_title($szTitle);
		}

		$res->free();
	}

	$db->disconnect();
?>

</body>
</html>

