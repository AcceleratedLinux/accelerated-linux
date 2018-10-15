<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Ralink Wireless Station Site Survey</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

var g_ssid;
var g_networktype;
var g_channel;
var g_auth;
var g_encry;
var g_bssid;

function countTime()
{
	//var connectstatus = '<!--#include ssi=getStaConnectionStatus() -->';

	//if (connectstatus == 1)  // 0 is NdisMediaStateConnected, 1 is NdisMediaStateDisconnected
		setTimeout("window.location.reload();", 1000*4);
}

function selectedSSIDChange(ssid, bssid, networktype, channel, encry, auth)
{
	g_ssid = ssid;
	g_networktype = networktype;
	g_channel = channel;
	g_auth = auth;
	g_encry = encry;
	g_bssid = bssid

	document.sta_site_survey.connectionButton.disabled=false;
	document.sta_site_survey.addProfileButton.disabled=false;
	document.sta_site_survey.connectedssid.disabled=true;

	//alert("ssid="+ssid+" networktype="+networktype+" channel="+channel+" auth="+auth+" bssid="+bssid);
}

function open_connection_page()
{
	cwin = window.open("site_survey_connection.asp","sta_site_survey_connection","toolbar=no, location=yes, scrollbars=yes, resizable=no, width=660, height=600");
}

function open_profile_page()
{
	pcwin = window.open("add_profile_page.asp","add_profile_page","toolbar=no, location=yes, scrollbars=yes, resizable=no, width=660, height=600");
}

function showConnectionSsid()
{
	cwin.document.forms["sta_site_survey_connection"].Ssid.value = g_ssid;
	cwin.document.forms["sta_site_survey_connection"].bssid.value = g_bssid;

	if (g_networktype == 1)
	{
		if (g_auth.indexOf("WPA2-PSK") >= 0)
			cwin.document.forms["sta_site_survey_connection"].security_infra_mode.value = 7;
		else if (g_auth.indexOf("WPA-PSK") >= 0)
			cwin.document.forms["sta_site_survey_connection"].security_infra_mode.value = 4;
		else if (g_auth.indexOf("WPA2") >= 0)
			cwin.document.forms["sta_site_survey_connection"].security_infra_mode.value = 6;
		else if (g_auth.indexOf("WPA") >= 0)
			cwin.document.forms["sta_site_survey_connection"].security_infra_mode.value = 3;
		else
			cwin.document.forms["sta_site_survey_connection"].security_infra_mode.value = 0;
	}
	else
	{
		if ( g_auth.indexOf("WPA-NONE") >= 0 || g_auth.indexOf("WPA2-NONE") >= 0)
			cwin.document.forms["sta_site_survey_connection"].security_adhoc_mode.value = 5;
		else
			cwin.document.forms["sta_site_survey_connection"].security_adhoc_mode.value = 0;
	}

	//encry
	if (g_encry.indexOf("Not Use") >= 0)
		cwin.document.forms["sta_site_survey_connection"].openmode.value = 1;
	else if (g_encry.indexOf("AES") >= 0)
		cwin.document.forms["sta_site_survey_connection"].cipher[1].checked = true;
	else if (g_encry.indexOf("TKIP") >= 0)
		cwin.document.forms["sta_site_survey_connection"].cipher[0].checked = true;
	else
		cwin.document.forms["sta_site_survey_connection"].openmode.value = 0;

	cwin.document.forms["sta_site_survey_connection"].network_type.value = g_networktype;
}

