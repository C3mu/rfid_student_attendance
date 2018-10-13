<?php
date_default_timezone_set('Asia/Ho_Chi_Minh');

///////////////////////////////////////// class use for data input////////////////////////////////////

class timetable // complete 1.0 test ok
{
//from Sunday to Friday
// we will add data in to below string like 12:30-14:29;13:30-15:30;\n
//						                    start end  ;start end  ;
//                                          time 1     ;time 2     ;end

    public $day1 = '1;';
    public $day2 = '2;';
	public $day3 = '3;';
	public $day4 = '4;';
	public $day5 = '5;';
	public $day6 = '6;';
	public $day7 = '7;';

	function put_time($day,$start,$end){ 
							//int;str;str
		switch($day){
		
		case 1: 
			$this->day1 .= $start."-".$end.";"; 
			break;
		case 2: 
			$this->day2 .= $start."-".$end.";"; 
			break;
		case 3: 
			$this->day3 .= $start."-".$end.";"; 
			break;
		case 4: 
			$this->day4 .= $start."-".$end.";"; 
			break;
		case 5: 
			$this->day5 .= $start."-".$end.";"; 
			break;
		case 6: 
			$this->day6 .= $start."-".$end.";"; 
			break;
		case 7: 
			$this->day7 .= $start."-".$end.";"; 
			break;
		}
	}
	// just forget to use array V:

}

class aboutstudent // complete 1.1 test oke
{
	public $Name = "";
	public $Cls = "";
	public $Id = "";
	public $Group = 0;//there is no group 0
	public $timelate = 0;
}

////////////////////////////////////////////////FUNCTION//////////////////////////////////////////////


function checkdata(){
	
}

function loaddata(){
	
	//loop all the timetable file in group foder
	$directory  = './Group';
	$listfile = array_diff(scandir($directory), array('..', '.'));
	if ($listfile == false){
		return false;
	}
	
	foreach ($listfile as $value) {
		echo $value;
		echo "loopfile </br>";
		// check all file
	$filename = $directory."/".strval($value)."/"."timetable.txt";
	$dayofweek = (time() + 86400*4 + 25200)/ 86400 % 7; // the problem is that is... thusday LOL
	echo $dayofweek; echo "<br>";
    $file = fopen($filename,"r");
	for($i = 0; $i < $dayofweek; $i++){echo "skipdate"; echo "<br>"; fgets($file);};
	//1;12:30-14:29;13:30-15:30;\n
	echo fgetc($file); echo "<br>";
	echo fgetc($file); echo "<br>";
	$string = fgets($file);
	print_r($string); echo "<br>";
	$timebasic = explode(';',$string);
	print_r($timebasic); echo "<br>";
	/*
	array 
	1=>12:30-14:29
	2=>13:30-15:30
	*/
	
	foreach ($timebasic as $val){
		echo "RUN</br>";
		//check time in array
		$timebasic2 = explode('-',$val);
		print_r($timebasic2); echo "<br>";
		/*
		array 
		1=>12:30
		2=>14:29
		*/
		if(intime($timebasic2[0])){
			echo "IN TIMES !!!</br>";
			if(!outime($timebasic2[1])){
				//loaddata
				echo "INNNNNNNN !</br>";
				return true;
			}
		}
		
	}
	}
	
	return False;
	
}

function intime($input){
	$timebasic3 = explode(':',$input);
	// array
	//1 => 12
	//2 => 30
	$hours = (time()+25200) / 3600 % 24;
	echo $hours; echo "h <br>";
	$minutes = (time()+25200) / 60 % 60;
	echo $minutes; echo "m <br>";
	if(intval($timebasic3[1]) >= $hours){//check hour
		if(intval($timebasic3[2]) >= $minutes){//check minute
			return true;
		}
	}
	return false;
	
}

function outime($input){
	$timebasic3 = explode(':',$input);
	$hours = (time()) / 3600 % 24;
	$minutes = (time()) / 60 % 60;
	if(intval($timebasic3[1]) <= $hours){//check hour
		if(intval($timebasic3[2]) <= $minutes){//check minute
			return true;
		}
	}
	return false;
}

//////////////////most hate (student) function////////////////

function SendEmail(){
	// this is for my teacher ... idk 
}

//created new group <=> class 
function Newgroup(){// complete 1.0 test ok
		$directory  = './Group';
		$files1 = array_diff(scandir($directory), array('..', '.'));
		if ($files1 == false){
			if(!mkdir("./Group/1")){
			//echo "FALSE";
			}
			return 1;
		}
		$groubname = 2;
		if($files1 == true){
			while($groubname-1 == intval($files1[$groubname])){	
			$groubname++;
			}
		}
		$foname = "./Group/".strval($groubname-1);
			if(!mkdir($foname)){
			//echo "FALSE";
			}
		return $groubname;	
}

//changetimefile
function ChangetimeGroup($timetb,$group){ //complete 1.0 test ok
    
	if(!is_a($timetb,'timetable')) // check the data input
	{
		return false;
	}
	$directory  = './Group'."/".strval($group);
	$filename = $directory."/timetable.txt";
	$file = fopen($filename, "w") or die("Unable to open file!");
	
	if ($file == false){
		return false;
	}
	// write data in to file
	$linedown = "\n";
	$writedata = $timetb->day1 .$linedown.
				 $timetb->day2 .$linedown.
				 $timetb->day3 .$linedown.
				 $timetb->day4 .$linedown.
				 $timetb->day5 .$linedown.
				 $timetb->day6 .$linedown.
				 $timetb->day7 .$linedown;
	fwrite($file,$writedata);
	fclose($file);
	return true;
}

///////////////////word with student//////////////////////////

function Newstudent($student){ //complete 1.0 test ok
				  //object
	if(!is_a($student,'aboutstudent')) // check the data input
	{
		return false;
	}
	if (($student->Group) == 0){
		return false;
	}
	$directory  = './Group'."/".strval($student->Group);
	
	if(!is_dir($directory)) {
		return false;
	}
	$datawrite = $student->Name."\t".
				 $student->Cls."\t".
	             $student->Id."\t".
	             strval($student->Group)."\t".
	             strval($student->timelate)."\n";
	$filename = $directory."/student.txt";
	$file = fopen($filename, "a") or die("Unable to open file!");
		if ($file == false){
			return false;
		}
	fwrite($file,$datawrite);
	fclose($file);
	return true;
}

//move student ... late

function Move(){
	
}

function ChangeStudent(){
	
}

function Getstudent(){ 
	
}

?>
























