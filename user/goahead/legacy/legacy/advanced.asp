<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>Advanced Wireless Settings</title>

<script language="JavaScript" type="text/javascript">
var basicRate = '<% getCfg3Zero(1, "BasicRate"); %>';
var bgProtection = '<% getCfg3Zero(1, "BGProtection"); %>';
//var dataRate = '<!--#include ssi=getLegacyDataRate()-->';
var beaconInterval = '<% getCfg3Zero(1, "BeaconPeriod"); %>';
var dtimValue = '<% getCfg3Zero(1, "DtimPeriod"); %>';
var fragmentThreshold = '<% getCfg3Zero(1, "FragThreshold"); %>';
var rtsThreshold = '<% getCfg3Zero(1, "RTSThreshold"); %>';
var shortPreamble = '<% getCfg3Zero(1, "TxPreamble"); %>';
var shortSlot = '<% getCfg3Zero(1, "ShortSlot"); %>';
var txBurst = '<% getCfg3Zero(1, "TxBurst"); %>';
var pktAggregate = '<% getCfg3Zero(1, "PktAggregate"); %>';
var wmmCapable = '<% getCfg3Zero(1, "WmmCapable"); %>';
var APSDCapable = '<% getCfg3Zero(1, "APSDCapable"); %>';
var DLSCapable = '<% getCfg3Zero(1, "DLSCapable"); %>';
var wirelessMode = '<% getCfg3Zero(1, "WirelessMode"); %>';
var ieee80211h  = '<% getCfg3Zero(1, "IEEE80211H"); %>';
var countrycode = '<% getCfg3General(1, "CountryCode"); %>';
var txPower = '<% getCfg3Zero(1, "TxPower"); %>';

function style_display_on()
{
	if (window.ActiveXObject)
	{ // IE
		return "block";
	}
	else if (window.XMLHttpRequest)
	{ // Mozilla, Safari,...
		return "table-row";
	}
}

