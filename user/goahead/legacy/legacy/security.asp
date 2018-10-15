<html><head><!-- Copyright (c), Ralink Technology Corporation All Rights Reserved. -->

<meta http-equiv="Pragma" content="no-cache">
<meta http-equiv="Expires" content="-1">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css"><title>Ralink Wireless Security Settings</title>

<script language="JavaScript" type="text/javascript">

var MBSSID_MAX 				= 8;
var ACCESSPOLICYLIST_MAX	= 64;

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

function checkMac(str){
	var len = str.length;
	if(len!=17)
		return false;

	for (var i=0; i<str.length; i++) {
		if((i%3) == 2){
			if(str.charAt(i) == ':')
				continue;
		}else{
			if (    (str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
					(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
					(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
			continue;
		}
		return false;
	}
	return true;
}

function checkRange(str, num, min, max)
{
    d = atoi(str,num);
    if(d > max || d < min)
        return false;
    return true;
}

function checkIpAddr(field)
{
    if(field.value == "")
        return false;

    if ( checkAllNum(field.value) == 0)
        return false;

    if( (!checkRange(field.value,1,0,255)) ||
        (!checkRange(field.value,2,0,255)) ||
        (!checkRange(field.value,3,0,255)) ||
        (!checkRange(field.value,4,1,254)) ){
        return false;
    }
   return true;
}

function atoi(str, num)
{
    i=1;
    if(num != 1 ){
        while (i != num && str.length != 0){
            if(str.charAt(0) == '.'){
                i++;
            }
            str = str.substring(1);
        }
        if(i != num )
            return -1;
    }

    for(i=0; i<str.length; i++){
        if(str.charAt(i) == '.'){
            str = str.substring(0, i);
            break;
        }
    }
    if(str.length == 0)
        return -1;
    return parseInt(str, 10);
}

function checkHex(str){
	var len = str.length;

	for (var i=0; i<str.length; i++) {
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
			(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
			(str.charAt(i) >= 'A' && str.charAt(i) <= 'F') ){
				continue;
		}else
	        return false;
	}
    return true;
}

function checkInjection(str)
{
	var len = str.length;
	for (var i=0; i<str.length; i++) {
		if ( str.charAt(i) == ';' || str.charAt(i) == ',' ||
			 str.charAt(i) == '\r' || str.charAt(i) == '\n'){
				return false;
		}else
	        continue;
	}
    return true;
}

function checkAllNum(str)
{
    for (var i=0; i<str.length; i++){
        if((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.' ))
            continue;
        return false;
    }
    return true;
}

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
			// LoadAP();
			// ShowAP(defaultShownMBSSID);

//			if(<% getWPSModeASP(); %> && <% isWPSConfiguredASP(); %>){
//				alert("Info: The security settings has been assigned under active WPS functions.\nYou still could change security setting manually but the existed WPS settings would be overwritted.");
//			}

		} else {
			alert('There was a problem with the request.');
		}
	}
}

function deleteAccessPolicyListHandler()
{
	window.location.reload(false);
}


function parseAllData(str)
{
	var all_str = new Array();
	// alert(str);
	all_str = str.split("\n");
	// alert(all_str);

	defaultShownMBSSID = parseInt(all_str[0]);
	// alert(defaultShownMBSSID);
	// alert(all_str.length);
	for (var i=0; i<all_str.length-2; i++) {
		var fields_str = new Array();
		fields_str = all_str[i+1].split("\r");

		SSID[i] = fields_str[0];
		// alert(SSID[i]);
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
	// alert(SSID[0]);
	// alert(AuthMode[0]);
	// alert(EncrypType[0]);
	// alert(IEEE8021X[0]);
}

function checkData()
{
	var securitymode;
//	var ssid = document.security_form.Ssid.value;
	
	securitymode = document.security_form.security_mode.value;
	if (securitymode == "OPEN" || securitymode == "SHARED" ||securitymode == "WEPAUTO")
	{
		if(! check_Wep(securitymode) )
			return false;
	}else if (securitymode == "WPAPSK" || securitymode == "WPA2PSK" || securitymode == "WPAPSKWPA2PSK" /* || security_mode == 5 */){
		var keyvalue = document.security_form.passphrase.value;

		if (keyvalue.length == 0){
			alert('Please input wpapsk key!');
			return false;
		}

		if (keyvalue.length < 8){
			alert('Please input at least 8 character of wpapsk key!');
			return false;
		}
		
		if(checkInjection(document.security_form.passphrase.value) == false){
			alert('Invalid characters in Pass Phrase.');
			return false;
		}

		if(document.security_form.cipher[0].checked != true && 
		   document.security_form.cipher[1].checked != true &&
   		   document.security_form.cipher[2].checked != true){
   		   alert('Please choose a WPA Algorithms.');
   		   return false;
		}

		if(checkAllNum(document.security_form.keyRenewalInterval.value) == false){
			alert('Please input a valid key renewal interval');
			return false;
		}
		if(document.security_form.keyRenewalInterval.value < 60){
			alert('Warning: A short key renewal interval.');
			// return false;
		}
		if(check_wpa() == false)
			return false;
	}
	//802.1x
	else if (securitymode == "IEEE8021X") // 802.1x
	{
		if( document.security_form.ieee8021x_wep[0].checked == false &&
			document.security_form.ieee8021x_wep[1].checked == false){
			alert('Please choose the 802.1x WEP option.');
			return false;
		}
		if(check_radius() == false)
			return false;
	}else if (securitymode == "WPA" || securitymode == "WPA1WPA2") //     WPA or WPA1WP2 mixed mode
	{
		if(check_wpa() == false)
			return false;
		if(check_radius() == false)
			return false;
	}else if (securitymode == "WPA2") //         WPA2
	{
		if(check_wpa() == false)
			return false;
		if( document.security_form.PreAuthentication[0].checked == false &&
			document.security_form.PreAuthentication[1].checked == false){
			alert('Please choose the Pre-Authentication options.');
			return false;
		}

		if(!document.security_form.PMKCachePeriod.value.length){
			alert('Please input the PMK Cache Period.');
			return false;
		}
		if(check_radius() == false)
			return false;
	}

	/* check Access Policy
	for(i=0; i<MBSSID_MAX; i++){


		if( document.getElementById("newap_text_" + i).value != ""){
			if(!checkMac(document.getElementById("newap_text_" + i).value)){
				alert("The mac address in Access Policy form is invalid.\n");
				return false;
			}
		}
	}
	*/

	return true;
}

function check_wpa()
{
		if(document.security_form.cipher[0].checked != true && 
		   document.security_form.cipher[1].checked != true &&
   		   document.security_form.cipher[2].checked != true){
   		   alert('Please choose a WPA Algorithms.');
   		   return false;
		}

		if(checkAllNum(document.security_form.keyRenewalInterval.value) == false){
			alert('Please input a valid key renewal interval');
			return false;
		}
		if(document.security_form.keyRenewalInterval.value < 60){
			alert('Warning: A short key renewal interval.');
			// return false;
		}
		return true;
}

function check_radius()
{
	if(!document.security_form.RadiusServerIP.value.length){
		alert('Please input the radius server ip address.');
		return false;		
	}
	if(!document.security_form.RadiusServerPort.value.length){
		alert('Please input the radius server port number.');
		return false;		
	}
	if(!document.security_form.RadiusServerSecret.value.length){
		alert('Please input the radius server shared secret.');
		return false;		
	}

	if(checkIpAddr(document.security_form.RadiusServerIP) == false){
		alert('Please input a valid radius server ip address.');
		return false;		
	}
	if( (checkRange(document.security_form.RadiusServerPort.value, 1, 1, 65535)==false) ||
		(checkAllNum(document.security_form.RadiusServerPort.value)==false)){
		alert('Please input a valid radius server port number.');
		return false;		
	}
	if(checkInjection(document.security_form.RadiusServerSecret.value)==false){
		alert('The shared secret contains invalid characters.');
		return false;		
	}

	if(document.security_form.RadiusServerSessionTimeout.value.length){
		if(checkAllNum(document.security_form.RadiusServerSessionTimeout.value)==false){
			alert('Please input a valid session timeout number or u may left it empty.');
			return false;	
		}	
	}

	return true;
}

function securityMode(c_f)
{
	var security_mode;


	changed = c_f;

	hideWep();


	document.getElementById("div_security_shared_mode").style.visibility = "hidden";
	document.getElementById("div_security_shared_mode").style.display = "none";
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
	document.security_form.cipher[0].disabled = true;
	document.security_form.cipher[1].disabled = true;
	document.security_form.cipher[2].disabled = true;
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
		document.security_form.cipher[0].disabled = false;
		document.security_form.cipher[1].disabled = false;

		// deal with TKIP-AES mixed mode
		if(security_mode == "WPAPSK" && document.security_form.cipher[2].checked)
			document.security_form.cipher[2].checked = false;
		// deal with TKIP-AES mixed mode
		if(security_mode == "WPA2PSK" || security_mode == "WPAPSKWPA2PSK")
			document.security_form.cipher[2].disabled = false;

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
		document.security_form.cipher[0].disabled = false;
		document.security_form.cipher[1].disabled = false;
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

		// deal with TKIP-AES mixed mode
		if(security_mode == "WPA" && document.security_form.cipher[2].checked)
			document.security_form.cipher[2].checked = false;
		// deal with TKIP-AES mixed mode
		if(security_mode == "WPA2"){
			document.security_form.cipher[2].disabled = false;
			document.getElementById("wpa_preAuthentication").style.visibility = "visible";
			document.getElementById("wpa_preAuthentication").style.display = style_display_on();
			document.security_form.PreAuthentication.disabled = false;
			document.getElementById("wpa_PMK_Cache_Period").style.visibility = "visible";
			document.getElementById("wpa_PMK_Cache_Period").style.display = style_display_on();
			document.security_form.PMKCachePeriod.disabled = false;
		}

		// deal with WPA1WPA2 mixed mode
		if(security_mode == "WPA1WPA2"){
			document.security_form.cipher[2].disabled = false;
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

	if(mode == "SHARED"){
		document.getElementById("div_security_shared_mode").style.visibility = "visible";
		document.getElementById("div_security_shared_mode").style.display = style_display_on();
	}
	//document.security_form.wep_auth_type.disabled = false;
}


function check_Wep(securitymode)
{
	var defaultid = document.security_form.wep_default_key.value;
	var key_input;

	if ( defaultid == 1 )
		var keyvalue = document.security_form.wep_key_1.value;
	else if (defaultid == 2)
		var keyvalue = document.security_form.wep_key_2.value;
	else if (defaultid == 3)
		var keyvalue = document.security_form.wep_key_3.value;
	else if (defaultid == 4)
		var keyvalue = document.security_form.wep_key_4.value;

	if (keyvalue.length == 0 &&  (securitymode == "SHARED" || securitymode == "OPEN")){ // shared wep  || md5
		alert('Please input wep key'+defaultid+' !');
		return false;
	}

	var keylength = document.security_form.wep_key_1.value.length;
	if (keylength != 0){
		if (document.security_form.WEP1Select.options.selectedIndex == 0){
			if(keylength != 5 && keylength != 13) {
				alert('Please input 5 or 13 characters of wep key1 !');
				return false;
			}
			if(checkInjection(document.security_form.wep_key_1.value)== false){
				alert('Wep key1 contains invalid characters.');
				return false;
			}
		}
		if (document.security_form.WEP1Select.options.selectedIndex == 1){
			if(keylength != 10 && keylength != 26) {
				alert('Please input 10 or 26 characters of wep key1 !');
				return false;
			}
			if(checkHex(document.security_form.wep_key_1.value) == false){
				alert('Invalid Wep key1 format!');
				return false;
			}
		}
	}

	keylength = document.security_form.wep_key_2.value.length;
	if (keylength != 0){
		if (document.security_form.WEP2Select.options.selectedIndex == 0){
			if(keylength != 5 && keylength != 13) {
				alert('Please input 5 or 13 characters of wep key2 !');
				return false;
			}
			if(checkInjection(document.security_form.wep_key_1.value)== false){
				alert('Wep key2 contains invalid characters.');
				return false;
			}			
		}
		if (document.security_form.WEP2Select.options.selectedIndex == 1){
			if(keylength != 10 && keylength != 26) {
				alert('Please input 10 or 26 characters of wep key2 !');
				return false;
			}
			if(checkHex(document.security_form.wep_key_2.value) == false){
				alert('Invalid Wep key2 format!');
				return false;
			}
		}
	}

	keylength = document.security_form.wep_key_3.value.length;
	if (keylength != 0){
		if (document.security_form.WEP3Select.options.selectedIndex == 0){
			if(keylength != 5 && keylength != 13) {
				alert('Please input 5 or 13 characters of wep key3 !');
				return false;
			}
			if(checkInjection(document.security_form.wep_key_1.value)== false){
				alert('Wep key3 contains invalid characters.');
				return false;
			}
		}
		if (document.security_form.WEP3Select.options.selectedIndex == 1){
			if(keylength != 10 && keylength != 26) {
				alert('Please input 10 or 26 characters of wep key3 !');
				return false;
			}
			if(checkHex(document.security_form.wep_key_3.value) == false){
				alert('Invalid Wep key3 format!');
				return false;
			}			
		}
	}

	keylength = document.security_form.wep_key_4.value.length;
	if (keylength != 0){
		if (document.security_form.WEP4Select.options.selectedIndex == 0){
			if(keylength != 5 && keylength != 13) {
				alert('Please input 5 or 13 characters of wep key4 !');
				return false;
			}
			if(checkInjection(document.security_form.wep_key_1.value)== false){
				alert('Wep key4 contains invalid characters.');
				return false;
			}			
		}
		if (document.security_form.WEP4Select.options.selectedIndex == 1){
			if(keylength != 10 && keylength != 26) {
				alert('Please input 10 or 26 characters of wep key4 !');
				return false;
			}

			if(checkHex(document.security_form.wep_key_4.value) == false){
				alert('Invalid Wep key4 format!');
				return false;
			}			
		}
	}
	return true;
}
	
function submit_apply()
{

	if (checkData() == true){
		changed = 0;

		document.security_form.submit();
//		opener.location.reload();
	}
}

function LoadFields(MBSSID)
{
	var result;
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
	if(MBSSID == 0)
		sp_select.options[sp_select.length] = new Option("802.1X",	"IEEE8021X",false, AuthMode[MBSSID] == "IEEE8021X");

	// WEP
	document.getElementById("WEP1").value = Key1Str[MBSSID];
	document.getElementById("WEP2").value = Key2Str[MBSSID];
	document.getElementById("WEP3").value = Key3Str[MBSSID];
	document.getElementById("WEP4").value = Key4Str[MBSSID];

	document.getElementById("WEP1Select").selectedIndex = (Key1Type[MBSSID] == "0" ? 1 : 0);
	document.getElementById("WEP2Select").selectedIndex = (Key2Type[MBSSID] == "0" ? 1 : 0);
	document.getElementById("WEP3Select").selectedIndex = (Key3Type[MBSSID] == "0" ? 1 : 0);
	document.getElementById("WEP4Select").selectedIndex = (Key4Type[MBSSID] == "0" ? 1 : 0);

	document.getElementById("wep_default_key").selectedIndex = parseInt(DefaultKeyID[MBSSID]) - 1 ;

	// SHARED && NONE
	if(AuthMode[MBSSID] == "SHARED" && EncrypType[MBSSID] == "NONE")
		document.getElementById("security_shared_mode").selectedIndex = 1;
	else
		document.getElementById("security_shared_mode").selectedIndex = 0;

	// WPA
	if(EncrypType[MBSSID] == "TKIP")
		document.security_form.cipher[0].checked = true;
	else if(EncrypType[MBSSID] == "AES")
		document.security_form.cipher[1].checked = true;
	else if(EncrypType[MBSSID] == "TKIPAES")
		document.security_form.cipher[2].checked = true;

	document.getElementById("passphrase").value = WPAPSK[MBSSID];
	document.getElementById("keyRenewalInterval").value = RekeyInterval[MBSSID];
	document.getElementById("PMKCachePeriod").value = PMKCachePeriod[MBSSID];
	//document.getElementById("PreAuthentication").value = PreAuth[MBSSID];
	if(PreAuth[MBSSID] == "0")
		document.security_form.PreAuthentication[0].checked = true;
	else
		document.security_form.PreAuthentication[1].checked = true;

	//802.1x wep
	if(IEEE8021X[MBSSID] == "1"){
		if(EncrypType[MBSSID] == "WEP")
			document.security_form.ieee8021x_wep[1].checked = true;
		else
			document.security_form.ieee8021x_wep[0].checked = true;
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
	for(i=0; i<MBSSID_MAX; i++){
		document.getElementById("apselect_"+i).selectedIndex	= AccessPolicy[i];
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
	// ShowAP(selected);

	// radio button special case
	WPAAlgorithms = EncrypType[selected];
	IEEE8021XWEP = IEEE8021X[selected];
	PreAuthentication = PreAuth[selected];

	changeSecurityPolicyTableTitle(SSID[selected]);

	//clear all new access policy list field
	for(i=0; i<MBSSID_MAX; i++)
		document.getElementById("newap_text_"+i).value = "";

	return true;
}

function changeSecurityPolicyTableTitle(t)
{
	var title = document.getElementById("sp_title");
	title.innerHTML = "Security Mode -- " + "\"" + t + "\"";
}

function delap(mbssid, num)
{
	makeRequest("/goform/APDeleteAccessPolicyList", mbssid+ "," +num, deleteAccessPolicyListHandler);
}

function initAll()
{
	makeRequest("/goform/legacywGetSecurity", "n/a", securityHandler);
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

function setChange(c){
	changed = c;
}

var WPAAlgorithms;
function onWPAAlgorithmsClick(type)
{
	if(type == 0 && WPAAlgorithms == "TKIP") return;
	if(type == 1 && WPAAlgorithms == "AES") return;
	if(type == 2 && WPAAlgorithms == "TKIPAES") return;
	setChange(1);
}

var IEEE8021XWEP;
function onIEEE8021XWEPClick(type)
{
	if(type == 0 && IEEE8021XWEP == false) return;
	if(type == 1 && IEEE8021XWEP == true) return;
	setChange(1);
}

var PreAuthentication;
function onPreAuthenticationClick(type)
{
	if(type == 0 && PreAuthentication == false) return;
	if(type == 1 && PreAuthentication == true) return;
	setChange(1);
}

</script>
</head>
<body onload="initAll()">
<table class="body"><tbody><tr><td>

<form method="post" name="security_form" action="/goform/LEGACYSecurity">

<!-- ---------------------  MBSSID Selection  --------------------- -->
<table border="1" cellpadding="2" cellspacing="1" width="540">
<tbody><tr>
  <td class="title" colspan="2">Select SSID</td>
</tr>
  <tr>
    <td class="head">SSID choice</td>
    <td>
      <select name="ssidIndex" size="1" onchange="selectMBSSIDChanged()">
			<!-- ....Javascript will update options.... -->
      </select>
    </td>
  </tr>
</tbody></table>

<hr />

<table border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540">
  <tbody><tr>

    <td class="title" colspan="2"> <span id="sp_title">Security Policy </span></td>
  </tr>
  <tr id="div_security_infra_mode" name="div_security_infra_mode"> 
    <td class="head">Security Mode</td>
    <td>
      <select name="security_mode" id="security_mode" size="1" onchange="securityMode(1)">
			<!-- ....Javascript will update options.... -->
      </select>

    </td>
  </tr>
  <tr id="div_security_shared_mode" name="div_security_shared_mode" style="visibility: hidden;">
    <td class="head">Encrypt Type</td>
    <td>
      <select name="security_shared_mode" id="security_shared_mode" size="1" onchange="securityMode(1)">
		<option value=WEP>WEP</option>
		<option value=None>None</option>
      </select>

    </td>
  </tr>

</tbody></table>
<br>

<!-- WEP -->
<table id="div_wep" name="div_wep" border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540" style="visibility: hidden;">
  <tbody><tr> 
    <td class="title" colspan="4">Wire Equivalence Protection (WEP)</td>
  </tr>

  <tr> 
    <td class="head" colspan="2">Default Key</td>
    <td colspan="2">
      <select name="wep_default_key" id="wep_default_key" size="1" onchange="setChange(1)">
	<option value="1">Key 1</option>
	<option value="2">Key 2</option>
	<option value="3">Key 3</option>
	<option value="4">Key 4</option>
      </select>
    </td>
  </tr>
  
  <tr> 
    <td class="head1" rowspan="4">WEP Keys</td>
    <td class="head2">WEP Key 1 :</td>
    <td><input name="wep_key_1" id="WEP1" maxlength="26" value="" onKeyUp="setChange(1)"></td>
    <td><select id="WEP1Select" name="WEP1Select" onchange="setChange(1)"> 
		<option value="1">ASCII</option>
		<option value="0">Hex</option>
		</select></td>
  </tr>

  <tr> 
    <td class="head2">WEP Key 2 : </td>
    <td><input name="wep_key_2" id="WEP2" maxlength="26" value="" onKeyUp="setChange(1)"></td>
    <td><select id="WEP2Select" name="WEP2Select" onchange="setChange(1)">
		<option value="1">ASCII</option>
		<option value="0">Hex</option>
		</select></td>
  </tr>
  <tr> 
    <td class="head2">WEP Key 3 : </td>
    <td><input name="wep_key_3" id="WEP3" maxlength="26" value="" onKeyUp="setChange(1)"></td>
    <td><select id="WEP3Select" name="WEP3Select" onchange="setChange(1)">
		<option value="1">ASCII</option>
		<option value="0">Hex</option>
		</select></td>
  </tr>
  <tr> 
    <td class="head2">WEP Key 4 : </td>
    <td><input name="wep_key_4" id="WEP4" maxlength="26" value="" onKeyUp="setChange(1)"></td>
    <td><select id="WEP4Select" name="WEP4Select" onchange="setChange(1)">
		<option value="1">ASCII</option>
		<option value="0">Hex</option>
		</select></td>
  </tr>

</tbody></table>
<!-- <br /> -->

<!-- WPA -->
<table id="div_wpa" name="div_wpa" border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540" style="visibility: hidden;">

  <tbody><tr>
    <td class="title" colspan="2">WPA</td>
  </tr>
  <tr id="div_wpa_algorithms" name="div_wpa_algorithms" style="visibility: hidden;">
    <td class="head">WPA Algorithms</td>
    <td>
      <input name="cipher" id="cipher" value="0" type="radio" onClick="onWPAAlgorithmsClick(0)">TKIP &nbsp;
      <input name="cipher" id="cipher" value="1" type="radio" onClick="onWPAAlgorithmsClick(1)">AES &nbsp;
      <input name="cipher" id="cipher" value="2" type="radio" onClick="onWPAAlgorithmsClick(2)">TKIPAES &nbsp;
    </td>
  </tr>
  <tr id="wpa_passphrase" name="wpa_passphrase" style="visibility: hidden;">
    <td class="head">Pass Phrase</td>
    <td>
      <input name="passphrase" id="passphrase" size="28" maxlength="64" value="" onKeyUp="setChange(1)">
    </td>
  </tr>

  <tr id="wpa_key_renewal_interval" name="wpa_key_renewal_interval" style="visibility: hidden;">
    <td class="head">Key Renewal Interval</td>
    <td>
      <input name="keyRenewalInterval" id="keyRenewalInterval" size="4" maxlength="4" value="" onKeyUp="setChange(1)"> seconds
    </td>
  </tr>

  <tr id="wpa_PMK_Cache_Period" name="wpa_PMK_Cache_Period" style="visibility: hidden;">
    <td class="head">PMK Cache Period</td>
    <td>
      <input name="PMKCachePeriod" id="PMKCachePeriod" size="4" maxlength="4" value="" onKeyUp="setChange(1)"> minute
    </td>
  </tr>

  <tr id="wpa_preAuthentication" name="wpa_preAuthentication" style="visibility: hidden;">
    <td class="head">Pre-Authentication</td>
    <td>
      <input name="PreAuthentication" id="PreAuthentication" value="0" type="radio" onClick="onPreAuthenticationClick(0)">Disable &nbsp;
      <input name="PreAuthentication" id="PreAuthentication" value="1" type="radio" onClick="onPreAuthenticationClick(1)">Enable &nbsp;
    </td>
  </tr>
</tbody></table>


<!-- 802.1x -->
<!-- WEP  -->
<table id="div_8021x_wep" name="div_8021x_wep" border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540" style="visibility: hidden;">
  <tbody>
  <tr>
    <td class="title" colspan="2">802.1x WEP</td>
  </tr>
  <tr>
		<td class="head"> WEP </td>
		<td>
	      <input name="ieee8021x_wep" id="ieee8021x_wep" value="0" type="radio" onClick="onIEEE8021XWEPClick(0)">Disable &nbsp;
    	  <input name="ieee8021x_wep" id="ieee8021x_wep" value="1" type="radio" onClick="onIEEE8021XWEPClick(1)">Enable
		</td>
  </tr>
</tbody></table>

<br>
<table id="div_radius_server" name="div_radius_server" border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540" style="visibility: hidden;">
<tbody>
   <tr>
    <td class="title" colspan="2">Radius Server</td>
   </tr>
    <tr> 
		<td bgcolor="#E8F8FF"  class="head"> IP Address </td>
		<td> <input name="RadiusServerIP" id="RadiusServerIP" size="16" maxlength="32" value="" onKeyUp="setChange(1)"> </td>
	</tr>
    <tr> 
		<td bgcolor="#E8F8FF"  class="head"> Port </td>
		<td> <input name="RadiusServerPort" id="RadiusServerPort" size="5" maxlength="5" value="" onKeyUp="setChange(1)"> </td>
	</tr>
    <tr> 
		<td bgcolor="#E8F8FF"  class="head"> Shared Secret </td>
		<td> <input name="RadiusServerSecret" id="RadiusServerSecret" size="16" maxlength="64" value="" onKeyUp="setChange(1)"> </td>
	</tr>
    <tr> 
		<td bgcolor="#E8F8FF"  class="head"> Session Timeout </td>
		<td> <input name="RadiusServerSessionTimeout" id="RadiusServerSessionTimeout" size="3" maxlength="4" value="" onKeyUp="setChange(1)"> </td>
	</tr>
    <tr> 
		<td bgcolor="#E8F8FF"  class="head"> Idle Timeout </td>
		<td> <input name="RadiusServerIdleTimeout" id="RadiusServerIdleTimeout" size="3" maxlength="4" value="" onKeyUp="setChange(1)" readonly> </td>
	</tr>

</tbody></table>


<!--									-->
<!--	AccessPolicy for mbssid 		-->
<!--

<script language="JavaScript" type="text/javascript">

var aptable;

for(aptable = 0; aptable < MBSSID_MAX; aptable++){
	document.write(" <table id=AccessPolicy_"+ aptable +" border=1 bordercolor=#9babbd cellpadding=3 cellspacing=1 hspace=2 vspace=2 width=540>");
	document.write(" <tbody> <tr> <td class=title colspan=2> Access Policy </td></tr>");
	document.write(" <tr> <td bgcolor=#E8F8FF class=head> Capable </td>");
	document.write(" <td> <select name=apselect_"+ aptable + " id=apselect_"+aptable+" size=1 onchange=\"setChange(1)\">");
	document.write(" 			<option value=0>Disable</option> <option value=1>Allow </option><option value=2>Reject </option></select> </td></tr>");

	for(i=0; i< ACCESSPOLICYLIST_MAX/2; i++){
		input_name = "newap_"+ aptable +"_" + (2*i);
		td_name = "newap_td_"+ aptable +"_" + (2*i);

		document.write(" <tr id=id_"+aptable+"_");
		document.write(i*2);
		document.write("> <td id=");
		document.write(td_name);
		document.write("> <input style=\"width: 30px;\" value=Del onclick=\"delap("+aptable+", ");
		document.write(2*i);
		document.write(")\" type=button > <input id=");
		document.write(input_name);
		document.write(" size=16 maxlength=20 readonly></td>");

		input_name = "newap_" + aptable + "_" + (2*i+1);
		td_name = "newap_td_" + aptable + "_" + (2*i+1);
		document.write("      <td id=");
		document.write(td_name);
		document.write("> <input style=\"width: 30px;\" value=Del onclick=\"delap("+aptable+", ");
		document.write(2*i+1);
		document.write(")\" type=button> <input id=");
		document.write(input_name);
		document.write(" size=16 maxlength=20 readonly></td> </tr>");
	}

	document.write("<tr><td bgcolor=#E8F8FF class=head> New:</td>");
	document.write("	<td>	<input name=newap_text_"+aptable+" id=newap_text_"+aptable+" size=16 maxlength=20>	</td>	</tr> </tbody></table>");
}
</script>
-->

<!-- <br /> -->
<table border="0" cellpadding="2" cellspacing="1" width="540">
  <tbody><tr align="center">

    <td>
      <input style="width: 120px;" value="Apply" onclick="submit_apply()" type="button"> &nbsp; &nbsp;
      <input style="width: 120px;" value="Cancel" type="reset" onClick="window.location.reload()" >
    </td>
  </tr>
</tbody></table>
</form>

</td></tr></tbody></table>
</body></html>
