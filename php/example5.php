<?
require('chart/chart.php');
require('chart/data.php');

$chart = new chart(300, 200, "example5");
$chart->plot($data);
$chart->set_labels("Day", "Money");
$chart->stroke();
?>

