<html><head><!-- Copyright (c), Ralink Technology Corporation All Rights Reserved. -->

<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<!--<META HTTP-EQUIV="refresh" CONTENT="3; URL=/user/user_awb_index_home.asp">-->

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script type="text/javascript" src="/common.js"></script>

<title>Ralink Wireless Security Settings</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("user");

var MBSSID_MAX 				= 8;
var ACCESSPOLICYLIST_MAX	= 32;

var changed = 0;
var old_MBSSID;

var defaultShownMBSSID = 0;
var SSID = new Array();
var PreAuth = new Array();
var AuthMode = new Array();
var EncrypType = new Array();
var DefaultKeyID = new Array();
var Key1Type = new Array();
var Key1Str = new Array();
var Key2Type = new Array();
var Key2Str = new Array();
var Key3Type = new Array();
var Key3Str = new Array();
var Key4Type = new Array();
var Key4Str = new Array();
var WPAPSK = new Array();
var RekeyMethod = new Array();
var RekeyInterval = new Array();
var PMKCachePeriod = new Array();
var IEEE8021X = new Array();
var RADIUS_Server = new Array();
var RADIUS_Port = new Array();
var RADIUS_Key = new Array();
var session_timeout_interval = new Array();
var AccessPolicy = new Array();
var AccessControlList = new Array();

function style_display_on()
{
	if (window.ActiveXObject) { // IE
		return "block";
	}
	else if (window.XMLHttpRequest) { // Mozilla, Safari,...
		return "table-row";
	}
}

var http_request = false;
function makeRequest(url, content, handler) {
	http_request = false;
	if (window.XMLHttpRequest) { // Mozilla, Safari,...
		http_request = new XMLHttpRequest();
		if (http_request.overrideMimeType) {
			http_request.overrideMimeType('text/xml');
		}
	} else if (window.ActiveXObject) { // IE
		try {
			http_request = new ActiveXObject("Msxml2.XMLHTTP");
		} catch (e) {
			try {
			http_request = new ActiveXObject("Microsoft.XMLHTTP");
			} catch (e) {}
		}
	}
	if (!http_request) {
		alert('Giving up :( Cannot create an XMLHTTP instance');
		return false;
	}
	http_request.onreadystatechange = handler;
	http_request.open('POST', url, true);
	http_request.send(content);
}

function securityHandler() {
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			parseAllData(http_request.responseText);
			UpdateMBSSIDList();
			LoadFields(defaultShownMBSSID);

			// load Access Policy for MBSSID[selected]
			LoadAP();
			ShowAP(defaultShownMBSSID);
		} else {
			alert('There was a problem with the request.');
		}
	}
}

function parseAllData(str)
{
	var all_str = new Array();
	all_str = str.split("\n");

	defaultShownMBSSID = parseInt(all_str[0]);

	for (var i=0; i<all_str.length-2; i++) {
		var fields_str = new Array();
		fields_str = all_str[i+1].split("\r");

		SSID[i] = fields_str[0];
		PreAuth[i] = fields_str[1];
		AuthMode[i] = fields_str[2];
		EncrypType[i] = fields_str[3];
		DefaultKeyID[i] = fields_str[4];
		Key1Type[i] = fields_str[5];
		Key1Str[i] = fields_str[6];
		Key2Type[i] = fields_str[7];
		Key2Str[i] = fields_str[8];
		Key3Type[i] = fields_str[9];
		Key3Str[i] = fields_str[10];
		Key4Type[i] = fields_str[11];
		Key4Str[i] = fields_str[12];
		WPAPSK[i] = fields_str[13];
		RekeyMethod[i] = fields_str[14];
		RekeyInterval[i] = fields_str[15];
		PMKCachePeriod[i] = fields_str[16];
		IEEE8021X[i] = fields_str[17];
		RADIUS_Server[i] = fields_str[18];
		RADIUS_Port[i] = fields_str[19];
		RADIUS_Key[i] = fields_str[20];
		session_timeout_interval[i] = fields_str[21];
		AccessPolicy[i] = fields_str[22];
		AccessControlList[i] = fields_str[23];

		/* !!!! IMPORTANT !!!!*/
		if(IEEE8021X[i] == "1")
			AuthMode[i] = "IEEE8021X";

		if(AuthMode[i] == "OPEN" && EncrypType[i] == "NONE" && IEEE8021X[i] == "0")
			AuthMode[i] = "Disable";
	}
}

