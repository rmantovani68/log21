<?php
	/* statistica di evasione giornaliera righe */
	include ("jpgraph/jpgraph.php");
	include ("jpgraph/jpgraph_bar.php");
	require_once 'DB.php';

	$date  = getdate();
	$month = date("m");
	$year  = date("Y");
	$day   = date("d");
	$day_of_week   = date("w");
	$data=sprintf("%02d-%02d-%04d",$day,$month,$year);

	$db = DB::connect("pgsql://roberto:3zin@localhost/mondadori", true);
	if(DB::isError($db)) {
		die($db->getMessage);
	}

	$query=sprintf("select sedteva,sum(senmcll),sum(senmrgh),sum(senmcpe) from stat_eva_tmp group by sedteva order by sedteva;");
	$res = $db->query($query);
	$i=0;
	while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
		$datay[]=$row[2];
		$datax[]=$row[0];
		$i++;
	}
	if(!DB::isError($res) && $res!=DB_OK){
		$res->free();
	}
	$db->disconnect();

	// We need some data

	// Setup the graph. 
	$graph = new Graph(800,300,"auto");	
	$graph->img->SetMargin(60,20,30,90);
	$graph->SetScale("textlin");
	$graph->SetMarginColor("silver");
	$graph->SetShadow();

	// Set up the title for the graph
	$title=sprintf("%02d-%02d-%04d - Statistica di evasione righe su base giornaliera",$day,$month,$year);
	$graph->title->Set($title);
	$graph->title->SetFont(FF_VERDANA,FS_NORMAL,12);
	$graph->title->SetColor("darkred");

	// Setup font for axis
	$graph->xaxis->SetFont(FF_VERDANA,FS_NORMAL,10);
	$graph->yaxis->SetFont(FF_VERDANA,FS_NORMAL,10);

	// Show 0 label on Y-axis (default is not to show)
	$graph->yscale->ticks->SupressZeroLabel(false);

	// Setup X-axis labels
	$graph->xaxis->SetTickLabels($datax); 
	$graph->xaxis->SetLabelAngle(50); 

	// Create the bar pot
	$bplot = new BarPlot($datay);
	$bplot->SetWidth(0.8);

	$bplot->SetFillColor("sienna2");
	$bplot->SetColor("sienna4");
	$graph->Add($bplot);

	// Finally send the graph to the browser
	$graph->Stroke();

?>
