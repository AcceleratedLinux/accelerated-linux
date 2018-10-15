<html>
<head>
<title>Wizard Internet Settings</title>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script language="JavaScript" type="text/javascript">
var step_num;
var pppoecdb = "<% getPPPOECDBuilt(); %>";

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

function checkInjection(str)
{
	var len = str.length;
	for (var i=0; i<str.length; i++) {
		if ( str.charAt(i) == '\r' || str.charAt(i) == '\n'){
				return false;
		}else
	        continue;
	}
    return true;
}

function change_step(offset)
{
	// alert("Offset: "+offset+"; From Step: "+ step_num);
	if (offset == 1)
	{
		if (step_num == 2)
		{
			if (!CheckWanValue())
				return false;
		}
		else if (step_num == 3)
		{
			if (!CheckWirelessValue())
				return false;
		}
	}
	hideAllTables();
	if (offset == 0)
		step_num = offset;
	else
		step_num += offset;

	if (step_num == 1)
	{
		// alert("step: "+step_num);
		document.getElementById("div_wan_opmode").style.visibility = "visible";
		document.getElementById("div_wan_opmode").style.display = style_display_on();
		document.getElementById("div_wan_title").style.visibility = "visible";
		document.getElementById("div_wan_title").style.display = style_display_on()
		document.wizard_form.back_motion.disabled = false;
		document.wizard_form.next_motion.disabled = false;
		document.wizard_form.cancel_motion.disabled = false;
		document.wizard_form.apply_motion.disabled = true;
	}
	else if (step_num == 2)
	{
		// alert("step: "+step_num);
		if (document.wizard_form.connectionType[0].checked == true) 
		{
			if (offset == 1)
				change_step(1);
			else if (offset == -1)
				change_step(-1);
		}
		else
		{
			document.getElementById("div_wan_title").style.visibility = "visible";
			document.getElementById("div_wan_title").style.display = style_display_on();
			connectionTypeSwitch();
			document.wizard_form.back_motion.disabled = false;
			document.wizard_form.next_motion.disabled = false;
			document.wizard_form.cancel_motion.disabled = false;
			document.wizard_form.apply_motion.disabled = true;
		}
	}
	else if (step_num == 3)
	{
		// alert("step: "+step_num);
		document.getElementById("div_wireless_title").style.visibility = "visible";
		document.getElementById("div_wireless_title").style.display = style_display_on();
		document.getElementById("div_wireless").style.visibility = "visible";
		document.getElementById("div_wireless").style.display = style_display_on();
		document.wizard_form.ssid.disabled = false;
		switch_security_mode();
		document.wizard_form.back_motion.disabled = false;
		document.wizard_form.next_motion.disabled = true;
		document.wizard_form.cancel_motion.disabled = false;
		document.wizard_form.apply_motion.disabled = false;
	}
	else if (step_num == 4)
	{
		// alert("step: "+step_num);
		document.wizard_form.submit();
	}
	else
	{
		// alert("step: "+step_num);
		window.location.reload();
	}
}

function hideAllTables()
{
	document.getElementById("div_wizard_home").style.visibility = "hidden";
	document.getElementById("div_wizard_home").style.display = "none";
	document.getElementById("div_wan_title").style.visibility = "hidden";
	document.getElementById("div_wan_title").style.display = "none";
	document.getElementById("div_wan_opmode").style.visibility = "hidden";
	document.getElementById("div_wan_opmode").style.display = "none";
	hideWanOption();
	document.getElementById("div_wireless_title").style.visibility = "hidden";
	document.getElementById("div_wireless_title").style.display = "none";
	document.getElementById("div_wireless").style.visibility = "hidden";
	document.getElementById("div_wireless").style.display = "none";
}

function hideWanOption()
{
	document.getElementById("div_wan_static").style.visibility = "hidden";
	document.getElementById("div_wan_static").style.display = "none";
	document.getElementById("div_wan_pppoe").style.visibility = "hidden";
	document.getElementById("div_wan_pppoe").style.display = "none";
	document.getElementById("div_wan_l2tp").style.visibility = "hidden";
	document.getElementById("div_wan_l2tp").style.display = "none";
	document.getElementById("div_wan_pptp").style.visibility = "hidden";
	document.getElementById("div_wan_pptp").style.display = "none";
	document.getElementById("div_wan_3G").style.visibility = "hidden";
	document.getElementById("div_wan_3G").style.display = "none";
}