function securityMode(c_f)
{
	var security_mode;


	changed = c_f;

	hideWep();


	document.getElementById("div_security_open_mode").style.visibility = "hidden";
	document.getElementById("div_security_open_mode").style.display = "none";
	document.getElementById("div_security_infra_mode").style.visibility = "hidden";
	document.getElementById("div_security_infra_mode").style.display = "none";
	document.getElementById("div_wpa").style.visibility = "hidden";
	document.getElementById("div_wpa").style.display = "none";
	document.getElementById("div_wpa_algorithms").style.visibility = "hidden";
	document.getElementById("div_wpa_algorithms").style.display = "none";
	document.getElementById("wpa_passphrase").style.visibility = "hidden";
	document.getElementById("wpa_passphrase").style.display = "none";
	document.getElementById("wpa_key_renewal_interval").style.visibility = "hidden";
	document.getElementById("wpa_key_renewal_interval").style.display = "none";
	document.getElementById("wpa_PMK_Cache_Period").style.visibility = "hidden";
	document.getElementById("wpa_PMK_Cache_Period").style.display = "none";
	document.getElementById("wpa_preAuthentication").style.visibility = "hidden";
	document.getElementById("wpa_preAuthentication").style.display = "none";
	document.security_form.passphrase.disabled = true;
	document.security_form.keyRenewalInterval.disabled = true;
	document.security_form.PMKCachePeriod.disabled = true;
	document.security_form.PreAuthentication.disabled = true;

	// 802.1x
	document.getElementById("div_radius_server").style.visibility = "hidden";
	document.getElementById("div_radius_server").style.display = "none";
	document.getElementById("div_8021x_wep").style.visibility = "hidden";
	document.getElementById("div_8021x_wep").style.display = "none";
	document.security_form.ieee8021x_wep.disable = true;
	document.security_form.RadiusServerIP.disable = true;
	document.security_form.RadiusServerPort.disable = true;
	document.security_form.RadiusServerSecret.disable = true;	
	document.security_form.RadiusServerSessionTimeout.disable = true;
	document.security_form.RadiusServerIdleTimeout.disable = true;	

	security_mode = document.security_form.security_mode.value;

	if (security_mode == "OPEN" || security_mode == "SHARED" ||security_mode == "WEPAUTO"){
		showWep(security_mode);
	}else if (security_mode == "WPAPSK" || security_mode == "WPA2PSK" || security_mode == "WPAPSKWPA2PSK"){
		<!-- WPA -->
		document.getElementById("div_wpa").style.visibility = "visible";
		if (window.ActiveXObject) { // IE
			document.getElementById("div_wpa").style.display = "block";
		}
		else if (window.XMLHttpRequest) { // Mozilla, Safari,...
			document.getElementById("div_wpa").style.display = "table";
		}

		document.getElementById("div_wpa_algorithms").style.visibility = "visible";
		document.getElementById("div_wpa_algorithms").style.display = style_display_on();

		document.getElementById("wpa_passphrase").style.visibility = "visible";
		document.getElementById("wpa_passphrase").style.display = style_display_on();
		document.security_form.passphrase.disabled = false;

		document.getElementById("wpa_key_renewal_interval").style.visibility = "visible";
		document.getElementById("wpa_key_renewal_interval").style.display = style_display_on();
		document.security_form.keyRenewalInterval.disabled = false;
	}else if (security_mode == "WPA" || security_mode == "WPA2" || security_mode == "WPA1WPA2") //wpa enterprise
	{
		document.getElementById("div_wpa").style.visibility = "visible";
		if (window.ActiveXObject) { // IE
			document.getElementById("div_wpa").style.display = "block";
		}else if (window.XMLHttpRequest) { // Mozilla, Safari,...
			document.getElementById("div_wpa").style.display = "table";
		}

		document.getElementById("div_wpa_algorithms").style.visibility = "visible";
		document.getElementById("div_wpa_algorithms").style.display = style_display_on();
		document.getElementById("wpa_key_renewal_interval").style.visibility = "visible";
		document.getElementById("wpa_key_renewal_interval").style.display = style_display_on();
		document.security_form.keyRenewalInterval.disabled = false;
	
		<!-- 802.1x -->
		document.getElementById("div_radius_server").style.visibility = "visible";
		document.getElementById("div_radius_server").style.display = style_display_on();
		document.security_form.RadiusServerIP.disable = false;
		document.security_form.RadiusServerPort.disable = false;
		document.security_form.RadiusServerSecret.disable = false;	
		document.security_form.RadiusServerSessionTimeout.disable = false;
		document.security_form.RadiusServerIdleTimeout.disable = false;	
		document.security_form.RadiusServerIdleTimeout.readOnly = false;

		if(security_mode == "WPA2"){
			document.getElementById("wpa_preAuthentication").style.visibility = "visible";
			document.getElementById("wpa_preAuthentication").style.display = style_display_on();
			document.security_form.PreAuthentication.disabled = false;
			document.getElementById("wpa_PMK_Cache_Period").style.visibility = "visible";
			document.getElementById("wpa_PMK_Cache_Period").style.display = style_display_on();
			document.security_form.PMKCachePeriod.disabled = false;
		}

		if(security_mode == "WPA1WPA2"){

		}

	}else if (security_mode == "IEEE8021X"){ // 802.1X-WEP
		document.getElementById("div_8021x_wep").style.visibility = "visible";
		document.getElementById("div_8021x_wep").style.display = style_display_on();

		document.getElementById("div_radius_server").style.visibility = "visible";
		document.getElementById("div_radius_server").style.display = style_display_on();
		document.security_form.ieee8021x_wep.disable = false;
		document.security_form.RadiusServerIP.disable = false;
		document.security_form.RadiusServerPort.disable = false;
		document.security_form.RadiusServerSecret.disable = false;	
		document.security_form.RadiusServerSessionTimeout.disable = false;
		//document.security_form.RadiusServerIdleTimeout.disable = false;
	}
}


