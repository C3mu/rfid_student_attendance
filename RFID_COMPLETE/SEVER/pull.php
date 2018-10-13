<?php
include 'Data/library.php';
echo("hmmm");
ini_set('max_execution_time', 70);
date_default_timezone_set('Asia/Ho_Chi_Minh');
if (isset($_SERVER['PHP_AUTH_USER'])) {
	if($_SERVER['PHP_AUTH_USER']== "NghiaAdmin"){
		if($_SERVER['PHP_AUTH_PW']== "applebanana"){
  	if (isset($_GET['KEEPSTREAM'])) {
		if ($_GET['KEEPSTREAM'] == "TmdoaWFBZG1pbjphcHBsZWJhbmFuYQ"){}
		
		$myfile = fopen("keppfile.txt", "a") or die("Unable to open file!");
		
/* 		while (connection_aborted() == 0){ //kiem tra ket noi
			//do job here v:
			echo(".");
			$daate = new DateTime();
			$tiimes = $daate->format('d-m-y-H-i-s'); 
			fwrite($myfile,$tiimes);
			fwrite($myfile," working...\n");
			sleep(3000);
		} */
		checkdata();
		echo(".het");
		fwrite($myfile," workingdone...\n");
		fclose($myfile);
	}
	}
	}
	}  
?>