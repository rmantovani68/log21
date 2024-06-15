<?
require('chart/chart.php');
require('chart/data.php');

$chart = new chart(300, 200, "example1");
$chart->plot($data);
$chart->stroke();
?>

