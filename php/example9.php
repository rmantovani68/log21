<?
require('chart/chart.php');
require('chart/data.php');

$chart = new chart(300, 200, "example9");
$chart->plot($data, false, "black", "impulse");
$chart->stroke();
?>