function hideWep()
{
	document.getElementById("div_wep").style.visibility = "hidden";
	document.getElementById("div_wep").style.display = "none";
}
function showWep(mode)
{
	<!-- WEP -->
	document.getElementById("div_wep").style.visibility = "visible";

	if (window.ActiveXObject) { // IE 
		document.getElementById("div_wep").style.display = "block";
	}
	else if (window.XMLHttpRequest) { // Mozilla, Safari...
		document.getElementById("div_wep").style.display = "table";
	}

	if(mode == "OPEN"){
		document.getElementById("div_security_open_mode").style.visibility = "visible";
		document.getElementById("div_security_open_mode").style.display = style_display_on();
	}
	//document.security_form.wep_auth_type.disabled = false;
}

function LoadFields(MBSSID)
{
	var result;
	var default_wep_key_index;
	// Security Policy
	sp_select = document.getElementById("security_mode");

	sp_select.options.length = 0;

    sp_select.options[sp_select.length] = new Option("Disable",	"Disable",	false, AuthMode[MBSSID] == "Disable");
    sp_select.options[sp_select.length] = new Option("OPEN",	"OPEN",		false, AuthMode[MBSSID] == "OPEN");
    sp_select.options[sp_select.length] = new Option("SHARED",	"SHARED", 	false, AuthMode[MBSSID] == "SHARED");
    sp_select.options[sp_select.length] = new Option("WEPAUTO", "WEPAUTO",	false, AuthMode[MBSSID] == "WEPAUTO");
    sp_select.options[sp_select.length] = new Option("WPA",		"WPA",		false, AuthMode[MBSSID] == "WPA");
    sp_select.options[sp_select.length] = new Option("WPA-PSK", "WPAPSK",	false, AuthMode[MBSSID] == "WPAPSK");
    sp_select.options[sp_select.length] = new Option("WPA2",	"WPA2",		false, AuthMode[MBSSID] == "WPA2");
    sp_select.options[sp_select.length] = new Option("WPA2-PSK","WPA2PSK",	false, AuthMode[MBSSID] == "WPA2PSK");
    sp_select.options[sp_select.length] = new Option("WPAPSKWPA2PSK","WPAPSKWPA2PSK",	false, AuthMode[MBSSID] == "WPAPSKWPA2PSK");
    sp_select.options[sp_select.length] = new Option("WPA1WPA2","WPA1WPA2",	false, AuthMode[MBSSID] == "WPA1WPA2");

	/* 
	 * until now we only support 8021X WEP for MBSSID[0]
	 */
	/*if(MBSSID == 0)*/
		sp_select.options[sp_select.length] = new Option("802.1X",	"IEEE8021X",false, AuthMode[MBSSID] == "IEEE8021X");

	document.getElementById("security_mode_show").value = AuthMode[MBSSID];

	// WEP
	document.getElementById("WEP1").value = Key1Str[MBSSID];
	document.getElementById("WEP2").value = Key2Str[MBSSID];
	document.getElementById("WEP3").value = Key3Str[MBSSID];
	document.getElementById("WEP4").value = Key4Str[MBSSID];

	if (Key1Type[MBSSID] == "0")
		document.getElementById("WEP1Select").value ="Hex";
	else
		document.getElementById("WEP1Select").value ="ASCII";
	
	if (Key2Type[MBSSID] == "0")
		document.getElementById("WEP2Select").value ="Hex";
	else
		document.getElementById("WEP2Select").value ="ASCII";

	if (Key3Type[MBSSID] == "0")
		document.getElementById("WEP3Select").value ="Hex";
	else
		document.getElementById("WEP3Select").value ="ASCII";

	if (Key4Type[MBSSID] == "0")
		document.getElementById("WEP4Select").value ="Hex";
	else
		document.getElementById("WEP4Select").value ="ASCII";
		

	default_wep_key_index=parseInt(DefaultKeyID[MBSSID]);
	
	if (default_wep_key_index==1)
		document.getElementById("wep_default_key").value =_("secure wep default key1");
	else if (default_wep_key_index==2)
		document.getElementById("wep_default_key").value =_("secure wep default key2");
	else if (default_wep_key_index==3)
		document.getElementById("wep_default_key").value =_("secure wep default key3");
	else if (default_wep_key_index==4)
		document.getElementById("wep_default_key").value =_("secure wep default key4");
	else
		document.getElementById("wep_default_key").value =_("secure wep default key1");


	// SHARED && NONE
	if(AuthMode[MBSSID] == "OPEN" && EncrypType[MBSSID] == "NONE")
		document.getElementById("security_open_mode").value = "None";
	else
		document.getElementById("security_open_mode").value = "WEP";

	// WPA
	if(EncrypType[MBSSID] == "TKIP")
		document.getElementById("cipher").value ="TKIP";
	else if(EncrypType[MBSSID] == "AES")
		document.getElementById("cipher").value ="AES";
	else if(EncrypType[MBSSID] == "TKIPAES")
		document.getElementById("cipher").value ="TKIPAES";

	document.getElementById("passphrase").value = WPAPSK[MBSSID];
	document.getElementById("keyRenewalInterval").value = RekeyInterval[MBSSID];
	document.getElementById("PMKCachePeriod").value = PMKCachePeriod[MBSSID];
	//document.getElementById("PreAuthentication").value = PreAuth[MBSSID];
	if(PreAuth[MBSSID] == "0")
		document.getElementById("PreAuthentication").value =_("wireless disable");
	else
		document.getElementById("PreAuthentication").value =_("wireless enable");

	//802.1x wep
	if(IEEE8021X[MBSSID] == "1"){
		if(EncrypType[MBSSID] == "WEP")
			document.getElementById("ieee8021x_wep").value =_("wireless enable");
		else
			document.getElementById("ieee8021x_wep").value =_("wireless disable");
	}
	
	document.getElementById("RadiusServerIP").value = RADIUS_Server[MBSSID];
	document.getElementById("RadiusServerPort").value = RADIUS_Port[MBSSID];
	document.getElementById("RadiusServerSecret").value = RADIUS_Key[MBSSID];			
	document.getElementById("RadiusServerSessionTimeout").value = session_timeout_interval[MBSSID];
	
	securityMode(0);

}