function initValue()
{
	var datarateArray;

	bgProtection = 1*bgProtection;
	document.wireless_advanced.bg_protection.options.selectedIndex = bgProtection;
	basicRate = 1*basicRate;

	if (basicRate == 3)
		document.wireless_advanced.basic_rate.options.selectedIndex = 0;
	else if (basicRate == 15)
		document.wireless_advanced.basic_rate.options.selectedIndex = 1;
	else if (basicRate == 351)
		document.wireless_advanced.basic_rate.options.selectedIndex = 2;

	wirelessMode = 1*wirelessMode;

	if (wirelessMode == 2)
		document.wireless_advanced.basic_rate.disabled = true;
	else
		document.wireless_advanced.ieee_80211h.disabled = true;

	beaconInterval = 1*beaconInterval;
	document.wireless_advanced.beacon.value = beaconInterval;
	dtimValue = 1*dtimValue;
	document.wireless_advanced.dtim.value = dtimValue;
	fragmentThreshold = 1*fragmentThreshold;
	document.wireless_advanced.fragment.value = fragmentThreshold;
	rtsThreshold = 1*rtsThreshold;
	document.wireless_advanced.rts.value = rtsThreshold;

	shortPreamble = 1*shortPreamble;
	if (shortPreamble == 1)
	{
		document.wireless_advanced.short_preamble[0].checked = true;
		document.wireless_advanced.short_preamble[1].checked = false;
	}
	else
	{
		document.wireless_advanced.short_preamble[0].checked = false;
		document.wireless_advanced.short_preamble[1].checked = true;
	}

	shortSlot = 1*shortSlot;
	if (shortSlot == 1)
	{
		document.wireless_advanced.short_slot[0].checked = true;
	}
	else
	{
		document.wireless_advanced.short_slot[1].checked = true;
	}

	txBurst = 1*txBurst;
	if (txBurst == 1)
	{
		document.wireless_advanced.tx_burst[0].checked = true;
		document.wireless_advanced.tx_burst[1].checked = false;
	}
	else
	{
		document.wireless_advanced.tx_burst[0].checked = false;
		document.wireless_advanced.tx_burst[1].checked = true;
	}

	pktAggregate = 1*pktAggregate;
	if (pktAggregate == 1)
	{
		document.wireless_advanced.pkt_aggregate[0].checked = true;
		document.wireless_advanced.pkt_aggregate[1].checked = false;
	}
	else
	{
		document.wireless_advanced.pkt_aggregate[0].checked = false;
		document.wireless_advanced.pkt_aggregate[1].checked = true;
	}

	ieee80211h = 1*ieee80211h;
	if (ieee80211h == 1)
		document.wireless_advanced.ieee_80211h[0].checked = true;
	else
		document.wireless_advanced.ieee_80211h[1].checked = true;

	wmmCapable = 1*wmmCapable;
	if (wmmCapable == 1)
	{
		document.wireless_advanced.wmm_capable[0].checked = true;
		document.wireless_advanced.wmm_capable[1].checked = false;
	}
	else
	{
		document.wireless_advanced.wmm_capable[0].checked = false;
		document.wireless_advanced.wmm_capable[1].checked = true;
	}

	document.getElementById("div_apsd_capable").style.visibility = "hidden";
	document.getElementById("div_apsd_capable").style.display = "none";
	document.wireless_advanced.apsd_capable.disabled = true;
	//document.getElementById("div_dls_capable").style.visibility = "hidden";
	//document.getElementById("div_dls_capable").style.display = "none";
	//document.wireless_advanced.dls_capable.disabled = true;

	if (wmmCapable == 1)
	{
		document.getElementById("div_apsd_capable").style.visibility = "visible";
		document.getElementById("div_apsd_capable").style.display = style_display_on();
		document.wireless_advanced.apsd_capable.disabled = false;
		//document.getElementById("div_dls_capable").style.visibility = "visible";
		//document.getElementById("div_dls_capable").style.display = style_display_on();
		//document.wireless_advanced.dls_capable.disabled = false;
	}

	APSDCapable = 1*APSDCapable;
	if (APSDCapable == 1)
	{
		document.wireless_advanced.apsd_capable[0].checked = true;
		document.wireless_advanced.apsd_capable[1].checked = false;
	}
	else
	{
		document.wireless_advanced.apsd_capable[0].checked = false;
		document.wireless_advanced.apsd_capable[1].checked = true;
	}

	//DLSCapable = 1*DLSCapable;
	//if (DLSCapable == 1)
	//{
		//document.wireless_advanced.dls_capable[0].checked = true;
		//document.wireless_advanced.dls_capable[1].checked = false;
	//}
	//else
	//{
		//document.wireless_advanced.dls_capable[0].checked = false;
		//document.wireless_advanced.dls_capable[1].checked = true;
	//}
	document.wireless_advanced.tx_power.value = txPower;

	if (countrycode == "US")
		document.wireless_advanced.country_code.options.selectedIndex = 0;
	else if (countrycode == "JP")
		document.wireless_advanced.country_code.options.selectedIndex = 1;
	else if (countrycode == "FR")
		document.wireless_advanced.country_code.options.selectedIndex = 2;
	else if (countrycode == "TW")
		document.wireless_advanced.country_code.options.selectedIndex = 3;
	else if (countrycode == "IE")
		document.wireless_advanced.country_code.options.selectedIndex = 4;
	else if (countrycode == "HK")
		document.wireless_advanced.country_code.options.selectedIndex = 5;
	else if (countrycode == "NONE")
		document.wireless_advanced.country_code.options.selectedIndex = 6;
	else
		document.wireless_advanced.country_code.options.selectedIndex = 6;
}

