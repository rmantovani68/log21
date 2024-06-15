<?php
	require_once 'DB.php';
	require_once 'HTML/Table.php';
	require_once 'HTML/Form.php';

	function page_title($titolo) {
		$date  = getdate();
		$month = date("m");
		$year  = date("Y");
		$day   = date("d");

		printf("<table border=1 width=100%%><center><td bgcolor=#6090b0 align=center><Font color=white size=5><B>%02d-%02d-%04d %s</B></td></center></table>",$day,$month,$year,$titolo);
	}

	function table_title($titolo) {
		printf("<table border=0 width=90%%><center><td bgcolor=#b09060 align=center><Font color=white size=4><B>%s</B></td></center></table>",$titolo);
	}

	function newline() {
		printf("&nbsp<br>");
	}

	function center($bCenter) {
		if($bCenter){
			printf("<center>");
		} else {
			printf("</center>");
		}
	}
	/*
	* do_distribuzione_cedola()
	*/
	function do_distribuzione_cedola($db,$numero_cedola)
	{
		$query=sprintf("drop table sel_dist_tmp;");
		$res=$db->query($query);
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}
		$query=sprintf("create table sel_dist_tmp (
			sdtpspe char(2),
			sdcdlin char(2),
			sddslin char(30),
			sdpspre int4,
			sdnmrgh int4,
			sdnmcpe int4,
			sdnmort int4,
			sdnmclt int4,
			sdnmori int4,
			sdnmcli int4,
			sdnmore int4,
			sdnmcle int4,
			sdnmors int4,
			sdnmcls int4,
			sdnmorx int4,
			sdnmclx int4,
			sdnmord int4,
			sdnmcld int4
		);");
		$res=$db->query($query);
		if(DB::isError($res)){
			printf("ERRORE 1");
			return FALSE;
		}
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}

		$query=sprintf("insert into sel_dist_tmp (sdtpspe,sdcdlin,sdnmort,sdnmclt,sdpspre,sdnmrgh,sdnmcpe) select rotpspe,rocdlin,count(ordprog),sum(ronmcll),sum(ropspre),sum(ronmrgh),sum(ronmcpe) from ric_ord group by rotpspe,rocdlin order by rotpspe,rocdlin;");
		$res=$db->query($query);
		if(DB::isError($res)){
			return FALSE;
		}
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}

	 
		$query=sprintf("select rostato,rotpspe,rocdlin,count(ordprog),sum(ronmcll),sum(ronmrgh),sum(ronmcpe),sum(ropspre) from ric_ord group by rostato,rotpspe,rocdlin order by rotpspe,rocdlin;");
		$res=$db->query($query);
		if(DB::isError($res)){
			return FALSE;
		}

		$szTPSPE="";
		$szCDLIN="";
		while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
			$szStato=$row[0];

			if($szTPSPE!=$row[1] || $szCDLIN!=$row[2]){
				$szTPSPE=$row[1];
				$szCDLIN=$row[2];

				$nOrdiniSpeditiHOST = 0;
				$nColliSpeditiHOST = 0;
				$nOrdiniStampatiDIST = 0; 
				$nColliStampatiDIST = 0;
				$nOrdiniStampatiXAB = 0; 
				$nColliStampatiXAB = 0;
				$nOrdiniEvasi = 0; 
				$nColliEvasi = 0;
				$nOrdiniInEvasione = 0; 
				$nColliInEvasione = 0;
				$nOrdiniTotali = 0; 
				$nColliTotali = 0; 
				$nRigheTotali = 0; 
				$nCopieTotali = 0;
				$nPesoTotale = 0;
			}
			switch($szStato){
				case "H":
					$nOrdiniSpeditiHOST+=$row[3];
					$nColliSpeditiHOST+=$row[4];
				case "D":
					$nOrdiniStampatiDIST+=$row[3];
					$nColliStampatiDIST+=$row[4];
				case "X":
					$nOrdiniStampatiXAB+=$row[3];
					$nColliStampatiXAB+=$row[4];
				case "E":
					$nOrdiniEvasi+=$row[3];
					$nColliEvasi+=$row[4];
				case "A":
				case "R":
				case "P":
				case "T":
					$nOrdiniInEvasione+=$row[3];
					$nColliInEvasione+=$row[4];
				default:
					$nOrdiniTotali+=$row[3];
					$nColliTotali+=$row[4];
				break;
			}
			$nRigheTotali+=$row[5];
			$nCopieTotali+=$row[6];
			$nPesoTotale+=$row[7];

			$query=sprintf("select sdtpspe,sdcdlin from sel_dist_tmp where sdtpspe='%s' and sdcdlin='%s';", $szTPSPE, $szCDLIN);

			$resLinea=$db->query($query);
			if(DB::isError($resLinea)){
				return FALSE;
			}

			if($rowLinea = $resLinea->fetchRow(DB_FETCHMODE_ORDERED)){
				/* esiste gia' */
				$query=sprintf(" update sel_dist_tmp set 
						sdnmort=%d,sdnmclt=%d,
						sdnmors=%d,sdnmcls=%d,
						sdnmord=%d,sdnmcld=%d,
						sdnmorx=%d,sdnmclx=%d,
						sdnmore=%d,sdnmcle=%d,
						sdnmori=%d,sdnmcli=%d where sdtpspe='%s' and sdcdlin='%s';",
					$nOrdiniTotali, $nColliTotali,
					$nOrdiniSpeditiHOST, $nColliSpeditiHOST,
					$nOrdiniStampatiDIST, $nColliStampatiDIST,
					$nOrdiniStampatiXAB, $nColliStampatiXAB,
					$nOrdiniEvasi, $nColliEvasi,
					$nOrdiniInEvasione, $nColliInEvasione,
					$szTPSPE,$szCDLIN);

			} else {
				/* non esiste */
				$query=sprintf("insert into sel_dist_tmp (sdtpspe,sdcdlin,sdnmrgh,sdnmcpe,sdpspre,sdnmort,sdnmclt, sdnmors,sdnmcls, sdnmord,sdnmcld, sdnmorx,sdnmclx, sdnmore,sdnmcle, sdnmori,sdnmcli) values ( '%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d );",
					$szTPSPE,$szCDLIN,
					$nRigheTotali, $nCopieTotali, $nPesoTotale,
					$nOrdiniTotali, $nColliTotali,
					$nOrdiniSpeditiHOST, $nColliSpeditiHOST,
					$nOrdiniStampatiDIST, $nColliStampatiDIST,
					$nOrdiniStampatiXAB, $nColliStampatiXAB,
					$nOrdiniEvasi, $nColliEvasi,
					$nOrdiniInEvasione, $nColliInEvasione);

			}
			$resUpdate=$db->query($query);

			if(DB::isError($resUpdate)){
				return FALSE;
			}
			if(!DB::isError($resUpdate) && $resUpdate!=DB_OK){
				$resUpdate->free();
			}
			if(!DB::isError($resLinea) && $resLinea!=DB_OK){
				$resLinea->free();
			}
		}
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}
		/* calcolo totali */
		$nOrdiniSpeditiHOST = 0;
		$nColliSpeditiHOST = 0;
		$nOrdiniStampatiDIST = 0; 
		$nColliStampatiDIST = 0;
		$nOrdiniStampatiXAB = 0; 
		$nColliStampatiXAB = 0;
		$nOrdiniEvasi = 0; 
		$nColliEvasi = 0;
		$nOrdiniInEvasione = 0; 
		$nColliInEvasione = 0;
		$nOrdiniTotali = 0; 
		$nColliTotali = 0; 
		$nRigheTotali = 0; 
		$nCopieTotali = 0;
		$nPesoTotale = 0;

		$query=sprintf("select rostato,rotpspe,rocdlin,count(ordprog),sum(ronmcll),sum(ronmrgh),sum(ronmcpe),sum(ropspre) from ric_ord group by rostato,rotpspe,rocdlin order by rotpspe,rocdlin;");
		$res=$db->query($query);
		if(DB::isError($res)){
			return FALSE;
		}

		$szTPSPE="";
		$szCDLIN="";
		while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){

			switch($row[0]){
				case "H":
					$nOrdiniSpeditiHOST+=$row[3];
					$nColliSpeditiHOST+=$row[4];
				case "D":
					$nOrdiniStampatiDIST+=$row[3];
					$nColliStampatiDIST+=$row[4];
				case "X":
					$nOrdiniStampatiXAB+=$row[3];
					$nColliStampatiXAB+=$row[4];
				case "E":
					$nOrdiniEvasi+=$row[3];
					$nColliEvasi+=$row[4];
				case "A":
				case "R":
				case "P":
				case "T":
					$nOrdiniInEvasione+=$row[3];
					$nColliInEvasione+=$row[4];
				default:
					$nOrdiniTotali+=$row[3];
					$nColliTotali+=$row[4];
				break;
			}
			$nRigheTotali+=$row[5];
			$nCopieTotali+=$row[6];
			$nPesoTotale+=$row[7];
		}
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}

		/* totali */
		$query=sprintf("insert into sel_dist_tmp (sdtpspe,sdcdlin,sddslin,sdnmrgh,sdnmcpe,sdpspre,sdnmort,sdnmclt, sdnmors,sdnmcls, sdnmord,sdnmcld, sdnmorx,sdnmclx, sdnmore,sdnmcle, sdnmori,sdnmcli) values ( '%s','%s','%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d );",
			"--","--","TOTALI",
			$nRigheTotali, $nCopieTotali, $nPesoTotale,
			$nOrdiniTotali, $nColliTotali,
			$nOrdiniSpeditiHOST, $nColliSpeditiHOST,
			$nOrdiniStampatiDIST, $nColliStampatiDIST,
			$nOrdiniStampatiXAB, $nColliStampatiXAB,
			$nOrdiniEvasi, $nColliEvasi,
			$nOrdiniInEvasione, $nColliInEvasione);
		$res=$db->query($query);
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}

		$query=sprintf("select sdtpspe,sdcdlin from sel_dist_tmp where sddslin is NULL;");
		$res=$db->query($query);
		while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){

			$szTPSPE=$row[0];
			$szCDLIN=$row[1];

			/* ricerca descrizione linea di spedizione  */
			/* 1 : cerco con scalo='**' e sottoscalo='**' */
			$bOK=FALSE;
			$query=sprintf("select elcdslin from ttlv where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s' and elccdlin='%s' and elccdsca='**' and elccdssc='**';", "AM","0000015",$szTPSPE,$szCDLIN);
			$resTTLV=$db->query($query);
			if($rowTTLV = $resTTLV->fetchRow(DB_FETCHMODE_ORDERED)){
				$szDSLIN=$rowTTLV[0];
				$bOK=TRUE;
			}
			if(!DB::isError($resTTLV) && $resTTLV!=DB_OK){
				$resTTLV->free();
			}
			if(!bOK){
				/* 2 : cerco con scalo='00' e sottoscalo='00' */
				$query=sprintf("select elcdslin from ttlv where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s' and elccdlin='%s' and elccdsca='00' and elccdssc='00';", "AM","0000015",$szTPSPE,$szCDLIN);
				$resTTLV=$db->query($query);
				if($rowTTLV = $resTTLV->fetchRow(DB_FETCHMODE_ORDERED)){
					$szDSLIN=$rowTTLV[0];
					$bOK=TRUE;
				}
				if(!DB::isError($resTTLV) && $resTTLV!=DB_OK){
					$resTTLV->free();
				}
			}
			if(!bOK){
					/* 3 : cerco solo con tpspe in ttts */
				$query=sprintf("select elcdstsp from ttts where elccdsoc='%s' and elccdmag='%s' and elctpspe='%s';",$szTPSPE,$szCDLIN);
				$resTTLV=$db->query($query);
				if($rowTTLV = $resTTLV->fetchRow(DB_FETCHMODE_ORDERED)){
					$szDSLIN=$rowTTLV[0];
					$bOK=TRUE;
				}
				if(!DB::isError($resTTLV) && $resTTLV!=DB_OK){
					$resTTLV->free();
				}
			}
			if(!bOK){
				$szDSLIN="DESCRIZIONE LINEA INESISTENTE";
			}

			$query=sprintf("update sel_dist_tmp set sddslin=%s where sdtpspe='%s' and sdcdlin='%s';", $db->quote($szDSLIN), $szTPSPE, $szCDLIN);
			$resDSLIN=$db->query($query);
			if(!DB::isError($resDSLIN) && $resDSLIN!=DB_OK){
				$resDSLIN->free();
			}
		}
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}
		return TRUE;
	}
	/*
	* do_statistica_evasione()
	*/
	function do_statistica_evasione($db,$numero_cedola)
	{
		$query=sprintf("drop table stat_eva_tmp;");
		$res=$db->query($query);
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}
		$query=sprintf("create table stat_eva_tmp (
			sedteva text,
			setmeva text,
			senmcll int4,
			senmrgh int4,
			senmcpe int4
		);");
		$res=$db->query($query);
		if(DB::isError($res)){
			return FALSE;
		}
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}

		$query=sprintf("insert into stat_eva_tmp ( sedteva, setmeva,senmcll, senmrgh, senmcpe ) select data(cptmeva),ora_HH(cptmeva),count(ordprog),sum(cpnmrgh),sum(cpnmcpe) from col_prod where cpstato in('E','H','D','X') group by data(cptmeva),ora_HH(cptmeva);");
		$res=$db->query($query);
		if(DB::isError($res)){
			return FALSE;
		}
		if(!DB::isError($res) && $res!=DB_OK){
			$res->free();
		}
	 
		return TRUE;
	}
	function do_chart ($file) {
		echo "<img src=\"$file.php\">\n";
	}
?>
