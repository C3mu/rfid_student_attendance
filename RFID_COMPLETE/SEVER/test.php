<?php
include 'library.php';

/* 
$wtable = new timetable();
$wtable->put_time(1,"12:00","13:00");
$wtable->put_time(1,"14:00","16:00");
echo ChangetimeGroup($wtable,1);
*/

/*
$student = new aboutstudent();
$student->Name = "Nguyen Do Quoc Anh";
$student->Cls = "1699";
$student->Id = "01649461014";
$student->Group = 1;
echo Newstudent($student);
*/
//echo loaddata();

/* $memcache = new Memcache;
$memcache->connect('localhost', 11211) or die ("Could not connect");

$version = $memcache->getVersion();
echo "Server's version: ".$version."<br/>\n";

$tmp_object = new stdClass;
$tmp_object->str_attr = 'test';
$tmp_object->int_attr = 123;

$memcache->set('key', $tmp_object, false, 10) or die ("Failed to save data at the server");
echo "Store data in the cache (data will expire in 10 seconds)<br/>\n";

$get_result = $memcache->get('key');
echo "Data from the cache:<br/>\n";

var_dump($get_result); */

print_r(loaddata());

?>