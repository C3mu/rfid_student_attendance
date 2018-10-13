<link rel="shortcut icon" href="/icon.png" />
<?php
echo '<br/>Dữ Liệu Chúng tui Nhận Được Là <br/>';
foreach ($_GET as $key => $val)
{
    echo '<strong>' . $key . ' => ' . $val . '</strong><br/>';
}
echo 'toan bo la day<br/>';
echo $_GET;
if ($_POST <> null){
$myfile = fopen("newfile.txt", "a") or die("Unable to open file!");
date_default_timezone_set("Asia/Ho_Chi_Minh");
	$date = new DateTime();
	$times = $date->format('d-m-y-H-i-s'); 
	$txt = " ";
foreach ($_POST as $key => $val)
{   	
    $txt .=  "{$key}=>{$val} ";	
}
fwrite($myfile, $times);
fwrite($myfile, $txt);
fwrite($myfile, "\n");
fclose($myfile);
}
var_dump($_GET);
echo 'du lieu cu <br/>';
$myfile = fopen("newfile.txt", "r") or die("Unable to open file!");
while(!feof($myfile)) {
  echo fgets($myfile) . "<br>";
}
fclose($myfile);
?>
