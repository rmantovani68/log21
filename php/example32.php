<?
require('chart/chart.php');
require('chart/data.php');

$chart = new chart(300, 200, "example32");
$chart->set_output_size(100, 50);
$chart->plot($data);
$chart->stroke();
?>