function ShowAP(MBSSID)
{
	var i;
	var tmp;
	for(i=0; i<MBSSID_MAX; i++){
		/*document.getElementById("apselect_"+i).selectedIndex	= AccessPolicy[i];*/
		
		tmp=AccessPolicy[i];
		if (tmp==1)
			document.getElementById("apselect_"+i).value	= _("wireless allow");
		else if (tmp==2)
			document.getElementById("apselect_"+i).value	= _("wireless reject");
		else
			document.getElementById("apselect_"+i).value	= _("wireless disable");

		document.getElementById("AccessPolicy_"+i).style.visibility = "hidden";
		document.getElementById("AccessPolicy_"+i).style.display = "none";
	}

	document.getElementById("AccessPolicy_"+MBSSID).style.visibility = "visible";
	if (window.ActiveXObject) {			// IE
		document.getElementById("AccessPolicy_"+MBSSID).style.display = "block";
	}else if (window.XMLHttpRequest) {	// Mozilla, Safari,...
		document.getElementById("AccessPolicy_"+MBSSID).style.display = "table";
	}
}

function LoadAP()
{
	for(var i=0; i<SSID.length; i++){
		var j=0;
		var aplist = new Array;

		if(AccessControlList[i].length != 0){
			aplist = AccessControlList[i].split(";");
			for(j=0; j<aplist.length; j++){
				document.getElementById("newap_"+i+"_"+j).value = aplist[j];
			}

			// hide the lastest <td>
			if(j%2){
				document.getElementById("newap_td_"+i+"_"+j).style.visibility = "hidden";
				document.getElementById("newap_td_"+i+"_"+j).style.display = "none";
				j++;
			}
		}

		// hide <tr> left
		for(; j<ACCESSPOLICYLIST_MAX; j+=2){
			document.getElementById("id_"+i+"_"+j).style.visibility = "hidden";
			document.getElementById("id_"+i+"_"+j).style.display = "none";
		}
	}
}

function selectMBSSIDChanged()

{
	// check if any security settings changed
	if(changed){
		ret = confirm("Are you sure to ignore changed?");
		if(!ret){
			document.security_form.ssidIndex.options.selectedIndex = old_MBSSID;
			return false;
		}
		else
			changed = 0;
	}

	var selected = document.security_form.ssidIndex.options.selectedIndex;

	// backup for user cancel action
	old_MBSSID = selected;

	MBSSIDChange(selected);
}

/*
 * When user select the different SSID, this function would be called.
 */ 
function MBSSIDChange(selected)
{
	// load wep/wpa/802.1x table for MBSSID[selected]
	LoadFields(selected);

	// update Access Policy for MBSSID[selected]
	ShowAP(selected);

	// radio button special case
	WPAAlgorithms = EncrypType[selected];
	IEEE8021XWEP = IEEE8021X[selected];
	PreAuthentication = PreAuth[selected];

	changeSecurityPolicyTableTitle(SSID[selected]);

	// clear all new access policy list field
	/*for(i=0; i<MBSSID_MAX; i++)
		document.getElementById("newap_text_"+i).value = "";*/

	return true;
}

