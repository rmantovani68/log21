<?
require('chart/chart.php');
require('chart/data.php');

$chart = new chart(300, 200, "example21");
$chart->set_grid_color("pink", false);
$chart->plot($data, false, "blue", "gradient", "green", 7);
$chart->plot($data, false, "red", "gradient", "yellow", 0);
$chart->stroke();
?>