function CheckValue()
{

	if (document.wireless_advanced.beacon.value == "" )
	{
		alert('Please specify Beacon Interval');
		document.wireless_advanced.beacon.focus();
		document.wireless_advanced.beacon.select();
		return false;
	}

	if (isNaN(document.wireless_advanced.beacon.value) || document.wireless_advanced.beacon.value < 20 || document.wireless_advanced.beacon.value > 999)
	{
		alert('Invalid Beacon Interval');
		document.wireless_advanced.beacon.focus();
		document.wireless_advanced.beacon.select();
		return false;
	}

	if (document.wireless_advanced.dtim.value == "" )
	{
		alert('Please specify DTIM Interval');
		document.wireless_advanced.dtim.focus();
		document.wireless_advanced.dtim.select();
		return false;
	}

	if (isNaN(document.wireless_advanced.dtim.value) || document.wireless_advanced.dtim.value < 1 || document.wireless_advanced.dtim.value > 255)
	{
		alert('Invalid DTIM Interval');
		document.wireless_advanced.dtim.focus();
		document.wireless_advanced.dtim.select();
		return false;
	}

	if (document.wireless_advanced.fragment.value == "" )
	{
		alert('Please specify Fragmentation Length');
		document.wireless_advanced.fragment.focus();
		document.wireless_advanced.fragment.select();
		return false;
	}

	if (isNaN(document.wireless_advanced.fragment.value) || document.wireless_advanced.fragment.value < 1 || document.wireless_advanced.fragment.value > 2346)
	{
		alert('Invalid Fragmentation Length');
		document.wireless_advanced.fragment.focus();
		document.wireless_advanced.fragment.select();
		return false;
	}

	if (document.wireless_advanced.rts.value == "" )
	{
		alert('Please specify RTS Threshold');
		document.wireless_advanced.rts.focus();
		document.wireless_advanced.rts.select();
		return false;
	}

	if (isNaN(document.wireless_advanced.rts.value) || document.wireless_advanced.rts.value < 1 || document.wireless_advanced.rts.value > 2347)
	{
		alert('Invalid RTS Threshold');
		document.wireless_advanced.rts.focus();
		document.wireless_advanced.rts.select();
		return false;
	}

	if (document.wireless_advanced.ieee_80211h[0].checked == true)
	{
		if (1*ieee80211h == 0)
			document.wireless_advanced.rebootAP.value = 1;
	}
	else
	{
		if (1*ieee80211h == 1)
			document.wireless_advanced.rebootAP.value = 1;
	}

	if (document.wireless_advanced.wmm_capable[0].checked == true)
	{
		if (1*wmmCapable == 0)
			document.wireless_advanced.rebootAP.value = 1;
	}
	else
	{
		document.wireless_advanced.apsd_capable[1].checked = true;
		//document.wireless_advanced.dls_capable[1].checked = true;

		if (1*wmmCapable == 1)
			document.wireless_advanced.rebootAP.value = 1;
	}

	if (document.wireless_advanced.apsd_capable[0].checked == true)
	{
		if (1*APSDCapable == 0)
			document.wireless_advanced.rebootAP.value = 1;
	}
	else
	{
		if (1*APSDCapable == 1)
			document.wireless_advanced.rebootAP.value = 1;
	}

	//if (document.wireless_advanced.dls_capable[0].checked == true)
	//{
		//if (1*DLSCapable == 0)
			//document.wireless_advanced.rebootAP.value = 1;
	//}
	//else
	//{
		//if (1*DLSCapable == 1)
			//document.wireless_advanced.rebootAP.value = 1;
	//}

	return true;
}

function open_wmm_window()
{
	window.open("wmm.asp","WMM_Parameters_List","toolbar=no, location=yes, scrollbars=yes, resizable=no, width=640, height=480")
}

function wmm_capable_enable_switch()
{
	document.getElementById("div_apsd_capable").style.visibility = "hidden";
	document.getElementById("div_apsd_capable").style.display = "none";
	//document.getElementById("div_dls_capable").style.visibility = "hidden";
	//document.getElementById("div_dls_capable").style.display = "none";
	document.wireless_advanced.apsd_capable.disabled = true;
	//document.wireless_advanced.dls_capable.disabled = true;

	if (document.wireless_advanced.wmm_capable[0].checked == true)
	{
		document.getElementById("div_apsd_capable").style.visibility = "visible";
		document.getElementById("div_apsd_capable").style.display = style_display_on();
		document.wireless_advanced.apsd_capable.disabled = false;
		//document.getElementById("div_dls_capable").style.visibility = "visible";
		//document.getElementById("div_dls_capable").style.display = style_display_on();
		//document.wireless_advanced.dls_capable.disabled = false;
	}
}
</script>
</head>