function changeSecurityPolicyTableTitle(t)
{
	var title = document.getElementById("sp_title");
	title.innerHTML = "\"" + t + "\"";
}

function showOpMode()
{
	var opmode = 1* <% getCfgZero(1, "OperationMode"); %>;
	if (opmode == 0)
		document.write("Bridge Mode");
	else if (opmode == 1)
		document.write("Gateway Mode");
	else if (opmode == 2)
		document.write("Ethernet Converter Mode");
	else if (opmode == 3)
		document.write("AP Client Mode");
	else
		document.write("Unknown");
}

function initTranslation()
{
	var e = document.getElementById("secureSelectSSID");
	e.innerHTML = _("secure select ssid");
	e = document.getElementById("secureSSIDChoice");
	e.innerHTML = _("secure ssid choice");

	e = document.getElementById("status title");
	e.innerHTML = _("status title");
	e = document.getElementById("status introduction");
	e.innerHTML = _("status introduction");
	e = document.getElementById("sp_title");
	e.innerHTML = _("secure security policy");
	e = document.getElementById("secureSecureMode_show");
	e.innerHTML = _("secure security mode");
	e = document.getElementById("secureEncrypType");
	e.innerHTML = _("secure encryp type");

	e = document.getElementById("secureWEP");
	e.innerHTML = _("secure wep");
	e = document.getElementById("secureWEPDefaultKey");
	e.innerHTML = _("secure wep default key");
	e = document.getElementById("secureWEPKey");
	e.innerHTML = _("secure wep key");
	e = document.getElementById("secureWEPKey1");
	e.innerHTML = _("secure wep key1");
	e = document.getElementById("secureWEPKey2");
	e.innerHTML = _("secure wep key2");
	e = document.getElementById("secureWEPKey3");
	e.innerHTML = _("secure wep key3");
	e = document.getElementById("secureWEPKey4");
	e.innerHTML = _("secure wep key4");
	
	e = document.getElementById("secreWPA");
	e.innerHTML = _("secure wpa");
	e = document.getElementById("secureWPAAlgorithm");
	e.innerHTML = _("secure wpa algorithm");
	e = document.getElementById("secureWPAPassPhrase");
	e.innerHTML = _("secure wpa pass phrase");
	e = document.getElementById("secureWPAKeyRenewInterval");
	e.innerHTML = _("secure wpa key renew interval");
	e = document.getElementById("secureWPAPMKCachePeriod");
	e.innerHTML = _("secure wpa pmk cache period");
	e = document.getElementById("secureWPAPreAuth");
	e.innerHTML = _("secure wpa preauth");
	
	e = document.getElementById("secure8021XWEP");
	e.innerHTML = _("secure 8021x wep");
	e = document.getElementById("secure1XWEP");
	e.innerHTML = _("secure 1x wep");
	
	e = document.getElementById("secureRadius");
	e.innerHTML = _("secure radius");
	e = document.getElementById("secureRadiusIPAddr");
	e.innerHTML = _("secure radius ipaddr");
	e = document.getElementById("secureRadiusPort");
	e.innerHTML = _("secure radius port");
	e = document.getElementById("secureRadiusSharedSecret");
	e.innerHTML = _("secure radius shared secret");
	e = document.getElementById("secureRadiusSessionTimeout");
	e.innerHTML = _("secure radius session timeout");
	e = document.getElementById("secureRadiusIdleTimeout");
	e.innerHTML = _("secure radius idle timeout");
	
	/* ----------------------------------------------------- */	
	
	e = document.getElementById("statusSysInfo");
	e.innerHTML = _("status system information");
	e = document.getElementById("statusSDKVersion");
	e.innerHTML = _("status sdk version");
	e = document.getElementById("statusOPMode");
	e.innerHTML = _("status operate mode");

	e = document.getElementById("statusInternetConfig");
	e.innerHTML = _("status internet config");
	e = document.getElementById("statusConnectedType");
	e.innerHTML = _("status connect type");
	e = document.getElementById("statusWANIPAddr");
	e.innerHTML = _("status wan ipaddr");
	e = document.getElementById("statusSubnetMask");
	e.innerHTML = _("status subnet mask");
	e = document.getElementById("statusDefaultGW");
	e.innerHTML = _("status default gateway");
	e = document.getElementById("statusPrimaryDNS");
	e.innerHTML = _("status primary dns");
	e = document.getElementById("statusSecondaryDNS");
	e.innerHTML = _("status secondary dns");
	e = document.getElementById("statusWANMAC");
	e.innerHTML = _("status mac");

	e = document.getElementById("statusLocalNet");
	e.innerHTML = _("status local network");
	e = document.getElementById("statusLANIPAddr");
	e.innerHTML = _("status lan ipaddr");
	e = document.getElementById("statusLocalNetmask");
	e.innerHTML = _("status local netmask");
	e = document.getElementById("statusLANMAC");
	e.innerHTML = _("status mac");
}

