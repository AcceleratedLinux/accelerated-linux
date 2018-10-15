<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Ralink Wireless Station Site Survey Connection</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

var ch1 = "<option value = 1 >1</option>";
var ch2 = "<option value = 2 >2</option>";
var ch3 = "<option value = 3 >3</option>";
var ch4 = "<option value = 4 >4</option>";
var ch5 = "<option value = 5 >5</option>";
var ch6 = "<option value = 6 >6</option>";
var ch7 = "<option value = 7 >7</option>";
var ch8 = "<option value = 8 >8</option>";
var ch9 = "<option value = 9 >9</option>";
var ch10 = "<option value = 10>10</option>";
var ch11 = "<option value = 11>11</option>";
var ch12 = "<option value = 12>12</option>";
var ch13 = "<option value = 13>13</option>";
var ch14 = "<option value = 14>14</option>";

var ch36 = "<option value = 36 >36</option>";
var ch40 = "<option value = 40 >40</option>";
var ch44 = "<option value = 44 >44</option>";
var ch48 = "<option value = 48 >48</option>";
var ch52 = "<option value = 52 >52</option>";
var ch56 = "<option value = 56 >56</option>";
var ch60 = "<option value = 60 >60</option>";
var ch64 = "<option value = 64 >64</option>";
var ch100 = "<option value = 100>100</option>";
var ch104 = "<option value = 104>104</option>";
var ch108 = "<option value = 108>108</option>";
var ch112 = "<option value = 112>112</option>";
var ch116 = "<option value = 116>116</option>";
var ch120 = "<option value = 120>120</option>";
var ch124 = "<option value = 124>124</option>";
var ch128 = "<option value = 128>128</option>";
var ch132 = "<option value = 132>132</option>";
var ch136 = "<option value = 136>136</option>";
var ch140 = "<option value = 140>140</option>";
var ch149 = "<option value = 149>149</option>";
var ch153 = "<option value = 153>153</option>";
var ch157 = "<option value = 157>157</option>";
var ch161 = "<option value = 161>161</option>";
var ch165 = "<option value = 165>165</option>";

var ch34 = "<option value = 34 >34</option>";
var ch38 = "<option value = 38 >38</option>";
var ch42 = "<option value = 42 >42</option>";
var ch46 = "<option value = 46 >46</option>";

function initTranslation()
{
	var e = document.getElementById("connectSSID");
	e.innerHTML = _("station ssid");
	e = document.getElementById("connectSecurePolicy");
	e.innerHTML = _("addprof secure policy");
	e = document.getElementById("connectSecureMode");
	e.innerHTML = _("secure security mode");
	e = document.getElementById("connectAdHocSecure");
	e.innerHTML = _("secure security mode");
	e = document.getElementById("connectWEP");
	e.innerHTML = _("secure wep");
	e = document.getElementById("connectWEPKeyLength");
	e.innerHTML = _("addprof wep key length");
	e = document.getElementById("connectWEPKeyEntryMethod");
	e.innerHTML = _("addprof wep key entry method");
	e = document.getElementById("connectHex");
	e.innerHTML = _("addprof hex");
	e = document.getElementById("connectASCII");
	e.innerHTML = _("addprof ascii");
	e = document.getElementById("connectWEPKey");
	e.innerHTML = _("secure wep key");
	e = document.getElementById("connectWEPKey1");
	e.innerHTML = _("secure wep key1");
	e = document.getElementById("connectWEPKey2");
	e.innerHTML = _("secure wep key2");
	e = document.getElementById("connectWEPKey3");
	e.innerHTML = _("secure wep key3");
	e = document.getElementById("connectWEPKey4");
	e.innerHTML = _("secure wep key4");
	e = document.getElementById("connectWEPDKey");
	e.innerHTML = _("secure wep default key");
	e = document.getElementById("connectDKey1");
	e.innerHTML = _("secure wep default key1");
	e = document.getElementById("connectDKey2");
	e.innerHTML = _("secure wep default key2");
	e = document.getElementById("connectDKey3");
	e.innerHTML = _("secure wep default key3");
	e = document.getElementById("connectDKey4");
	e.innerHTML = _("secure wep default key4");
	e = document.getElementById("connectWPA");
	e.innerHTML = _("secure wpa");
	e = document.getElementById("connectWPAAlg");
	e.innerHTML = _("secure wpa algorithm");
	e = document.getElementById("connectPassPhrase");
	e.innerHTML = _("secure wpa pass phrase");
	e = document.getElementById("connect1XAuth");
	e.innerHTML = _("addprof 8021X Auth Type");
	e = document.getElementById("connectWPAAuth");
	e.innerHTML = _("addprof 8021X Auth Type");
	e = document.getElementById("connectPEAPTunnelAuth");
	e.innerHTML = _("addprof tunnel auth");
	e = document.getElementById("connectTTLSTunnelAuth");
	e.innerHTML = _("addprof tunnel auth");
	e = document.getElementById("connectIdentity");
	e.innerHTML = _("addprof identity");
	e = document.getElementById("connectPasswd");
	e.innerHTML = _("addprof passwd");
	e = document.getElementById("connectClientCert");
	e.innerHTML = _("addprof client cert");
	e = document.getElementById("connectClientCertUsed");
	e.innerHTML = _("station used");
	e = document.getElementById("connectClientCertPath");
	e.innerHTML = _("addprof client cert path");
	e = document.getElementById("connectPriKeyPath");
	e.innerHTML = _("addprof private key path");
	e = document.getElementById("connectPriKeyPasswd");
	e.innerHTML = _("addprof private key passwd");
	e = document.getElementById("connectCACert");
	e.innerHTML = _("addprof ca cert");
	e = document.getElementById("connectCACertUsed");
	e.innerHTML = _("station used");
	e = document.getElementById("connectCACertPath");
	e.innerHTML = _("addprof ca cert path");
	e = document.getElementById("connectApply");
	e.value = _("wireless apply");
	e = document.getElementById("connectCancel");
	e.value = _("wireless cancel");
}
	
