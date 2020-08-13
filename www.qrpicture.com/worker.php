<?php
/*
 *  This file is part of qrpicture, photo realistic QR-codes.
 *  Copyright (C) 2007, xyzzy@rockingship.net
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
 
ini_set("display_errors", 1);
ini_set("display_startup_errors", 1);
error_reporting(-1);

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

	$rawImg = str_replace("data:image/png;base64,", "", $row['imageb64']);
	$rawImg = base64_decode($rawImg);
	$rawImg = imagecreatefromstring($rawImg);

	// save image
	if (!@imagepng($rawImg, 'images/' . $jobId . '-186x186-upload.png'))
		die(json_encode(array('error' => 'Failed to save image')));

	// resize to 93x93
	$im = @imagescale($rawImg, 93, 93, IMG_BICUBIC);

	// save image
	if (!@imagepng($im, 'images/' . $jobId . '-93x93-upload.png'))
		die(json_encode(array('error' => 'Failed to save image')));

	$docroot = @$_SERVER["DOCUMENT_ROOT"];
	if (empty($docroot))
		$docroot = '.';

	$cmd = $docroot . '/bin/qrwork "' . addslashes($txt) . '" images/' . $jobId . '-93x93-upload.png images/' . $jobId . '-93x93-mask.png --outline=' . $outlineNr . ' --maxsalt=0';
	$json = `$cmd`;

	$cmd = $docroot . '/bin/qrscq images/' . $jobId . '-186x186-upload.png images/' . $jobId . '-93x93-mask.png 250 images/' . $jobId . '-186x186.png --filter=1';
	$json = `$cmd`;

	// update status
	$query = 'UPDATE queue SET status=2, result="' . addslashes($json) . '", imagefilename="' . addslashes('images/' . $jobId . '-186x186.png') . '" WHERE id=' . $rowId;
	$result = $db->query($query) or die(json_encode(array('error' => 'Invalid query: ' . $db->error)));

}
die(json_encode(array('error' => 'done')));
