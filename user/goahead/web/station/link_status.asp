<!-- Copyright (c), Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="refresh" CONTENT="3; URL=./link_status.asp">
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Station Link Status</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

function initTranslation()
{
	var e = document.getElementById("linkTitle");
	e.innerHTML = _("link title");
	e = document.getElementById("linkIntroduction");
	e.innerHTML = _("link introduction");

	e = document.getElementById("linkLinkStatus");
	e.innerHTML = _("link link status");
	e = document.getElementById("linkStatus");
	e.innerHTML = _("link status");
	e = document.getElementById("linkExtraInfo");
	e.innerHTML = _("link extra info");
	e = document.getElementById("linkChannel");
	e.innerHTML = _("station channel");
	e = document.getElementById("linkSpeed");
	e.innerHTML = _("link speed");
	e = document.getElementById("linkThroughput");
	e.innerHTML = _("link throughput");
	e = document.getElementById("linkQuality");
	e.innerHTML = _("link quality");
	e = document.getElementById("linkSigStrength1");
	e.innerHTML = _("link signal strength");
	e = document.getElementById("linkSigStrength2");
	e.innerHTML = _("link signal strength");
	e = document.getElementById("linkSigStrength3");
	e.innerHTML = _("link signal strength");
	e = document.getElementById("linkNoiseLevel");
	e.innerHTML = _("link noise level");

	e = document.getElementById("linkHTTitle");
	e.innerHTML = _("link ht");
	e = document.getElementById("linkSNR0");
	e.innerHTML = _("link snr");
	e = document.getElementById("linkSNR1");
	e.innerHTML = _("link snr");
}

function PageInit()
{
	initTranslation();
	var n_mode = "<% getLinkingMode(); %>";

	if (n_mode == "0") {
		document.getElementById("linkHT").style.visibility = "hidden";
		document.getElementById("linkHT").style.display = "none";
	} else {
		document.getElementById("linkHT").style.visibility = "visible";
		if (window.ActiveXObject) { // IE
			document.getElementById("linkHT").style.display = "block";
		}
		else if (window.XMLHttpRequest) { // Mozilla, Safari,...
			document.getElementById("linkHT").style.display = "table";
		}
	}
}
</script>
</head>

<body onload="PageInit()">
<table class="body"><tr><td>

<h1 id="linkTitle">Station Link Status</h1>
<p id="linkIntroduction">The Status page shows the settings and current operation status of the Station.</p>
<hr />

<form method=post name="sta_link_status" action="/goform/setStaDbm">
<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td colspan="3" class="title" id="linkLinkStatus">Link Status</td>
  </tr>
  <tr>
    <td class="head" id="linkStatus">Status</td>
    <td colspan="2"><% getStaLinkStatus(); %></td>
  </tr>
  <tr>
    <td class="head" id="linkExtraInfo">Extra Info</td>
    <td colspan="2"><% getStaExtraInfo(); %></td>
  </tr>
  <tr>
    <td class="head" id="linkChannel">Channel</td>
    <td colspan="2"><% getStaLinkChannel(); %></td>
  </tr>
  <tr>
    <td class="head" id="linkSpeed">Link Speed</td>
    <td>Tx(Mbps)&nbsp;&nbsp;<% getStaLinkTxRate(); %></td>
    <td>Rx(Mbps)&nbsp;&nbsp;<% getStaLinkRxRate(); %></td>
  </tr>
  <tr>
    <td class="head" id="linkThroughput">Throughput</td>
    <td>Tx(Kbps)&nbsp;&nbsp;<% getStaTxThroughput(); %></td>
    <td>Rx(Kbps)&nbsp;&nbsp;<% getStaRxThroughput(); %></td>
  </tr>
  <tr>
    <td class="head" id="linkQuality">Link Quality</td>
    <td colspan="2"><% getStaLinkQuality(); %></td>
  </tr>
  <tr>
    <td class="head"><font id="linkSigStrength1">Signal Strength </font>1</td>
    <td><% getStaSignalStrength(); %></td>
    <td rowspan="4"><input type="checkbox" name="dbmChecked"
	<% dbm = getStaDbm(); if (dbm == "1") write("checked"); %>
        OnClick="submit();">dBm format</td>
  </tr>
  <tr>
    <td class="head"><font id="linkSigStrength2">Signal Strength </font>2</td>
    <td><% getStaSignalStrength_1(); %></td>
  </tr>
  <tr>
    <td class="head"><font id="linkSigStrength3">Signal Strength </font>3</td>
    <td><% getStaSignalStrength_2(); %></td>
  </tr>
  <tr>
    <td class="head" id="linkNoiseLevel">Noise Level</td>
    <td><% getStaNoiseLevel(); %></td>
  </tr>
</table>
<br />

<table id="linkHT" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" id="linkHTTitle">HT</td>
  </tr><% getStaHT(); %><tr>
    <td class="head"><font id="linkSNR0">SNR</font>0</td>
    <td><% getStaSNR(0); %></td>
  </tr>
  <tr>
    <td class="head"><font id="linkSNR1">SNR</font>1</td>
    <td><% getStaSNR(1); %></td>
  </tr>
  <tr>
    <td class="head"><font id="linkSNR0">SNR</font>2</td>
    <td><% getStaSNR(2); %></td>
  </tr>
</table>
<input type=hidden name=dummyData value="1">
</form>

</td></tr></table>
</body>
</html>


