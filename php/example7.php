<?
require('chart/chart.php');
require('chart/data.php');

$chart = new chart(300, 200, "example7");
$chart->plot($data, false, "black", "square");
$chart->stroke();
?>

