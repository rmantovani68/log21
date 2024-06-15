<?php
	/* statistica di evasione oraria copie */
	include ("jpgraph/jpgraph.php");
	include ("jpgraph/jpgraph_bar.php");
	require_once 'DB.php';

	$date  = getdate();
	$month = date("m");
	$year  = date("Y");
	$day   = date("d");
	$data=sprintf("%02d-%02d-%04d",$day,$month,$year);

	$datax=array( "07:00", "08:00", "09:00", "10:00", "11:00", "12:00", "13:00", "14:00", "15:00", "16:00", "17:00", "18:00", "19:00", "20:00", "21:00", "22:00");
	$datay=array(       0,      0,       0,       0,      0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0); 

	$db = DB::connect("pgsql://roberto:3zin@localhost/mondadori", true);
	if(DB::isError($db)) {
		die($db->getMessage);
	}

	$query=sprintf("select sedteva,setmeva,senmcll,senmrgh,senmcpe from stat_eva_tmp where sedteva='%s' order by sedteva,setmeva;",$data);
	$res = $db->query($query);
	while($row = $res->fetchRow(DB_FETCHMODE_ORDERED)){
		$datay[$row[1]-7]=$row[4];
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
	$title=sprintf("%02d-%02d-%04d - Statistica di evasione copie su base oraria",$day,$month,$year);
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

	$bplot->SetFillColor("seagreen3");
	$bplot->SetColor("seagreen");
	$graph->Add($bplot);

	// Finally send the graph to the browser
	$graph->Stroke();

?>
