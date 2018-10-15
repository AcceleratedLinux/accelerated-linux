<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>Web Camera Settings</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("usb");
var dir_count = 0;

function initTranslation()
{
	var e = document.getElementById("itunesTitle");
	e.innerHTML = _("itunes title");
	e = document.getElementById("itunesIntroduction");
	e.innerHTML = _("itunes introduction");

	e = document.getElementById("itunesSettings");
	e.innerHTML = _("itunes settings");
	e = document.getElementById("itunesCapability");
	e.innerHTML = _("itunes capability");
	e = document.getElementById("itunesEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("itunesDisable");
	e.innerHTML = _("usb disable");
	e = document.getElementById("itunesSrvName");
	e.innerHTML = _("itunes srv_name");
	e = document.getElementById("itunesShowDisk");
	e.innerHTML = _("itunes showDisk");
	e = document.getElementById("itunesDirPath");
	e.innerHTML = _("storage disk dirpath");
	e = document.getElementById("itunesDirPart");
	e.innerHTML = _("storage disk dirpart");

	e = document.getElementById("itunesApply");
	e.value = _("usb apply");
	e = document.getElementById("itunesCancel");
	e.value = _("usb cancel");
}

function initValue()
{
	initTranslation();
	var iTunesSrvebl = '<% getCfgZero(1, "iTunesEnable"); %>';
	var iTunesSrvName = '<% getCfgGeneral(1, "iTunesSrvName"); %>';

	if (iTunesSrvebl == "1")
	{
		document.itunes_form.enabled[0].checked = true;
		var iTunesDir = '<% getCfgZero(1, "iTunesDir"); %>';
		for (var i=0;i<dir_count;i++)
		{
			if (iTunesDir == document.itunes_form.dir_path[i].value)
			{
				document.itunes_form.dir_path[i].checked = true;
				break;
			}
		}
	}
	else
	{
		document.itunes_form.enabled[1].checked = true;
	}
	if (iTunesSrvName != "")
		document.itunes_form.srv_name.value = iTunesSrvName;
}

function checkData()
{
	// alert("dir_count: "+dir_count);
	if (document.itunes_form.srv_name.value == "")
	{
		alert("please provide iTunes Server Name!");
		return false;
	}
	if (dir_count <= 0)
	{
		alert("No any option can be choosed!");
		return false;
	}
	else if (dir_count == 1)
	{
		if (document.itunes_form.dir_path.checked == false)	
		{
			alert("please select one option!");
			return false;
		}
		// document.itunes_form.selectDirIndex.value = 0;
	}
	else
	{
		for(i=0;i<dir_count;i++)
		{
			if (document.itunes_form.dir_path[i].checked == true)
				break;
		}
		if (i == dir_count)
		{
			alert("please select one option!");
			return false;
		}
	}

	return true;
}
</script>
</head>

<body onLoad="initValue()">
<table class="body"><tr><td>


<h1 id="itunesTitle">iTunes Server Settings </h1>
<p id="itunesIntroduction"></p>
<hr />

<form method=post name=itunes_form action="/goform/iTunesSrv" onSubmit="return checkData()">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="itunesSettings">iTunes Server Setup</td>
  </tr>
  <tr> 
    <td class="head" id="itunesCapability">Capability</td>
    <td>
      <input type="radio" name="enabled" value="1"><font id="itunesEnable">Enable</font>
      <input type="radio" name="enabled" value="0"><font id="itunesDisable">Disable</font>
    </td>
  </tr>
  <tr> 
    <td class="head" id="itunesSrvName">Server Name</td>
    <td>
      <input type="text" name="srv_name" value="Ralink" size=20 maxlength=32>
    </td>
  </tr>
</table>
<br />
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="4"><font id="itunesShowDisk">Media Library Selection</font></td>
  </tr>
  <tr align=center> 
    <td bgcolor="#E8F8FF" width=15px>&nbsp;</td>
    <td bgcolor="#E8F8FF" id="itunesDirPath">Directory Path</td>
    <td bgcolor="#E8F8FF" id="itunesDirPart">Partition</td>
  </tr>
  <% ShowAllDir(); %>
<script language="JavaScript" type="text/javascript">
dir_count = parseInt('<% getCount(1, "AllDir"); %>');
</script>
</table>
<hr />
<br />
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply" id="itunesApply"> &nbsp; &nbsp;
      <input type=button style="{width:120px;}" value="Cancel" id="itunesCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>
</body>
</html>