function disableWanSettings()
{
	document.wizard_form.staticIp.disabled = true;
	document.wizard_form.staticNetmask.disabled = true;
	document.wizard_form.staticGateway.disabled = true;
	document.wizard_form.staticPriDns.disabled = true;
	document.wizard_form.staticSecDns.disabled = true;
	document.wizard_form.pppoeUser.disabled = true;
	document.wizard_form.pppoePass.disabled = true;
	document.wizard_form.pppoePass2.disabled = true;
	document.wizard_form.pppoeOPMode.disabled = true;
	document.wizard_form.pppoeRedialPeriod.disabled = true;
	document.wizard_form.pppoeIdleTime.disabled = true;
	document.wizard_form.l2tpServer.disabled = true;
	document.wizard_form.l2tpUser.disabled = true;
	document.wizard_form.l2tpPass.disabled = true;
	document.wizard_form.l2tpMode.disabled = true;
	document.wizard_form.l2tpIp.disabled = true;
	document.wizard_form.l2tpNetmask.disabled = true;
	document.wizard_form.l2tpGateway.disabled = true;
	document.wizard_form.l2tpOPMode.disabled = true;
	document.wizard_form.l2tpRedialPeriod.disabled = true;
	document.wizard_form.pptpServer.disabled = true;
	document.wizard_form.pptpUser.disabled = true;
	document.wizard_form.pptpPass.disabled = true;
	document.wizard_form.pptpMode.disabled = true;
	document.wizard_form.pptpIp.disabled = true;
	document.wizard_form.pptpNetmask.disabled = true;
	document.wizard_form.pptpGateway.disabled = true;
	document.wizard_form.pptpOPMode.disabled = true;
	document.wizard_form.pptpRedialPeriod.disabled = true;
	document.wizard_form.Dev3G.disabled = true;
}

function disableWirelessSettings()
{
	document.wizard_form.ssid.disabled = true;
	document.wizard_form.security_key.disabled = true;
}

function connectionTypeSwitch()
{
	hideWanOption();
	if (document.wizard_form.connectionType[1].checked == true) {
		document.getElementById("div_wan_static").style.visibility = "visible";
		document.getElementById("div_wan_static").style.display = style_display_on();
		document.wizard_form.staticIp.disabled = false;
		document.wizard_form.staticNetmask.disabled = false;
		document.wizard_form.staticGateway.disabled = false;
		document.wizard_form.staticPriDns.disabled = false;
		document.wizard_form.staticSecDns.disabled = false;
	}
	else if (document.wizard_form.connectionType[2].checked == true) {
		document.getElementById("div_wan_pppoe").style.visibility = "visible";
		document.getElementById("div_wan_pppoe").style.display = style_display_on();
		document.wizard_form.pppoeUser.disabled = false;
		document.wizard_form.pppoePass.disabled = false;
		document.wizard_form.pppoePass2.disabled = false;
		if (pppoecdb == "1")
		{
			document.getElementById("div_pppoe_opmode").style.visibility = "hidden";
			document.getElementById("div_pppoe_opmode").style.display = "none";
		}
		else 
		{
			document.wizard_form.pppoeOPMode.disabled = false;
		}
		pppoeOPModeSwitch();
	}
	else if (document.wizard_form.connectionType[3].checked == true) {
		document.getElementById("div_wan_l2tp").style.visibility = "visible";
		document.getElementById("div_wan_l2tp").style.display = style_display_on();
		document.wizard_form.l2tpServer.disabled = false;
		document.wizard_form.l2tpUser.disabled = false;
		document.wizard_form.l2tpPass.disabled = false;
		document.wizard_form.l2tpMode.disabled = false;
		document.wizard_form.l2tpIp.disabled = false;
		document.wizard_form.l2tpNetmask.disabled = false;
		document.wizard_form.l2tpGateway.disabled = false;
		document.wizard_form.l2tpOPMode.disabled = false;
		l2tpOPModeSwitch();
	}
	else if (document.wizard_form.connectionType[4].checked == true) {
		document.getElementById("div_wan_pptp").style.visibility = "visible";
		document.getElementById("div_wan_pptp").style.display = style_display_on();
		document.wizard_form.pptpServer.disabled = false;
		document.wizard_form.pptpUser.disabled = false;
		document.wizard_form.pptpPass.disabled = false;
		document.wizard_form.pptpMode.disabled = false;
		document.wizard_form.pptpIp.disabled = false;
		document.wizard_form.pptpNetmask.disabled = false;
		document.wizard_form.pptpGateway.disabled = false;
		document.wizard_form.pptpOPMode.disabled = false;
		pptpOPModeSwitch();
	}
	else if (document.wizard_form.connectionType[5].checked == true) {
		document.getElementById("div_wan_3G").style.visibility = "visible";
		document.getElementById("div_wan_3G").style.display = style_display_on();
		document.wizard_form.Dev3G.disabled = false;
	}
}

