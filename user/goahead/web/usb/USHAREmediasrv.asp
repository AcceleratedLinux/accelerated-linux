<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>Media Server Settings</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("usb");
var dir_count = 0;

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
	e = document.getElementById("ftpSrvPort");
	e.innerHTML = _("ftp server port");
	e = document.getElementById("ftpSrvMaxUsers");
	e.innerHTML = _("ftp server maxusers");
	e = document.getElementById("ftpSrvLoginTimeout");
	e.innerHTML = _("ftp server logintimeout");
	e = document.getElementById("ftpSrvStayTimeout");
	e.innerHTML = _("ftp server staytimeout");

	e = document.getElementById("ftpApply");
	e.value = _("usb apply");
	e = document.getElementById("ftpReset");
	e.value = _("usb reset");
}

function initValue()
{
	// initTranslation();

	var mediasrvebl = '<% getCfgZero(1, "mediaSrvEnabled"); %>';

	document.storage_media.media_enabled[1].checked = true;
	document.storage_media.media_name.value == '<% getCfgGeneral(1, "mediaSrvName"); %>';
	document.storage_media.media_dir_add.disabled = true;
	document.storage_media.media_dir_del.disabled = true;

	if (mediasrvebl == "1")
	{
		document.storage_media.media_enabled[0].checked = true;
		document.storage_media.media_dir_add.disabled = false;
		document.storage_media.media_dir_del.disabled = false;
	}
}

function CheckValue()
{
	if (document.storage_media.media_name.value == "")
	{
		alert('Please specify Media Server Name');
		document.storage_media.media_name.focus();
		document.storage_media.media_name.select();
		return false;
	}
	else if (document.storage_media.media_name.value.match(/[ `~!@#$%^&*\()+\|{}\[\]:;\"\'<,>.\/\\?]/))
	{
		alert('Don\'t enter /[ `~!@#$%^&*\()+\|{}\[\]:;\"\'<,>.\/\\?]/ in this feild');
		document.storage_media.media_name.focus();
		document.storage_media.media_name.select();
		return false;
	}

	return true;
}

function CheckSelect()
{
	if (dir_count <= 0)
	{
		alert("No any option can be choosed!");
		return false;
	}
	else if (dir_count == 1)
	{
		if (document.storage_media.media_dir.checked == false)	
		{
			alert("please select one option!");
			return false;
		}
	}
	else
	{
		for(i=0;i<dir_count;i++)
		{
			if (document.storage_media.media_dir[i].checked == true)
			{
				break;
			}
		}
		if (i == dir_count)
		{
			alert("please select one option!");
			return false;
		}
	}

	return true;
}

function open_media_diradd_window()
{
	window.open("USHAREaddmediadir.asp","Media_Dir_Add","toolbar=no, location=no, scrollbars=yes, resizable=no, width=640, height=640")
}

function media_enable_switch()
{
	if (document.storage_media.media_enabled[1].checked == true)
	{
		document.storage_media.media_dir_add.disabled = true;
		document.storage_media.media_dir_del.disabled = true;
	}
	else
	{
		document.storage_media.media_dir_add.disabled = false;
		document.storage_media.media_dir_del.disabled = false;
	}
}

function submit_apply(parm)
{
	if (parm == "delete")
	{
		if (CheckSelect())
		{
			document.storage_media.hiddenButton.value = parm;
			document.storage_media.submit();
		}
	}
	else if (parm == "apply")
	{
		if (CheckValue())
		{
			document.storage_media.hiddenButton.value = parm;
			document.storage_media.submit();
		}
	}
}
</script>
</head>

<body onLoad="initValue()">
<table class="body"><tr><td>


<h1 id="mediaSrvTitle">Media Server Settings </h1>
<p id="mediaSrvIntroduction"></p>
<hr />

<form method=post name=storage_media action="/goform/storageMediaSrv">
<input type=hidden name=hiddenButton value="">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="mediaSrvSet">Media Server Setup</td>
  </tr>
  <tr> 
    <td class="head" id="mediaSrv">Media Server</td>
    <td>
      <input type=radio name=media_enabled value="1" onClick="media_enable_switch()"><font id="mediaSrvEnable">Enable</font>&nbsp;
      <input type=radio name=media_enabled value="0" onClick="media_enable_switch()" checked><font id="mediaSrvDisable">Disable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="mediaSrvName">Media Server Name</td>
    <td>
      <input type=text name=media_name size=16 maxlength=16 value="RalinkSoC">
    </td>
  </tr>
</table>
<hr />
<br />
<center>
<table width="360" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr align="center">
    <td class="title" colspan="3" id="mediaSrvDir">Media Server Shared Directory</td>
  </tr>
  <tr align="center">
    <td bgcolor="#E8F8FF" width=20%>&nbsp;</td>
    <td align="center" bgcolor="#E8F8FF" width=80% id="mediaDirPath">Path</td>
  </tr>
  <% ShowMediaDir(); %>
<script language="JavaScript" type="text/javascript">
dir_count = parseInt('<% getCount(1, "AllMediaDir"); %>');
</script>
</table>
</center>
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=button style="{width:120px;}" value="Add" name="media_dir_add" id="mediaAdd" onClick="open_media_diradd_window()"> &nbsp; &nbsp;
      <input type=button style="{width:120px;}" value="Delete" name="media_dir_del" id="mediaDel" onClick="submit_apply('delete')">
    </td>
  </tr>
</table>
<hr />
<br />
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=button style="{width:120px;}" value="Apply" id="mediaApply" onClick="submit_apply('apply')"> &nbsp; &nbsp;
      <input type=button style="{width:120px;}" value="Cancel" id="mediaReset" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
</body>
</html>

