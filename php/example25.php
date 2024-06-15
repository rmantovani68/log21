<?
require('chart/chart.php');
require('chart/data.php');

$chart = new chart(300, 200, "example25");
$chart->plot($data, false, "gray", "box", "black");
$chart->stroke();
?>