function switch_security_mode()
{
	document.getElementById("div_security_key").style.visibility = "hidden";
	document.getElementById("div_security_key").style.display = "none";

	if (document.wizard_form.security_mode.selectedIndex != 0)
	{
		document.getElementById("div_security_key").style.visibility = "visible";
		document.getElementById("div_security_key").style.display = style_display_on();
		document.wizard_form.security_key.disabled = false;
	}
}

function l2tpModeSwitch()
{
	if (document.wizard_form.l2tpMode.selectedIndex == 0) {
		document.getElementById("div_l2tpIp").style.visibility = "visible";
		document.getElementById("div_l2tpIp").style.display = style_display_on();
		document.getElementById("div_l2tpNetmask").style.visibility = "visible";
		document.getElementById("div_l2tpNetmask").style.display = style_display_on();
		document.getElementById("div_l2tpGateway").style.visibility = "visible";
		document.getElementById("div_l2tpGateway").style.display = style_display_on();
	}
	else {
		document.getElementById("div_l2tpIp").style.visibility = "hidden";
		document.getElementById("div_l2tpIp").style.display = "none";
		document.getElementById("div_l2tpNetmask").style.visibility = "hidden";
		document.getElementById("div_l2tpNetmask").style.display = "none";
		document.getElementById("div_l2tpGateway").style.visibility = "hidden";
		document.getElementById("div_l2tpGateway").style.display = "none";
	}
}

function pptpModeSwitch()
{
	if (document.wizard_form.pptpMode.selectedIndex == 0) {
		document.getElementById("div_pptpIp").style.visibility = "visible";
		document.getElementById("div_pptpIp").style.display = style_display_on();
		document.getElementById("div_pptpNetmask").style.visibility = "visible";
		document.getElementById("div_pptpNetmask").style.display = style_display_on();
		document.getElementById("div_pptpGateway").style.visibility = "visible";
		document.getElementById("div_pptpGateway").style.display = style_display_on();
	}
	else {
		document.getElementById("div_pptpIp").style.visibility = "hidden";
		document.getElementById("div_pptpIp").style.display = "none";
		document.getElementById("div_pptpNetmask").style.visibility = "hidden";
		document.getElementById("div_pptpNetmask").style.display = "none";
		document.getElementById("div_pptpGateway").style.visibility = "hidden";
		document.getElementById("div_pptpGateway").style.display = "none";
	}
}

function pppoeOPModeSwitch()
{
	document.wizard_form.pppoeRedialPeriod.disabled = true;
	document.wizard_form.pppoeIdleTime.disabled = true;
	if (pppoecdb == "1")
	{
		document.getElementById("div_pppoe_opmode_setting").style.visibility = "hidden";
		document.getElementById("div_pppoe_opmode_setting").style.display = "none";
	}
	else
	{
		if (document.wizard_form.pppoeOPMode.selectedIndex == 0) 
			document.wizard_form.pppoeRedialPeriod.disabled = false;
		else if (document.wizard_form.pppoeOPMode.selectedIndex == 1)
			document.wizard_form.pppoeIdleTime.disabled = false;
	}
}

function l2tpOPModeSwitch()
{
	document.wizard_form.l2tpRedialPeriod.disabled = true;
	// document.wizard_form.l2tpIdleTime.disabled = true;
	if (document.wizard_form.l2tpOPMode.selectedIndex == 0) 
		document.wizard_form.l2tpRedialPeriod.disabled = false;
	/*
	else if (document.wizard_form.l2tpOPMode.selectedIndex == 1)
		document.wizard_form.l2tpIdleTime.disabled = false;
	*/
}