<body onLoad="initValue()">
<table class="body"><tr><td>


<h1>Advanced Wireless Settings </h1>
<p>Use the Advanced Setup page to make detailed settings for the Wireless. Advanced Setup includes items that are not available from the Basic Setup page, such as Beacon Interval, Control Tx Rates and Basic Data Rates. </p>
<hr />

<form method=post name=wireless_advanced action="/goform/legacyAdvanced" onSubmit="return CheckValue()">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2">Advanced Wireless</td>
  </tr>
  <tr> 
    <td class="head">BG Protection Mode</td>
    <td>
      <select name="bg_protection" size="1">
	<option value=0 selected>Auto</option>
	<option value=1>On</option>
	<option value=2>Off</option>
      </select>
    </td>
  </tr>
  <tr> 
    <td class="head">Basic Data Rates</td>
    <td>
      <select name="basic_rate" size="1">
	<option value="3" >1-2 Mbps</option>
	<option value="15" >Default(1-2-5.5-11 Mbps)</option>
	<option value="351" >All(1-2-5.5-6-11-12-24 Mbps)</option>
      </select>
    </td>
  </tr>
  <!--
  <tr id="div_11a_tx_rate" name="div_11a_tx_rate"> 
    <td class="head">Data Tx Rate</td>
    <td>
      <select name="tx_rate_11a" size="1">
	<option value="0" selected>Auto</option>
	<option value="1">6 Mbps</option>
	<option value="2">9 Mbps</option>
	<option value="3">12 Mbps</option>
	<option value="4">18 Mbps</option>
	<option value="5">24 Mbps</option>
	<option value="6">36 Mbps</option>
	<option value="7">48 Mbps</option>
	<option value="8">54 Mbps</option>
      </select>
    </td>
  </tr>
  <tr id="div_11b_tx_rate" name="div_11b_tx_rate"> 
    <td class="head">Data Tx Rate</td>
    <td>
      <select name="tx_rate_11b" size="1" style="{font-family:arial; width:120px; color:#003366; font-weight: bold; font-size: 9pt;}">
	<option value="0" selected>Auto</option>
	<option value="1">1 Mbps</option>
	<option value="2">2 Mbps</option>
	<option value="3">5.5 Mbps</option>
	<option value="4">11 Mbps</option>
      </select>
    </td>
  </tr>
  <tr id="div_11bg_tx_rate" name="div_11bg_tx_rate"> 
    <td class="head">Data Tx Rate</td>
    <td>
      <select name="tx_rate_11bg" size="1" style="{font-family:arial; width:120px; color:#003366; font-weight: bold; font-size: 9pt;}">
	<option value="0" selected>Auto</option>
	<option value="1">1 Mbps</option>
	<option value="2">2 Mbps</option>
	<option value="3">5.5 Mbps</option>
	<option value="4">11 Mbps</option>
	<option value="5">6 Mbps</option>
	<option value="6">9 Mbps</option>
	<option value="7">12 Mbps</option>
	<option value="8">18 Mbps</option>
	<option value="9">24 Mbps</option>
	<option value="10">36 Mbps</option>
	<option value="11">48 Mbps</option>
	<option value="12">54 Mbps</option>
      </select>
    </td>
  </tr>
  <tr id="div_11g_tx_rate" name="div_11g_tx_rate"> 
    <td class="head">Data Tx Rate</td>
    <td>
      <select name="tx_rate_11g" size="1" style="{font-family:arial; width:120px; color:#003366; font-weight: bold; font-size: 9pt;}">
	<option value="0" selected>Auto</option>
	<option value="17">6 Mbps</option>
	<option value="18">9 Mbps</option>
	<option value="19">12 Mbps</option>
	<option value="20">18 Mbps</option>
	<option value="21">24 Mbps</option>
	<option value="22">36 Mbps</option>
	<option value="23">48 Mbps</option>
	<option value="24">54 Mbps</option>
      </select>
    </td>
  </tr>
  -->
  <tr> 
    <td class="head">Beacon Interval</td>
    <td>
      <input type=text name=beacon size=5 maxlength=3 value="100"> ms <font color="#808080">(range 20 - 999, default 100)</font>
    </td>
  </tr>
  <tr> 
    <td class="head">Data Beacon Rate (DTIM) </td>
    <td>
      <input type=text name=dtim size=5 maxlength=3 value="1"> ms <font color="#808080">(range 1 - 255, default 1)</font>
    </td>
  </tr>
  <tr> 
    <td class="head">Fragment Threshold</td>
    <td>
      <input type=text name=fragment size=5 maxlength=4 value=""> <font color="#808080">(range 256 - 2346, default 2346)</font>
    </td>
  </tr>
  <tr> 
    <td class="head">RTS Threshold</td>
    <td>
      <input type=text name=rts size=5 maxlength=4 value=""> <font color="#808080">(range 1 - 2347, default 2347)</font>
    </td>
  </tr>
  <tr> 
    <td class="head">TX Power</td>
    <td>
      <input type=text name=tx_power size=5 maxlength=3 value="100"> <font color="#808080">(range 1 - 100, default 100)</font>
    </td>
  </tr>
  <tr> 
    <td class="head">Short Preamble</td>
    <td>
      <input type=radio name=short_preamble value="1" checked>Enable &nbsp;
      <input type=radio name=short_preamble value="0">Disable
    </td>
  </tr>
  <tr> 
    <td class="head">Short Slot</td>
    <td>
      <input type=radio name=short_slot value="1" checked>Enable &nbsp;
      <input type=radio name=short_slot value="0">Disable
    </td>
  </tr>
  <tr> 
    <td class="head">Tx Burst</td>
    <td>
      <input type=radio name=tx_burst value="1" checked>Enable &nbsp;
      <input type=radio name=tx_burst value="0">Disable
    </td>
  </tr>
  <tr> 
    <td class="head">Pkt_Aggregate</td>
    <td>
      <input type=radio name=pkt_aggregate value="1">Enable &nbsp;
      <input type=radio name=pkt_aggregate value="0" checked>Disable
    </td>
  </tr>
  <tr> 
    <td class="head">IEEE 802.11H Support</td>
    <td>
      <input type=radio name=ieee_80211h value="1">Enable &nbsp;
      <input type=radio name=ieee_80211h value="0" checked>Disable <font color="#808080">(only in A band)</font>
    </td>
  </tr>
  <tr> 
    <td class="head">Country Code</td>
    <td>
      <select name="country_code">
        <option value="US">US (United States)</option>
        <option value="JP">JP (Japan)</option>
        <option value="FR">FR (France)</option>
        <option value="TW">TW (Taiwan)</option>
        <option value="IE">IE (Ireland)</option>
        <option value="HK">HK (Hong Kong)</option>
        <option value="NONE" selected>NONE</option>
      </select>
    </td>
  </tr>
