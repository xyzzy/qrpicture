<?php
    /*
     * @date 2020-08-12 12:28:35
     * This is a disabled experimental service to allow URL shortening hosted on server.
     */
	if (0 && count($_GET) == 1 && strlen($k=key($_GET)) == 6) {
		$fname = 'data/'.$k.'.json';
		if (!file_exists($fname))
			die('QR contents is no longer available');
		$json = json_decode(@file_get_contents($fname));

		if (!empty($json->mime))
			header('Content-type: '.$json->mime);
		if (!empty($json->url))
			header('Location: '.$json->url);
		die($json->data);
	}
		
	// open session
	session_start();
?><!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
<meta name="description" content="Create photo, image and colour realistic QR codes" />
<meta name="robots" content="noindex,nofollow" />
<link rel="shortcut icon" href="favicon.ico" />
<title>www.QRpicture.com</title>
<style type="text/css">
body {
	font-family: Verdana, Geneva, sans-serif;
	font-size: 10pt;
	font-weight: normal;
	position: absolute;
	margin: 0px;
	padding: 5px;
	left: 0px;
	width: 100%;
	right: 0px;
	top: 0px;
	height: 100%;
	bottom: 0px;
}
img {
	border: none;
}
.example {
	border: none;
	padding-bottom:160px;
}
#image {
	position: relative;
	margin: auto;
	padding: 0px;
	left: 0px;
	right: 0px;
	top: 0px;
	bottom: 0px;
}

#qr_canvas {
  position: absolute;
  border: 2px solid #fff;
}
#qr_drag {
  position: absolute;
  border: none;
  background: white;
}
#draggables {
  position: relative;
  background: none;
}
.dragview {
	position: absolute;
}
.dragthumb {
	position: absolute;
	border: 1px solid black;
	width: 4px;
	height: 4px;
	background: #0f0;
}
.maskPreview {
	position: relative;
	border: 1px solid #999;
}
.logo {
	position: relative;
	left: 0;
	right: 0;
	margin: auto;
	width: 846px;
}
.textContent {
	  position: relative;
	  width: 508px;
	  height: 416px;
	  border: none;
	  padding: 0;
	  margin: 10px 0;
	  overflow: hidden;
	  border: 1px solid black;
}
.textSlider {
	  border: none;
	  position: absolute;
	  width: 5100px;
	  height: 416px;
	  margin: 0;
	  padding: 0;
}
.textFrame {
	  position: relative;
	  float: left;
	  width: 498px;
	  height: 406px;
	  margin: 0;
	  padding: 5px;
}


/*****/

.textTabs {
	  position: relative;
	  width: 510px;
	  height: 44px;
	  border: none;
	  padding: 0;
	  margin: 0;
}

/*****/

#wrapper {
	width: 600px;
	margin: 0 auto;
}

.tabsWindow {
	  position: relative;
	  width: 510px;
	  height: 44px;
	  border: none;
	  padding: 0;
	  margin: 0;
}

ul.tabs {
	list-style-type: none;
	padding: 0;
	margin: 0;
	float: left;
	height: 44px;
}

ul.tabs li {
	margin: 10px 4px;
	display: inline;
	float: left;
	padding: 4px 10px;
	-webkit-border-radius: 16px;
	-moz-border-radius: 16px;
	text-shadow: 1px 1px 1px #eee;
	background: #999;
	color: #000;
	font-weight: bold;
	cursor: default;
}

