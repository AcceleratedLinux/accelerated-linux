<!-- Copyright (c), Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Ralink Wireless Station Add Profile</title>
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
	var e = document.getElementById("addprofSysConf");
	e.innerHTML = _("addprof system config");
	e = document.getElementById("addprofProfName");
	e.innerHTML = _("addprof profile name");
	e = document.getElementById("addprofSSID");
	e.innerHTML = _("station ssid");
	e = document.getElementById("addprofNetType");
	e.innerHTML = _("station network type");
	e = document.getElementById("addprofAdHoc");
	e.innerHTML = _("addprof adhoc");
	e = document.getElementById("addprofInfra");
	e.innerHTML = _("addprof infrastructure");
	e = document.getElementById("addprofPWSave");
	e.innerHTML = _("addprof power save");
	e = document.getElementById("addprofCAM");
	e.innerHTML = _("addprof cam");
	e = document.getElementById("addprofPWSaveMode");
	e.innerHTML = _("addprof power save");
	e = document.getElementById("addprofChannel");
	e.innerHTML = _("station channel");
	e = document.getElementById("addprofPreambleType");
	e.innerHTML = _("addprof preamble type");
	e = document.getElementById("addprofPremableAuto");
	e.innerHTML = _("wireless auto");
	e = document.getElementById("addprofPremableLong");
	e.innerHTML = _("wireless long");
	e = document.getElementById("addprofRTS");
	e.innerHTML = _("adv rts threshold");
	e = document.getElementById("addprofRTSUsed");
	e.innerHTML = _("station used");
	e = document.getElementById("addprofFrag");
	e.innerHTML = _("adv fragment threshold");
	e = document.getElementById("addprofFragUsed");
	e.innerHTML = _("station used");
	e = document.getElementById("addprofSecurePolicy");
	e.innerHTML = _("addprof secure policy");
	e = document.getElementById("addprofSecureMode");
	e.innerHTML = _("secure security mode");
	e = document.getElementById("addprofAdHocSecure");
	e.innerHTML = _("secure security mode");
	e = document.getElementById("addprofWEP");
	e.innerHTML = _("secure wep");
	e = document.getElementById("addprofWEPKeyLength");
	e.innerHTML = _("addprof wep key length");
	e = document.getElementById("addprofWEPKeyEntryMethod");
	e.innerHTML = _("addprof wep key entry method");
	e = document.getElementById("addprofHex");
	e.innerHTML = _("addprof hex");
	e = document.getElementById("addprofASCII");
	e.innerHTML = _("addprof ascii");
	e = document.getElementById("addprofWEPKey");
	e.innerHTML = _("secure wep key");
	e = document.getElementById("addprofWEPKey1");
	e.innerHTML = _("secure wep key1");
	e = document.getElementById("addprofWEPKey2");
	e.innerHTML = _("secure wep key2");
	e = document.getElementById("addprofWEPKey3");
	e.innerHTML = _("secure wep key3");
	e = document.getElementById("addprofWEPKey4");
	e.innerHTML = _("secure wep key4");
	e = document.getElementById("addprofDefaultKey");
	e.innerHTML = _("secure wep default key");
	e = document.getElementById("addprofDKey1");
	e.innerHTML = _("secure wep default key1");
	e = document.getElementById("addprofDKey2");
	e.innerHTML = _("secure wep default key2");
	e = document.getElementById("addprofDKey3");
	e.innerHTML = _("secure wep default key3");
	e = document.getElementById("addprofDKey4");
	e.innerHTML = _("secure wep default key4");
	e = document.getElementById("addprofWPA");
	e.innerHTML = _("secure wpa");
	e = document.getElementById("addprofWPAAlg");
	e.innerHTML = _("secure wpa algorithm");
	e = document.getElementById("addprofPassPhrase");
	e.innerHTML = _("secure wpa pass phrase");
	e = document.getElementById("addprof1XAuthType");
	e.innerHTML = _("addprof 8021X Auth Type");
	e = document.getElementById("addprofWPAAuthType");
	e.innerHTML = _("addprof 8021X Auth Type");
	e = document.getElementById("addprofPEAPTunnelAtuth");
	e.innerHTML = _("addprof tunnel auth");
	e = document.getElementById("addprofTTLSTunnelAuth");
	e.innerHTML = _("addprof tunnel auth");
	e = document.getElementById("addprofIdentity");
	e.innerHTML = _("addprof identity");
	e = document.getElementById("addprofPasswd");
	e.innerHTML = _("addprof passwd");
	e = document.getElementById("addprofClientCert");
	e.innerHTML = _("addprof client cert");
	e = document.getElementById("addprofClientCertUsed");
	e.innerHTML = _("station used");
	e = document.getElementById("addprofClientCertPath");
	e.innerHTML = _("addprof client cert path");
	e = document.getElementById("addprofPrivateKeyPath");
	e.innerHTML = _("addprof private key path");
	e = document.getElementById("addprofPrivateKeyPasswd");
	e.innerHTML = _("addprof private key passwd");
	e = document.getElementById("addprofCACert");
	e.innerHTML = _("addprof ca cert");
	e = document.getElementById("addprofCACertUsed");
	e.innerHTML = _("station used");
	e = document.getElementById("addprofCACertPath");
	e.innerHTML = _("addprof ca cert path");
	e = document.getElementById("addprofApply");
	e.value = _("wireless apply");
	e = document.getElementById("addprofCancel");
	e.value = _("wireless cancel");
}
	
