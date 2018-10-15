<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<script type="text/javascript" src="/common.js"></script>
<title>Wireless Distribution System</title>

<script language="JavaScript" type="text/javascript">
<!--
Butterlate.setTextDomain("wireless");

restartPage_init();

var sbuttonMax=1;

var wdsMode  = '<% getCfgZero(1, "WdsEnable"); %>';
var wdsList  = '<% getCfgGeneral(1, "WdsList"); %>';
var wdsPhyMode  = '<% getCfgZero(1, "WdsPhyMode"); %>';
var wdsEncrypType  = '<% getCfgGeneral(1, "WdsEncrypType"); %>';
var wdsEncrypKey0  = '<% getCfgGeneral(1, "Wds0Key"); %>';
var wdsEncrypKey1  = '<% getCfgGeneral(1, "Wds1Key"); %>';
var wdsEncrypKey2  = '<% getCfgGeneral(1, "Wds2Key"); %>';
var wdsEncrypKey3  = '<% getCfgGeneral(1, "Wds3Key"); %>';

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

function WdsSecurityOnChange(i)
{
	if (eval("document.wireless_wds.wds_encryp_type"+i).options.selectedIndex >= 1)
		eval("document.wireless_wds.wds_encryp_key"+i).disabled = false;
	else
		eval("document.wireless_wds.wds_encryp_key"+i).disabled = true;
}

