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
	var e = document.getElementById("webcamTitle");
	e.innerHTML = _("webcam title");
	e = document.getElementById("webcamIntroduction");
	e.innerHTML = _("webcam introduction");

	e = document.getElementById("webcamSettings");
	e.innerHTML = _("webcam settings");
	e = document.getElementById("webcamCapability");
	e.innerHTML = _("webcam capability");
	e = document.getElementById("webcamEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("webcamDisable");
	e.innerHTML = _("usb disable");
	e = document.getElementById("webcamResolution");
	e.innerHTML = _("webcam resolution");
	e = document.getElementById("webcamFPS");
	e.innerHTML = _("webcam fps");
	e = document.getElementById("webcamPort");
	e.innerHTML = _("webcam port");

	e = document.getElementById("webcamApply");
	e.value = _("usb apply");
	e = document.getElementById("webcamCancel");
	e.value = _("usb cancel");
}

function initValue()
{
	// initTranslation();

	var webcamebl = '<% getCfgZero(1, "WebCamEnabled"); %>';

	document.webcam.enabled[1].checked = true;
	document.webcam.resolution.disabled = true;
	document.webcam.fps.disabled = true;
	document.webcam.port.disabled = true;

	if (webcamebl == "1")
	{
		var resolution = '<% getCfgGeneral(1, "WebCamResolution"); %>';
		var fps = '<% getCfgGeneral(1, "WebCamFPS"); %>';
		var port = '<% getCfgGeneral(1, "WebCamPort"); %>';

		document.webcam.resolution.disabled = false;
		document.webcam.fps.disabled = false;
		document.webcam.port.disabled = false;

		document.webcam.enabled[0].checked = true;
		switch(resolution)
		{
			case "160x120":
				document.webcam.resolution.options.selectedIndex = 0;
				break;
			case "320x240":
				document.webcam.resolution.options.selectedIndex = 1;
				break;
			case "640x480":
				document.webcam.resolution.options.selectedIndex = 2;
				break;
		}
		switch(fps)
		{
			case "5":
				document.webcam.fps.options.selectedIndex = 0;
				break;
			case "10":
				document.webcam.fps.options.selectedIndex = 1;
				break;
			case "15":
				document.webcam.fps.options.selectedIndex = 2;
				break;
			case "20":
				document.webcam.fps.options.selectedIndex = 3;
				break;
			case "25":
				document.webcam.fps.options.selectedIndex = 4;
				break;
			case "30":
				document.webcam.fps.options.selectedIndex = 5;
				break;
		}
		document.webcam.port.value = port;
	}
}

function CheckValue()
{
	if (document.webcam.port.value == "")
	{
		alert('Please specify Web Camera Port');
		document.webcam.port.focus();
		document.webcam.port.select();
		return false;
	}
	else if (isNaN(document.webcam.port.value) ||
		 parseInt(document.webcam.port.value, 10) > 65535)
	{
		alert('Please specify valid number');
		document.webcam.port.focus();
		document.webcam.port.select();
		return false;
	}

	return true;
}

function webcam_enable_switch()
{
	if (document.webcam.enabled[1].checked == true)
	{
		document.webcam.resolution.disabled = true;
		document.webcam.fps.disabled = true;
		document.webcam.port.disabled = true;
	}
	else
	{
		document.webcam.resolution.disabled = false;
		document.webcam.fps.disabled = false;
		document.webcam.port.disabled = false;
	}
}

function submit_apply(parm)
{
	if (CheckValue())
		document.webcam.submit();
}
</script>
</head>

<body onLoad="initValue()">
<table class="body"><tr><td>


<h1 id="webcamTitle">Web Camera Settings </h1>
<p id="webcamIntroduction"></p>
<hr />

<form method=post name=webcam action="/goform/webcamra">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="webcamSettings">Web Camera Setup</td>
  </tr>
  <tr> 
    <td class="head" id="webcamCapability">Capability</td>
    <td>
      <input type="radio" name="enabled" value="1" id="webcamEnable" onclick="webcam_enable_switch()">Enable
      <input type="radio" name="enabled" value="0" id="webcamDisable" onClick="webcam_enable_switch()">Disable
    </td>
  </tr>
  <tr> 
    <td class="head" id="webcamResolution">Resolution</td>
    <td>
      <select name="resolution">
        <option value="160x120">160x120
        <option value="320x240">320x240
        <option value="640x480" selected>640x480
      </select>
    </td>
  </tr>
  <tr>
    <td class="head" id="webcamFPS">Frames Per Second</td>
    <td>
      <select name="fps">
        <option value="5">5
        <option value="10">10
        <option value="15">15
        <option value="20">20
        <option value="25" selected>25
        <option value="30">30
      </select>
    </td>
  </tr>
  <tr>
    <td class="head" id="webcamPort">Port</td>
    <td>
      <input type="text" name="port" size="5" maxlength="5" value="8080">
    </td>
  </tr>
</table>
<hr />
<br />
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=button style="{width:120px;}" value="Apply" id="webcamApply" onClick="submit_apply('apply')"> &nbsp; &nbsp;
      <input type=button style="{width:120px;}" value="Cancel" id="webcamCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>
</body>
</html>