function initValue()
{
	initTranslation();
	document.getElementById("div_power_saving_mode").style.visibility = "hidden";
	document.getElementById("div_power_saving_mode").style.display = "none";
	document.profile_page.power_saving_mode.disabled = true;

	document.getElementById("div_channel").style.visibility = "hidden";
	document.getElementById("div_channel").style.display = "none";
	document.profile_page.channel.disabled = true;

	document.getElementById("div_b_premable_type").style.visibility = "hidden";
	document.getElementById("div_b_premable_type").style.display = "none";
	document.profile_page.b_premable_type.disabled = true;

	var tmp = "<% getStaNewProfileName(); %>";
	document.profile_page.profile_name.value = tmp;

	if (opener.showProfileSsid) {
		opener.showProfileSsid();
	}

	networkTypeChange();
	RTSThresholdChange();
	FragmentThresholdChange();
}

function getChannel()
{
	var channel = "<% getStaAdhocChannel(); %>";
	var wireless_mode = "<% getCfgZero(1, "WirelessMode"); %>";

	var bg_channel = channel & 0xFF;
	var a_channel = (channel >> 8) & 0xFF;

	switch (wireless_mode)
	{
	case 3: // A/B/G mixed
	case 5: // A/B/G/N mixed
		getBGChannel(bg_channel);
	case 2: // A only
	case 8: // A/N mixed
		getAChannel(a_channel);
		break;
	default:
		getBGChannel(bg_channel);
		getAChannel(a_channel);
		break;
	}
}

function getBGChannel( channel )
{
	switch (channel)
	{
	case 0:
		document.write(ch1+ch2+ch3+ch4+ch5+ch6+ch7+ch8+ch9+ch10+ch11);
		break;
	case 1:
		document.write(ch1+ch2+ch3+ch4+ch5+ch6+ch7+ch8+ch9+ch10+ch11+ch12+ch13);
		break;
	case 2:
		document.write(ch10+ch11);
		break;
	case 3:
		document.write(ch10+ch11+ch12+ch13);
		break;
	case 4:
		document.write(ch14);
		break;
	case 5:
		document.write(ch1+ch2+ch3+ch4+ch5+ch6+ch7+ch8+ch9+ch10+ch11+ch12+ch13+ch14);
		break;
	case 6:
		document.write(ch3+ch4+ch5+ch6+ch7+ch8+ch9);
		break;
	case 7:
		document.write(ch5+ch6+ch7+ch8+ch9+ch10+ch11+ch12+ch13);
		break;
	default:
		break;
	}
}