function initValue()
{
	initTranslation();
	opener.showConnectionSsid();
	var wpa_supplicantb = "<% getWPASupplicantBuilt(); %>";
	if (wpa_supplicantb == "1" )
	{
		var mode = document.sta_site_survey_connection.security_infra_mode;

		mode.options[mode.length] = new Option("WPA-Enterprise", "3");
		mode.options[mode.length] = new Option("WPA2-Enterprise", "6");
		mode.options[mode.length] = new Option("802.1x", "8");
	}
	securityMode();
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

function wep_switch_key_length()
{
	document.sta_site_survey_connection.wep_key_1.value = "";
	document.sta_site_survey_connection.wep_key_2.value = "";
	document.sta_site_survey_connection.wep_key_3.value = "";
	document.sta_site_survey_connection.wep_key_4.value = "";

	if (document.sta_site_survey_connection.wep_key_length.options.selectedIndex == 0) {
		<!-- KEY length 64 bits -->
		if (document.sta_site_survey_connection.wep_key_entry_method.options.selectedIndex == 0) {
			<!-- HEX -->
			document.sta_site_survey_connection.wep_key_1.maxLength = 10;
			document.sta_site_survey_connection.wep_key_2.maxLength = 10;
			document.sta_site_survey_connection.wep_key_3.maxLength = 10;
			document.sta_site_survey_connection.wep_key_4.maxLength = 10;
		}
		else {
			<!-- ASCII -->
			document.sta_site_survey_connection.wep_key_1.maxLength = 5;
			document.sta_site_survey_connection.wep_key_2.maxLength = 5;
			document.sta_site_survey_connection.wep_key_3.maxLength = 5;
			document.sta_site_survey_connection.wep_key_4.maxLength = 5;
		}
	}
	else {
		<!-- KEY length 128 bits -->
		if (document.sta_site_survey_connection.wep_key_entry_method.options.selectedIndex == 0) {
			<!-- HEX -->
			document.sta_site_survey_connection.wep_key_1.maxLength = 26;
			document.sta_site_survey_connection.wep_key_2.maxLength = 26;
			document.sta_site_survey_connection.wep_key_3.maxLength = 26;
			document.sta_site_survey_connection.wep_key_4.maxLength = 26;
		}
		else {
			<!-- ASCII -->
			document.sta_site_survey_connection.wep_key_1.maxLength = 13;
			document.sta_site_survey_connection.wep_key_2.maxLength = 13;
			document.sta_site_survey_connection.wep_key_3.maxLength = 13;
			document.sta_site_survey_connection.wep_key_4.maxLength = 13;
		}
	}
}

function securityMode()
{
	var security_mode;

	document.getElementById("div_open_none").style.visibility = "hidden";
	document.getElementById("div_open_none").style.display = "none";

	document.getElementById("div_security_infra_mode").style.visibility = "hidden";
	document.getElementById("div_security_infra_mode").style.display = "none";
	document.sta_site_survey_connection.security_infra_mode.disabled = true;

	document.getElementById("div_security_adhoc_mode").style.visibility = "hidden";
	document.getElementById("div_security_adhoc_mode").style.display = "none";
	document.sta_site_survey_connection.security_adhoc_mode.disabled = true;

	hideWep();

	document.getElementById("div_wpa").style.visibility = "hidden";
	document.getElementById("div_wpa").style.display = "none";
	document.getElementById("div_wpa_algorithms").style.visibility = "hidden";
	document.getElementById("div_wpa_algorithms").style.display = "none";
	document.getElementById("wpa_passphrase").style.visibility = "hidden";
	document.getElementById("wpa_passphrase").style.display = "none";
	document.sta_site_survey_connection.cipher[0].disabled = true;
	document.sta_site_survey_connection.cipher[1].disabled = true;
	document.sta_site_survey_connection.passphrase.disabled = true;

	// 802.1x
	document.getElementById("div_8021x").style.visibility = "hidden";
	document.getElementById("div_8021x").style.display = "none";
	document.getElementById("div_8021x_cert_from_wpa").style.visibility = "hidden";
	document.getElementById("div_8021x_cert_from_wpa").style.display = "none";
	document.getElementById("div_8021x_cert_from_1x").style.visibility = "hidden";
	document.getElementById("div_8021x_cert_from_1x").style.display = "none";
	document.sta_site_survey_connection.cert_auth_type_from_wpa.disabled = true;
	document.sta_site_survey_connection.cert_auth_type_from_1x.disabled = true;

	document.sta_site_survey_connection.cert_tunnel_auth_peap.disabled = true;
	document.sta_site_survey_connection.cert_tunnel_auth_ttls.disabled = true;
	document.sta_site_survey_connection.cert_id.disabled = true;
	document.sta_site_survey_connection.cert_password.disabled = true;
	document.sta_site_survey_connection.cert_client_cert_path.disabled = true;
	document.sta_site_survey_connection.cert_private_key_path.disabled = true;
	document.sta_site_survey_connection.cert_private_key_password.disabled = true;
	document.sta_site_survey_connection.cert_ca_cert_path.disabled = true;
	

	if (document.sta_site_survey_connection.network_type.value == 1) //infra
	{
		security_mode = document.sta_site_survey_connection.security_infra_mode.value;
		document.getElementById("div_security_infra_mode").style.visibility = "visible";
		document.getElementById("div_security_infra_mode").style.display = style_display_on();
		document.sta_site_survey_connection.security_infra_mode.disabled = false;
	}
	else
	{
		security_mode = document.sta_site_survey_connection.security_adhoc_mode.value;
		document.getElementById("div_security_adhoc_mode").style.visibility = "visible";
		document.getElementById("div_security_adhoc_mode").style.display = style_display_on();
		document.sta_site_survey_connection.security_adhoc_mode.disabled = false;
	}

	if (security_mode == 0 && document.sta_site_survey_connection.openmode.value == 1)
	{
		/*document.getElementById("div_hr").style.visibility = "hidden";
		document.getElementById("div_hr").style.display = "none";
		document.sta_site_survey_connection.applybtn.value="OK";
		document.getElementById("div_open_none").style.visibility = "visible";
		if (window.ActiveXObject) { // IE
			document.getElementById("div_open_none").style.display = "block";
		}
		else if (window.XMLHttpRequest) { // Mozilla, Safari,...
			document.getElementById("div_open_none").style.display = "table";
		}

		document.getElementById("div_ssid").style.visibility = "hidden";
		document.getElementById("div_ssid").style.display = "none";
		document.getElementById("div_security_mode").style.visibility = "hidden";
		document.getElementById("div_security_mode").style.display = "none";
		*/

		/*
		document.sta_site_survey_connection.Ssid.disabled = true;
		if (document.sta_site_survey_connection.network_type.value == 1) //infra
		{
			document.sta_site_survey_connection.security_infra_mode.disabled = true;
		}
		else
			document.sta_site_survey_connection.security_adhoc_mode.disabled = true;
		*/


		document.getElementById("div_open_none").style.visibility = "visible";
		if (window.ActiveXObject) { // IE
			document.getElementById("div_open_none").style.display = "block";
		}
		else if (window.XMLHttpRequest) { // Mozilla, Safari,...
			document.getElementById("div_open_none").style.display = "table";
		}
	}
	else if (security_mode == 0 || security_mode == 1)
	{
		showWep();
	}
	else if (security_mode == 4 || security_mode == 7 || security_mode == 5) {
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
		document.sta_site_survey_connection.cipher[0].disabled = false;
		document.sta_site_survey_connection.cipher[1].disabled = false;;

		document.getElementById("wpa_passphrase").style.visibility = "visible";
		document.getElementById("wpa_passphrase").style.display = style_display_on();
		document.sta_site_survey_connection.passphrase.disabled = false;
	}
	else if (security_mode == 3 || security_mode == 6 || security_mode == 8) //wpa enterprise, 802.1x
	{

		if (security_mode != 8)
		{
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
			document.sta_site_survey_connection.cipher[0].disabled = false;
			document.sta_site_survey_connection.cipher[1].disabled = false;;
		}
	
		<!-- 802.1x -->
		document.getElementById("div_8021x").style.visibility = "visible";
		if (window.ActiveXObject) { // IE
			document.getElementById("div_8021x").style.display = "block";
		}
		else if (window.XMLHttpRequest) { // Mozilla, Safari,...
			document.getElementById("div_8021x").style.display = "table";
		}
		
		if( security_mode != 8) //802.1x
		{
			document.getElementById("div_8021x_cert_from_wpa").style.visibility = "visible";
			document.getElementById("div_8021x_cert_from_wpa").style.display = style_display_on();
			document.sta_site_survey_connection.cert_auth_type_from_wpa.disabled = false;
		}
		else
		{
			document.getElementById("div_8021x_cert_from_1x").style.visibility = "visible";
			document.getElementById("div_8021x_cert_from_1x").style.display = style_display_on();
			document.sta_site_survey_connection.cert_auth_type_from_1x.disabled = false;
		}
		document.sta_site_survey_connection.cert_tunnel_auth_peap.disabled = false;
		document.sta_site_survey_connection.cert_tunnel_auth_ttls.disabled = false;
		document.sta_site_survey_connection.cert_id.disabled = false;
		document.sta_site_survey_connection.cert_password.disabled = false;

		use_ca_cert();
		use_client_cert();
		certAuthModeChange();
	}
}

function use_client_cert()
{
	if (document.sta_site_survey_connection.cert_use_client_cert.checked)
	{
		document.getElementById("div_client_cert_path").style.visibility = "visible";
		document.getElementById("div_client_cert_path").style.display = style_display_on();

		document.getElementById("div_private_key_path").style.visibility = "visible";
		document.getElementById("div_private_key_path").style.display = style_display_on();

		document.getElementById("div_private_key_password").style.visibility = "visible";
		document.getElementById("div_private_key_password").style.display = style_display_on();

		document.sta_site_survey_connection.cert_private_key_path.disabled = false;
		document.sta_site_survey_connection.cert_private_key_password.disabled = false;
		document.sta_site_survey_connection.cert_client_cert_path.disabled = false;
	}
	else
	{
		document.getElementById("div_client_cert_path").style.visibility = "hidden";
		document.getElementById("div_client_cert_path").style.display = "none";

		document.getElementById("div_private_key_path").style.visibility = "hidden";
		document.getElementById("div_private_key_path").style.display = "none";

		document.getElementById("div_private_key_password").style.visibility = "hidden";
		document.getElementById("div_private_key_password").style.display = "none";

		document.sta_site_survey_connection.cert_private_key_path.disabled = true;
		document.sta_site_survey_connection.cert_private_key_password.disabled = true;
		document.sta_site_survey_connection.cert_client_cert_path.disabled = true;
	}
}

function use_ca_cert()
{
	if (document.sta_site_survey_connection.cert_use_ca_cert.checked)
	{
		document.getElementById("div_ca_cert_path").style.visibility = "visible";
		document.getElementById("div_ca_cert_path").style.display = style_display_on();
		document.sta_site_survey_connection.cert_ca_cert_path.disabled = false;
	}
	else
	{
		document.getElementById("div_ca_cert_path").style.visibility = "hidden";
		document.getElementById("div_ca_cert_path").style.display = "none";
		document.sta_site_survey_connection.cert_ca_cert_path.disabled = true;
	}
}

function certAuthModeChange()
{
	var auth_mode;
	var security_infra_mode = document.sta_site_survey_connection.security_infra_mode.value;

	if (security_infra_mode == 3 || security_infra_mode == 6) //wpa-enterprise
		auth_mode = document.sta_site_survey_connection.cert_auth_type_from_wpa.value;
	else if (security_infra_mode == 8) // 802.1x
		auth_mode = document.sta_site_survey_connection.cert_auth_type_from_1x.value;

	hideWep();
	document.getElementById("div_tunnel_auth_peap").style.visibility = "hidden";
	document.getElementById("div_tunnel_auth_peap").style.display = "none";
	document.sta_site_survey_connection.cert_tunnel_auth_peap.disabled = true;

	document.getElementById("div_tunnel_auth_ttls").style.visibility = "hidden";
	document.getElementById("div_tunnel_auth_ttls").style.display = "none";
	document.sta_site_survey_connection.cert_tunnel_auth_ttls.disabled = true;
	
	document.getElementById("div_password").style.visibility = "hidden";
	document.getElementById("div_password").style.display = "none";
	document.sta_site_survey_connection.cert_password.disabled = true;
	
	document.sta_site_survey_connection.cert_id.disabled = true;

	document.sta_site_survey_connection.cert_use_client_cert.checked = false;
	document.getElementById("div_use_client_cert").style.visibility = "hidden";
	document.getElementById("div_use_client_cert").style.display = "none";
	document.sta_site_survey_connection.cert_use_client_cert.disabled = true;

	document.sta_site_survey_connection.cert_private_key_path.disabled = true;
	document.sta_site_survey_connection.cert_private_key_password.disabled = true;
	document.sta_site_survey_connection.cert_client_cert_path.disabled = true;
	if (auth_mode == 5 || auth_mode == 6) // PEAP & TTLS
	{
		if (auth_mode == 5)
		{
			document.getElementById("div_tunnel_auth_peap").style.visibility = "visible";
			document.getElementById("div_tunnel_auth_peap").style.display = style_display_on();
			document.sta_site_survey_connection.cert_tunnel_auth_peap.disabled = false;
		}
		else 
		{
			document.getElementById("div_tunnel_auth_ttls").style.visibility = "visible";
			document.getElementById("div_tunnel_auth_ttls").style.display = style_display_on();
			document.sta_site_survey_connection.cert_tunnel_auth_ttls.disabled = false;
		}
		
		document.sta_site_survey_connection.cert_id.disabled = false;
		
		document.getElementById("div_password").style.visibility = "visible";
		document.getElementById("div_password").style.display = style_display_on();
		document.sta_site_survey_connection.cert_password.disabled = false;

		document.getElementById("div_use_client_cert").style.visibility = "visible";
		document.getElementById("div_use_client_cert").style.display = style_display_on();
		document.sta_site_survey_connection.cert_use_client_cert.disabled = false;
	}
	else if( auth_mode == 4) //TLS
	{
		document.getElementById("div_use_client_cert").style.visibility = "visible";
		document.getElementById("div_use_client_cert").style.display = style_display_on();
		document.sta_site_survey_connection.cert_use_client_cert.disabled = true;

		document.sta_site_survey_connection.cert_use_client_cert.checked = true;

		document.sta_site_survey_connection.cert_id.disabled = false;
		use_client_cert();
	}
	else if ( auth_mode == 0) //MD5
	{
		document.sta_site_survey_connection.cert_id.disabled = false;
		
		document.getElementById("div_password").style.visibility = "visible";
		document.getElementById("div_password").style.display = style_display_on();
		document.sta_site_survey_connection.cert_password.disabled = false;
		showWep();
	}
	use_ca_cert();
	use_client_cert();

}

function checkData()
{
	var ssid;
	var securitymode;

	ssid = document.sta_site_survey_connection.Ssid.value;
	if (document.sta_site_survey_connection.network_type.value == 1) //infra
		securitymode = document.sta_site_survey_connection.security_infra_mode.value;
	else
		securitymode = document.sta_site_survey_connection.security_adhoc_mode.value;

	var keylen = 0;
	if (ssid.length <= 0)
	{
		alert('Pleaes input the SSID!');
		return false;
	}
	else if( securitymode  == 0 || securitymode  == 1)
	{
		return check_Wep(securitymode);
	}
	else if( securitymode  == 4 ||securitymode == 7 || securitymode == 5)
	{
		var keyvalue = document.sta_site_survey_connection.passphrase.value;

		if ( keyvalue.length == 0)
		{
			alert('Please input wpapsk key!');
			return false;
		}

		if ( keyvalue.length < 8)
		{
			alert('Please input at least 8 character of wpapsk key !');
			return false;
		}
	}
	//802.1x
	else if (securitymode == 3 || securitymode == 6 || securitymode == 8) //wpa enterprise, 802.1x
	{
		var certid = document.sta_site_survey_connection.cert_id.value;
		if ( certid.length == 0)
		{
			alert('Please input the 802.1x identity !');
			return false;
		}

		if ( document.sta_site_survey_connection.cert_password.disable == false)
		{
			var certpassword = document.sta_site_survey_connection.cert_password.value;
			if ( certpassword.length == 0)
			{
				alert('Please input the 802.1x password !');
				return false;
			}
		}

		if ( document.sta_site_survey_connection.cert_use_client_cert.checked == true)
		{
			var client_cert = document.sta_site_survey_connection.cert_client_cert_path.value;
			var private_key = document.sta_site_survey_connection.cert_private_key_path.value;
			var private_key_password = document.sta_site_survey_connection.cert_private_key_password.value;

			if( client_cert.length == 0)
			{
				alert('Please input the 802.1x Client Certificate Path !');
				return false;
			}

			if( private_key.length == 0)
			{
				alert('Please input the 802.1x Private Key Path !');
				return false;
			}

			if( private_key_password.length == 0)
			{
				alert('Please input the 802.1x Private Key Password !');
				return false;
			}
			
		}

		if ( document.sta_site_survey_connection.cert_use_ca_cert.checked == true)
		{
			var ca_cert_path = document.sta_site_survey_connection.cert_ca_cert_path.value;

			if( ca_cert_path.length == 0)
			{
				alert('Please input the 802.1x CA Certificate Path !');
				return false;
			}
		}

		if ( document.sta_site_survey_connection.cert_auth_type_from_1x.value == 0) //md5
			return check_Wep(securitymode);
	}
	return true;
}

function hideWep()
{
	document.getElementById("div_wep").style.visibility = "hidden";
	document.getElementById("div_wep").style.display = "none";
	//document.sta_site_survey_connection.wep_auth_type.disabled = true;
	document.sta_site_survey_connection.wep_key_length.disabled = true;
	document.sta_site_survey_connection.wep_key_entry_method.disabled = true;
	document.sta_site_survey_connection.wep_key_1.disabled = true;
	document.sta_site_survey_connection.wep_key_2.disabled = true;
	document.sta_site_survey_connection.wep_key_3.disabled = true;
	document.sta_site_survey_connection.wep_key_4.disabled = true;
	document.sta_site_survey_connection.wep_default_key.disabled = true;
}

function showWep()
{
	<!-- WEP -->
	document.getElementById("div_wep").style.visibility = "visible";

	if (window.ActiveXObject) { // IE 
		document.getElementById("div_wep").style.display = "block";
	}
	else if (window.XMLHttpRequest) { // Mozilla, Safari...
		document.getElementById("div_wep").style.display = "table";
	}

	//document.sta_site_survey_connection.wep_auth_type.disabled = false;
	document.sta_site_survey_connection.wep_key_length.disabled = false;
	document.sta_site_survey_connection.wep_key_entry_method.disabled = false;
	document.sta_site_survey_connection.wep_key_1.disabled = false;
	document.sta_site_survey_connection.wep_key_2.disabled = false;
	document.sta_site_survey_connection.wep_key_3.disabled = false;
	document.sta_site_survey_connection.wep_key_4.disabled = false;
	document.sta_site_survey_connection.wep_default_key.disabled = false;

	if (document.sta_site_survey_connection.wep_key_length.options.selectedIndex == 0) {
		<!-- KEY length 64 bits -->
		if (document.sta_site_survey_connection.wep_key_entry_method.options.selectedIndex == 0) {
		  <!-- HEX -->
		  document.sta_site_survey_connection.wep_key_1.maxLength = 10;
		  document.sta_site_survey_connection.wep_key_2.maxLength = 10;
		  document.sta_site_survey_connection.wep_key_3.maxLength = 10;
		  document.sta_site_survey_connection.wep_key_4.maxLength = 10;
		}
		else {
		  <!-- ASCII -->
		  document.sta_site_survey_connection.wep_key_1.maxLength = 5;
		  document.sta_site_survey_connection.wep_key_2.maxLength = 5;
		  document.sta_site_survey_connection.wep_key_3.maxLength = 5;
		  document.sta_site_survey_connection.wep_key_4.maxLength = 5;
		}
	}
	else {
		<!-- KEY length 128 bits -->
		if (document.sta_site_survey_connection.wep_key_entry_method.options.selectedIndex == 0) {
		  <!-- HEX -->
		  document.sta_site_survey_connection.wep_key_1.maxLength = 26;
		  document.sta_site_survey_connection.wep_key_2.maxLength = 26;
		  document.sta_site_survey_connection.wep_key_3.maxLength = 26;
		  document.sta_site_survey_connection.wep_key_4.maxLength = 26;
		}
		else {
		  <!-- ASCII -->
		  document.sta_site_survey_connection.wep_key_1.maxLength = 13;
		  document.sta_site_survey_connection.wep_key_2.maxLength = 13;
		  document.sta_site_survey_connection.wep_key_3.maxLength = 13;
		  document.sta_site_survey_connection.wep_key_4.maxLength = 13;
		}
	}
}

function open_cert_upload(target)
{
	if (target == "cacl")
		window.open("cert_cacl_upload.asp","cert_ca_client_upload","toolbar=no, location=yes, scrollbars=yes, resizable=no, width=500, height=500");
	else if (target == "key")
		window.open("cert_key_upload.asp","cert_key_upload","toolbar=no, location=yes, scrollbars=yes, resizable=no, width=500, height=500");
}

function check_Wep(securitymode)
{
	var defaultid = document.sta_site_survey_connection.wep_default_key.value;
	var keylen = 0 ;

	if ( defaultid == 1 )
		var keyvalue = document.sta_site_survey_connection.wep_key_1.value;
	else if (defaultid == 2)
		var keyvalue = document.sta_site_survey_connection.wep_key_2.value;
	else if (defaultid == 3)
		var keyvalue = document.sta_site_survey_connection.wep_key_3.value;
	else if (defaultid == 4)
		var keyvalue = document.sta_site_survey_connection.wep_key_4.value;

	if (document.sta_site_survey_connection.wep_key_length.options.selectedIndex == 0) {
		<!-- KEY length 64 bits -->
		if (document.sta_site_survey_connection.wep_key_entry_method.options.selectedIndex == 0) {
			<!-- HEX -->
			keylen = 10;
		}
		else
		{
			<!-- ASCII -->
			keylen = 5;
		}
	}
	else {
		<!-- KEY length 128 bits -->
		if (document.sta_site_survey_connection.wep_key_entry_method.options.selectedIndex == 0) {
			<!-- HEX -->
			keylen = 26;
		}
		else {
			<!-- ASCII -->
			keylen = 13;
		}
	}

	if (keyvalue.length == 0 && ((securitymode == 0 && document.sta_site_survey_connection.openmode.value == 0) ||
	    securitymode == 1 || document.sta_site_survey_connection.cert_auth_type_from_1x.value == 3)) // shared wep  || md5
	{
		alert('Please input wep key'+defaultid+' !');
		return false;
	}

	if ( keyvalue.length != 0)
	{
		if ( keyvalue.length != keylen)
		{
			alert('Please input '+keylen+' character of wep key !');
			return false;
		}
	}
	return true;
}
function submit_apply()
{
	if (checkData() == true)
	{
		/*
		document.sta_site_survey_connection.Ssid.disabled = false;
		if (document.sta_site_survey_connection.network_type.value == 1) //infra
			document.sta_site_survey_connection.security_infra_mode.disabled = false;
		else
			document.sta_site_survey_connection.security_adhoc_mode.disabled = false;
		*/

		document.sta_site_survey_connection.submit();
		opener.location.reload();
		//opener.location.href = "link_status.asp";
		window.close();
	}
}
</script>
</head>


<body onLoad="initValue()">
<table class="body"><tr><td>


<form method=post name="sta_site_survey_connection" id="sta_site_survey_connection" action="/goform/setStaConnect">
<table id="div_ssid" name="div_ssid" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr>
    <td class="head" id="connectSSID">SSID</td>
    <td><input type="text" name="Ssid" id="Ssid" maxlength=32></td>
  </tr>
</table>
<br />
<hr />

<table id="div_security_mode" name="div_security_mode" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" id="connectSecurePolicy">Security Policy</td>
  </tr>
  <tr id="div_security_infra_mode" name="div_security_infra_mode"> 
    <td class="head" id="connectSecureMode">Security Mode</td>
    <td>
      <select name="security_infra_mode" id="security_infra_mode" size="1" onChange="securityMode()">
	<option value=0 selected>OPEN</option>
	<option value=1>SHARED</option>
	<option value=4>WPA-Personal</option>
	<option value=7>WPA2-Personal</option>
	<!--
	<option value=3>WPA-Enterprise</option>
	<option value=6>WPA2-Enterprise</option>
	<option value=8>802.1x</option>
	-->
    </select>
    </td>
  </tr>
  <tr id="div_security_adhoc_mode" name="div_security_adhoc_mode"> 
    <td class="head" id="connectAdHocSecure">Security Mode</td>
    <td>
      <select name="security_adhoc_mode" id="security_adhoc_mode" size="1" onChange="securityMode()">
	<option value=0 selected>OPEN</option>
	<!--
	<option value=1>SHARED</option>
	-->
	<option value=5>WPA-NONE</option>
      </select>
    </td>
  </tr>
</table>
<input type=hidden name="network_type" >
<input type=hidden name="channel" >
<input type=hidden name="bssid" >
<input type=hidden name="openmode" >

<table id="div_open_none" name="div_open_none">
  <tr>
    <td align="center">
      <script> 
	document.write("&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;".fontsize(10));
	document.write("This is no any security. Are you sure to connect AP?".fontsize(3)); 
      </script>
    </td>
  </tr>
</table>
<br />

<table id="div_wep" name="div_wep" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="3" id="connectWEP">Wire Equivalence Protection (WEP)</td>
  </tr>
  <tr> 
    <td class="head" colspan="2" id="connectWEPKeyLength">WEP Key Length</td>
    <td>
      <select name="wep_key_length" size="1" onChange="wep_switch_key_length()">
	<option value=0 selected>64 bit (10 hex digits/ 5 ascii keys)</option>
	<option value=1>128 bit (26 hex digits/13 ascii keys)</option>
      </select>
    </td>
  </tr>
  <tr> 
    <td class="head" colspan="2" id="connectWEPKeyEntryMethod">WEP Key Entry Method</td>
    <td>
      <select name="wep_key_entry_method" size="1" onChange="wep_switch_key_length()">
	<option value=0 id="connectHex" selected>Hexadecimal</option>
	<option value=1 id="connectASCII">Ascii Text</option>
      </select>
    </td>
  </tr>
  <tr> 
    <td class="head1" rowspan="4" id="connectWEPKey">WEP Keys</td>
    <td class="head2" bgcolor="#E8F8FF" id="connectWEPKey1" nowrap>WEP Key 1 :</td>
    <td><input type=password name=wep_key_1 maxlength=26 value=""></td>
  </tr>
  <tr> 
    <td class="head2" id="connectWEPKey2">WEP Key 2 : </td>
    <td><input type=password name=wep_key_2 maxlength=26 value=""></td>
  </tr>
  <tr> 
    <td class="head2" id="connectWEPKey3">WEP Key 3 : </td>
    <td><input type=password name=wep_key_3 maxlength=26 value=""></td>
  </tr>
  <tr> 
    <td class="head2" id="connectWEPKey4">WEP Key 4 : </td>
    <td><input type=password name=wep_key_4 maxlength=26 value=""></td>
  </tr>
  <tr> 
    <td class="head" id="connectWEPDKey">Default Key</td>
    <td>
      <select name="wep_default_key" size="1">
	<option value=1 id="connectDKey1" selected>Key 1</option>
	<option value=2 id="connectDKey2">Key 2</option>
	<option value=3 id="connectDKey3">Key 3</option>
	<option value=4 id="connectDKey4">Key 4</option>
      </select>
    </td>
  </tr>
</table>

<table id="div_wpa" name="div_wpa" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" id="connectWPA">WPA</td>
  </tr>
  <tr id="div_wpa_algorithms" name="div_wpa_algorithms"> 
    <td class="head" id="connectWPAAlg">WPA Algorithms</td>
    <td>
	  <input type=radio name="cipher" id="cipher" value="0" checked>TKIP &nbsp;
	  <input type=radio name="cipher" id="cipher" value="1">AES &nbsp;
    </td>
  </tr>
  <tr id="wpa_passphrase" name="wpa_passphrase">
    <td class="head" id="connectPassPhrase">Pass Phrase</td>
    <td>
      <input type=password name=passphrase size=28 maxlength=64 value="">
    </td>
  </tr>
</table>
<br />

<!-- 802.1x -->
<table id="div_8021x" name="div_8021x" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" id="connect1X">802.1x</td>
  </tr>
  <tr id="div_8021x_cert_from_1x" name="div_8021x_cert_from_1x">
    <td class="head" id="connect1XAuth">Authentication Type</td>
    <td>
      <select name="cert_auth_type_from_1x" id="cert_auth_type_from_1x" size="1" onChange="certAuthModeChange()">
	<option value=5 selected>PEAP</option>
	<option value=6>TTLS</option>
	<option value=4>TLS</option>
	<option value=0>MD5</option>
      </select>
    </td>
  </tr>
  <tr id="div_8021x_cert_from_wpa" name="div_8021x_cert_from_wpa">
    <td class="head" id="connectWPAAuth">Authentication Type</td>
    <td>
      <select name="cert_auth_type_from_wpa" id="cert_auth_type_from_wpa" size="1" onChange="certAuthModeChange()">
	<option value=5 selected>PEAP</option>
	<option value=6>TTLS</option>
	<option value=4>TLS</option>
      </select>
    </td>
  </tr>
  <tr id="div_tunnel_auth_peap" name="div_tunnel_auth_peap">
    <td class="head" id="connectPEAPTunnelAuth">Tunnel Authentication</td>
    <td>
      <select name="cert_tunnel_auth_peap" id="cert_tunnel_auth_peap" size="1">
	<option value=1 selected>MSCHAP v2</option>
      </select>
    </td>
  </tr>
  <tr id="div_tunnel_auth_ttls" name="div_tunnel_auth_ttls">
    <td class="head" id="connectTTLSTunnelAuth">Tunnel Authentication</td>
    <td>
      <select name="cert_tunnel_auth_ttls" id="cert_tunnel_auth_ttls" size="1">
	<option value=0 selected>MSCHAP</option>
	<option value=1>MSCHAP v2</option>
	<option value=2>PAP</option>
      </select>
    </td>
  </tr>
  <tr id="div_identity" name="div_identity">
    <td class="head" id="connectIdentity">Identity</td>
    <td>
      <input type=text name="cert_id" maxlength=32>
    </td>
  </tr>
  <tr id="div_password" name="div_password">
    <td class="head" id="connectPasswd">Password</td>
    <td>
      <input type=password name="cert_password" maxlength=32>
    </td>
  </tr>
  <tr id="div_use_client_cert" name="div_use_client_cert">
    <td class="head" id="connectClientCert">Client Certificate</td>
    <td>
      <input type=checkbox name="cert_use_client_cert" onClick="use_client_cert()" ><font id="connectClientCertUsed">Used</font>
    </td>
  </tr>
  <tr id="div_client_cert_path" name="div_client_cert_path">
    <td class="head">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font id="connectClientCertPath">Client Certificate Path</font></td>
    <td>
      <select name="cert_client_cert_path" size="1">
        <% getCACLCertList(); %>
      </select>
      <input type="button" value="Install" name="ca_cl_cert_upload" id="ca_cl_cert_upload" onClick="open_cert_upload('cacl')">
    </td>
  </tr>
  <tr id="div_private_key_path" name="div_private_key_path">
    <td class="head">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font id="connectPriKeyPath">Private Key Path</font></td>
    <td>
      <select name="cert_private_key_path" size="1">
        <% getKeyCertList(); %>
      </select>
      <input type="button" value="Install" name="key_cert_upload" id="key_cert_upload" onClick="open_cert_upload('key')">
    </td>
  </tr>
  <tr id="div_private_key_password" name="div_private_key_password">
    <td class="head">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font id="connectPriKeyPasswd">Private Key Password</font></td>
    <td>
      <input type=password name="cert_private_key_password" maxlength=32>
    </td>
  </tr>
  <tr id="div_use_ca_cert" name="div_use_ca_cert">
    <td class="head" id="connectCACert">CA Certificate</td>
    <td>
      <input type=checkbox name="cert_use_ca_cert" onClick="use_ca_cert()" ><font id="connectCACertUsed">Used</font>
    </td>
  </tr>
  <tr id="div_ca_cert_path" name="div_ca_cert_path">
    <td class="head">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font id="connectCACertPath">CA Certificate Path</font></td>
    <td>
      <select name="cert_ca_cert_path" size="1">
        <% getCACLCertList(); %>
      </select>
    </td>
  </tr>
</table>

<br />
<table width="540" border="0" cellpadding="2" cellspacing="1">
  <tr align="center">
    <td>
      <input type=button style="{width:120px;}" value="Apply" id="connectApply" onClick="submit_apply()"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="connectCancel" onClick="window.close()">
    </td>
  </tr>
</table>
</form>


</td></tr></table>
</body>