function initAll()
{
	initTranslation();
	makeRequest("/goform/wirelessGetSecurity", "n/a", securityHandler);
}

function UpdateMBSSIDList()
{
	document.security_form.ssidIndex.length = 0;

	for(var i=0; i<SSID.length; i++){
		var j = document.security_form.ssidIndex.options.length;
		document.security_form.ssidIndex.options[j] = new Option(SSID[i], i, false, false);
	}
	
	document.security_form.ssidIndex.options.selectedIndex = defaultShownMBSSID;
	old_MBSSID = defaultShownMBSSID;
	changeSecurityPolicyTableTitle(SSID[defaultShownMBSSID]);
}


</script>
</head>
<body onload="initAll()" bgcolor="#FFFFFF">
<div align="center">
 <center>
<table class="body"><tbody><tr><td>

<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="status title">Status</td>
</tr>
<tr>

<tr>
<td colspan="2">
<p class="head" id="status introduction">Displays the status of the device.</p>
</td>
<tr>
</table>

<br>

<table width="540" border="1" cellpadding="2" cellspacing="1">
<!-- ================= System Info ================= -->
<tr>
  <td class="title" colspan="2" id="statusSysInfo">System Info</td>
</tr>
<tr>
  <td class="head" id="statusSDKVersion">Firmware Version</td>
  <td><% getAWBVersion(); %> (<% getSysBuildTime(); %>)</td>
</tr>
<tr>
  <td class="head" id="statusOPMode">Operation Mode</td>
  <td><script type="text/javascript">showOpMode();</script>&nbsp;</td>
</tr>
</table>
<!-- ================= Internet Configurations ================= -->
<br>
<table width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="statusInternetConfig">Internet Configurations</td>
</tr>
<tr>
  <td class="head" id="statusConnectedType">Connected Type</td>
  <!--<td><% getCfgGeneral(1, "wanConnectionMode"); %> &nbsp;</td>-->
  <td><input name="statusConnectedType_show" id="statusConnectedType_show" value="<% getCfgGeneral(1, "wanConnectionMode"); %>" readonly size="20"></td>
</tr>
<tr>
  <td class="head" id="statusWANIPAddr">WAN IP Address</td>
  <!--<td><% getWanIp(); %>&nbsp;</td>-->
  <td><input name="statusWANIPAddr_show" id="statusWANIPAddr_show" value="<% getWanIp(); %>" readonly size="20"></td>
</tr>
<tr>
  <td class="head" id="statusSubnetMask">Subnet Mask</td>
  <!--<td><% getWanNetmask(); %>&nbsp;</td>-->
  <td><input name="statusSubnetMask_show" id="statusSubnetMask_show" value="<% getWanNetmask(); %>" readonly size="20"></td>
</tr>
<tr>
  <td class="head" id="statusDefaultGW">Default Gateway</td>
  <!--<td><% getWanGateway(); %>&nbsp;</td>-->
  <td><input name="statusDefaultGW_show" id="statusDefaultGW_show" value="<% getWanGateway(); %>" readonly size="20"></td>
</tr>
<tr>
  <td class="head" id="statusPrimaryDNS">Primary Domain Name Server</td>
  <!--<td><% getDns(1); %>&nbsp;</td>-->
  <td><input name="statusPrimaryDNS_show" id="statusPrimaryDNS_show" value="<% getDns(1); %>" readonly size="20"></td>
</tr>
<tr>
  <td class="head" id="statusSecondaryDNS">Secondary Domain Name Server</td>
  <!--<td><% getDns(2); %>&nbsp;</td>-->
  <td><input name="statusSecondaryDNS_show" id="statusSecondaryDNS_show" value="<% getDns(2); %>" readonly size="20"></td>
</tr>
<tr>
  <td class="head" id="statusWANMAC">MAC Address</td>
  <!--<td><% getWanMac(); %>&nbsp;</td>-->
  <td><input name="statusWANMAC_show" id="statusWANMAC_show" value="<% getWanMac(); %>" readonly size="20"></td>
</tr>
</table>
<!-- ================= Local Network ================= -->
<br>
<table width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="statusLocalNet">Local Network</td>
</tr>
<tr>
  <td class="head" id="statusLANIPAddr">Local IP Address</td>
  <!--<td><% getLanIp(); %>&nbsp;</td>-->
  <td><input name="statusLANIPAddr_show" id="statusLANIPAddr_show" value="<% getLanIp(); %>" readonly size="20"></td>
</tr>
<tr>
  <td class="head" id="statusLocalNetmask">Local Netmask</td>
  <!--<td><% getLanNetmask(); %>&nbsp;</td>-->
  <td><input name="statusLocalNetmask_show" id="statusLocalNetmask_show" value="<% getLanNetmask(); %>" readonly size="20"></td>
</tr>
<tr>
  <td class="head" id="statusLANMAC">MAC Address</td>
  <!--<td><% getLanMac(); %>&nbsp;</td>-->
  <td><input name="statusLANMAC_show" id="statusLANMAC_show" value="<% getLanMac(); %>" readonly size="20"></td>
