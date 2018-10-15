<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<META HTTP-EQUIV="refresh" CONTENT="6; URL=./stainfo.asp">

<title>Station List</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

function initTranslation()
{
	var e = document.getElementById("stalistTitle");
	e.innerHTML = _("stalist title");
	e = document.getElementById("stalistIntroduction");
	e.innerHTML = _("stalist introduction");
	e = document.getElementById("stalistWirelessNet");
	e.innerHTML = _("stalist wireless network");
	e = document.getElementById("stalistMacAddr");
	e.innerHTML = _("stalist macaddr");
}

function PageInit()
{
	initTranslation();
}
</script>
</head>


<body onLoad="PageInit()" bgcolor="#FFFFFF">
<div align="center">
 <center>
<table class="body"><tr><td>

<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="stalistTitle">Station List</td>
</tr>
<tr>

<tr>
<td colspan="2">
<p class="head" id="stalistIntroduction">You could monitor stations which associated to this AP here. </p>
</td>
<tr>
</table>

<br>

<table width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="8" id="stalistWirelessNet">Wireless Network</td>
  </tr>
  <tr>
    <td class="head" bgcolor=#DBE2EC id="stalistMacAddr">MAC Address</td>
	<!--
    <td class="head" bgcolor=#DBE2EC>Aid</td>
    <td class="head" bgcolor=#DBE2EC>PSM</td>
    <td class="head" bgcolor=#DBE2EC>MimoPS</td>
    <td class="head" bgcolor=#DBE2EC>MCS</td>
    <td class="head" bgcolor=#DBE2EC>BW</td>
    <td class="head" bgcolor=#DBE2EC>SGI</td>
    <td class="head" bgcolor=#DBE2EC>STBC</td>
	-->
  </tr>
  <script language="JavaScript" type="text/javascript">
  if( '<% getCfgZero(1, "RadioOff"); %>' == '0')
	document.write("<% getWlanStaInfo(); %>");
  </script>
</table>

</td></tr></table>
 </center>
</div>
</body>
</html>
