<?
require('chart/chart.php');
require('chart/data.php');

$chart = new chart(300, 200, "example23");
$chart->plot($data);
$chart->set_extrema(9, 81);
$chart->stroke();
?>