function pptpOPModeSwitch()
{
	document.wizard_form.pptpRedialPeriod.disabled = true;
	// document.wizard_form.pptpIdleTime.disabled = true;
	if (document.wizard_form.pptpOPMode.selectedIndex == 0) 
		document.wizard_form.pptpRedialPeriod.disabled = false;
	/*
	else if (document.wizard_form.pptpOPMode.selectedIndex == 1)
		document.wizard_form.pptpIdleTime.disabled = false;
	*/
}

function atoi(str, num)
{
	i = 1;
	if (num != 1) {
		while (i != num && str.length != 0) {
			if (str.charAt(0) == '.') {
				i++;
			}
			str = str.substring(1);
		}
		if (i != num)
			return -1;
	}

	for (i=0; i<str.length; i++) {
		if (str.charAt(i) == '.') {
			str = str.substring(0, i);
			break;
		}
	}
	if (str.length == 0)
		return -1;
	return parseInt(str, 10);
}

function checkRange(str, num, min, max)
{
	d = atoi(str, num);
	if (d > max || d < min)
		return false;
	return true;
}

function isAllNum(str)
{
	for (var i=0; i<str.length; i++) {
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.' ))
			continue;
		return 0;
	}
	return 1;
}

function checkIpAddr(field, ismask)
{
	if (field.value == "") {
		alert("Error. IP address is empty.");
		field.value = field.defaultValue;
		field.focus();
		return false;
	}

	if (isAllNum(field.value) == 0) {
		alert('It should be a [0-9] number.');
		field.value = field.defaultValue;
		field.focus();
		return false;
	}

	if (ismask) {
		if ((!checkRange(field.value, 1, 0, 256)) ||
				(!checkRange(field.value, 2, 0, 256)) ||
				(!checkRange(field.value, 3, 0, 256)) ||
				(!checkRange(field.value, 4, 0, 256)))
		{
			alert('IP adress format error.');
			field.value = field.defaultValue;
			field.focus();
			return false;
		}
	}
	else {
		if ((!checkRange(field.value, 1, 0, 255)) ||
				(!checkRange(field.value, 2, 0, 255)) ||
				(!checkRange(field.value, 3, 0, 255)) ||
				(!checkRange(field.value, 4, 1, 254)))
		{
			alert('IP adress format error.');
			field.value = field.defaultValue;
			field.focus();
			return false;
		}
	}
	return true;
}