</tr>
<!-- ================= Other Information ================= -->
</table>

<br>

<form method="post" name="security_form" action="">
<!-- ---------------------  MBSSID Selection  --------------------- -->
<table border="1" cellpadding="2" cellspacing="1" width="540">
<tbody><tr>
  <td class="title" colspan="2" id="secureSelectSSID">Select SSID</td>
</tr>
  <tr>
    <td class="head" id="secureSSIDChoice">SSID choice</td>
    <td>
      <select name="ssidIndex" size="1" onchange="selectMBSSIDChanged()">
			<!-- ....Javascript will update options.... -->
      </select>
    </td>
  </tr>
</tbody></table>

<br>

<table border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540">
  <tbody><tr>

    <td class="title" colspan="2"> <span id="sp_title">Security Policy </span></td>
  </tr>
  <tr id="div_security_infra_mode" name="div_security_infra_mode" style="visibility: hidden;"> 
    <td class="head" id="secureSecureMode">Security Mode</td>
    <td>
      <select name="security_mode" id="security_mode" size="1" readonly>
			<!-- ....Javascript will update options.... -->
      </select>
  </tr>
  
   <tr id="div_security_infra_mode_show" name="div_security_infra_mode_show"> 
	<td class="head" id="secureSecureMode_show">Security Mode</td>
  	<td><input name="security_mode_show" id="security_mode_show" value="" readonly size="20"></td>
    </td>
  </tr>
  
  <tr id="div_security_open_mode" name="div_security_open_mode" style="visibility: hidden;"> 
    <td class="head" id="secureEncrypType">Encrypt Type</td>
    <td>
	<input name="security_open_mode" id="security_open_mode" value="WEP" readonly size="6">
    </td>
  </tr>

</tbody></table>
<br>

<!-- WEP -->
<table id="div_wep" name="div_wep" border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540" style="visibility: hidden;">
  <tbody><tr> 
    <td class="title" colspan="4" id="secureWEP">Wire Equivalence Protection (WEP)</td>
  </tr>

  <tr> 
    <td class="head" colspan="2" id="secureWEPDefaultKey">Default Key</td>
    <td colspan="2">
    <input name="wep_default_key" id="wep_default_key" value="" readonly size="6">
    </td>
  </tr>
  
  <tr> 
    <td class="head1" rowspan="4" id="secureWEPKey">WEP Keys</td>
    <td class="head2" id="secureWEPKey1">WEP Key 1 :</td>
    <td>
    <input name="wep_key_1" id="WEP1" maxlength="26" value="" readonly size="20"></td>
    <td>
    	<input id="WEP1Select" name="WEP1Select" value="" readonly size="6">
	</td>
  </tr>

  <tr> 
    <td class="head2" id="secureWEPKey2">WEP Key 2 : </td>
    <td>
    <input name="wep_key_2" id="WEP2" maxlength="26" value="" readonly size="20"></td>
    <td>
	<input id="WEP2Select" name="WEP2Select" value="" readonly size="6">		
	</td>
  </tr>
  <tr> 
    <td class="head2" id="secureWEPKey3">WEP Key 3 : </td>
    <td>
    <input name="wep_key_3" id="WEP3" maxlength="26" value="" readonly size="20"></td>
    <td>
		<input id="WEP3Select" name="WEP3Select" value="" readonly size="6">
	</td>
  </tr>
  <tr> 
    <td class="head2" id="secureWEPKey4">WEP Key 4 : </td>
    <td>
    <input name="wep_key_4" id="WEP4" maxlength="26" value="" readonly size="20"></td>
    <td>
	<input id="WEP4Select" name="WEP4Select" value="" readonly size="6">
	</td>
  </tr>

</tbody></table>
<!-- <br /> -->

<!-- WPA -->
<table id="div_wpa" name="div_wpa" border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540" style="visibility: hidden;">

  <tbody><tr>
    <td class="title" colspan="2" id="secreWPA">WPA</td>
  </tr>
  <tr id="div_wpa_algorithms" name="div_wpa_algorithms" style="visibility: hidden;"> 
    <td class="head" id="secureWPAAlgorithm">WPA Algorithms</td>
    <td>
      <input name="cipher" id="cipher" value="" readonly size="10">
	</td>
  </tr>
  <tr id="wpa_passphrase" name="wpa_passphrase" style="visibility: hidden;">
    <td class="head" id="secureWPAPassPhrase">Pass Phrase</td>
    <td>
      <input name="passphrase" id="passphrase" size="28" maxlength="64" value="" readonly>
    &nbsp;</td>
  </tr>

  <tr id="wpa_key_renewal_interval" name="wpa_key_renewal_interval" style="visibility: hidden;">
    <td class="head" id="secureWPAKeyRenewInterval">Key Renewal Interval</td>
    <td>
     <input name="keyRenewalInterval" id="keyRenewalInterval" size="4" maxlength="4" value="3600" readonly> seconds 
    </td>
  </tr>

  <tr id="wpa_PMK_Cache_Period" name="wpa_PMK_Cache_Period" style="visibility: hidden;">
    <td class="head" id="secureWPAPMKCachePeriod">PMK Cache Period</td>
    <td>
      <input name="PMKCachePeriod" id="PMKCachePeriod" size="4" maxlength="4" value="" readonly> minute
    </td>
  </tr>

  <tr id="wpa_preAuthentication" name="wpa_preAuthentication" style="visibility: hidden;">
    <td class="head" id="secureWPAPreAuth">Pre-Authentication</td>
    <td>
      <input name="PreAuthentication" id="PreAuthentication" value="" readonly size="10">
	</td>
  </tr>
