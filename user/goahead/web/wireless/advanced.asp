<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<script type="text/javascript" src="/common.js"></script>
<title>Advanced Wireless Settings</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

restartPage_init();

var sbuttonMax=1;

var basicRate = '<% getCfgZero(1, "BasicRate"); %>';
/*
var bgProtection = '<% getCfgZero(1, "BGProtection"); %>';
var dataRate = '<!--#include ssi=getWlanDataRate()-->';
*/
var beaconInterval = '<% getCfgZero(1, "BeaconPeriod"); %>';
var dtimValue = '<% getCfgZero(1, "DtimPeriod"); %>';
var fragmentThreshold = '<% getCfgZero(1, "FragThreshold"); %>';
var rtsThreshold = '<% getCfgZero(1, "RTSThreshold"); %>';
var shortPreamble = '<% getCfgZero(1, "TxPreamble"); %>';
/*
var shortSlot = '<% getCfgZero(1, "ShortSlot"); %>';
var txBurst = '<% getCfgZero(1, "TxBurst"); %>';
var pktAggregate = '<% getCfgZero(1, "PktAggregate"); %>';
*/
var wmmCapable = '<% getCfgZero(1, "WmmCapable"); %>';
var APSDCapable = '<% getCfgZero(1, "APSDCapable"); %>';
var DLSCapable = '<% getCfgZero(1, "DLSCapable"); %>';
var wirelessMode = '<% getCfgZero(1, "WirelessMode"); %>';
var ieee80211h  = '<% getCfgZero(1, "IEEE80211H"); %>';
var countrycode = '<% getCfgGeneral(1, "CountryCode"); %>';
var txPower = '<% getCfgZero(1, "TxPower"); %>';
var DLSBuilt = '<% getDLSBuilt(); %>';
var m2uBuilt = '<% getWlanM2UBuilt(); %>';
var m2uEnabled = '<% getCfgZero(1, "M2UEnabled"); %>';
var dfsb = '<% getDFSBuilt(); %>';
var carrierib = '<% getCarrierBuilt(); %>';

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