function CheckWanValue()
{
	if (document.wizard_form.connectionType[1].checked == true)		//STATIC
	{      
		if (!checkIpAddr(document.wizard_form.staticIp, false))
		{
			document.wizard_form.staticIp.select();
			return false;
		}
		if (!checkIpAddr(document.wizard_form.staticNetmask, true))
		{
			document.wizard_form.staticNetmask.select();
			return false;
		}

		if (document.wizard_form.staticGateway.value != "")
			if (!checkIpAddr(document.wizard_form.staticGateway, false))
			{
				document.wizard_form.staticGateway.select();
				return false;
			}
		if (document.wizard_form.staticPriDns.value != "")
			if (!checkIpAddr(document.wizard_form.staticPriDns, false))
			{
				document.wizard_form.staticPriDns.select();
				return false;
			}
		if (document.wizard_form.staticSecDns.value != "")
			if (!checkIpAddr(document.wizard_form.staticSecDns, false))
			{
				document.wizard_form.staticPriDns.select();
				return false;
			}
	}
	else if (document.wizard_form.connectionType[2].checked == true)	//PPPOE
	{ 
		if (document.wizard_form.pppoeUser.value.length == 0)
		{
			alert("Please enter user name!");
			document.wizard_form.pppoeUser.select();
			return false;
		}
		if (document.wizard_form.pppoePass.value.length == 0)
		{
			alert("Please enter password!");
			document.wizard_form.pppoePass.select();
			return false;
		}
		if (document.wizard_form.pppoePass.value != document.wizard_form.pppoePass2.value) 
		{
			alert("Password mismatched!");
			return false;
		}
		if (pppoecdb != "1")
		{
			if (document.wizard_form.pppoeOPMode.selectedIndex == 0)
			{
				if (document.wizard_form.pppoeRedialPeriod.value == "")
				{
					alert("Please specify Redial Period");
					document.wizard_form.pppoeRedialPeriod.select();
					return false;
				}
			}
			else if (document.wizard_form.pppoeOPMode.selectedIndex == 1);
			{
				if (document.wizard_form.pppoeIdleTime.value == "")
				{
					alert("Please specify Idle Time");
					document.wizard_form.pppoeIdleTime.select();
					return false;
				}
			}
		}
	}
	else if (document.wizard_form.connectionType[3].checked == true) 	//L2TP
	{ 
		if (!checkIpAddr(document.wizard_form.l2tpServer, false))
		{
			document.wizard_form.l2tpServer.select();
			return false;
		}
		if (document.wizard_form.l2tpUser.value.length == 0)
		{
			alert("Please enter user name!");
			document.wizard_form.l2tpUser.select();
			return false;
		}
		if (document.wizard_form.l2tpPass.value.length == 0)
		{
			alert("Please enter password!");
			document.wizard_form.l2tpPass.select();
			return false;
		}
		if (document.wizard_form.l2tpMode.selectedIndex == 0)
		{
			if (!checkIpAddr(document.wizard_form.l2tpIp, false))
			{
				document.wizard_form.staticIp.select();
				return false;
			}
			if (!checkIpAddr(document.wizard_form.l2tpNetmask, true))
			{
				document.wizard_form.l2tpNetmask.select();
				return false;
			}
			if (!checkIpAddr(document.wizard_form.l2tpGateway, false))
			{
				document.wizard_form.l2tpGateway.select();
				return false;
			}
		}
		if (document.wizard_form.l2tpOPMode.selectedIndex == 0)
		{
			if (document.wizard_form.l2tpRedialPeriod.value == "")
			{
				alert("Please specify Redial Period");
				document.wizard_form.l2tpRedialPeriod.select();
				return false;
			}
		}
		/*
		else if (document.wizard_form.l2tpOPMode.selectedIndex == 1)
		{
			if (document.wizard_form.l2tpIdleTime.value == "")
			{
				alert("Please specify Idle Time");
				document.wizard_form.l2tpIdleTime.focus();
				document.wizard_form.l2tpIdleTime.select();
				return false;
			}
		}
		*/
	}
	else if (document.wizard_form.connectionType[4].checked == true) 	//PPTP
	{ 
		if (!checkIpAddr(document.wizard_form.pptpServer, false))
		{
			document.wizard_form.pptpServer.select();
			return false;
		}
		if (document.wizard_form.pptpUser.value.length == 0)
		{
			alert("Please enter user name!");
			document.wizard_form.pptpUser.select();
			return false;
		}
		if (document.wizard_form.pptpPass.value.length == 0)
		{
			alert("Please enter password!");
			document.wizard_form.pptpPass.select();
			return false;
		}
		if (document.wizard_form.pptpMode.selectedIndex == 0) 
		{
			if (!checkIpAddr(document.wizard_form.pptpIp, false))
			{
				document.wizard_form.pptpIp.select();
				return false;
			}
			if (!checkIpAddr(document.wizard_form.pptpNetmask, true))
			{
				document.wizard_form.pptpNetmask.select();
				return false;
			}
			if (!checkIpAddr(document.wizard_form.pptpGateway, false))
			{
				document.wizard_form.pptpGateway.select();
				return false;
			}
		}
		if (document.wizard_form.pptpOPMode.selectedIndex == 0)
		{
			if (document.wizard_form.pptpRedialPeriod.value == "")
			{
				alert("Please specify Redial Period");
				document.wizard_form.pptpRedialPeriod.select();
				return false;
			}
		}
		/*
		else if(document.wizard_form.pptpOPMode.selectedIndex == 1)
		{
			if (document.wizard_form.pptpIdleTime.value == "")
			{
				alert("Please specify Idle Time");
				document.wizard_form.pptpIdleTime.select();
				return false;
			}
		}
		*/
	}

	return true;
}

