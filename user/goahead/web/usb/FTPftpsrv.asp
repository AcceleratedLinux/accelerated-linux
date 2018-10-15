<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>FTP Settings</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("usb");

var ftpenabled = '<% getCfgZero(1, "FtpEnabled"); %>';
var anonymous = '<% getCfgZero(1, "FtpAnonymous"); %>';
var ftpname = '<% getCfgGeneral(1, "FtpName"); %>';
var port = '<% getCfgGeneral(1, "FtpPort"); %>';
var maxsessions = '<% getCfgGeneral(1, "FtpMaxSessions"); %>';
var adddir = '<% getCfgZero(1, "FtpAddDir"); %>';
var rename = '<% getCfgZero(1, "FtpRename"); %>';
var remove = '<% getCfgZero(1, "FtpRemove"); %>';
var read = '<% getCfgZero(1, "FtpRead"); %>';
var write = '<% getCfgZero(1, "FtpWrite"); %>';
var download = '<% getCfgZero(1, "FtpDownload"); %>';
var upload = '<% getCfgZero(1, "FtpUpload"); %>';

function initTranslation()
{
	var e = document.getElementById("ftpTitle");
	e.innerHTML = _("ftp title");
	e = document.getElementById("ftpIntroduction");
	e.innerHTML = _("ftp introduction");

	e = document.getElementById("ftpSrvSet");
	e.innerHTML = _("ftp server setup");
	e = document.getElementById("ftpSrv");
	e.innerHTML = _("ftp server enable");
	e = document.getElementById("ftpSrvEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("ftpSrvDisable");
	e.innerHTML = _("usb disable");
	e = document.getElementById("ftpSrvAnonymous");
	e.innerHTML = _("ftp server anonymous");
	e = document.getElementById("ftpSrvAnonymousEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("ftpSrvAnonymousDisable");
	e.innerHTML = _("usb disable");
	e = document.getElementById("ftpSrvName");
	e.innerHTML = _("ftp server name");
	e = document.getElementById("ftpSrvPort");
	e.innerHTML = _("ftp server port");
	e = document.getElementById("ftpSrvMaxSessions");
	e.innerHTML = _("ftp server maxsessions");
	e = document.getElementById("ftpAddDir");
	e.innerHTML = _("ftp server adddir");
	e = document.getElementById("ftpAddDirEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("ftpAddDirDisable");
	e.innerHTML = _("usb disable");
	e = document.getElementById("ftpRename");
	e.innerHTML = _("ftp server rename");
	e = document.getElementById("ftpRenameEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("ftpRenameDisable");
	e.innerHTML = _("usb disable");
	e = document.getElementById("ftpRemove");
	e.innerHTML = _("ftp server remove");
	e = document.getElementById("ftpRemoveEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("ftpRemoveDisable");
	e.innerHTML = _("usb disable");
	e = document.getElementById("ftpRead");
	e.innerHTML = _("ftp server read");
	e = document.getElementById("ftpReadEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("ftpReadDisable");
	e.innerHTML = _("usb disable");
	e = document.getElementById("ftpWrite");
	e.innerHTML = _("ftp server write");
	e = document.getElementById("ftpWriteEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("ftpWriteDisable");
	e.innerHTML = _("usb disable");
	e = document.getElementById("ftpDownload");
	e.innerHTML = _("ftp server download");
	e = document.getElementById("ftpDownloadEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("ftpDownloadDisable");
	e.innerHTML = _("usb disable");
	e = document.getElementById("ftpUpload");
	e.innerHTML = _("ftp server upload");
	e = document.getElementById("ftpUploadEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("ftpUploadDisable");
	e.innerHTML = _("usb disable");

	e = document.getElementById("ftpApply");
	e.value = _("usb apply");
	e = document.getElementById("ftpReset");
	e.value = _("usb reset");
}

function initValue()
{
	initTranslation();

	/*
	alert(ftpenabled);
	alert(anonymous);
	alert(port);
	alert(maxsessions);
	*/

	document.storage_ftp.ftp_anonymous[0].disabled = true;
	document.storage_ftp.ftp_anonymous[1].disabled = true;
	document.storage_ftp.ftp_name.disabled = true;
	document.storage_ftp.ftp_port.disabled = true;
	document.storage_ftp.ftp_max_sessions.disabled = true;
	document.storage_ftp.ftp_adddir[0].disabled = true;
	document.storage_ftp.ftp_adddir[1].disabled = true;
	document.storage_ftp.ftp_rename[0].disabled = true;
	document.storage_ftp.ftp_rename[1].disabled = true;
	document.storage_ftp.ftp_remove[0].disabled = true;
	document.storage_ftp.ftp_remove[1].disabled = true;
	document.storage_ftp.ftp_read[0].disabled = true;
	document.storage_ftp.ftp_read[1].disabled = true;
	document.storage_ftp.ftp_write[0].disabled = true;
	document.storage_ftp.ftp_write[1].disabled = true;
	document.storage_ftp.ftp_download[0].disabled = true;
	document.storage_ftp.ftp_download[1].disabled = true;
	document.storage_ftp.ftp_upload[0].disabled = true;
	document.storage_ftp.ftp_upload[1].disabled = true;

	if (ftpenabled == "1")
	{
		document.storage_ftp.ftp_enabled[0].checked = true;
		document.storage_ftp.ftp_anonymous[0].disabled = false;
		document.storage_ftp.ftp_anonymous[1].disabled = false;
		if (anonymous == 1)
			document.storage_ftp.ftp_anonymous[0].checked = true;
		else
			document.storage_ftp.ftp_anonymous[1].checked = true;
		document.storage_ftp.ftp_name.disabled = false;
		document.storage_ftp.ftp_name.value = ftpname;

		document.storage_ftp.ftp_port.disabled = false;
		document.storage_ftp.ftp_port.value = port;

		document.storage_ftp.ftp_max_sessions.disabled = false;
		document.storage_ftp.ftp_max_sessions.value = maxsessions;

		document.storage_ftp.ftp_adddir[0].disabled = false;
		document.storage_ftp.ftp_adddir[1].disabled = false;
		if (adddir == 1)
			document.storage_ftp.ftp_adddir[0].checked = true;
		else
			document.storage_ftp.ftp_adddir[1].checked = true;

		document.storage_ftp.ftp_rename[0].disabled = false;
		document.storage_ftp.ftp_rename[1].disabled = false;
		if (rename == 1)
			document.storage_ftp.ftp_rename[0].checked = true;
		else
			document.storage_ftp.ftp_rename[1].checked = true;
		
		document.storage_ftp.ftp_remove[0].disabled = false;
		document.storage_ftp.ftp_remove[1].disabled = false;
		if (remove == 1)
			document.storage_ftp.ftp_remove[0].checked = true;
		else
			document.storage_ftp.ftp_remove[1].checked = true;

		document.storage_ftp.ftp_read[0].disabled = false;
		document.storage_ftp.ftp_read[1].disabled = false;
		if (read == 1)
			document.storage_ftp.ftp_read[0].checked = true;
		else
			document.storage_ftp.ftp_read[1].checked = true;

		document.storage_ftp.ftp_write[0].disabled = false;
		document.storage_ftp.ftp_write[1].disabled = false;
		if (write == 1)
			document.storage_ftp.ftp_write[0].checked = true;
		else
			document.storage_ftp.ftp_write[1].checked = true;

		document.storage_ftp.ftp_download[0].disabled = false;
		document.storage_ftp.ftp_download[1].disabled = false;
		if (download == 1)
			document.storage_ftp.ftp_download[0].checked = true;
		else
			document.storage_ftp.ftp_download[1].checked = true;

		document.storage_ftp.ftp_upload[0].disabled = false;
		document.storage_ftp.ftp_upload[1].disabled = false;
		if (upload == 1)
			document.storage_ftp.ftp_upload[0].checked = true;
		else
			document.storage_ftp.ftp_upload[1].checked = true;
	}
	else
	{
		// alert("FTP D");
		document.storage_ftp.ftp_enabled[1].checked = true;
	}
}

function CheckValue()
{
	if (document.storage_ftp.ftp_enabled[0].checked == true)
	{
		if (document.storage_ftp.ftp_name.value == "")
		{
			alert('Please specify FTP Server Name');
			document.storage_ftp.ftp_name.focus();
			document.storage_ftp.ftp_name.select();
			return false;
		}
		if (document.storage_ftp.ftp_port.value == "")
		{
			alert('Please specify FTP Port');
			document.storage_ftp.ftp_port.focus();
			document.storage_ftp.ftp_port.select();
			return false;
		}
		else if (isNaN(document.storage_ftp.ftp_port.value) ||
			 parseInt(document.storage_ftp.ftp_port.value,10) > 65535)
		{
			alert('Please specify valid number');
			document.storage_ftp.ftp_port.focus();
			document.storage_ftp.ftp_port.select();
			return false;
		}

		if (document.storage_ftp.ftp_max_sessions.value == "")
		{
			alert('Please specify FTP Max. Users');
			document.storage_ftp.ftp_max_sessions.focus();
			document.storage_ftp.ftp_max_sessions.select();
			return false;
		}
		else if (isNaN(document.storage_ftp.ftp_max_sessions.value))
		{
			alert('Please specify valid number');
			document.storage_ftp.ftp_max_sessions.focus();
			document.storage_ftp.ftp_max_sessions.select();
			return false;
		}
	}

	return true;
}

function ftp_enable_switch()
{
	if (document.storage_ftp.ftp_enabled[1].checked == true)
	{
		document.storage_ftp.ftp_anonymous[0].disabled = true;
		document.storage_ftp.ftp_anonymous[1].disabled = true;
		document.storage_ftp.ftp_name.disabled = true;
		document.storage_ftp.ftp_port.disabled = true;
		document.storage_ftp.ftp_max_sessions.disabled = true;
		document.storage_ftp.ftp_adddir[0].disabled = true;
		document.storage_ftp.ftp_adddir[1].disabled = true;
		document.storage_ftp.ftp_rename[0].disabled = true;
		document.storage_ftp.ftp_rename[1].disabled = true;
		document.storage_ftp.ftp_remove[0].disabled = true;
		document.storage_ftp.ftp_remove[1].disabled = true;
		document.storage_ftp.ftp_read[0].disabled = true;
		document.storage_ftp.ftp_read[1].disabled = true;
		document.storage_ftp.ftp_write[0].disabled = true;
		document.storage_ftp.ftp_write[1].disabled = true;
		document.storage_ftp.ftp_download[0].disabled = true;
		document.storage_ftp.ftp_download[1].disabled = true;
		document.storage_ftp.ftp_upload[0].disabled = true;
		document.storage_ftp.ftp_upload[1].disabled = true;
	}
	else
	{
		document.storage_ftp.ftp_anonymous[0].disabled = false;
		document.storage_ftp.ftp_anonymous[1].disabled = false;
		document.storage_ftp.ftp_name.disabled = false;
		document.storage_ftp.ftp_port.disabled = false;
		document.storage_ftp.ftp_max_sessions.disabled = false;
		document.storage_ftp.ftp_adddir[0].disabled = false;
		document.storage_ftp.ftp_adddir[1].disabled = false;
		document.storage_ftp.ftp_rename[0].disabled = false;
		document.storage_ftp.ftp_rename[1].disabled = false;
		document.storage_ftp.ftp_remove[0].disabled = false;
		document.storage_ftp.ftp_remove[1].disabled = false;
		document.storage_ftp.ftp_read[0].disabled = false;
		document.storage_ftp.ftp_read[1].disabled = false;
		document.storage_ftp.ftp_write[0].disabled = false;
		document.storage_ftp.ftp_write[1].disabled = false;
		document.storage_ftp.ftp_download[0].disabled = false;
		document.storage_ftp.ftp_download[1].disabled = false;
		document.storage_ftp.ftp_upload[0].disabled = false;
		document.storage_ftp.ftp_upload[1].disabled = false;
	}
}
</script>
</head>

<body onLoad="initValue()">
<table class="body"><tr><td>


<h1 id="ftpTitle">FTP Settings </h1>
<p id="ftpIntroduction"></p>
<hr />

<form method=post name=storage_ftp action="/goform/storageFtpSrv" onSubmit="return CheckValue()">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="ftpSrvSet">FTP Server Setup</td>
  </tr>
  <tr> 
    <td class="head" id="ftpSrv">FTP Server</td>
    <td>
      <input type=radio name=ftp_enabled value="1" onClick="ftp_enable_switch()"><font id="ftpSrvEnable">Enable</font>&nbsp;
      <input type=radio name=ftp_enabled value="0" onClick="ftp_enable_switch()" checked><font id="ftpSrvDisable">Disable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="ftpSrvName">FTP Server Name</td>
    <td>
      <input type=text name=ftp_name size=16 maxlength=16 value="RalinkFTP">
    </td>
  </tr>
  <tr> 
    <td class="head" id="ftpSrvAnonymous">Anonymous Login</td>
    <td>
      <input type=radio name=ftp_anonymous value="1"><font id="ftpSrvAnonymousEnable">Enable</font>&nbsp;
      <input type=radio name=ftp_anonymous value="0" checked><font id="ftpSrvAnonymousDisable">Disable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="ftpSrvPort">FTP Port</td>
    <td>
      <input type=text name=ftp_port size=5 maxlength=5 value="21">
    </td>
  </tr>
  <tr>
    <td class="head" id="ftpSrvMaxSessions">Max. Sessions</td>
    <td>
      <input type=text name=ftp_max_sessions size=2 maxlength=2 value="10">
    </td>
  </tr>
  <tr>
    <td class="head" id="ftpAddDir">Create Directory</td>
    <td>
      <input type=radio name=ftp_adddir value="1" checked><font id="ftpAddDirEnable">Enable</font>&nbsp;
      <input type=radio name=ftp_adddir value="0"><font id="ftpAddDirDisable">Disable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="ftpRename">Rename File/Directory</td>
    <td>
      <input type=radio name=ftp_rename value="1" checked><font id="ftpRenameEnable">Enable</font>&nbsp;
      <input type=radio name=ftp_rename value="0"><font id="ftpRenameDisable">Disable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="ftpRemove">Remove File/Directory</td>
    <td>
      <input type=radio name=ftp_remove value="1" checked><font id="ftpRemoveEnable">Enable</font>&nbsp;
      <input type=radio name=ftp_remove value="0"><font id="ftpRemoveDisable">Disable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="ftpRead">Read File</td>
    <td>
      <input type=radio name=ftp_read value="1" checked><font id="ftpReadEnable">Enable</font>&nbsp;
      <input type=radio name=ftp_read value="0"><font id="ftpReadDisable">Disable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="ftpWrite">Write File</td>
    <td>
      <input type=radio name=ftp_write value="1" checked><font id="ftpWriteEnable">Enable</font>&nbsp;
      <input type=radio name=ftp_write value="0"><font id="ftpWriteDisable">Disable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="ftpDownload">Download Capability</td>
    <td>
      <input type=radio name=ftp_download value="1" checked><font id="ftpDownloadEnable">Enable</font>&nbsp;
      <input type=radio name=ftp_download value="0"><font id="ftpDownloadDisable">Disable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="ftpUpload">Upload Capability</td>
    <td>
      <input type=radio name=ftp_upload value="1" checked><font id="ftpUploadEnable">Enable</font>&nbsp;
      <input type=radio name=ftp_upload value="0"><font id="ftpUploadDisable">Disable</font>
    </td>
  </tr>
</table>
<hr />
<br />
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply" id="ftpApply"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Reset" id="ftpReset" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
</body>
</html>