function WdsModeOnChange()
{
	document.getElementById("div_wds_phy_mode").style.visibility = "hidden";
	document.getElementById("div_wds_phy_mode").style.display = "none";
	document.wireless_wds.wds_phy_mode.disabled = true;
	document.getElementById("div_wds_encryp_type0").style.visibility = "hidden";
	document.getElementById("div_wds_encryp_type0").style.display = "none";
	document.wireless_wds.wds_encryp_type0.disabled = true;
	document.getElementById("div_wds_encryp_type1").style.visibility = "hidden";
	document.getElementById("div_wds_encryp_type1").style.display = "none";
	document.wireless_wds.wds_encryp_type1.disabled = true;
	document.getElementById("div_wds_encryp_type2").style.visibility = "hidden";
	document.getElementById("div_wds_encryp_type2").style.display = "none";
	document.wireless_wds.wds_encryp_type2.disabled = true;
	document.getElementById("div_wds_encryp_type3").style.visibility = "hidden";
	document.getElementById("div_wds_encryp_type3").style.display = "none";
	document.wireless_wds.wds_encryp_type3.disabled = true;
	document.getElementById("div_wds_encryp_key0").style.visibility = "hidden";
	document.getElementById("div_wds_encryp_key0").style.display = "none";
	document.wireless_wds.wds_encryp_key0.disabled = true;
	document.getElementById("div_wds_encryp_key1").style.visibility = "hidden";
	document.getElementById("div_wds_encryp_key1").style.display = "none";
	document.wireless_wds.wds_encryp_key1.disabled = true;
	document.getElementById("div_wds_encryp_key2").style.visibility = "hidden";
	document.getElementById("div_wds_encryp_key2").style.display = "none";
	document.wireless_wds.wds_encryp_key2.disabled = true;
	document.getElementById("div_wds_encryp_key3").style.visibility = "hidden";
	document.getElementById("div_wds_encryp_key3").style.display = "none";
	document.wireless_wds.wds_encryp_key3.disabled = true;
	document.getElementById("wds_mac_list_1").style.visibility = "hidden";
	document.getElementById("wds_mac_list_1").style.display = "none";
	document.wireless_wds.wds_1.disabled = true;
	document.getElementById("wds_mac_list_2").style.visibility = "hidden";
	document.getElementById("wds_mac_list_2").style.display = "none";
	document.wireless_wds.wds_2.disabled = true;
	document.getElementById("wds_mac_list_3").style.visibility = "hidden";
	document.getElementById("wds_mac_list_3").style.display = "none";
	document.wireless_wds.wds_3.disabled = true;
	document.getElementById("wds_mac_list_4").style.visibility = "hidden";
	document.getElementById("wds_mac_list_4").style.display = "none";
	document.wireless_wds.wds_4.disabled = true;

	if (document.wireless_wds.wds_mode.options.selectedIndex >= 1) {
		document.getElementById("div_wds_phy_mode").style.visibility = "visible";
		document.getElementById("div_wds_phy_mode").style.display = style_display_on();
		document.wireless_wds.wds_phy_mode.disabled = false;
		document.getElementById("div_wds_encryp_type0").style.visibility = "visible";
		document.getElementById("div_wds_encryp_type0").style.display = style_display_on();
		document.wireless_wds.wds_encryp_type0.disabled = false;
		document.getElementById("div_wds_encryp_type1").style.visibility = "visible";
		document.getElementById("div_wds_encryp_type1").style.display = style_display_on();
		document.wireless_wds.wds_encryp_type1.disabled = false;
		document.getElementById("div_wds_encryp_type2").style.visibility = "visible";
		document.getElementById("div_wds_encryp_type2").style.display = style_display_on();
		document.wireless_wds.wds_encryp_type2.disabled = false;
		document.getElementById("div_wds_encryp_type3").style.visibility = "visible";
		document.getElementById("div_wds_encryp_type3").style.display = style_display_on();
		document.wireless_wds.wds_encryp_type3.disabled = false;
		document.getElementById("div_wds_encryp_key0").style.visibility = "visible";
		document.getElementById("div_wds_encryp_key0").style.display = style_display_on();
		document.wireless_wds.wds_encryp_key0.disabled = false;
		document.getElementById("div_wds_encryp_key1").style.visibility = "visible";
		document.getElementById("div_wds_encryp_key1").style.display = style_display_on();
		document.wireless_wds.wds_encryp_key1.disabled = false;
		document.getElementById("div_wds_encryp_key2").style.visibility = "visible";
		document.getElementById("div_wds_encryp_key2").style.display = style_display_on();
		document.wireless_wds.wds_encryp_key2.disabled = false;
		document.getElementById("div_wds_encryp_key3").style.visibility = "visible";
		document.getElementById("div_wds_encryp_key3").style.display = style_display_on();
		document.wireless_wds.wds_encryp_key3.disabled = false;
	}

	WdsSecurityOnChange(0);
	WdsSecurityOnChange(1);
	WdsSecurityOnChange(2);
	WdsSecurityOnChange(3);

	if (document.wireless_wds.wds_mode.options.selectedIndex >= 2) {
		document.getElementById("wds_mac_list_1").style.visibility = "visible";
		document.getElementById("wds_mac_list_1").style.display = style_display_on();
		document.wireless_wds.wds_1.disabled = false;
		document.getElementById("wds_mac_list_2").style.visibility = "visible";
		document.getElementById("wds_mac_list_2").style.display = style_display_on();
		document.wireless_wds.wds_2.disabled = false;
		document.getElementById("wds_mac_list_3").style.visibility = "visible";
		document.getElementById("wds_mac_list_3").style.display = style_display_on();
		document.wireless_wds.wds_3.disabled = false;
		document.getElementById("wds_mac_list_4").style.visibility = "visible";
		document.getElementById("wds_mac_list_4").style.display = style_display_on();
		document.wireless_wds.wds_4.disabled = false;
	}
}