ul.tabs li.active { background: #ddd; }
ul.tabs li:active { background: #666; }

.contentsWindow {
	  position: relative;
	  width: 520px;
	  height: 500px;
	  border: none;
	  padding: 0;
	  margin: 10px 0;
	  overflow: hidden;
	  border: 1px solid black;
}
.contentsSlider {
	  border: none;
	  position: absolute;
	  width: 5100px;
	  height: 500px;
	  margin: 0;
	  padding: 0;
}
.contentsFrame {
	  position: relative;
	  float: left;
	  width: 510px;
	  height: 490px;
	  margin: 0;
	  padding: 5px;
}

#next1 {
	margin-bottom: 5px;
}
.outlineImage {
    margin: 2px;
    border: 2px solid none;
}
.contentsFrame img.active {
    border: 2px solid blue;
}
.contentsFrame ul {
	margin: 2px;
}

.vcardRow {
	position: relative;
	border: 1px solid 	#A0A0A0;
	width: 478px;
}
.vcardCol1 {
	background-color: #D0D0D0;
	width: 160px;
	position: relative;
	line-height: 26px;
	text-align: center;
}
div.vcardCol2 {
	position: absolute;
	background-color: #E0E0E0;
	height: 100%;
	width: 320px;
	top: 0;
	right: 0;
	margin; 0;
	padding: 0;
}
input.vcardCol2  {
	border: 1px solid #999;
	position: absolute;
	width: 97%;
	height: 18px;
	top: 0;
	bottom: 0;
	left: 0;
	right: 0;
	margin: auto;
	padding: 0;
}

</style>
<script src="mootools-core-1.4.5.js" type="text/javascript"></script>
<script src="mootools-more-1.4.0.1.js" type="text/javascript"></script>
<script src="qrpicture.js" type="text/javascript"></script>
</head>
<body>
<table border="0" align="center">
	<tbody><tr>
		<td><img width="210" height="210" src="assets/qrSpiral.anim.col.210x210.gif"></td>
		<td width="420" valign="middle" nowrap="nowrap" align="center"><p><span style="font-size: xx-large; font-weight: bold; font-family: Arial, Helvetica, sans-serif;">www.QRpicture.com</span></p>
			<p><span style="font-size: large; font-family: Arial, Helvetica, sans-serif; font-style: italic;">Photo realistic QR codes</span></p></td>
		<td><img width="210" height="210" src="assets/qrAwesome.anim.col.210x210.gif"></td>
	</tr>
</tbody></table>

<form id="formId" action="#">
<div id="wrapper">
	<div class="tabsWindow">
        <ul class="tabs">
            <li class="active">1: Image</li>
            <li>2: Outline</li>
            <li>3: Clip</li>
            <li>4: Text</li>
            <li>4: Work</li>
        </ul>
    </div>
	<div class="contentsWindow">
        <div class="contentsSlider">
            <div class="contentsFrame">
				<p style="margin: 0"><b>Select your image</b></p>
				<ul>
					<li>minimal 186x186 pixels</li>
					<li>supported formats JPG/PNG/GIF</li>
				</ul>
				<input type="file" onchange="clip.onFileUpload(event)" name="files[]" id="qrlogo_files" size="50">
            </div>
            <div class="contentsFrame">
				<p style="margin: 0"><b>Select outline</b></p>
                <img id="qrOutline0" class="outlineImage" src="assets/outline0-97x97.png" align="absmiddle" onclick="clip.onSetOutline(event,0)" />
                <img id="qrOutline1" class="outlineImage" src="assets/outline1-97x97.png" align="absmiddle" onclick="clip.onSetOutline(event,1)" />
                <img id="qrOutline2" class="outlineImage" src="assets/outline2-97x97.png" align="absmiddle" onclick="clip.onSetOutline(event,2)" />
                <img id="qrOutline3" class="outlineImage" src="assets/outline3-97x97.png" align="absmiddle" onclick="clip.onSetOutline(event,3)" />
                <!--
                <img id="qrOutline4" class="outlineImage" src="assets/outline4-97x97.png" align="absmiddle" onclick="clip.onSetOutline(event,4)" />
                <img id="qrOutline5" class="outlineImage" src="assets/outline5-97x97.png" align="absmiddle" onclick="clip.onSetOutline(event,5)" />
                -->
            </div>
            <div class="contentsFrame">
				<p style="margin: 0"><b>Position and size clip rectangle</b>
				<button id="next1" onclick="return window.tabs1.nextSlide()" >Next</button></p>
				<div>
					<canvas id="qr_canvas" height="16"></canvas>
					<div id="draggables">
					  <canvas id="qr_drag" class="dragview"  style="cursor: move; height: 16px;"></canvas>
					  <div class="dragthumb" style="cursor: nw-resize"></div>
					  <div class="dragthumb" style="cursor: n-resize"></div>
					  <div class="dragthumb" style="cursor: ne-resize"></div>
					  <div class="dragthumb" style="cursor: w-resize"></div>
					  <div class="dragthumb" style="cursor: e-resize"></div>
					  <div class="dragthumb" style="cursor: sw-resize"></div>
					  <div class="dragthumb" style="cursor: s-resize"></div>
					  <div class="dragthumb" style="cursor: se-resize"></div>
					</div>
				</div>
            </div>
            <div class="contentsFrame">
				<p style="margin: 0"><b>Select type of message</b>
				<button id="next2" onclick="return window.tabs1.nextSlide()" >Next</button></p>

				<div class="textWindow">
					<ul class="tabs">
						<li class="active">Text</li>
						<li>VCard</li>
						<li>VEvent</li>
						<li>Geo</li>
					</ul>
				</div>
				<div class="textContent">
					<div class="textSlider">
						<div class="textFrame">
							<ul>
								<li>maximum 100 charachters</b></li>
								<li>shorter text gives better image quality</li>
							</ul>
							<input id="qrText" type="text" size="64" maxlength="100"/>
						</div>
						<div class="textFrame" style="overflow-y: scroll;">
                            NOTE: Not supported (yet)
							<ul>
								<li>This VCard is stored on this server</b></li>
								<li>The QR will have a http:// link to the VCard</li>
								<li>When scanning the QR you will need internet access to download the VCard</li>
							</ul>
							<div class="vcardRow"><div class="vcardCol1">Prefix</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="prefix"></div></div>
							<div class="vcardRow"><div class="vcardCol1">First name</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="firstname"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Last name</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="lastname"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Suffix</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="suffix"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Title</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="title"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Organization</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="organization"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Street address (home)</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="homeaddr"></div></div>
							<div class="vcardRow"><div class="vcardCol1">City (home)</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="homecity"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Region (home)</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="homeregion"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Zip code (home)</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="homezip"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Country (home)</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="homecountry"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Street address (office)</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="workaddr"></div></div>
							<div class="vcardRow"><div class="vcardCol1">City (office)</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="workcity"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Region (office)</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="workregion"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Zip code (office)</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="workzip"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Country (office)</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="workcountry"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Home phone</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="homephone"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Office phone</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="workphone"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Fax</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="lastname"></div></div>
							<div class="vcardRow"><div class="vcardCol1">Mobile/cell phone</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="workfax"></div></div>
							<div class="vcardRow"><div class="vcardCol1">E-mail</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="mobile"></div></div>
							<div class="vcardRow"><div class="vcardCol1">URL</div><div class="vcardCol2"><input type="text" class="vcardCol2" name="url"></div></div>

						<table>
						<tbody>
						<tr>
							<th class="c2">Preferred phone</th>
							<td><select name="pref">
								<option value="0" selected="selected">None</option>
								<option value="TEL;HOME">Home phone</option>
								<option value="TEL;WORK">Office phone</option>
								<option value="TEL;CELL">Mobile/cell phone</option>
							</select></td>
						</tr>
						</tbody></table>
						</div>
					</div>
				</div>

			</div>
            <div class="contentsFrame">
				<p style="margin: 0"><b>Choose what to create:</b></p>
				<ul>
					<li>!! Please be patient, the generator needs at least 20 seconds</li>
					<li>right-click to save</li>
				</ul>
				<p><input type="radio" name="optDither" value="0" />Black/White
				<br/><input type="radio" name="optDither" value="1" />Colour drawing (large areas of same colour)
				<br/><input type="radio" name="optDither" value="2" checked="checked" />Colour picture (a lot of colours/shades)</p>
				<button type="button" id="qrTextButton"  onclick="clip.onGenerate(event)" disabled="disabled">Generate</button>
				<div id="info"></div>
				<div id="result"></div>
				<div id="divReGenerate" style="display:none">
				<p>If QR does not scan, then <button type="button" id="qrReGenerateButton"  onclick="clip.onReGenerate(event)">Re-Generate</button> with more (<span id="regenlevel"></span>) safer settings</p>
				</div>
            </div>
        </div>
    </div>
</div>
</form>

<div id="qrHQMaskInfo"></div>
<div id="qrHQMaskInfo2"></div>

</body>
</html>