function getAChannel( channel )
{
	switch (channel)
	{
	case 0:
		document.write(ch36+ch40+ch44+ch48+ch52+ch56+ch60+ch64+ch149+ch153+ch157+ch161+ch165);
		break;
	case 1:
		document.write(ch36+ch40+ch44+ch48+ch52+ch56+ch60+ch64+ch100+ch104+ch108+ch112+ch116+ch120+ch124+ch128+ch132+ch136+ch140);
		break;
	case 2:
		document.write(ch36+ch40+ch44+ch48+ch52+ch56+ch60+ch64);
		break;
	case 3:
		document.write(ch52+ch56+ch60+ch64+ch149+ch153+ch157+ch161);
		break;
	case 4:
		document.write(ch149+ch153+ch157+ch161+ch165);
		break;
	case 5:
		document.write(ch149+ch153+ch157+ch161);
		break;
	case 6:
		document.write(ch36+ch40+ch44+ch48);
		break;
	case 7:
		document.write(ch36+ch40+ch44+ch48+ch52+ch56+ch60+ch64+ch100+ch104+ch108+ch112+ch116+ch120+ch124+ch128+ch132+ch136+ch140+ch149+ch153+ch157+ch161+ch165);
		break;
	case 8:
		document.write(ch52+ch56+ch60+ch64);
		break;
	case 9:
		document.write(ch34+ch38+ch42+ch46);
		break;
	case 10:
		document.write(ch34+ch36+ch38+ch40+ch42+ch44+ch46+ch48+ch52+ch56+ch60+ch64);
		break;
	default:
		break;
	}
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

function networkTypeChange()
{
	var nmode;

	document.getElementById("div_b_premable_type").style.visibility = "hidden";
	document.getElementById("div_b_premable_type").style.display = "none";
	document.profile_page.b_premable_type.disabled = true;

	document.getElementById("div_power_saving_mode").style.visibility = "hidden";
	document.getElementById("div_power_saving_mode").style.display = "none";
	document.profile_page.power_saving_mode.disabled = true;

	document.getElementById("div_channel").style.visibility = "hidden";
	document.getElementById("div_channel").style.display = "none";
	document.profile_page.channel.disabled = true;

	document.getElementById("div_security_infra_mode").style.visibility = "hidden";
	document.getElementById("div_security_infra_mode").style.display = "none";
	document.profile_page.security_infra_mode.disabled = true;

	document.getElementById("div_security_adhoc_mode").style.visibility = "hidden";
	document.getElementById("div_security_adhoc_mode").style.display = "none";
	document.profile_page.security_adhoc_mode.disabled = true;

	document.getElementById("div_security_encryp_mode").style.visibility = "hidden";
	document.getElementById("div_security_encryp_mode").style.display = "none";
	document.profile_page.security_encryp_mode.disabled = true;

	nmode = 1*document.profile_page.network_type.options.selectedIndex;
	if (nmode==1)
	{
		document.getElementById("div_power_saving_mode").style.visibility = "visible";
		document.getElementById("div_power_saving_mode").style.display = style_display_on();
		document.profile_page.power_saving_mode.disabled = false;

		document.getElementById("div_security_infra_mode").style.visibility = "visible";
		document.getElementById("div_security_infra_mode").style.display = style_display_on();
		document.profile_page.security_infra_mode.disabled = false;
	}
	else 
	{
		document.getElementById("div_b_premable_type").style.visibility = "visible";
		document.getElementById("div_b_premable_type").style.display = style_display_on();
		document.profile_page.b_premable_type.disabled = false;

		document.getElementById("div_channel").style.visibility = "visible";
		document.getElementById("div_channel").style.display = style_display_on();
		document.profile_page.channel.disabled = false;

		document.getElementById("div_security_adhoc_mode").style.visibility = "visible";
		document.getElementById("div_security_adhoc_mode").style.display = style_display_on();
		document.profile_page.security_adhoc_mode.disabled = false;
	}
	securityMode();
}


function RTSThresholdChange()
{
	if( document.profile_page.rts_threshold.checked )
		document.profile_page.rts_thresholdvalue.disabled = false;
	else
		document.profile_page.rts_thresholdvalue.disabled = true;
}

function FragmentThresholdChange()
{
	if( document.profile_page.fragment_threshold.checked )
		document.profile_page.fragment_thresholdvalue.disabled = false;
	else
		document.profile_page.fragment_thresholdvalue.disabled = true;
}

function checkData()
{
	var securitymode;
	var profilename = document.profile_page.profile_name.value;
	var ssid = document.profile_page.Ssid.value;

	if (document.profile_page.network_type.value == 1) //infra
		securitymode = document.profile_page.security_infra_mode.value;
	else
		securitymode = document.profile_page.security_adhoc_mode.value;

	if (profilename.length <=0)
	{
		alert('Pleaes input the Profile Name!');
		return false;
	}
	else if (ssid.length <= 0)
	{
		alert('Pleaes input the SSID!');
		return false;
	}
	if (document.profile_page.fragment_threshold.checked &&
	    document.profile_page.fragment_thresholdvalue.value < 256 &&
	    document.profile_page.fragment_thresholdvalue.value > 2356)
	{
		alert('The range of Fragment Threshold is between 256 and 2346!');
		return false;
	}
	else if (securitymode  == 0 || securitymode  == 1)
	{
		return check_Wep(securitymode);
	}
	else if (securitymode  == 4 ||securitymode == 7 || securitymode == 5)
	{
		var keyvalue = document.profile_page.passphrase.value;

		if (keyvalue.length == 0)
		{
			alert('Please input wpapsk key!');
			return false;
		}

		if (keyvalue.length < 8)
		{
			alert('Please input at least 8 character of wpapsk key !');
			return false;
		}
	}
	//802.1x
	else if (securitymode == 3 || securitymode == 6 || securitymode == 8) //wpa enterprise, 802.1x
	{
		var certid = document.profile_page.cert_id.value;
		if (certid.length == 0)
		{
			alert('Please input the 802.1x identity !');
			return false;
		}

		if (document.profile_page.cert_password.disabled == false)
		{
			var certpassword = document.profile_page.cert_password.value;
			if (certpassword.length == 0)
			{
				alert('Please input the 802.1x password !');
				return false;
			}
		}

		if (document.profile_page.cert_use_client_cert.checked == true)
		{
			var client_cert = document.profile_page.cert_client_cert_path.value;
			var private_key = document.profile_page.cert_private_key_path.value;
			var private_key_password = document.profile_page.cert_private_key_password.value;

			if (client_cert.length == 0)
			{
				alert('Please input the 802.1x Client Certificate Path !');
				return false;
			}

			if (private_key.length == 0)
			{
				alert('Please input the 802.1x Private Key Path !');
				return false;
			}

			if (private_key_password.length == 0)
			{
				alert('Please input the 802.1x Private Key Password !');
				return false;
			}

		}
		else
		{
			document.profile_page.cert_client_cert_path.options.selectedIndex = 0;
			document.profile_page.cert_private_key_path.options.selectedIndex = 0;
		}

		if (document.profile_page.cert_use_ca_cert.checked == true)
		{
			// alert("CA cert check");
			var ca_cert_path = document.profile_page.cert_ca_cert_path.value;

			if (ca_cert_path.length == 0)
			{
				alert('Please input the 802.1x CA Certificate Path !');
				return false;
			}
		}
		else
			document.profile_page.cert_ca_cert_path.options.selectedIndex = 0;

		if (document.profile_page.cert_auth_type_from_1x.value == 0) //md5
			return check_Wep(securitymode);

	}
	else
	{
		var tmp = "<% getStaAllProfileName(); %>";
		if (tmp.indexOf(profilename) >= 0 && (tmp.length == profilename.length)) {
			alert('Duplicate Profile Name!');
			return false;
		}
	}
	return true;
}

function profileClose()
{
	opener.location.reload();
}

function wep_switch_key_length()
{
	document.profile_page.wep_key_1.value = "";
	document.profile_page.wep_key_2.value = "";
	document.profile_page.wep_key_3.value = "";
	document.profile_page.wep_key_4.value = "";

	if (document.profile_page.wep_key_length.options.selectedIndex == 0) {
		<!-- KEY length 64 bits -->
			if (document.profile_page.wep_key_entry_method.options.selectedIndex == 0) {
				<!-- HEX -->
					document.profile_page.wep_key_1.maxLength = 10;
				document.profile_page.wep_key_2.maxLength = 10;
				document.profile_page.wep_key_3.maxLength = 10;
				document.profile_page.wep_key_4.maxLength = 10;
			}
			else {
				<!-- ASCII -->
					document.profile_page.wep_key_1.maxLength = 5;
				document.profile_page.wep_key_2.maxLength = 5;
				document.profile_page.wep_key_3.maxLength = 5;
				document.profile_page.wep_key_4.maxLength = 5;
			}
	}
	else {
		<!-- KEY length 128 bits -->
			if (document.profile_page.wep_key_entry_method.options.selectedIndex == 0) {
				<!-- HEX -->
					document.profile_page.wep_key_1.maxLength = 26;
				document.profile_page.wep_key_2.maxLength = 26;
				document.profile_page.wep_key_3.maxLength = 26;
				document.profile_page.wep_key_4.maxLength = 26;
			}
			else {
				<!-- ASCII -->
					document.profile_page.wep_key_1.maxLength = 13;
				document.profile_page.wep_key_2.maxLength = 13;
				document.profile_page.wep_key_3.maxLength = 13;
				document.profile_page.wep_key_4.maxLength = 13;
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
	document.profile_page.security_infra_mode.disabled = true;

	document.getElementById("div_security_adhoc_mode").style.visibility = "hidden";
	document.getElementById("div_security_adhoc_mode").style.display = "none";
	document.profile_page.security_adhoc_mode.disabled = true;

	document.getElementById("div_security_encryp_mode").style.visibility = "hidden";
	document.getElementById("div_security_encryp_mode").style.display = "none";
	document.profile_page.security_encryp_mode.disabled = true;

	hideWep();

	document.getElementById("div_wpa").style.visibility = "hidden";
	document.getElementById("div_wpa").style.display = "none";
	document.getElementById("div_wpa_algorithms").style.visibility = "hidden";
	document.getElementById("div_wpa_algorithms").style.display = "none";
	document.getElementById("wpa_passphrase").style.visibility = "hidden";
	document.getElementById("wpa_passphrase").style.display = "none";
	document.profile_page.cipher[0].disabled = true;
	document.profile_page.cipher[1].disabled = true;
	document.profile_page.passphrase.disabled = true;

	// 802.1x
	document.getElementById("div_8021x").style.visibility = "hidden";
	document.getElementById("div_8021x").style.display = "none";
	document.getElementById("div_8021x_cert_from_wpa").style.visibility = "hidden";
	document.getElementById("div_8021x_cert_from_wpa").style.display = "none";
	document.getElementById("div_8021x_cert_from_1x").style.visibility = "hidden";
	document.getElementById("div_8021x_cert_from_1x").style.display = "none";
	document.profile_page.cert_auth_type_from_wpa.disabled = true;
	document.profile_page.cert_auth_type_from_1x.disabled = true;

	document.profile_page.cert_tunnel_auth_peap.disabled = true;
	document.profile_page.cert_tunnel_auth_ttls.disabled = true;
	document.profile_page.cert_id.disabled = true;
	document.profile_page.cert_password.disabled = true;
	document.profile_page.cert_client_cert_path.disabled = true;
	document.profile_page.cert_private_key_path.disabled = true;
	document.profile_page.cert_private_key_password.disabled = true;
	document.profile_page.cert_ca_cert_path.disabled = true;
	
	if (document.profile_page.network_type.value == 1) //infra
	{

		security_mode = document.profile_page.security_infra_mode.value;
		document.getElementById("div_security_infra_mode").style.visibility = "visible";
		document.getElementById("div_security_infra_mode").style.display = style_display_on();
		document.profile_page.security_infra_mode.disabled = false;
		var wpa_supplicantb = "<% getWPASupplicantBuilt(); %>";
		if (wpa_supplicantb == "1" && document.profile_page.security_infra_mode.length <= 4)
		{
			var mode = document.profile_page.security_infra_mode;

			mode.options[mode.length] = new Option("WPA-Enterprise", "3");
			mode.options[mode.length] = new Option("WPA2-Enterprise", "6");
			mode.options[mode.length] = new Option("802.1x", "8");
		}
	}
	else
	{
		security_mode = document.profile_page.security_adhoc_mode.value;
		document.getElementById("div_security_adhoc_mode").style.visibility = "visible";
		document.getElementById("div_security_adhoc_mode").style.display = style_display_on();
		document.profile_page.security_adhoc_mode.disabled = false;
	}

	if (security_mode == 0)
	{
		document.getElementById("div_security_encryp_mode").style.visibility = "visible";
		document.getElementById("div_security_encryp_mode").style.display = style_display_on();
		document.profile_page.security_encryp_mode.disabled = false;
		var encrypt_mode = document.profile_page.security_encryp_mode.value;

		if (encrypt_mode == 1) {
			showWep();
		} else {
			document.getElementById("div_open_none").style.visibility = "visible";
			if (window.ActiveXObject) { // IE
				document.getElementById("div_open_none").style.display = "block";
			}
			else if (window.XMLHttpRequest) { // Mozilla, Safari,...
				document.getElementById("div_open_none").style.display = "table";
			}
		}

	} else if (security_mode == 1) {
		showWep();
	}
	else if (security_mode == 4 || security_mode == 7 || security_mode == 5)
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
		document.profile_page.cipher[0].disabled = false;
		document.profile_page.cipher[1].disabled = false;;

		document.getElementById("wpa_passphrase").style.visibility = "visible";
		document.getElementById("wpa_passphrase").style.display = style_display_on();
		document.profile_page.passphrase.disabled = false;
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
			document.profile_page.cipher[0].disabled = false;
			document.profile_page.cipher[1].disabled = false;;
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
			document.profile_page.cert_auth_type_from_wpa.disabled = false;
		}
		else
		{
			document.getElementById("div_8021x_cert_from_1x").style.visibility = "visible";
			document.getElementById("div_8021x_cert_from_1x").style.display = style_display_on();
			document.profile_page.cert_auth_type_from_1x.disabled = false;
		}
		document.profile_page.cert_tunnel_auth_peap.disabled = false;
		document.profile_page.cert_tunnel_auth_ttls.disabled = false;
		document.profile_page.cert_id.disabled = false;
		document.profile_page.cert_password.disabled = false;

		use_ca_cert();
		use_client_cert();
		certAuthModeChange();
	}

}

function use_client_cert()
{
	if (document.profile_page.cert_use_client_cert.checked)
	{
		document.getElementById("div_client_cert_path").style.visibility = "visible";
		document.getElementById("div_client_cert_path").style.display = style_display_on();

		document.getElementById("div_private_key_path").style.visibility = "visible";
		document.getElementById("div_private_key_path").style.display = style_display_on();

		document.getElementById("div_private_key_password").style.visibility = "visible";
		document.getElementById("div_private_key_password").style.display = style_display_on();

		document.profile_page.cert_private_key_path.disabled = false;
		document.profile_page.cert_private_key_password.disabled = false;
		document.profile_page.cert_client_cert_path.disabled = false;
	}
	else
	{
		document.getElementById("div_client_cert_path").style.visibility = "hidden";
		document.getElementById("div_client_cert_path").style.display = "none";

		document.getElementById("div_private_key_path").style.visibility = "hidden";
		document.getElementById("div_private_key_path").style.display = "none";

		document.getElementById("div_private_key_password").style.visibility = "hidden";
		document.getElementById("div_private_key_password").style.display = "none";

		document.profile_page.cert_private_key_path.disabled = true;
		document.profile_page.cert_private_key_password.disabled = true;
		document.profile_page.cert_client_cert_path.disabled = true;
	}
}

function use_ca_cert()
{
	if (document.profile_page.cert_use_ca_cert.checked)
	{
		document.getElementById("div_ca_cert_path").style.visibility = "visible";
		document.getElementById("div_ca_cert_path").style.display = style_display_on();
		document.profile_page.cert_ca_cert_path.disabled = false;
	}
	else
	{
		document.getElementById("div_ca_cert_path").style.visibility = "hidden";
		document.getElementById("div_ca_cert_path").style.display = "none";
		document.profile_page.cert_ca_cert_path.disabled = true;
	}
}

function certAuthModeChange()
{
	var auth_mode;
	var security_infra_mode = document.profile_page.security_infra_mode.value;

	if (security_infra_mode == 3 || security_infra_mode == 6) //wpa-enterprise
		auth_mode = document.profile_page.cert_auth_type_from_wpa.value;
	else if (security_infra_mode == 8) // 802.1x
		auth_mode = document.profile_page.cert_auth_type_from_1x.value;

	hideWep();

	document.getElementById("div_tunnel_auth_peap").style.visibility = "hidden";
	document.getElementById("div_tunnel_auth_peap").style.display = "none";
	document.profile_page.cert_tunnel_auth_peap.disabled = true;

	document.getElementById("div_tunnel_auth_ttls").style.visibility = "hidden";
	document.getElementById("div_tunnel_auth_ttls").style.display = "none";
	document.profile_page.cert_tunnel_auth_ttls.disabled = true;

	document.getElementById("div_password").style.visibility = "hidden";
	document.getElementById("div_password").style.display = "none";
	document.profile_page.cert_password.disabled = true;

	document.profile_page.cert_id.disabled = true;

	document.profile_page.cert_use_client_cert.checked = false;
	document.getElementById("div_use_client_cert").style.visibility = "hidden";
	document.getElementById("div_use_client_cert").style.display = "none";
	document.profile_page.cert_use_client_cert.disabled = true;

	document.profile_page.cert_private_key_path.disabled = true;
	document.profile_page.cert_private_key_password.disabled = true;
	document.profile_page.cert_client_cert_path.disabled = true;
	if (auth_mode == 5 || auth_mode == 6) // PEAP & TTLS
	{
		if (auth_mode == 5)
		{
			document.getElementById("div_tunnel_auth_peap").style.visibility = "visible";
			document.getElementById("div_tunnel_auth_peap").style.display = style_display_on();
			document.profile_page.cert_tunnel_auth_peap.disabled = false;
		}
		else 
		{
			document.getElementById("div_tunnel_auth_ttls").style.visibility = "visible";
			document.getElementById("div_tunnel_auth_ttls").style.display = style_display_on();
			document.profile_page.cert_tunnel_auth_ttls.disabled = false;
		}

		document.profile_page.cert_id.disabled = false;

		document.getElementById("div_password").style.visibility = "visible";
		document.getElementById("div_password").style.display = style_display_on();
		document.profile_page.cert_password.disabled = false;

		document.getElementById("div_use_client_cert").style.visibility = "visible";
		document.getElementById("div_use_client_cert").style.display = style_display_on();
		document.profile_page.cert_use_client_cert.disabled = false;

	}
	else if( auth_mode == 4) //TLS
	{
		document.getElementById("div_use_client_cert").style.visibility = "visible";
		document.getElementById("div_use_client_cert").style.display = style_display_on();
		document.profile_page.cert_use_client_cert.disabled = true;

		document.profile_page.cert_use_client_cert.checked = true;

		document.profile_page.cert_id.disabled = false;
	}
	else if ( auth_mode == 0) //MD5
	{
		document.profile_page.cert_id.disabled = false;

		document.getElementById("div_password").style.visibility = "visible";
		document.getElementById("div_password").style.display = style_display_on();
		document.profile_page.cert_password.disabled = false;

		showWep();
	}
	use_client_cert();
	use_ca_cert();
}
function hideWep()
{
	document.getElementById("div_wep").style.visibility = "hidden";
	document.getElementById("div_wep").style.display = "none";
	//document.profile_page.wep_auth_type.disabled = true;
	document.profile_page.wep_key_length.disabled = true;
	document.profile_page.wep_key_entry_method.disabled = true;
	document.profile_page.wep_key_1.disabled = true;
	document.profile_page.wep_key_2.disabled = true;
	document.profile_page.wep_key_3.disabled = true;
	document.profile_page.wep_key_4.disabled = true;
	document.profile_page.wep_default_key.disabled = true;
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

		//document.profile_page.wep_auth_type.disabled = false;
		document.profile_page.wep_key_length.disabled = false;
		document.profile_page.wep_key_entry_method.disabled = false;
		document.profile_page.wep_key_1.disabled = false;
		document.profile_page.wep_key_2.disabled = false;
		document.profile_page.wep_key_3.disabled = false;
		document.profile_page.wep_key_4.disabled = false;
		document.profile_page.wep_default_key.disabled = false;

		if (document.profile_page.wep_key_length.options.selectedIndex == 0) {
			<!-- KEY length 64 bits -->
				if (document.profile_page.wep_key_entry_method.options.selectedIndex == 0) {
					<!-- HEX -->
						document.profile_page.wep_key_1.maxLength = 10;
					document.profile_page.wep_key_2.maxLength = 10;
					document.profile_page.wep_key_3.maxLength = 10;
					document.profile_page.wep_key_4.maxLength = 10;
				}
				else {
					<!-- ASCII -->
						document.profile_page.wep_key_1.maxLength = 5;
					document.profile_page.wep_key_2.maxLength = 5;
					document.profile_page.wep_key_3.maxLength = 5;
					document.profile_page.wep_key_4.maxLength = 5;
				}
		}
		else {
			<!-- KEY length 128 bits -->
				if (document.profile_page.wep_key_entry_method.options.selectedIndex == 0) {
					<!-- HEX -->
						document.profile_page.wep_key_1.maxLength = 26;
					document.profile_page.wep_key_2.maxLength = 26;
					document.profile_page.wep_key_3.maxLength = 26;
					document.profile_page.wep_key_4.maxLength = 26;
				}
				else {
					<!-- ASCII -->
						document.profile_page.wep_key_1.maxLength = 13;
					document.profile_page.wep_key_2.maxLength = 13;
					document.profile_page.wep_key_3.maxLength = 13;
					document.profile_page.wep_key_4.maxLength = 13;
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
	var defaultid = document.profile_page.wep_default_key.value;
	var keylen = 0 ;

	if ( defaultid == 1 )
		var keyvalue = document.profile_page.wep_key_1.value;
	else if (defaultid == 2)
		var keyvalue = document.profile_page.wep_key_2.value;
	else if (defaultid == 3)
		var keyvalue = document.profile_page.wep_key_3.value;
	else if (defaultid == 4)
		var keyvalue = document.profile_page.wep_key_4.value;

	if (document.profile_page.wep_key_length.options.selectedIndex == 0) {
		<!-- KEY length 64 bits -->
			if (document.profile_page.wep_key_entry_method.options.selectedIndex == 0) {
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
			if (document.profile_page.wep_key_entry_method.options.selectedIndex == 0) {
				<!-- HEX -->
					keylen = 26;
			}
			else {
				<!-- ASCII -->
					keylen = 13;
			}
	}

	if (keyvalue.length == 0 && ((securitymode == 0 && document.profile_page.openmode.value == 0) ||
	    securitymode == 1 || document.profile_page.cert_auth_type_from_1x.value == 3)) // shared wep  || md5
	{
		alert('Please input wep key'+defaultid+' !');
		return false;
	}

	if (keyvalue.length != 0)
	{
		if (keyvalue.length != keylen)
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
		document.profile_page.submit();
		opener.location.reload();
		window.close();
	}
}
</script>
</head>

<body onLoad="initValue()" onUnload="profileClose()">
<table class="body"><tr><td>


<form method=post name="profile_page" action="/goform/addStaProfile">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" id="addprofSysConf">System Configuration</td>
  </tr>
  <tr>
    <td class="head" id="addprofProfName">Profile Name</td>
    <td><input type=text name="profile_name" maxlength=32></td>
  </tr>
  <tr>
    <td class="head" id="addprofSSID">SSID</td>
    <td><input type=text name="Ssid" maxlength=32></td>
  </tr>
  <tr>
    <td class="head" id="addprofNetType">Network Type</td>
    <td>
      <select name="network_type" size="1" onChange="networkTypeChange()">
	<option value=0 id="addprofAdHoc">802.11 Ad Hoc</option>
	<option value=1 id="addprofInfra" selected>Infrastructure</option>
      </select>
    </td>
  </tr>
  <tr id="div_power_saving_mode" name="div_power_saving_mode">
    <td class="head" id="addprofPWSave">Power Saving Mode</td>
    <td>
      <input type=radio name="power_saving_mode" value="0" checked><font id="addprofCAM">CAM (Constantly Awake Mode)</font>
      <br>
      <input type=radio name="power_saving_mode" value="1"><font id="addprofPWSaveMode">Power Saving Mode</font>
    </td>
  </tr>
  <tr id="div_channel" name="div_channel">
    <td class="head" id="addprofChannel">Channel</td>
    <td>
      <select name="channel" size="1">
	<script>getChannel();</script>
      </select>
    </td>
  </tr>
  <tr id="div_b_premable_type" name="div_b_premable_type">
    <td class="head" id="addprofPreambleType">11B Premable Type</td>
    <td>
      <select name="b_premable_type" size="1">
	<option value=0 id="addprofPremableAuto" selected>Auto</option>
	<option value=1 id="addprofPremableLong">Long</option>
      </select>
    </td>
  </tr>
  <tr>
    <td class="head" id="addprofRTS"> RTS Threshold </td>
    <td>
      <input type=checkbox name=rts_threshold onClick="RTSThresholdChange()"><font id="addprofRTSUsed"> Used &nbsp;&nbsp;</font>
      <input type=text name=rts_thresholdvalue value=2347>
    </td>
  </tr>
  <tr>
    <td class="head" id="addprofFrag"> Fragement Threshold </td>
    <td>
      <input type=checkbox name=fragment_threshold onClick="FragmentThresholdChange()"><font id="addprofFragUsed"> Used &nbsp;&nbsp;</font>
      <input type=text name=fragment_thresholdvalue value=2346>
    </td>
  </tr>
</table>
<hr width="540" align="left">

<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" id="addprofSecurePolicy">Security Policy</td>
  </tr>
  <tr id="div_security_infra_mode" name="div_security_infra_mode"> 
    <td class="head" id="addprofSecureMode">Security Mode</td>
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
    <td class="head"i id="addprofAdHocSecure">Security Mode</td>
    <td>
      <select name="security_adhoc_mode" id="security_adhoc_mode" size="1" onChange="securityMode()">
	<option value=0 id="addprofAdHocOpen" selected>OPEN</option>
	<option value=5 id="addprofAdHocWPA-None">WPA-NONE</option>
      </select>
    </td>
  </tr>
  <tr id="div_security_encryp_mode" name="div_security_encryp_mode"> 
    <td class="head"i id="addprofEncryp">Encryption Mode</td>
    <td>
      <select name="security_encryp_mode" id="security_encryp_mode" size="1" onChange="securityMode()">
	<option value=0 id="addprofNONE" selected>NONE</option>
	<option value=1 id="addprofWep">WEP</option>
      </select>
    </td>
  </tr>
</table>
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

<!-- WEP -->
<table id="div_wep" name="div_wep" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="3" id="addprofWEP">Wire Equivalence Protection (WEP)</td>
  </tr>
  <tr> 
    <td class="head" colspan="2" id="addprofWEPKeyLength">WEP Key Length</td>
    <td>
      <select name="wep_key_length" size="1" onChange="wep_switch_key_length()">
	<option value=0 selected>64 bit (10 hex digits / 5 ascii keys)</option>
	<option value=1>128 bit (26 hex digits / 13 ascii keys)</option>
      </select>
    </td>
  </tr>
  <tr> 
    <td class="head" colspan="2" id="addprofWEPKeyEntryMethod">WEP Key Entry Method</td>
    <td>
      <select name="wep_key_entry_method" size="1" onChange="wep_switch_key_length()">
	<option value=0 id="addprofHex" selected >Hexadecimal</option>
	<option value=1 id="addprofASCII">Ascii Text</option>
      </select>
    </td>
  </tr>
  <tr> 
    <td class="head1" rowspan="4" id="addprofWEPKey">WEP Keys</td>
    <td class="head2" id="addprofWEPKey1">WEP Key 1 :</td>
    <td><input type=password name=wep_key_1 maxlength=26 value=""></td>
  </tr>
  <tr> 
    <td class="head2" id="addprofWEPKey2">WEP Key 2 : </td>
    <td><input type=password name=wep_key_2 maxlength=26 value=""></td>
  </tr>
  <tr> 
    <td class="head2" id="addprofWEPKey3">WEP Key 3 : </td>
    <td><input type=password name=wep_key_3 maxlength=26 value=""></td>
  </tr>
  <tr> 
    <td class="head2" id="addprofWEPKey4">WEP Key 4 : </td>
    <td><input type=password name=wep_key_4 maxlength=26 value=""></td>
  </tr>
  <tr> 
    <td class="head" colspan="2" id="addprofDefaultKey">Default Key</td>
    <td>
      <select name="wep_default_key" size="1">
	<option value=1 selected id="addprofDKey1">Key 1</option>
	<option value=2 id="addprofDKey2">Key 2</option>
	<option value=3 id="addprofDKey3">Key 3</option>
	<option value=4 id="addprofDKey4">Key 4</option>
      </select>
    </td>
  </tr>
</table>
<br />

<!-- WPA -->
<table id="div_wpa" name="div_wpa" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" id="addprofWPA">WPA</td>
  </tr>
  <tr id="div_wpa_algorithms" name="div_wpa_algorithms"> 
    <td class="head" id="addprofWPAAlg">WPA Algorithms</td>
    <td>
      <input type=radio name="cipher" id="cipher" value="0" checked>TKIP &nbsp;
      <input type=radio name="cipher" id="cipher" value="1">AES &nbsp;
    </td>
  </tr>
  <tr id="wpa_passphrase" name="wpa_passphrase">
    <td class="head" id="addprofPassPhrase">Pass Phrase</td>
    <td>
      <input type=password name=passphrase size=28 maxlength=64 value="">
    </td>
  </tr>
</table>
<br />

<!-- 802.1x -->
<table id="div_8021x" name="div_8021x" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" >802.1x</td>
  </tr>
  <tr id="div_8021x_cert_from_1x" name="div_8021x_cert_from_1x">
    <td class="head" id="addprof1XAuthType">Authentication Type</td>
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
    <td class="head" id="addprofWPAAuthType">Authentication Type</td>
    <td>
      <select name="cert_auth_type_from_wpa" id="cert_auth_type_from_wpa" size="1" onChange="certAuthModeChange()">
	<option value=5 selected>PEAP</option>
	<option value=6>TTLS</option>
	<option value=4>TLS</option>
      </select>
    </td>
  </tr>
  <tr id="div_tunnel_auth_peap" name="div_tunnel_auth_peap">
    <td class="head" id="addprofPEAPTunnelAtuth">Tunnel Authentication</td>
    <td>
      <select name="cert_tunnel_auth_peap" id="cert_tunnel_auth_peap" size="1">
	<option value=1 selected>MSCHAP v2</option>
      </select>
    </td>
  </tr>
  <tr id="div_tunnel_auth_ttls" name="div_tunnel_auth_ttls">
    <td class="head" id="addprofTTLSTunnelAuth">Tunnel Authentication</td>
    <td>
      <select name="cert_tunnel_auth_ttls" id="cert_tunnel_auth_ttls" size="1">
	<option value=0 selected>MSCHAP</option>
	<option value=1>MSCHAP v2</option>
	<option value=2>PAP</option>
      </select>
    </td>
  </tr>
  <tr id="div_identity" name="div_identity">
    <td class="head" id="addprofIdentity">Identity</td>
    <td>
      <input type=text name="cert_id" maxlength=32>
    </td>
  </tr>
  <tr id="div_password" name="div_password">
    <td class="head" id="addprofPasswd">Password</td>
    <td>
      <input type=password name="cert_password" maxlength=32>
    </td>
  </tr>
  <tr id="div_use_client_cert" name="div_use_client_cert">
    <td class="head" id="addprofClientCert">Client Certificate</td>
    <td>
      <input type=checkbox name="cert_use_client_cert" onClick="use_client_cert()"><font id="addprofClientCertUsed">Used</font>
    </td>
  <tr id="div_client_cert_path" name="div_client_cert_path">
    <td class="head">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font id="addprofClientCertPath">Client Certificate Path</font></td>
    <td>
      <select name="cert_client_cert_path" size="1">
        <% getCACLCertList(); %>
      </select>
      <input type="button" value="Install" name="ca_cl_cert_upload" id="ca_cl_cert_upload" onClick="open_cert_upload('cacl')">
    </td>
  </tr>
  </tr>
  <tr id="div_private_key_path" name="div_private_key_path">
    <td class="head">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font id="addprofPrivateKeyPath">Private Key Path</font></td>
    <td>
      <select name="cert_private_key_path" size="1">
        <% getKeyCertList(); %>
      </select>
      <input type="button" value="Install" name="key_cert_upload" id="key_cert_upload" onClick="open_cert_upload('cacl')">
    </td>
  </tr>
  <tr id="div_private_key_password" name="div_private_key_password">
    <td class="head">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font id="addprofPrivateKeyPasswd">Private Key Password</font></td>
    <td>
      <input type=password name="cert_private_key_password" maxlength=32>
    </td>
  </tr>
  <tr id="div_use_ca_cert" name="div_use_ca_cert">
    <td class="head" id="addprofCACert">CA Certificate</td>
    <td>
      <input type=checkbox name="cert_use_ca_cert" onClick="use_ca_cert()"><font id="addprofCACertUsed">Used</font>
    </td>
  </tr>
  <tr id="div_ca_cert_path" name="div_ca_cert_path">
    <td class="head">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<font id="addprofCACertPath">CA Certificate Path</font></td>
    <td>
      <select name="cert_ca_cert_path" size="1">
        <% getCACLCertList(); %>
      </select>
    </td>
  </tr>
</table>
<br />

<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=button style="{width:120px;}" value="Apply" id="addprofApply" onClick="submit_apply()"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="addprofCancel" onClick="window.close()">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