function initTranslation()
{
	var e = document.getElementById("basicWDSTitle");
	e.innerHTML = _("basic wds title");
	
	e = document.getElementById("basicWDSTitle_t");
	e.innerHTML = _("basic wds title");
	
	e = document.getElementById("basicWDSIntroduction");
	e.innerHTML = _("basic wds introduction");
	// ======================================================================
	e = document.getElementById("basicWDSIntroduction2");
	e.innerHTML = _("basic wds introduction2");
		
	e = document.getElementById("basicWDSMode");
	e.innerHTML = _("basic wds mode");
	e = document.getElementById("basicWDSDisable");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("basicWDSPhyMode");
	e.innerHTML = _("basic wds phy mode");
	e = document.getElementById("basicWDSEncrypType");
	e.innerHTML = _("basic wds encryp type");
	e = document.getElementById("basicWDSEncrypKey");
	e.innerHTML = _("basic wds encryp key");

	e = document.getElementById("basicWDSEncrypType1");
	e.innerHTML = _("basic wds encryp type");
	e = document.getElementById("basicWDSEncrypKey1");
	e.innerHTML = _("basic wds encryp key");

	e = document.getElementById("basicWDSEncrypType2");
	e.innerHTML = _("basic wds encryp type");
	e = document.getElementById("basicWDSEncrypKey2");
	e.innerHTML = _("basic wds encryp key");

	e = document.getElementById("basicWDSEncrypType3");
	e.innerHTML = _("basic wds encryp type");
	e = document.getElementById("basicWDSEncrypKey3");
	e.innerHTML = _("basic wds encryp key");

	e = document.getElementById("wdsApply");
	e.value = _("wireless apply");
	e = document.getElementById("wdsCancel");
	e.value = _("wireless cancel");

	e = document.getElementById("basicWDSAPMacAddr1");
	e.innerHTML = _("basic wds ap macaddr");
	e = document.getElementById("basicWDSAPMacAddr2");
	e.innerHTML = _("basic wds ap macaddr");
	e = document.getElementById("basicWDSAPMacAddr3");
	e.innerHTML = _("basic wds ap macaddr");
	e = document.getElementById("basicWDSAPMacAddr4");
	e.innerHTML = _("basic wds ap macaddr");
}

function initValue()
{
	var wdslistArray;
	var wdsEncTypeArray;

	initTranslation();

	wdsMode = 1*wdsMode;
	if (wdsMode == 0)
		document.wireless_wds.wds_mode.options.selectedIndex = 0;
	else if (wdsMode == 4)
		document.wireless_wds.wds_mode.options.selectedIndex = 1;
	else if (wdsMode == 2)
		document.wireless_wds.wds_mode.options.selectedIndex = 2;
	else if (wdsMode == 3)
		document.wireless_wds.wds_mode.options.selectedIndex = 3;

	if (wdsPhyMode.indexOf("CCK") >= 0 || wdsPhyMode.indexOf("cck") >= 0)
		document.wireless_wds.wds_phy_mode.options.selectedIndex = 0;
	else if (wdsPhyMode.indexOf("OFDM") >= 0 || wdsPhyMode.indexOf("ofdm") >= 0)
		document.wireless_wds.wds_phy_mode.options.selectedIndex = 1;
	else if (wdsPhyMode.indexOf("HTMIX") >= 0 || wdsPhyMode.indexOf("htmix") >= 0)
		document.wireless_wds.wds_phy_mode.options.selectedIndex = 2;
	/*
	else if (wdsPhyMode.indexOf("GREENFIELD") >= 0 || wdsPhyMode.indexOf("greenfield") >= 0)
		document.wireless_wds.wds_phy_mode.options.selectedIndex = 3;
	*/

	if (wdsEncrypType != "") {
		wdsEncTypeArray = wdsEncrypType.split(";");
		for (i = 1; i <= wdsEncTypeArray.length; i++) {
			k = i - 1;
			if (wdsEncTypeArray[k] == "NONE" || wdsEncTypeArray[k] == "none")
				eval("document.wireless_wds.wds_encryp_type"+k).options.selectedIndex = 0;
			else if (wdsEncTypeArray[k] == "WEP" || wdsEncTypeArray[k] == "wep")
				eval("document.wireless_wds.wds_encryp_type"+k).options.selectedIndex = 1;
			else if (wdsEncTypeArray[k] == "TKIP" || wdsEncTypeArray[k] == "tkip")
				eval("document.wireless_wds.wds_encryp_type"+k).options.selectedIndex = 2;
			else if (wdsEncTypeArray[k] == "AES" || wdsEncTypeArray[k] == "aes")
				eval("document.wireless_wds.wds_encryp_type"+k).options.selectedIndex = 3;
		}
	}

	WdsModeOnChange();

	document.wireless_wds.wds_encryp_key0.value = wdsEncrypKey0;
	document.wireless_wds.wds_encryp_key1.value = wdsEncrypKey1;
	document.wireless_wds.wds_encryp_key2.value = wdsEncrypKey2;
	document.wireless_wds.wds_encryp_key3.value = wdsEncrypKey3;

	if (wdsList != "") {
		wdslistArray = wdsList.split(";");
		for (i = 1; i <= wdslistArray.length; i++)
			eval("document.wireless_wds.wds_"+i).value = wdslistArray[i - 1];
	}
}

