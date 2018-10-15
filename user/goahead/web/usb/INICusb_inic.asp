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

function initTranslation()
{
	var e = document.getElementById("usbiNICTitle");
	e.innerHTML = _("usbiNIC title");
	e = document.getElementById("usbiNICIntroduction");
	e.innerHTML = _("usbiNIC introduction");

	e = document.getElementById("usbiNICSettings");
	e.innerHTML = _("usbiNIC settings");
	e = document.getElementById("usbiNICCapability");
	e.innerHTML = _("usbiNIC capability");
	e = document.getElementById("usbiNICEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("usbiNICDisable");
	e.innerHTML = _("usb disable");

	e = document.getElementById("usbiNICApply");
	e.value = _("usb apply");
	e = document.getElementById("usbiNICCancel");
	e.value = _("usb cancel");
}

function initValue()
{
	initTranslation();

	var usbiNICebl = '<% getCfg2Zero(1, "InicUSBEnable"); %>';

	if (usbiNICebl == "1")
	{
		document.usbiNIC.inic_enable[0].checked = true;
	}
	else
	{
		document.usbiNIC.inic_enable[1].checked = true;
	}
}
</script>
</head>

<body onLoad="initValue()">
<table class="body"><tr><td>


<h1 id="usbiNICTitle">USB iNIC Settings </h1>
<p id="usbiNICIntroduction"></p>
<hr />

<form method=post name=usbiNIC action="/goform/USBiNIC">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="usbiNICSettings">USB iNIC Setup</td>
  </tr>
  <tr> 
    <td class="head" id="usbiNICCapability">Capability</td>
    <td>
      <input type="radio" name="inic_enable" value="1"><font id="usbiNICEnable">Enable</font>
      <input type="radio" name="inic_enable" value="0"><font id="usbiNICDisable">Disable</font>
    </td>
  </tr>
</table>
<hr />
<br />
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply" id="usbiNICApply"> &nbsp; &nbsp;
      <input type=button style="{width:120px;}" value="Cancel" id="usbiNICCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>
</body>
</html>

