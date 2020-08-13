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
$jobId = @$_REQUEST['jobid'];

// now, get the number of waiters
$query = 'SELECT id,status,result,imagefilename FROM queue WHERE jobid = "' . $db->real_escape_string($jobId) . '"';
$result = $db->query($query) or die(json_encode(array('error' => 'Invalid query: ' . $db->error)));
$row = $result->fetch_row();
if (!$row)
	die(json_encode(array('error' => 'Your QR has been removed from the queue')));
$rowId = $row[0];

if ($row[1] == 2) {
	$json = json_decode($row[2]);
	if (!empty($json->error))
		die(json_encode(array('jobid' => $jobId, 'error' => $json->error, 'imagefilename' => $row[3])));
	if (!empty($json->info))
		die(json_encode(array('jobid' => $jobId, 'info' => $json->info, 'imagefilename' => $row[3])));
	if (!empty($json->elapsed))
		die(json_encode(array('jobid' => $jobId, 'info' => 'Used ' . $json->elapsed . ' CPU seconds. Right-click to save.', 'imagefilename' => $row[3])));
	die(json_encode(array('jobid' => $jobId, 'imagefilename' => $row[3])));
}

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
	