function CheckEncKey(i)
{
	var key;
	key = eval("document.wireless_wds.wds_encryp_key"+i).value;

	if (eval("document.wireless_wds.wds_encryp_type"+i).options.selectedIndex == 1) {
		if (key.length == 10 || key.length == 26) {
			var re = /[A-Fa-f0-9]{10,26}/;
			if (!re.test(key)) {
				alert(_("wds alert wds")+i+_("wds alert key should be 10/26 hex or 5/13 ascii"));
				eval("document.wireless_wds.wds_encryp_key"+i).focus();
				eval("document.wireless_wds.wds_encryp_key"+i).select();
				return false;
			}
			else
				return true;
		}
		else if (key.length == 5 || key.length == 13) {
			return true;
		}
		else {
			alert(_("wds alert wds")+i+_("wds alert key should be 10/26 hex or 5/13 ascii"));
			eval("document.wireless_wds.wds_encryp_key"+i).focus();
			eval("document.wireless_wds.wds_encryp_key"+i).select();
			return false;
		}
	}
	else if (eval("document.wireless_wds.wds_encryp_type"+i).options.selectedIndex == 2 ||
			eval("document.wireless_wds.wds_encryp_type"+i).options.selectedIndex == 3)
	{
		if (key.length < 8 || key.length > 64) {
			alert(_("wds alert wds")+i+_("wds alert key should be with 8 to 64 length"));
			eval("document.wireless_wds.wds_encryp_key"+i).focus();
			eval("document.wireless_wds.wds_encryp_key"+i).select();
			return false;
		}
		if (key.length == 64) {
			var re = /[A-Fa-f0-9]{64}/;
			if (!re.test(key)) {
				alert(_("wds alert wds")+i+_("wds alert key should be 64 hex"));
				eval("document.wireless_wds.wds_encryp_key"+i).focus();
				eval("document.wireless_wds.wds_encryp_key"+i).select();
				return false;
			}
			else
				return true;
		}
		else
			return true;
	}
	return true;
}

function CheckValue()
{
	var all_wds_list;
	var all_wds_enc_type;

	all_wds_enc_type = document.wireless_wds.wds_encryp_type0.value+";"+
		document.wireless_wds.wds_encryp_type1.value+";"+
		document.wireless_wds.wds_encryp_type2.value+";"+
		document.wireless_wds.wds_encryp_type3.value;
	document.wireless_wds.wds_encryp_type.value = all_wds_enc_type;

	if (!CheckEncKey(0) || !CheckEncKey(1) || !CheckEncKey(2) || !CheckEncKey(3))
		return false;

	all_wds_list = '';
	if (document.wireless_wds.wds_mode.options.selectedIndex >= 2)
	{
		var re = /[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}/;
		for (i = 1; i <= 4; i++)
		{
			if (eval("document.wireless_wds.wds_"+i).value.toLowerCase() == "ff:ff:ff:ff:ff:ff")
			{
				alert(_("wds alert boradcast string is not allowed"));
				eval("document.wireless_wds.wds_"+i).focus();
				eval("document.wireless_wds.wds_"+i).select();
				return false;
			}
			if (eval("document.wireless_wds.wds_"+i).value == "")
				continue;
			if (!re.test(eval("document.wireless_wds.wds_"+i).value)) {
				alert(_("wds alert please fill wds remote ap mac address in correct format"));
				eval("document.wireless_wds.wds_"+i).focus();
				eval("document.wireless_wds.wds_"+i).select();
				return false;
			}
			else {
				all_wds_list += eval("document.wireless_wds.wds_"+i).value;
				all_wds_list += ';';
			}
		}
		var buf;
		for (x = 1; x <=4; x++)
		{
			buf = eval("document.wireless_wds.wds_" + x).value.toLowerCase();
			for (i = 1; i <= 4; i++)
			{
				if (x == i || buf == "") continue;
				if (buf == eval("document.wireless_wds.wds_" + i).value.toLowerCase())
				{
					alert(_("wds alert wds mac address cannot be the same"));
					eval("document.wireless_wds.wds_" + i).focus();
					eval("document.wireless_wds.wds_" + i).select();
					return false;
				}
			}
		}
		if (all_wds_list == "")
		{
			alert(_("wds alert wds remote ap mac address are empty"));
			document.wireless_wds.wds_1.focus();
			document.wireless_wds.wds_1.select(); 
			return false;
		}
		else
		{
			document.wireless_wds.wds_list.value = all_wds_list;
			document.wireless_wds.wds_1.disabled = true;
			document.wireless_wds.wds_2.disabled = true;
			document.wireless_wds.wds_3.disabled = true;
			document.wireless_wds.wds_4.disabled = true;
		}
	}

	sbutton_disable(sbuttonMax); 
	restartPage_block();

	return true;
}
-->
</script>
</head>


