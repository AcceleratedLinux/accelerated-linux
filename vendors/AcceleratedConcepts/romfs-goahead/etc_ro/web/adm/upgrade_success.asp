<html>
<head>
<title>Upload Firmware Success</title>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("admin");

function initTranslation()
{
	var e = document.getElementById("uploadSuccessTitle");
	e.innerHTML = _("upload success title");

	e = document.getElementById("uploadSuccessIntroduction");
	e.innerHTML = _("upload success introduction");

	e = document.getElementById("uploadSuccessFW");
	e.innerHTML = _("upload firmware details");

	e = document.getElementById("statusSysUpTime");
	e.innerHTML = _("status system up time");
	
	e = document.getElementById("currentSoftwareVersion");
	e.innerHTML = _("current Software Version");
}

function PageInit(){
	initTranslation();
}

</script>
</head>
<body onLoad="PageInit()" bgcolor="#FFFFFF">
<div align="center">
 <center>
<table class="body"><tbody><tr><td>

<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="uploadSuccessTitle">Firmware Upgrade Success</td>
</tr>
<tr>

<tr>
<td colspan="2">
<p class="head" id="uploadSuccessIntroduction">The firmware upgrade was
successful.  They new firmware version should be displayed below.  You may
now continue to configure your device.</p>

</td>
<tr>
</table>
<br>

<!-- ----------------- Upload firmware Settings ----------------- -->

<table border="1" cellpadding="2" cellspacing="1" width="540">
<tbody><tr>
  <td class="title" colspan="2" id="uploadSuccessDetails">Firmware Details</td>
</tr>
<tr>
  <td class="head" id="currentSoftwareVersion">Software Version:</td>
  <td>&nbsp;&nbsp;<% getAWBVersion(); %> (<% getSysBuildTime(); %>)</td>
</tr>
<tr>
  <td class="head" id="statusSysUpTime">Time since boot:</td>
  <td>&nbsp;&nbsp;<% getSysUptime(); %></td>
</tr>
</tbody></table>

</td></tr></tbody></table>

 </center>
</div>
</body></html>