</table>
<hr />

<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2">Wi-Fi Multimedia</td>
  </tr>
  <tr> 
    <td class="head">WMM Capable</td>
    <td>
      <input type=radio name=wmm_capable value="1" onClick="wmm_capable_enable_switch()" checked>Enable &nbsp;
      <input type=radio name=wmm_capable value="0" onClick="wmm_capable_enable_switch()">Disable
    </td>
  </tr>
  <tr id="div_apsd_capable" name="div_apsd_capable">
    <td class="head">APSD Capable</td>
    <td>
      <input type=radio name=apsd_capable value="1">Enable &nbsp;
      <input type=radio name=apsd_capable value="0" checked>Disable
    </td>
  </tr>
  <!--
  <tr id="div_dls_capable" name="div_dls_capable">
    <td class="head">DLS Capable</td>
    <td>
      <input type=radio name=dls_capable value="1">Enable &nbsp;
      <input type=radio name=dls_capable value="0" checked>Disable
    </td>
  </tr>
  -->
  <tr> 
    <td class="head">WMM Parameters</td>
    <td>
      <input type=button name="wmm_list" value="WMM Configuration" onClick="open_wmm_window()">
    </td>
  </tr>
  <input type="hidden" name="rebootAP" value="0">
</table>
<br>
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>


</td></tr></table>
</body>
</html>

