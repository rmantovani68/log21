<?
require('chart/chart.php');
require('chart/data.php');

$chart = new chart(300, 200, "example14");
$chart->plot($data, false, "red", "gradient", "black", 2);
$chart->stroke();
?>

