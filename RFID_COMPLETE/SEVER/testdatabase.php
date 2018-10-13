<?php
$servername = "mysql.hostinger.vn";
$username = "u258326596_c3mu";
$password = "uJZYi8|>K@sf#Qm83=";
//$dbname = "u258326596_test";

// Create connection
$conn = mysqli_connect($servername, $username, $password);

// Check connection
if (!$conn) {
    die("Connection failed: " . mysqli_connect_error());
}
echo "Connected successfully";

$sql = "CREATE DATABASE myDB";
if ($conn->query($sql) === TRUE) {
    echo "Database created successfully";
} else {
    echo "Error creating database: " . $conn->error;
}

$conn->close();


?>