function CheckWirelessValue()
{
	if (document.wizard_form.ssid.value.length == 0)
	{
		alert("Please enter SSID!");
		document.wizard_form.ssid.select();
		return false;
	}

	if (document.wizard_form.security_mode.selectedIndex == 1)
	{
		var keyvalue = document.wizard_form.security_key.value;
		if (keyvalue.length == 0)
		{
			alert('Please input wep key!');
			document.wizard_form.security_key.select();
			return false;
		}
		else if (keyvalue.length != 5 && keyvalue.length != 13 && keyvalue.length != 10 && keyvalue.length != 26)
		{
			alert('Please input 5, 13 (ASCII), 10, or 26 (HEX) characters of wep key!');
			document.wizard_form.security_key.select();
			return false;
		}
		if(checkInjection(keyvalue)== false)
		{
			alert('Wep key contains invalid characters.');
			document.wizard_form.security_key.select();
			return false;
		}
	}
	else if (document.wizard_form.security_mode.selectedIndex == 2)
	{
		var keyvalue = document.wizard_form.security_key.value;
		if (keyvalue.length == 0)
		{
			alert('Please input wpapsk key!');
			document.wizard_form.security_key.select();
			return false;
		}
		if (keyvalue.length < 8){
			alert('Please input at least 8 character of wpapsk key!');
			document.wizard_form.security_key.select();
			return false;
		}
		if(checkInjection(keyvalue) == false){
			alert('Invalid characters in Pass Phrase.');
			document.wizard_form.security_key.select();
			return false;
		}
	}

	return true;
}

function initValue()
{
	hideAllTables();
	disableWanSettings();
	disableWirelessSettings();
	step_num = 0;
	document.getElementById("div_wizard_home").style.visibility = "visible";
	document.getElementById("div_wizard_home").style.display = style_display_on();
	document.wizard_form.back_motion.disabled = true;
	document.wizard_form.next_motion.disabled = false;
	document.wizard_form.cancel_motion.disabled = false;
	document.wizard_form.apply_motion.disabled = true;
}
</script>
</head>

<body onLoad="initValue()">
<table class="body"><tr><td>
<h1>Ralink SoC Quick Settings</h1>
<p></p>
<hr />
<form method=post name="wizard_form" action="/goform/setWizard">
<!-- ============= Wizard Home Page ============= -->
<table id="div_wizard_home" width="640" border="0" cellpadding="2" cellspacing="1">
<tr><th align="left">Step 1: configure Internet connection</th></tr>
<tr><th align="left">Step 2: configure Wireless Settings</th></tr>
</table>

<h2 id="div_wan_title">Step 1: configure Internet connection</h2>
<!-- ============= Internet OP Mode ============= -->
<table id="div_wan_opmode" width="640" border="0" cellpadding="2" cellspacing="1">
<tr>
  <td width="15px"><input type="radio" name="connectionType" value="DHCP" checked></td>
  <th align="left">DHCP (Auto Config)</th>
</tr>
<tr>
  <td>&nbsp; &nbsp;</td>
  <td>Choose this, ISP automatically configure this device.</td>
</tr> 
<tr>
  <td width="15px"><input type="radio" name="connectionType" value="STATIC"></td>
  <th align="left">Static Mode (fixed IP)</th>
</tr>
<tr>
  <td>&nbsp; &nbsp;</td>
  <td>Choose this, user need to configure this device manually.</td>
</tr>
  <td width="15px"><input type="radio" name="connectionType" value="PPPOE"></td>
  <th align="left">PPPOE (ADSL)</th>
</tr>
<tr>
  <td>&nbsp; &nbsp;</td>
  <td>Choose this, user must input a username and password for ISP authenticator.</td>
</tr>
<tr>
  <td width="15px"><input type="radio" name="connectionType" value="L2TP"></td>
  <th align="left">L2TP</th>
</tr>
<tr>
  <td>&nbsp; &nbsp;</td>
  <td align="left">L2TP Client</td>
</tr>
<tr>
  <td width="15px"><input type="radio" name="connectionType" value="PPTP"></td>
  <th align="left">PPTP</th>
</tr>
<tr>
  <td>&nbsp; &nbsp;</td>
  <td>PPTP Client</td>
</tr>
<tr>
  <td width="15px"><input type="radio" name="connectionType" value="3G"></td>
  <th align="left">3G</th>
</tr>
<tr>
  <td>&nbsp; &nbsp;</td>
  <td>3G Client</td>
</tr>
</table>