function showProfileSsid()
{
	pcwin.document.forms["profile_page"].Ssid.value = g_ssid;


	if(g_networktype == 1 )
	{
		if (g_auth.indexOf("WPA2-PSK") >= 0)
			pcwin.document.forms["profile_page"].security_infra_mode.value = 7;
		else if (g_auth.indexOf("WPA-PSK") >= 0)
			pcwin.document.forms["profile_page"].security_infra_mode.value = 4;
		else if (g_auth.indexOf("WPA2") >= 0)
			pcwin.document.forms["profile_page"].security_infra_mode.value = 6;
		else if (g_auth.indexOf("WPA") >= 0)
			pcwin.document.forms["profile_page"].security_infra_mode.value = 3;
		else		
			pcwin.document.forms["profile_page"].security_infra_mode.value = 0;
	}
	else
	{
		if ( g_auth.indexOf("WPA-NONE") >= 0 || g_auth.indexOf("WPA2-NONE") >= 0)
			pcwin.document.forms["profile_page"].security_adhoc_mode.value = 5;
		else
			pcwin.document.forms["profile_page"].security_adhoc_mode.value = 0;
	}

	//encry
	if (g_encry.indexOf("Not Use") >= 0)
		pcwin.document.forms["profile_page"].openmode.value = 1;
	else if (g_encry.indexOf("TKIP") >= 0)
		pcwin.document.forms["profile_page"].cipher[0].checked = true;
	else if (g_encry.indexOf("AES") >= 0)
		pcwin.document.forms["profile_page"].cipher[1].checked = true;
	else
		pcwin.document.forms["profile_page"].openmode.value = 0;

	pcwin.document.forms["profile_page"].network_type.value = g_networktype;

	pcwin.document.forms["profile_page"].channel.value = g_channel;
}

function initTranslation()
{
	var e = document.getElementById("scanTitle");
	e.innerHTML = _("scan title");
	e = document.getElementById("scanIntroduction");
	e.innerHTML = _("scan introduction");
	
	e = document.getElementById("scanSiteSurvey");
	e.innerHTML = _("scan site survey");
	e = document.getElementById("scanSelect");
	e.innerHTML = _("station select");
	e = document.getElementById("scanSSID");
	e.innerHTML = _("station ssid");
	e = document.getElementById("scanBSSID");
	e.innerHTML = _("scan bssid");
	e = document.getElementById("scanRSSI");
	e.innerHTML = _("scan rssi");
	e = document.getElementById("scanChannel");
	e.innerHTML = _("station channel");
	e = document.getElementById("scanEncryp");
	e.innerHTML = _("station encryp");
	e = document.getElementById("scanAuth");
	e.innerHTML = _("station auth");
	e = document.getElementById("scanNetType");
	e.innerHTML = _("station network type");
	e = document.getElementById("scanConnect");
	e.value = _("scan connet");
	e = document.getElementById("scanRescan");
	e.value = _("scan rescan");
	e = document.getElementById("scanAddProfile");
	e.value = _("scan add profile");
}

function PageInit()
{
	initTranslation();
}
</script>
</head>


<body onload="PageInit()">
<table class="body"><tr><td>

<h1 id="scanTitle">Station Site Survey</h1>
<p id="scanIntroduction">Site survey page shows information of APs nearby. You may choose one of these APs connecting or adding it to profile.</p>
<hr />

<form method=post name="sta_site_survey">
<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td class="title" colspan="8" id="scanSiteSurvey">Site Survey</td>
  </tr>
  <tr>
    <td bgcolor="#E8F8FF" id="scanSelect">&nbsp;</td>
    <td bgcolor="#E8F8FF" id="scanSSID">SSID</td>
    <td bgcolor="#E8F8FF" id="scanBSSID">BSSID</td>
    <td bgcolor="#E8F8FF" id="scanRSSI">RSSI</td>
    <td bgcolor="#E8F8FF" id="scanChannel">Channel</td>
    <td bgcolor="#E8F8FF" id="scanEncryp">Encryption</td>
    <td bgcolor="#E8F8FF" id="scanAuth">Authentication</td>
    <td bgcolor="#E8F8FF" id="scanNetType">Network Type</td>
  </tr>
  <% getStaBSSIDList(); %>

  <table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
    <tr>
      <td>
	<input type=text name="connectedssid" size=28 value="<% getStaConnectionSSID(); %>" disabled>&nbsp;&nbsp;&nbsp;
	<input type=button style="{width:100px;}" name="connectionButton" value="Connect" id="scanConnect" disabled onClick="open_connection_page()">&nbsp;
	<input type=button style="{width:100px;}" value="Rescan" id="scanRescan" onClick="location.href=location.href">&nbsp;
	<!--
	<input type=button style="{width:100px;}" value="Rescan" onClick="location.href=location.href">&nbsp; 
	<input type=button style="{width:100px;}" value="Rescan" onClick="window.location.reload()">&nbsp;
	-->
	<input type=button style="{width:100px;}" name="addProfileButton" value="Add Profile" id="scanAddProfile" disabled onClick="open_profile_page()">
      </td>
    </tr>
  </table>
</table>
</form>


</td></tr></table>
</body>
</html>