<body onLoad="initValue()" bgcolor="#FFFFFF">
<div align="center">
 <center>
<table class="body"><tr><td>


<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="basicWDSTitle">Wireless Distribution System</td>
</tr>


<tr>
<td colspan="1" bgcolor="#DBE2EC" >
<p class="head" id="basicWDSIntroduction">The Wireless Distribution page allows configuration of WDS parameters for the purpose of bridging or creating a repeater application. </p>
<p class="head" id="basicWDSIntroduction2">When using WDS Lazy mode in the network, at last one unit must be set Bridge mode or Repeater mode.  </p>
</td>
</tr>
</table>

<form method=post name=wireless_wds action="/goform/wirelessWds" onSubmit="return CheckValue()">

<table width="540" border="1" cellspacing="1" cellpadding="3">
  <tr>
    <td class="title" id="basicWDSTitle_t" colspan="2">Wireless Distribution System(WDS)</td>
  </tr>
  <tr>
    <td class="head" id="basicWDSMode">WDS Mode</td>
    <td>
      <select name="wds_mode" id="wds_mode" size="1" onchange="WdsModeOnChange()">
	<option value=0 id="basicWDSDisable">Disable</option>
	<option value=4>Lazy Mode</option>
	<option value=2>Bridge Mode</option>
	<option value=3>Repeater Mode</option>
      </select>
    </td>
  </tr>
  <tr id="div_wds_phy_mode" name="div_wds_phy_mode"> 
    <td class="head" id="basicWDSPhyMode">Phy Mode</td>
    <td>
      <select name="wds_phy_mode" id="wds_phy_mode" size="1">
	<option value="CCK;CCK;CCK;CCK">CCK</option>
	<option value="OFDM;OFDM;OFDM;OFDM">OFDM</option>
	<option value="HTMIX;HTMIX;HTMIX;HTMIX">HTMIX</option>
	<!--
	<option value="GREENFIELD;GREENFIELD;GREENFIELD;GREENFIELD">GREENFIELD</option>
	-->
      </select>
    </td>
  </tr>
<tr><td></td><td></td></tr>  
</table>

<table width="540" border="1" cellspacing="1" cellpadding="3">
  <tr id="div_wds_encryp_type0" name="div_wds_encryp_type0">
    <td class="head" id="basicWDSEncrypType">EncrypType</td>
    <td>
      <select name="wds_encryp_type0" id="wds_encryp_type0" size="1" onchange="WdsSecurityOnChange(0)">
	<option value="NONE">NONE</option>
	<option value="WEP">WEP</option>
	<option value="TKIP">TKIP</option>
	<option value="AES">AES</option>
      </select>
    </td>
  </tr>
  <tr id="div_wds_encryp_key0" name="div_wds_encryp_key0">
    <td class="head" id="basicWDSEncrypKey">Encryp Key</td>
    <td><input type="password" name=wds_encryp_key0 id=wds_encryp_key0 size=28 maxlength=64 value="" ></td>
  </tr>

  <tr id="wds_mac_list_1" name="wds_mac_list_1">
    <td class="head" id="basicWDSAPMacAddr1">AP MAC Address</td>
    <td><input type=text name=wds_1 size=20 maxlength=17 value=""></td>
  </tr>