<!-- ================= STATIC Mode ================= -->
<table id="div_wan_static" width="640" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2">Static Mode</td>
</tr>
<tr>
  <td class="head">IP Address</td>
  <td><input name="staticIp" maxlength=15></td>
</tr>
<tr>
  <td class="head">Subnet Mask</td>
  <td><input name="staticNetmask" maxlength=15>
  </td>
</tr>
<tr>
  <td class="head">Default Gateway</td>
  <td><input name="staticGateway" maxlength=15>
  </td>
</tr>
<tr>
  <td class="head">Primary DNS Server</td>
  <td><input name="staticPriDns" maxlength=15></td>
</tr>
<tr>
  <td class="head">Secondary DNS Server</td>
  <td><input name="staticSecDns" maxlength=15></td>
</tr>
</table>

<!-- ================= PPPOE Mode ================= -->
<table id="div_wan_pppoe" width="640" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2">PPPoE Mode</td>
</tr>
<tr>
  <td class="head">User Name</td>
  <td><input name="pppoeUser" maxlength=32 size=32></td>
</tr>
<tr>
  <td class="head">Password</td>
  <td><input type="password" name="pppoePass" maxlength=32 size=32></td>
</tr>
<tr>
  <td class="head">Verify Password</td>
  <td><input type="password" name="pppoePass2" maxlength=32 size=32></td>
</tr>
<tr id="div_pppoe_opmode">
  <td class="head" rowspan="2">Operation Mode</td>
  <td>
    <select name="pppoeOPMode" size="1" onChange="pppoeOPModeSwitch()">
      <option value="KeepAlive" selected>Keep Alive</option>
      <option value="OnDemand">On Demand</option>
      <option value="Manual">Manual</option>
    </select>
  </td>
</tr>
<tr id="div_pppoe_opmode_setting">
  <td>
    Keep Alive Mode: Redial Period
    <input type="text" name="pppoeRedialPeriod" maxlength="5" size="3" value="60">
    senconds
    <br />
    On demand Mode:  Idle Time
    <input type="text" name="pppoeIdleTime" maxlength="3" size="2" value="5">
    minutes
  </td>
</tr>
</table>

<!-- ================= L2TP Mode ================= -->
<table id="div_wan_l2tp" width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2">L2TP Mode</td>
</tr>
<tr>
  <td class="head">L2TP Server IP Address</td>
  <td><input name="l2tpServer" maxlength="15" size=15></td>
</tr>
<tr>
  <td class="head">User Name</td>
  <td><input name="l2tpUser" maxlength="20" size=20></td>
</tr>
<tr>
  <td class="head">Password</td>
  <td><input type="password" name="l2tpPass" maxlength="32" size=32></td>
</tr>
<tr>
  <td class="head">Address Mode</td>
  <td>
    <select name="l2tpMode" size="1" onChange="l2tpModeSwitch()">
      <option value="0">Static</option>
      <option value="1">Dynamic</option>
    </select>
  </td>
</tr>
<tr id="div_l2tpIp">
  <td class="head">IP Address</td>
  <td><input name="l2tpIp" maxlength=15 size=15></td>
</tr>
<tr id="div_l2tpNetmask">
  <td class="head">Subnet Mask</td>
  <td><input name="l2tpNetmask" maxlength=15 size=15>
  </td>
</tr>
<tr id="div_l2tpGateway">
  <td class="head">Default Gateway</td>
  <td><input name="l2tpGateway" maxlength=15 size=15>
  </td>
</tr>
<tr>
  <td class="head" rowspan="3">Operation Mode</td>
  <td>
    <select name="l2tpOPMode" size="1" onChange="l2tpOPModeSwitch()">
      <option value="KeepAlive" selected>Keep Alive</option>
      <!--
      <option value="OnDemand" id="wL2tpOnDemand">On Demand</option>
      -->
      <option value="Manual">Manual</option>
    </select>
  </td>
</tr>
<tr>
  <td>
    Keep Alive Mode: Redial Period
    <input type="text" name="l2tpRedialPeriod" maxlength="5" size="3" value="60">
    senconds
    <!--
    <br />
    On demand Mode:  Idle Time
    <input type="text" name="l2tpIdleTime" maxlength="3" size="2" value="5">
    minutes
    -->
  </td>
