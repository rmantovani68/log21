<?
require('chart/chart.php');
require('chart/data.php');

$chart = new chart(300, 200, "example2");
$chart->plot($data3, false, "blue", "impulse");
$chart->plot($data);
$chart->stroke();
?>