<tr><td></td><td></td></tr>
</table>

<table width="540" border="1" cellspacing="1" cellpadding="3">  
  <tr id="div_wds_encryp_type1" name="div_wds_encryp_type1">
    <td class="head" id="basicWDSEncrypType1">EncrypType</td>
    <td>
      <select name="wds_encryp_type1" id="wds_encryp_type1" size="1" onchange="WdsSecurityOnChange(1)">
	<option value="NONE">NONE</option>
	<option value="WEP">WEP</option>
	<option value="TKIP">TKIP</option>
	<option value="AES">AES</option>
      </select>
    </td>
  </tr>
  <tr id="div_wds_encryp_key1" name="div_wds_encryp_key1">
    <td class="head" id="basicWDSEncrypKey1">Encryp Key</td>
    <td><input type="password" name=wds_encryp_key1 id=wds_encryp_key1 size=28 maxlength=64 value="" ></td>
  </tr>

  <tr id="wds_mac_list_2" name="wds_mac_list_2">
    <td class="head" id="basicWDSAPMacAddr2">AP MAC Address</td>
    <td><input type=text name=wds_2 size=20 maxlength=17 value=""></td>
  </tr>
<tr><td></td><td></td></tr>  
</table>

<table width="540" border="1" cellspacing="1" cellpadding="3">
  <tr id="div_wds_encryp_type2" name="div_wds_encryp_type2">
    <td class="head" id="basicWDSEncrypType2">EncrypType</td>
    <td>
      <select name="wds_encryp_type2" id="wds_encryp_type2" size="1" onchange="WdsSecurityOnChange(2)">
	<option value="NONE">NONE</option>
	<option value="WEP">WEP</option>
	<option value="TKIP">TKIP</option>
	<option value="AES">AES</option>
      </select>
    </td>
  </tr>
  <tr id="div_wds_encryp_key2" name="div_wds_encryp_key2">
    <td class="head" id="basicWDSEncrypKey2">Encryp Key</td>
    <td><input type="password" name=wds_encryp_key2 id=wds_encryp_key2 size=28 maxlength=64 value=""></td>
  </tr>

  <tr id="wds_mac_list_3" name="wds_mac_list_3">
    <td class="head" id="basicWDSAPMacAddr3">AP MAC Address</td>
    <td><input type=text name=wds_3 size=20 maxlength=17 value=""></td>
  </tr>
<tr><td></td><td></td></tr>  
</table>

<table width="540" border="1" cellspacing="1" cellpadding="3" >
  <tr id="div_wds_encryp_type3" name="div_wds_encryp_type3">
    <td class="head" id="basicWDSEncrypType3">EncrypType</td>
    <td>
      <select name="wds_encryp_type3" id="wds_encryp_type3" size="1" onchange="WdsSecurityOnChange(3)">
	<option value="NONE">NONE</option>
	<option value="WEP">WEP</option>
	<option value="TKIP">TKIP</option>
	<option value="AES">AES</option>
      </select>
    </td>
  </tr>
  <tr id="div_wds_encryp_key3" name="div_wds_encryp_key3">
    <td class="head" id="basicWDSEncrypKey3">Encryp Key</td>
    <td><input type="password" name=wds_encryp_key3 id=wds_encryp_key3 size=28 maxlength=64 value=""></td>
  </tr>
  <input type="hidden" name="wds_encryp_type" value="">

  <tr id="wds_mac_list_4" name="wds_mac_list_4">
    <td class="head" id="basicWDSAPMacAddr4">AP MAC Address</td>
    <td><input type=text name=wds_4 size=20 maxlength=17 value=""></td>
  </tr>
  <input type="hidden" name="wds_list" value="">
<tr><td></td><td></td></tr>  
</table>

<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr id="sbutton0" align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply" id="wdsApply" onClick=""> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="wdsCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
 </center>
</div>
</body>
</html>
