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
	var e = document.getElementById("printerTitle");
	e.innerHTML = _("printer title");
	e = document.getElementById("printerIntroduction");
	e.innerHTML = _("printer introduction");

	e = document.getElementById("printerSettings");
	e.innerHTML = _("printer settings");
	e = document.getElementById("printerCapability");
	e.innerHTML = _("printer capability");
	e = document.getElementById("printerEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("printerDisable");
	e.innerHTML = _("usb disable");

	e = document.getElementById("printerApply");
	e.value = _("usb apply");
	e = document.getElementById("printerCancel");
	e.value = _("usb cancel");
}

function initValue()
{
	initTranslation();
	var printersrvebl = '<% getCfgZero(1, "PrinterSrvEnabled"); %>';

	if (printersrvebl == "1")
	{
		document.printer.enabled[0].checked = true;
	}
	else
	{
		document.printer.enabled[1].checked = true;
	}
}
</script>
</head>

<body onLoad="initValue()">
<table class="body"><tr><td>


<h1 id="printerTitle">Printer Server Settings </h1>
<p id="printerIntroduction"></p>
<hr />

<form method=post name=printer action="/goform/printersrv">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="printerSettings">Printer Server Setup</td>
  </tr>
  <tr> 
    <td class="head" id="printerCapability">Capability</td>
    <td>
      <input type="radio" name="enabled" value="1"><font id="printerEnable">Enable</font>
      <input type="radio" name="enabled" value="0"><font id="printerDisable">Disable</font>
    </td>
  </tr>
</table>
<hr />
<br />
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply" id="printerApply"> &nbsp; &nbsp;
      <input type=button style="{width:120px;}" value="Cancel" id="printerCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>
</body>
</html>