function initTranslation()
{
	var e = document.getElementById("advTitle");
	e.innerHTML = _("adv title");
	e = document.getElementById("advIntroduction");
	e.innerHTML = _("adv introduction");

	e = document.getElementById("advWireless");
	e.innerHTML = _("adv wireless");
/*
	e = document.getElementById("advBGProtect");
	e.innerHTML = _("adv bgpro");
	e = document.getElementById("advBGProAuto");
	e.innerHTML = _("wireless auto");
	e = document.getElementById("advBGProOn");
	e.innerHTML = _("wireless on");
	e = document.getElementById("advBGProOff");
	e.innerHTML = _("wireless off");
	e = document.getElementById("advBasicDtRt");
	e.innerHTML = _("adv basic data rate");
	e = document.getElementById("advBasicDtRtDefault");
	e.innerHTML = _("adv basic data rate default");
	e = document.getElementById("advBasicDtRtAll");
	e.innerHTML = _("adv basic data rate all");
 */
	e = document.getElementById("advBeaconInterval");
	e.innerHTML = _("adv beacon interval");
	e = document.getElementById("advBeaconIntervalRange");
	e.innerHTML = _("adv beacon interval range");
	e = document.getElementById("advDTIM");
	e.innerHTML = _("adv dtim");
	e = document.getElementById("advDTIMRange");
	e.innerHTML = _("adv dtim range");
	e = document.getElementById("advFrag");
	e.innerHTML = _("adv fragment threshold");
	e = document.getElementById("advFragRange");
	e.innerHTML = _("adv fragment threshold range");
	e = document.getElementById("advRTS");
	e.innerHTML = _("adv rts threshold");
	e = document.getElementById("advRTSRange");
	e.innerHTML = _("adv rts threshold range");
	e = document.getElementById("advTxPW");
	e.innerHTML = _("adv tx power");
	e = document.getElementById("advTxPWRange");
	e.innerHTML = _("adv tx power range");
	e = document.getElementById("advShortPre");
	e.innerHTML = _("adv short preamble");
	e = document.getElementById("advShortPreEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("advShortPreDisable");
	e.innerHTML = _("wireless disable");
/*
	e = document.getElementById("advShortSlot");
	e.innerHTML = _("adv short slot");
	e = document.getElementById("advShortSlotEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("advShortSlotDisable");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("advTxBurst");
	e.innerHTML = _("adv tx burst");
	e = document.getElementById("advTxBurstEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("advTxBurstDisable");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("advPktAggr");
	e.innerHTML = _("adv pkt aggregate");
	e = document.getElementById("advPktAggrEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("advPktAggrDisable");
	e.innerHTML = _("wireless disable");
 */
	e = document.getElementById("adv80211H");
	e.innerHTML = _("adv 80211h");
	e = document.getElementById("adv80211HEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("adv80211HDisable");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("adv80211HDisableDescribe");
	e.innerHTML = _("adv 80211h disable describe");
	e = document.getElementById("advRDRegion");
	e.innerHTML = _("adv rdregion");
	//e = document.getElementById("advCountryCode");
	//e.innerHTML = _("adv country code");
	/*e = document.getElementById("advCountryCodeUS");
	e.innerHTML = _("adv country code us");
	e = document.getElementById("advCountryCodeJP");
	e.innerHTML = _("adv country code jp");
	e = document.getElementById("advCountryCodeFR");
	e.innerHTML = _("adv country code fr");
	e = document.getElementById("advCountryCodeTW");
	e.innerHTML = _("adv country code tw");
	e = document.getElementById("advCountryCodeIE");
	e.innerHTML = _("adv country code ie");
	e = document.getElementById("advCountryCodeHK");
	e.innerHTML = _("adv country code hk");
	e = document.getElementById("advCountryCodeNONE");
	e.innerHTML = _("wireless none");  */
	e = document.getElementById("advCarrierDetect");
	e.innerHTML = _("adv carrier");
	e = document.getElementById("advCarrierDetectEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("advCarrierDetectDisable");
	e.innerHTML = _("wireless disable");

	e = document.getElementById("advWiFiMM");
	e.innerHTML = _("adv wmm");
	e = document.getElementById("advWMM");
	e.innerHTML = _("adv wmm capable");
	e = document.getElementById("advWMMEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("advWMMDisable");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("advAPDS");
	e.innerHTML = _("adv apds capable");
	e = document.getElementById("advAPDSEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("advAPDSDisable");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("advDLS");
	e.innerHTML = _("adv dls capable");
	e = document.getElementById("advDLSEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("advDLSDisable");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("advWMMParameter");
	e.innerHTML = _("adv wmm parameter");
	e = document.getElementById("advWMMConf");
	e.value = _("adv wmm configuration");
	e = document.getElementById("advMul2UniConver");
	e.innerHTML = _("adv multicast2unicast converter");
	e = document.getElementById("advMul2Uni");
	e.innerHTML = _("adv multicast2unicast");
	e = document.getElementById("advMul2UniEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("advMul2UniDisable");
	e.innerHTML = _("wireless disable");

	e = document.getElementById("advApply");
	e.value = _("wireless apply");
	e = document.getElementById("advCancel");
	e.value = _("wireless cancel");
}

function initValue()
{
	var datarateArray;

	initTranslation();
/*
	bgProtection = 1*bgProtection;
	document.wireless_advanced.bg_protection.options.selectedIndex = bgProtection;
	basicRate = 1*basicRate;

	if (basicRate == 3)
		document.wireless_advanced.basic_rate.options.selectedIndex = 0;
	else if (basicRate == 15)
		document.wireless_advanced.basic_rate.options.selectedIndex = 1;
	else if (basicRate == 351)
		document.wireless_advanced.basic_rate.options.selectedIndex = 2;
 */

	wirelessMode = 1*wirelessMode;

	if ((wirelessMode == 2) || (wirelessMode == 8))
		; //document.wireless_advanced.basic_rate.disabled = true;
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

/*
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
 */

	ieee80211h = 1*ieee80211h;
	if (ieee80211h == 1)
		document.wireless_advanced.ieee_80211h[0].checked = true;
	else
		document.wireless_advanced.ieee_80211h[1].checked = true;

	ieee80211h_switch();
	if (dfsb == "1" && ieee80211h == 1) // ======== rdregion ============
	{
		var rdregion = '<% getCfgGeneral(1, "RDRegion"); %>';
		if (rdregion == "CE")
			document.wireless_advanced.rd_region.options.selectedIndex = 2;
		else if (rdregion == "JAP")
			document.wireless_advanced.rd_region.options.selectedIndex = 1;
		else
			document.wireless_advanced.rd_region.options.selectedIndex = 0;
	}

	carrierib = 1*carrierib;
	if (carrierib == 1)
	{
		document.getElementById("div_carrier_detect").style.visibility = "visible";
		document.getElementById("div_carrier_detect").style.display = style_display_on();
		document.wireless_advanced.carrier_detect.disabled = false;
		var carrierebl = '<% getCfgZero(1, "CarrierDetect"); %>';
		if (carrierebl == "1")
		{
			document.wireless_advanced.carrier_detect[0].checked = true;
			document.wireless_advanced.carrier_detect[1].checked = false;
		}
		else
		{
			document.wireless_advanced.carrier_detect[0].checked = false;
			document.wireless_advanced.carrier_detect[1].checked = true;
		}
	}
	else
	{
		document.getElementById("div_carrier_detect").style.visibility = "hidden";
		document.getElementById("div_carrier_detect").style.display = "none";
		document.wireless_advanced.carrier_detect.disabled = true;
	}

	if (wmmCapable.indexOf("1") >= 0)
	{
		document.wireless_advanced.wmm_capable[0].checked = true;
		document.wireless_advanced.wmm_capable[1].checked = false;
	}
	else
	{
		document.wireless_advanced.wmm_capable[0].checked = false;
		document.wireless_advanced.wmm_capable[1].checked = true;
	}

	wmm_capable_enable_switch();

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

	if (DLSBuilt == 1)
	{
		DLSCapable = 1*DLSCapable;
		if (DLSCapable == 1)
		{
			document.wireless_advanced.dls_capable[0].checked = true;
			document.wireless_advanced.dls_capable[1].checked = false;
		}
		else
		{
			document.wireless_advanced.dls_capable[0].checked = false;
			document.wireless_advanced.dls_capable[1].checked = true;
		}
	}
	document.wireless_advanced.tx_power.value = txPower;
/*	// ==================================================================================================countrycode =======================================================
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
*/
	//multicase to unicast converter
	m2uBuilt = 1*m2uBuilt;
	document.getElementById("div_m2u").style.display = "none";
	if (m2uBuilt == 1) {
		if (window.ActiveXObject) // IE
			document.getElementById("div_m2u").style.display = "block";
		else
			document.getElementById("div_m2u").style.display = "table";

		m2uEnabled = 1*m2uEnabled;
		if (m2uEnabled == 1)
		{
			document.wireless_advanced.m2u_enable[0].checked = true;
			document.wireless_advanced.m2u_enable[1].checked = false;
		}
		else
		{
			document.wireless_advanced.m2u_enable[0].checked = false;
			document.wireless_advanced.m2u_enable[1].checked = true;
		}
	}
}

function CheckValue()
{

	if (document.wireless_advanced.beacon.value == "" )
	{
		alert(_("adv alert null beacon"));
		document.wireless_advanced.beacon.focus();
		document.wireless_advanced.beacon.select();
		return false;
	}

	if (isNaN(document.wireless_advanced.beacon.value) || document.wireless_advanced.beacon.value < 20 || document.wireless_advanced.beacon.value > 999 ||
	(document.wireless_advanced.beacon.value.indexOf('-') != -1) ||
	(document.wireless_advanced.beacon.value.indexOf('.') != -1))
	{
		alert(_("adv alert invalid beacon"));
		document.wireless_advanced.beacon.focus();
		document.wireless_advanced.beacon.select();
		return false;
	}

	if (document.wireless_advanced.dtim.value == "" )
	{
		alert(_("adv alert null dtim"));
		document.wireless_advanced.dtim.focus();
		document.wireless_advanced.dtim.select();
		return false;
	}

	if (isNaN(document.wireless_advanced.dtim.value) || document.wireless_advanced.dtim.value < 1 || document.wireless_advanced.dtim.value > 255 ||
	(document.wireless_advanced.dtim.value.indexOf('-') != -1) ||
	(document.wireless_advanced.dtim.value.indexOf('.') != -1))	
	{
		alert(_("adv alert invalid dtim"));
		document.wireless_advanced.dtim.focus();
		document.wireless_advanced.dtim.select();
		return false;
	}

	if (document.wireless_advanced.fragment.value == "" )
	{
		alert(_("adv alert null fragment"));
		document.wireless_advanced.fragment.focus();
		document.wireless_advanced.fragment.select();
		return false;
	}

	if (isNaN(document.wireless_advanced.fragment.value) || document.wireless_advanced.fragment.value < 1 || document.wireless_advanced.fragment.value > 2346 ||
	document.wireless_advanced.fragment.value < 256 ||
	(document.wireless_advanced.fragment.value.indexOf('-') != -1) ||
	(document.wireless_advanced.fragment.value.indexOf('.') != -1) )
	{
		alert(_("adv alert invalid fragment"));
		document.wireless_advanced.fragment.focus();
		document.wireless_advanced.fragment.select();
		return false;
	}

	if (document.wireless_advanced.rts.value == "" )
	{
		alert(_("adv alert null rts threshold"));
		document.wireless_advanced.rts.focus();
		document.wireless_advanced.rts.select();
		return false;
	}

	if (isNaN(document.wireless_advanced.rts.value) || document.wireless_advanced.rts.value < 1 || document.wireless_advanced.rts.value > 2347 ||
	(document.wireless_advanced.rts.value.indexOf('-') != -1) ||
	(document.wireless_advanced.rts.value.indexOf('.') != -1) )
	{
		alert(_("adv alert invalid rts threshold"));
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

	DLSBuilt = 1*DLSBuilt;
	if (document.wireless_advanced.wmm_capable[0].checked == true)
	{
		if (1*wmmCapable == 0)
			document.wireless_advanced.rebootAP.value = 1;
	}
	else
	{
		document.wireless_advanced.apsd_capable[1].checked = true;
		if (DLSBuilt == 1)
		{
			document.wireless_advanced.dls_capable[1].checked = true;
		}

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

	if (DLSBuilt == 1)
	{
		if (document.wireless_advanced.dls_capable[0].checked == true)
		{
			if (1*DLSCapable == 0)
				document.wireless_advanced.rebootAP.value = 1;
		}
		else
		{
			if (1*DLSCapable == 1)
				document.wireless_advanced.rebootAP.value = 1;
		}
	}
	/*if (isNaN(document.wireless_advanced.tx_powwer.value) ||
		document.wireless_advanced.tx_powwer.value.index0f('.') == -1 ||
		document.wireless_advanced.tx_powwer.value < 1 ||
		document.wireless_advanced.tx_powwer.value > 100)*/
	/* tx_powwer --> tx_power, [Yian,2009/07/10] */
	if (isNaN(document.wireless_advanced.tx_power.value) ||
		document.wireless_advanced.tx_power.value.indexOf('.') != -1 ||
		document.wireless_advanced.tx_power.value.indexOf('-') != -1 ||
		document.wireless_advanced.tx_power.value < 1 ||
		document.wireless_advanced.tx_power.value > 100)
	{
		alert(_("adv alert invalid txpower"));
		document.wireless_advanced.tx_power.focus();
		return false;
	}
	
	sbutton_disable(sbuttonMax); 
	restartPage_block();

	return true;
}

function open_wmm_window()
{
	window.open("wmm.asp","WMM_Parameters_List","toolbar=no, location=no, direction=no, scrollbars=yes, resizable=no, width=640, height=480")
}

function wmm_capable_enable_switch()
{
	document.getElementById("div_apsd_capable").style.visibility = "hidden";
	document.getElementById("div_apsd_capable").style.display = "none";
	document.wireless_advanced.apsd_capable.disabled = true;
	document.getElementById("div_dls_capable").style.visibility = "hidden";
	document.getElementById("div_dls_capable").style.display = "none";
	document.wireless_advanced.dls_capable.disabled = true;

	DLSBuilt = 1*DLSBuilt;
	if (document.wireless_advanced.wmm_capable[0].checked == true)
	{
		document.getElementById("div_apsd_capable").style.visibility = "visible";
		document.getElementById("div_apsd_capable").style.display = style_display_on();
		document.wireless_advanced.apsd_capable.disabled = false;
		if (DLSBuilt == 1)
		{
			document.getElementById("div_dls_capable").style.visibility = "visible";
			document.getElementById("div_dls_capable").style.display = style_display_on();
			document.wireless_advanced.dls_capable.disabled = false;
		}
	}
}

function ieee80211h_switch()
{
	if (dfsb == "1" && document.wireless_advanced.ieee_80211h[0].checked == true)
	{
		document.getElementById("div_dfs_rdregion").style.visibility = "visible";
		document.getElementById("div_dfs_rdregion").style.display = style_display_on();
		document.wireless_advanced.rd_region.disabled = false;
	}
	else
	{
		document.getElementById("div_dfs_rdregion").style.visibility = "hidden";
		document.getElementById("div_dfs_rdregion").style.display = "none";
		document.wireless_advanced.rd_region.disabled = true;
	}
}
</script>
</head>

<body onLoad="initValue()" bgcolor="#FFFFFF">
<div align="center">
 <center>
<table class="body"><tr><td>

<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="advTitle">Advanced Wireless Settings</td>
</tr>
<tr>

<tr>
<td colspan="2">
<p class="head" id="advIntroduction"> Use the Advanced Setup page to make detailed settings for the Wireless. Advanced Setup includes items that are not available from the Basic Setup page, such as Beacon Interval, Control Tx Rates and Basic Data Rates. </p>
</td>
<tr>
</table>

<br>

<form method=post name=wireless_advanced action="/goform/wirelessAdvanced" onSubmit="return CheckValue()">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="advWireless">Advanced Wireless</td>
  </tr>
  <!--
  <tr> 
    <td class="head" id="advBGProtect">BG Protection Mode</td>
    <td>
      <select name="bg_protection" size="1">
	<option value=0 selected id="advBGProAuto">Auto</option>
	<option value=1 id="advBGProOn">On</option>
	<option value=2 id="advBGProOff">Off</option>
      </select>
    </td>
  </tr>
  <tr> 
    <td class="head" id="advBasicDtRt">Basic Data Rates</td>
    <td>
      <select name="basic_rate" size="1">
	<option value="3" >1-2 Mbps</option>
	<option value="15" id="advBasicDtRtDefault">Default(1-2-5.5-11 Mbps)</option>
	<option value="351" id="advBasicDtRtAll">All(1-2-5.5-6-11-12-24 Mbps)</option>
      </select>
    </td>
  </tr>
  -->
  <tr> 
    <td class="head" id="advBeaconInterval">Beacon Interval</td>
    <td>
      <input type=text name=beacon size=5 maxlength=3 value="100"> ms <font color="#808080" id="advBeaconIntervalRange">(range 20 - 999, default 100)</font>
    </td>
  </tr>
  <tr> 
    <td class="head" id="advDTIM">Data Beacon Rate (DTIM) </td>
    <td>
      <input type=text name=dtim size=5 maxlength=3 value="1"> ms <font color="#808080" id="advDTIMRange">(range 1 - 255, default 1)</font>
    </td>
  </tr>
  <tr> 
    <td class="head" id="advFrag">Fragment Threshold</td>
    <td>
      <input type=text name=fragment size=5 maxlength=4 value=""> <font color="#808080" id="advFragRange">(range 256 - 2346, default 2346)</font>
    </td>
  </tr>
  <tr> 
    <td class="head" id="advRTS">RTS Threshold</td>
    <td>
      <input type=text name=rts size=5 maxlength=4 value=""> <font color="#808080" id="advRTSRange">(range 1 - 2347, default 2347)</font>
    </td>
  </tr>
  <tr> 
    <td class="head" id="advTxPW">TX Power</td>
    <td>
      <input type=text name=tx_power size=5 maxlength=3 value="100"> <font color="#808080" id="advTxPWRange">(range 1 - 100, default 100)</font>
    </td>
  </tr>
  <tr> 
    <td class="head" id="advShortPre">Short Preamble</td>
    <td>
      <input type=radio name=short_preamble value="1" checked><font id="advShortPreEnable">Enable &nbsp;</font>
      <input type=radio name=short_preamble value="0"><font id="advShortPreDisable">Disable</font>
    </td>
  </tr>

<!-- not options under hostapd
  <tr> 
    <td class="head" id="advShortSlot">Short Slot</td>
    <td>
      <input type=radio name=short_slot value="1" checked><font id="advShortSlotEnable">Enable &nbsp;</font>
      <input type=radio name=short_slot value="0"><font id="advShortSlotDisable">Disable</font>
    </td>
  </tr>
  <tr> 
    <td class="head" id="advTxBurst">Tx Burst</td>
    <td>
      <input type=radio name=tx_burst value="1" checked><font id="advTxBurstEnable">Enable &nbsp;</font>
      <input type=radio name=tx_burst value="0"><font id="advTxBurstDisable">Disable</font>
    </td>
  </tr>
  <tr> 
    <td class="head" id="advPktAggr">Pkt_Aggregate</td>
    <td>
      <input type=radio name=pkt_aggregate value="1"><font id="advPktAggrEnable">Enable &nbsp;</font>
      <input type=radio name=pkt_aggregate value="0" checked><font id="advPktAggrDisable">Disable</font>
    </td>
  </tr>
-->

  <tr id="adv80211H_tr" style="display:none"> 
    <td class="head" id="adv80211H">IEEE 802.11H Support</td>
    <td>
      <input type=radio name=ieee_80211h value="1" onClick="ieee80211h_switch()"><font id="adv80211HEnable">Enable &nbsp;</font>
      <input type=radio name=ieee_80211h value="0" checked onClick="ieee80211h_switch()"><font id="adv80211HDisable">Disable </font><font color="#808080" id="adv80211HDisableDescribe">(only in A band)</font>
    </td>
  </tr>
  <tr id="div_dfs_rdregion" name="div_dfs_rdregion" disabled="disabled">
    <td class="head" id="advRDRegion">DFS RDRegion</td>
    <td>
    	<select name="rd_region">
	  <option value="FCC">FCC : 1-11 US, TW, HK</option>
	  <option value="JAP">JAP</option>
	  <option value="CE">CE</option>
	</select>
    </td>
  </tr>
  <!--   <tr> 
    <td class="head" id="advCountryCode">Country Code</td>
    <td>
      <select name="country_code" disabled="disable">
        <option value="US" id="advCountryCodeUS">US (United States)</option>
        <option value="JP" id="advCountryCodeJP">JP (Japan)</option>
        <option value="FR" id="advCountryCodeFR">FR (France)</option>
        <option value="TW" id="advCountryCodeTW">TW (Taiwan)</option>
        <option value="IE" id="advCountryCodeIE">IE (Ireland)</option>
        <option value="HK" id="advCountryCodeHK">HK (Hong Kong)</option>
        <option value="NONE" selected id="advCountryCodeNONE">NONE</option>
      </select>
    </td>
  </tr>
  -->
  <tr id="div_carrier_detect" name="div_carrier_detect">
    <td class="head" id="advCarrierDetect">Carrier Detect</td>
    <td>
      <input type="radio" name="carrier_detect" value="1"><font id="advCarrierDetectEnable">Enable</font>&nbsp;&nbsp;
      <input type="radio" name="carrier_detect" value="0" checked><font id="advCarrierDetectDisable">Disable</font>
    </td>
  </tr>
</table>
<br>
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="advWiFiMM">Wi-Fi Multimedia</td>
  </tr>
  <tr> 
    <td class="head" id="advWMM">WMM Capable</td>
    <td>
      <input type=radio name=wmm_capable value="1" onClick="wmm_capable_enable_switch()" checked><font id="advWMMEnable">Enable &nbsp;</font>
      <input type=radio name=wmm_capable value="0" onClick="wmm_capable_enable_switch()"><font id="advWMMDisable">Disable</font>
    </td>
  </tr>
  <tr id="div_apsd_capable" name="div_apsd_capable">
    <td class="head" id="advAPDS">APSD Capable</td>
    <td>
      <input type=radio name=apsd_capable value="1"><font id="advAPDSEnable">Enable &nbsp;</font>
      <input type=radio name=apsd_capable value="0" checked><font id="advAPDSDisable">Disable</font>
    </td>
  </tr>
  <tr id="div_dls_capable" name="div_dls_capable">
    <td class="head" id="advDLS">DLS Capable</td>
    <td>
      <input type=radio name=dls_capable value="1"><font id="advDLSEnable">Enable &nbsp;</font>
      <input type=radio name=dls_capable value="0" checked><font id="advDLSDisable">Disable</font>
    </td>
  </tr>
  <tr> 
    <td class="head" id="advWMMParameter">WMM Parameters</td>
    <td>
      <input type=button name="wmm_list" value="WMM Configuration" id="advWMMConf" onClick="open_wmm_window()">
    </td>
  </tr>
  <input type="hidden" name="rebootAP" value="0">
</table>
<br>
<table id="div_m2u" name="div_m2u" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="advMul2UniConver">Multicast-to-Unicast Converter</td>
  </tr>
  <tr> 
    <td class="head" id="advMul2Uni">Multicast-to-Unicast</td>
    <td>
      <input type=radio name="m2u_enable" value="1"><font id="advMul2UniEnable">Enable &nbsp;</font>
      <input type=radio name="m2u_enable" value="0"><font id="advMul2UniDisable">Disable</font>
    </td>
  </tr>
</table>

<br>
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr id="sbutton0" align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply" id="advApply" > &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="advCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>
</td></tr></table>
 </center>
</div>
</body>
</html>
