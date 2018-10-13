
<?php
header('Location: home.html');
ini_set('max_execution_time', 70);
date_default_timezone_set('Asia/Ho_Chi_Minh');

if (!isset($_SERVER['PHP_AUTH_USER'])) {
    header('WWW-Authenticate: Basic realm="Cemu#628173"');
    header('HTTP/1.0 401 Unauthorized');
	header("Connection: keep-alive \r\n");
    echo 'Text to send if user hits Cancel button';
	} else {
    if($_SERVER['PHP_AUTH_USER']== "NghiaAdmin"){
		if($_SERVER['PHP_AUTH_PW']== "applebanana"){
		if ($_POST <> null){
		$myfile = fopen("Data/history.txt", "a") or die("Unable to open file!");
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
	}
	}
}
/* 

echo "The time is " . date("h:i:sa");
echo '<br/>Dữ Liệu Chúng tui Nhận Được Là <br/>';


foreach ($_GET as $key => $val)
{
    echo '<strong>' . $key . ' => ' . $val . '</strong><br/>';
}

echo 'toan bo la day<br/>';
var_dump($_GET);
echo 'du lieu cu <br/>';

$myfile = fopen("newfile.txt", "r") or die("Unable to open file!");
while(!feof($myfile)) {
  echo fgets($myfile) . "<br>";
}
fclose($myfile); */
?>
