<?
require('chart/chart.php');
require('chart/data.php');

$chart = new chart(300, 200, "example8");
$chart->plot($data, false, "black", "points");
$chart->stroke();
?>