</tbody></table>

<!-- 802.1x -->
<!-- WEP  -->
<table id="div_8021x_wep" name="div_8021x_wep" border="1" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540" style="visibility: hidden;">
  <tbody>
  <tr>
    <td class="title" colspan="2" width="540" id="secure8021XWEP">802.1x WEP</td>
  </tr>
  
 <tr> 
    <td class="head" id="secure1XWEP" width="540">WEP</td>
    <td>
      <input name="ieee8021x_wep" id="ieee8021x_wep" value="" readonly size="10">
    </td>
  </tr>
</tbody>
</table>

<table id="div_radius_server" name="div_radius_server" border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540" style="visibility: hidden;">
<tbody>
   <tr>
    <td class="title" colspan="2" width="540" id="secureRadius">Radius Server</td>
   </tr>
    <tr> 
		<td width="540" class="head" id="secureRadiusIPAddr"> IP Address </td>
		<td> <input name="RadiusServerIP" id="RadiusServerIP" size="16" maxlength="32" value="" readonly> </td>
	</tr>
    <tr> 
		<td width="540" class="head" id="secureRadiusPort"> Port </td>
		<td> <input name="RadiusServerPort" id="RadiusServerPort" size="5" maxlength="5" value="" readonly> </td>
	</tr>
    <tr> 
		<td width="540" class="head" id="secureRadiusSharedSecret"> Shared Secret </td>
		<td> <input name="RadiusServerSecret" id="RadiusServerSecret" size="16" maxlength="64" value="" readonly> </td>
	</tr>
    <tr> 
		<td width="540" class="head" id="secureRadiusSessionTimeout"> Session Timeout </td>
		<td> <input name="RadiusServerSessionTimeout" id="RadiusServerSessionTimeout" size="3" maxlength="4" value="0" readonly> </td>
	</tr>
    <tr> 
		<td width="540" class="head" id="secureRadiusIdleTimeout"> Idle Timeout </td>
		<td> <input name="RadiusServerIdleTimeout" id="RadiusServerIdleTimeout" size="3" maxlength="4" value="" readonly> </td>
	</tr>
</tbody></table>

<br>
<!--									-->
<!--	AccessPolicy for mbssid 		-->
<!--									-->

<script language="JavaScript" type="text/javascript">
var aptable;

for(aptable = 0; aptable < MBSSID_MAX; aptable++){
	document.write(" <table id=AccessPolicy_"+ aptable +" border=1 bordercolor=#9babbd cellpadding=3 cellspacing=1 hspace=2 vspace=2 width=540>");
	document.write(" <tbody> <tr> <td class=title colspan=2 >"+_("secure access policy")+"</td></tr>");
	document.write(" <tr> <td bgcolor=#E8F8FF class=head >"+_("secure access policy capable")+"</td>");
	document.write(" <td> ");
	document.write("<input name=apselect_"+ aptable + " id=apselect_"+aptable+" size=\"6\" value=\"\" readonly>");
	document.write("</td></tr>");

	for(i=0; i< ACCESSPOLICYLIST_MAX/2; i++){
		input_name = "newap_"+ aptable +"_" + (2*i);
		td_name = "newap_td_"+ aptable +"_" + (2*i);

		document.write(" <tr id=id_"+aptable+"_");
		document.write(i*2);
		document.write("> <td id=");
		document.write(td_name);
		document.write(">");
		document.write("<input id=");
		document.write(input_name);
		document.write(" size=16 maxlength=20 readonly></td>");

		input_name = "newap_" + aptable + "_" + (2*i+1);
		td_name = "newap_td_" + aptable + "_" + (2*i+1);
		document.write("      <td id=");
		document.write(td_name);
		document.write(">");
		document.write("<input id=");
		document.write(input_name);
		document.write(" size=16 maxlength=20 readonly></td> </tr>");
	}

	document.write("</tbody></table>");
}
</script>

</form>

<table width="540" border="0" cellpadding="2" cellspacing="1">
<tr>
  <td colspan="2" border="0">
  	<p align="center">
  	<input type=reset  value="Refresh" onClick="window.location.reload()">
  </td>
</tr>
</table>

</td></tr></tbody></table>
 </center>
</div>
</body></html>
