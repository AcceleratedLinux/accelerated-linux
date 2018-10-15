<html><head><title>Upload Firmware</title>

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<style type="text/css">
<!--
#loading {
       width: 680px;
       height: 527px;
       background-color: #DBE2EC;
       position: absolute;
       left: 100%;
       top: 100%;
       margin-top: -527px;
       margin-left: -680px;
       text-align: center;
}
-->
</style>

<script language="JavaScript" type="text/javascript">
document.write('<div id="loading" style="display: none;"><br><br><br>Upgrading firmware <br><br> Please be patient, thanks...</div>');
Butterlate.setTextDomain("admin");

var storageb = '<% getStorageBuilt(); %>';
var isStorageBuilt = <% getStorageBuilt(); %>;

function style_display_on()
{
	if (window.ActiveXObject)
	{ // IE
		return "block";
	}
	else if (window.XMLHttpRequest)
	{ // Mozilla, Safari,...
		return "table-row";
	}
}

var _singleton = 0;
function uploadFirmwareCheck()
{
	if(_singleton)
		return false;
	if(document.UploadFirmware.filename.value == ""){
		alert("Firmware Upgrade: Please specify a file.");
		return false;
	}

	//document.UploadFirmware.UploadFirmwareSubmit.disabled = true;
	//document.UploadFirmware.filename.disabled = true;

    document.getElementById("loading").style.display="block";

	_singleton = 1;
	return true;
}

function initTranslation()
{
	var e = document.getElementById("uploadTitle");
	e.innerHTML = _("upload title");
	e = document.getElementById("uploadIntroduction2");
	e.innerHTML = _("upload introduction2");

	e = document.getElementById("uploadFW");
	e.innerHTML = _("upload firmware");
	e = document.getElementById("uploadFWLocation");
	e.innerHTML = _("upload firmware location");
	e = document.getElementById("uploadFWParams");
	e.innerHTML = _("upload firmware params");
	e = document.getElementById("uploadFWApply");
	e.value = _("admin apply");
	
	e = document.getElementById("currentSoftwareVersion");
	e.innerHTML = _("current Software Version");
}

function pageInit(){
	initTranslation();

    document.UploadFirmware.UploadFirmwareSubmit.disabled = false;

//	document.UploadFirmware.filename.disabled = false;
	document.getElementById("loading").style.display="none";
}

</script></head><body onLoad="pageInit()" bgcolor="#FFFFFF">
<div align="center">
 <center>
<table class="body"><tbody><tr><td>

<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="uploadTitle">Firmware Upgrade</td>
</tr>
<tr>

<tr>
<td colspan="2">
<p class="head" id="uploadIntroduction2">It takes around 3 minutes to upgrade, so please be patient. &nbsp;The NetReach will not boot if the firmware is corrupted!</p>

</td>
<tr>
</table>
<br>

<!-- ----------------- Upload firmware Settings ----------------- -->
<form method="post" name="UploadFirmware" action="/cgi-bin/upload.cgi" enctype="multipart/form-data">
<table border="1" cellpadding="2" cellspacing="1" width="540">
<tbody><tr>
  <td class="title" colspan="2" id="uploadFW">Firmware Upgrade</td>
</tr>
<tr>
  <td class="head" id="currentSoftwareVersion">Software Version:</td>
  <td >&nbsp;&nbsp;<% getAWBVersion(); %></td>
</tr>
<tr>
  <td class="head" id="uploadFWLocation">Location:</td>
	<td> <input name="data" size="20" maxlength="256" type="file"> </td>
</tr>
<tr>
  <td class="head" id="uploadFWParams">Optional Parameters:</td>
	<td> <input name="params" size="20" maxlength="256" type="text"> </td>
</tr>
</tbody></table>
<input value="Apply" id="uploadFWApply" name="UploadFirmwareSubmit" type="submit" onClick="return uploadFirmwareCheck();">
</form>

</td></tr></tbody></table>

 </center>
</div>
</body></html>
