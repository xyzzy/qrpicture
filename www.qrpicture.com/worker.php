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
	die('Could not connect: ' . mysqli_connect_error());
$query = "set charset utf8";
$result = $db->query($query);
if (!$result) die(json_encode(array('error' => 'Invalid query: ' . $db->error)));
//---

// test if worker already active
$query = "SELECT GET_LOCK('qrpicture_worker',1)";
$result = $db->query($query) or die(json_encode(array('error' => 'Invalid query: ' . $db->error)));
$row = $result->fetch_row();
$lock = $row[0];

if (!$lock)
	die(json_encode(array('error' => 'Queue worker already running')));

for (; ;) {
	// get first QR
	$query = 'SELECT min(id) FROM queue WHERE status=0';
	$result = $db->query($query) or die(json_encode(array('error' => 'Invalid query: ' . $db->error)));
	$row = $result->fetch_row();
	$rowId = $row[0];
	if (!$rowId)
		die(json_encode(array('error' => 'Queue empty')));

	// get full record
	$query = 'SELECT * FROM queue WHERE id=' . $rowId;
	$result = $db->query($query) or die(json_encode(array('error' => 'Invalid query: ' . $db->error)));
	$row = $result->fetch_assoc();

	$jobId = $row['jobid'];
	$txt = $row['txt'];
	$outlineNr = $row['outlinenr'];
	$rawImg = base64_decode($row['imageb64']);

	ini_set("display_errors", 1);
	ini_set("display_startup_errors", 1);
	error_reporting(-1);

	if (1) {
		$rawImg = str_replace("data:image/png;base64,", "", $row['imageb64']);
		$rawImg = base64_decode($rawImg);
		$rawImg = imagecreatefromstring($rawImg);
		// resize to 93x93
		$im = @imagescale($rawImg, 93, 93, IMG_BICUBIC);

	} else {
		// create image
		$im = @imagecreatetruecolor(93, 93);
		$w = imagecolorallocate($im, 255, 255, 255);
		for ($y = 0; $y < 93; $y++) {
			for ($x = 0; $x < 93; $x++) {
				$i = $x;
				$j = $y;
				if ($i < 0 || $j < 0 || $i >= 93 || $j >= 93) {
					imagesetpixel($im, $x, $y, $w);
				} else {
					imagesetpixel($im, $x, $y, imagecolorallocate($im, ord($rawImg[$j * 279 + $i * 3 + 0]), ord($rawImg[$j * 279 + $i * 3 + 1]), ord($rawImg[$j * 279 + $i * 3 + 2])));
				}
			}
		}
	}

	// save image
	if (!@imagepng($im, 'images.incoming/' . $jobId . '.png'))
		die(json_encode(array('error' => 'Failed to save image')));

	$docroot = @$_SERVER["DOCUMENT_ROOT"];
	if (empty($docroot))
		$docroot = '.';

	$cmd = $docroot . '/bin/qrwork "' . addslashes($txt) . '" images.incoming/' . $jobId . '.png images/' . $jobId . '.png --outline=' . $outlineNr . ' --maxsalt=0';
	$json = `$cmd`;

	// update status
	$query = 'UPDATE queue SET status=2, result="' . addslashes($json) . '", imagefilename="' . addslashes('images/' . $jobId . '.png') . '" WHERE id=' . $rowId;
	$result = $db->query($query) or die(json_encode(array('error' => 'Invalid query: ' . $db->error)));

}
die(json_encode(array('error' => 'done')));