</tr>
</table>

<!-- ================= PPTP Mode ================= -->
<table id="div_wan_pptp" width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2">PPTP Mode</td>
</tr>
<tr>
  <td class="head">PPTP Server IP Address</td>
  <td><input name="pptpServer" maxlength="15" size=15></td>
</tr>
<tr>
  <td class="head">User Name</td>
  <td><input name="pptpUser" maxlength="20" size=20></td>
</tr>
<tr>
  <td class="head">Password</td>
  <td><input type="password" name="pptpPass" maxlength="32" size=32></td>
</tr>
<tr>
  <td class="head">Address Mode</td>
  <td>
    <select name="pptpMode" size="1" onChange="pptpModeSwitch()">
      <option value="0">Static</option>
      <option value="1">Dynamic</option>
    </select>
  </td>
</tr>
<tr id="div_pptpIp">
  <td class="head">IP Address</td>
  <td><input name="pptpIp" maxlength=15 size=15></td>
</tr>
<tr id="div_pptpNetmask">
  <td class="head">Subnet Mask</td>
  <td><input name="pptpNetmask" maxlength=15 size=15>
  </td>
</tr>
<tr id="div_pptpGateway">
  <td class="head">Default Gateway</td>
  <td><input name="pptpGateway" maxlength=15 size=15>
  </td>
</tr>
<tr>
  <td class="head" rowspan="3">Operation Mode</td>
  <td>
    <select name="pptpOPMode" size="1" onChange="pptpOPModeSwitch()">
      <option value="KeepAlive" selected>Keep Alive</option>
      <!--
      <option value="OnDemand" id="wPptpOnDemand">On Demand</option>
      -->
      <option value="Manual">Manual</option>
    </select>
  </td>
</tr>
<tr>
  <td>
    Keep Alive Mode: Redial Period
    <input type="text" name="pptpRedialPeriod" maxlength="5" size="3" value="60">
    senconds
    <!--
    <br />
    On demand Mode:  Idle Time
    <input type="text" name="pptpIdleTime" maxlength="3" size="2" value="5">
    minutes
    -->
  </td>
</tr>
</table>

<!-- =========== 3G Modular =========== -->
<table id="div_wan_3G" width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2">3G Mode</td>
</tr>
<tr>
  <td class="head" rowspan="3">USB 3G modem</td>
  <td>
    <select name="Dev3G" size="1">
      <option value="MU-Q101">NU MU-Q101</option>
      <option value="HUAWEI-E169">HUAWEI E169</option>
      <option value="BandLuxe-C270">BandLuxe C270</option>
      <option value="OPTION-ICON225">OPTION ICON 225</option>
    </select>
  </td>
</tr>
</table>

<h2 id="div_wireless_title">Step 2: configure Wireless Settings</h2>
<!-- =========== Wireless Settings =========== -->
<table id="div_wireless" width="540" border="1" cellpadding="2" cellspacing="1">
<tr><td class="title" colspan="2">Wireless Settings</td></tr>
<tr>
  <td class="head">Network Name (SSID)</td>
  <td><input type="text" name="ssid" size="20" maxlength="32"></td>
</tr>
<tr>
  <td class="head" rowspan="2">Security</td>
  <td>
    <select name="security_mode" size="1" onChange="switch_security_mode();">
      <option value="Disable" selected>Disable</option>
      <option value="WEPAUTO">WEP</option>
      <option value="WPAPSKWPA2PSK">WPA-PSK/WPA2-PSK</option>
    </select>
  </td>
</tr>
<tr id="div_security_key">
  <td>KEY:<input type="text" name="security_key" size="28" maxlength="64"></td>
</tr>
</table>

<table width="540" cellpadding="2" cellspacing="1">
<tr align="center">
  <td>
    <input type="button" name="back_motion" style="{width:120px;}" value="Back" onClick="change_step(-1);">
    <input type="button" name="next_motion" style="{width:120px;}" value="Next" onClick="change_step(1);">
    <input type="button" name="cancel_motion" style="{width:120px;}" value="Cancel" onClick="change_step(0);">
    <input type="button" name="apply_motion" style="{width:120px;}" value="Apply" onClick="change_step(1);">
  </td>
</tr>
</table>
</form>

</td></tr></table>
</body>
</html>

