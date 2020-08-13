<?php
// attach to session
session_start();

//---
require('config.php');

// connecting, selecting database
$db = mysqli_init();
if (!$db)
	die(json_encode(array('error' => 'mysqli_init failed')));
if (!@$db->real_connect($host, $user, $password, $database))
	die(json_encode(array('error' => 'Could not connect: ' . mysqli_connect_error())));
$query = "set charset utf8";
$result = $db->query($query);
if (!$result) die(json_encode(array('error' => 'Invalid query: ' . $db->error)));
//---

// get posted values
$options = array(
	'text' => @$_POST['text'],
	'outlinenr' => @$_POST['outlinenr'],
	'dither' => @$_POST['dither'],
	'colour' => @$_POST['colour']
);
if (isset($_POST['highcontrast'])) {
	$s = $_POST['highcontrast'];
	if ($s == '1' || $s == 'yes' || $s == 'on' || $s == 'true')
		$options['highcontrast'] = 1;
}
if (isset($_POST['colourlow'])) {
	$s = $_POST['colourlow'];
	if ($s == '1' || $s == 'yes' || $s == 'on' || $s == 'true')
		$options['colourlow'] = 1;
}
$imageB64 = @$_POST['image'];
if (empty($imageB64))
	die(json_encode(array('error' => 'Missing image')));

// lock the table
$query = "LOCK TABLES queue LOW_PRIORITY WRITE";
$result = $db->query($query) or die(json_encode(array('error' => 'Invalid query: ' . $db->error)));
// create job ID
$jobId = '';
for ($retry = 0; $retry < 10; $retry++) {

	// create idstring
	$keyString = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	$v = rand();
	for ($i = 0; $i < 6; $i++) {
		$jobId .= $keyString[$v % 62];
		$v /= 62;
	}

	$query = 'SELECT count(*) FROM queue WHERE jobid="' . $db->real_escape_string($jobId) . '"';
	$result = $db->query($query) or die(json_encode(array('error' => 'Invalid query: ' . $db->error)));
	$row = $result->fetch_row();
	if (!$row[0])
		break; // free to use

	$jobId = '';
}
if (!$jobId)
	die(json_encode(array('error' => 'Failed to create Job ID')));

$query = 'INSERT INTO queue (jobid, options, txt, outlinenr, imageb64) VALUES (' .
	'"' . $db->real_escape_string($jobId) . '",' .
	'"' . $db->real_escape_string(json_encode($options)) . '",' .
	'"' . $db->real_escape_string($options['text']) . '",' .
	'"' . $db->real_escape_string($options['outlinenr']) . '",' .
	'"' . $db->real_escape_string($imageB64) . '")';

$result = $db->query($query) or die(json_encode(array('error' => 'Invalid query: ' . $db->error)));
$rowId = $db->insert_id;

// unlock tables
$query = "UNLOCK TABLES";
$result = $db->query($query) or die(json_encode(array('error' => 'Invalid query: ' . $db->error)));

// now, get the number of waiters
$query = 'SELECT count(*) FROM queue WHERE id<' . $rowId . ' AND status=0';
$result = $db->query($query) or die(json_encode(array('error' => 'Invalid query: ' . $db->error)));
$row = $result->fetch_row();
$numWaiters = $row[0];

if ($numWaiters == 0)
	die(json_encode(array('jobid' => $jobId, 'delay' => 1000, 'info' => 'Your QR is being generated. Please wait.')));
if ($numWaiters < 5)
	die(json_encode(array('jobid' => $jobId, 'delay' => 1000, 'info' => 'Your QR is queued at position #' . $numWaiters)));
die(json_encode(array('jobid' => $jobId, 'delay' => 10000, 'info' => 'Your QR is queued at position #' . $numWaiters)));
	
