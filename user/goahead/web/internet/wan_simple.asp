<html>
<head>
<title>Wide Area Network (WAN) Settings</title>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script language="JavaScript" type="text/javascript">
var http_request = false;
Butterlate.setTextDomain("internet");

function macCloneMacFillSubmit()
{
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
        alert('Cannot create an XMLHTTP instance');
        return false;
    }
    http_request.onreadystatechange = doFillMyMAC;

    http_request.open('POST', '/goform/getMyMAC', true);
    http_request.send('n\a');
}

function doFillMyMAC()
{
    if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			document.getElementById("macCloneMac").value = http_request.responseText;
		} else {
			alert("Can\'t get the mac address.");
		}
	}
}


function macCloneSwitch()
{
	if (document.wanCfg.macCloneEnbl.options.selectedIndex == 1) {
		document.getElementById("macCloneMacRow").style.visibility = "visible";
		document.getElementById("macCloneMacRow").style.display = style_display_on();
	}
	else {
		document.getElementById("macCloneMacRow").style.visibility = "hidden";
		document.getElementById("macCloneMacRow").style.display = "none";
	}
}

function connectionTypeSwitch()
{
	document.getElementById("static").style.visibility = "hidden";
	document.getElementById("static").style.display = "none";
	document.getElementById("dhcp").style.visibility = "hidden";
	document.getElementById("dhcp").style.display = "none";
	document.getElementById("pppoe").style.visibility = "hidden";
	document.getElementById("pppoe").style.display = "none";
	document.getElementById("l2tp").style.visibility = "hidden";
	document.getElementById("l2tp").style.display = "none";
	document.getElementById("pptp").style.visibility = "hidden";
	document.getElementById("pptp").style.display = "none";

	if (document.wanCfg.connectionType.options.selectedIndex == 0) {
		document.getElementById("static").style.visibility = "visible";
		document.getElementById("static").style.display = "block";
	}
	else if (document.wanCfg.connectionType.options.selectedIndex == 1) {
		document.getElementById("dhcp").style.visibility = "visible";
		document.getElementById("dhcp").style.display = "block";
	}
	else if (document.wanCfg.connectionType.options.selectedIndex == 2) {
		document.getElementById("pppoe").style.visibility = "visible";
		document.getElementById("pppoe").style.display = "block";
	}
	else if (document.wanCfg.connectionType.options.selectedIndex == 3) {
		document.getElementById("l2tp").style.visibility = "visible";
		document.getElementById("l2tp").style.display = "block";
		l2tpOPModeSwitch();
	}
	else if (document.wanCfg.connectionType.options.selectedIndex == 4) {
		document.getElementById("pptp").style.visibility = "visible";
		document.getElementById("pptp").style.display = "block";
		pptpOPModeSwitch();
	}
	else {
		document.getElementById("static").style.visibility = "visible";
		document.getElementById("static").style.display = "block";
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

function l2tpModeSwitch()
{
	if (document.wanCfg.l2tpMode.selectedIndex == 0) {
		document.getElementById("l2tpIp").style.visibility = "visible";
		document.getElementById("l2tpIp").style.display = style_display_on();
		document.getElementById("l2tpNetmask").style.visibility = "visible";
		document.getElementById("l2tpNetmask").style.display = style_display_on();
		document.getElementById("l2tpGateway").style.visibility = "visible";
		document.getElementById("l2tpGateway").style.display = style_display_on();
	}
	else {
		document.getElementById("l2tpIp").style.visibility = "hidden";
		document.getElementById("l2tpIp").style.display = "none";
		document.getElementById("l2tpNetmask").style.visibility = "hidden";
		document.getElementById("l2tpNetmask").style.display = "none";
		document.getElementById("l2tpGateway").style.visibility = "hidden";
		document.getElementById("l2tpGateway").style.display = "none";
	}
}

function pptpModeSwitch()
{
	if (document.wanCfg.pptpMode.selectedIndex == 0) {
		document.getElementById("pptpIp").style.visibility = "visible";
		document.getElementById("pptpIp").style.display = style_display_on();
		document.getElementById("pptpNetmask").style.visibility = "visible";
		document.getElementById("pptpNetmask").style.display = style_display_on();
		document.getElementById("pptpGateway").style.visibility = "visible";
		document.getElementById("pptpGateway").style.display = style_display_on();
	}
	else {
		document.getElementById("pptpIp").style.visibility = "hidden";
		document.getElementById("pptpIp").style.display = "none";
		document.getElementById("pptpNetmask").style.visibility = "hidden";
		document.getElementById("pptpNetmask").style.display = "none";
		document.getElementById("pptpGateway").style.visibility = "hidden";
		document.getElementById("pptpGateway").style.display = "none";
	}
}

function l2tpOPModeSwitch()
{
	document.wanCfg.l2tpRedialPeriod.disabled = true;
	document.wanCfg.l2tpIdleTime.disabled = true;
	if (document.wanCfg.l2tpOPMode.options.selectedIndex == 0) 
		document.wanCfg.l2tpRedialPeriod.disabled = false;
	else if (document.wanCfg.l2tpOPMode.options.selectedIndex == 1)
		document.wanCfg.l2tpIdleTime.disabled = false;
}

function pptpOPModeSwitch()
{
	document.wanCfg.pptpRedialPeriod.disabled = true;
	document.wanCfg.pptpIdleTime.disabled = true;
	if (document.wanCfg.pptpOPMode.options.selectedIndex == 0) 
		document.wanCfg.pptpRedialPeriod.disabled = false;
	else if (document.wanCfg.pptpOPMode.options.selectedIndex == 1)
		document.wanCfg.pptpIdleTime.disabled = false;
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

function CheckValue()
{
	if (document.wanCfg.connectionType.selectedIndex == 0) {      //STATIC
		if (!checkIpAddr(document.wanCfg.staticIp, false))
			return false;
		if (!checkIpAddr(document.wanCfg.staticNetmask, true))
			return false;
		if (document.wanCfg.staticGateway.value != "")
			if (!checkIpAddr(document.wanCfg.staticGateway, false))
				return false;
		if (document.wanCfg.staticPriDns.value != "")
			if (!checkIpAddr(document.wanCfg.staticPriDns, false))
				return false;
		if (document.wanCfg.staticSecDns.value != "")
			if (!checkIpAddr(document.wanCfg.staticSecDns, false))
				return false;
		if (document.wanCfg.macCloneEnbl.options.selectedIndex == 1) {
			var re = /[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}/;
			if (document.wanCfg.macCloneMac.value.length == 0) {
				alert("MAC Address should not be empty!");
				document.wanCfg.macCloneMac.focus();
				return false;
			}
			if (!re.test(document.wanCfg.macCloneMac.value)) {
				alert("Please fill the MAC Address in correct format! (XX:XX:XX:XX:XX:XX)");
				document.wanCfg.macCloneMac.focus();
				return false;
			}
		}
	}
	else if (document.wanCfg.connectionType.selectedIndex == 1) { //DHCP
	}
	else if (document.wanCfg.connectionType.selectedIndex == 2) { //PPPOE
		if (document.wanCfg.pppoePass.value != document.wanCfg.pppoePass2.value)
		{
			alert("Twin input password are different!");
			document.wanCfg.pppoePass.focus();
			document.wanCfg.pppoePass.select();
			return false;
		}
	}
	else if (document.wanCfg.connectionType.selectedIndex == 3) { //L2TP
		if (document.wanCfg.l2tpOPMode.options.selectedIndex == 0)
		{
			if (document.wanCfg.l2tpRedialPeriod.value == "")
			{
				alert("Please specify Redial Period");
				document.wanCfg.l2tpRedialPeriod.focus();
				document.wanCfg.l2tpRedialPeriod.select();
				return false;
			}
		}
		else if (document.wanCfg.l2tpOPMode.options.selectedIndex == 1)
		{
			if (document.wanCfg.l2tpIdleTime.value == "")
			{
				alert("Please specify Idle Time");
				document.wanCfg.l2tpIdleTime.focus();
				document.wanCfg.l2tpIdleTime.select();
				return false;
			}
		}
	}
	else if (document.wanCfg.connectionType.selectedIndex == 4) { //PPTP
		if (document.wanCfg.pptpPass.value != document.wanCfg.pptpPass2.value) {
			alert("Password mismatched!");
			return false;
		}
		if (!checkIpAddr(document.wanCfg.pptpServer, false))
			return false;
		if (document.wanCfg.pptpMode.selectedIndex == 0) {
			if (!checkIpAddr(document.wanCfg.pptpIp, false))
				return false;
			if (!checkIpAddr(document.wanCfg.pptpNetmask, true))
				return false;
			if (!checkIpAddr(document.wanCfg.pptpGateway, false))
				return false;
		}
		if (document.wanCfg.pptpOPMode.options.selectedIndex == 0)
		{
			if (document.wanCfg.pptpRedialPeriod.value == "")
			{
				alert("Please specify Redial Period");
				document.wanCfg.pptpRedialPeriod.focus();
				document.wanCfg.pptpRedialPeriod.select();
				return false;
			}
		}
		else if(document.wanCfg.pptpOPMode.options.selectedIndex == 1)
		{
			if (document.wanCfg.pptpIdleTime.value == "")
			{
				alert("Please specify Idle Time");
				document.wanCfg.pptpIdleTime.focus();
				document.wanCfg.pptpIdleTime.select();
				return false;
			}
		}
	}
	else
		return false;
	return true;
}

function initTranslation()
{
	var e = document.getElementById("wTitle");
	e.innerHTML = _("wan title");
	e = document.getElementById("wIntroduction");
	e.innerHTML = _("wan introduction");

	e = document.getElementById("wConnectionType");
	e.innerHTML = _("wan connection type");
	e = document.getElementById("wConnTypeStatic");
	e.innerHTML = _("wan connection type static");
	e = document.getElementById("wConnTypeDhcp");
	e.innerHTML = _("wan connection type dhcp");
	e = document.getElementById("wConnTypePppoe");
	e.innerHTML = _("wan connection type pppoe");
	e = document.getElementById("wConnTypeL2tp");
	e.innerHTML = _("wan connection type l2tp");
	e = document.getElementById("wConnTypePptp");
	e.innerHTML = _("wan connection type pptp");

	e = document.getElementById("wStaticMode");
	e.innerHTML = _("wan static mode");
	e = document.getElementById("wStaticIp");
	e.innerHTML = _("inet ip");
	e = document.getElementById("wStaticNetmask");
	e.innerHTML = _("inet netmask");
	e = document.getElementById("wStaticGateway");
	e.innerHTML = _("inet gateway");
	e = document.getElementById("wStaticPriDns");
	e.innerHTML = _("inet pri dns");
	e = document.getElementById("wStaticSecDns");
	e.innerHTML = _("inet sec dns");

	e = document.getElementById("wDhcpMode");
	e.innerHTML = _("wan dhcp mode");
	e = document.getElementById("wDhcpHost");
	e.innerHTML = _("inet hostname");

	e = document.getElementById("wPppoeMode");
	e.innerHTML = _("wan pppoe mode");
	e = document.getElementById("wPppoeUser");
	e.innerHTML = _("inet user");
	e = document.getElementById("wPppoePassword");
	e.innerHTML = _("inet password");
	e = document.getElementById("wPppoePass2");
	e.innerHTML = _("inet pass2");

	e = document.getElementById("wL2tpMode");
	e.innerHTML = _("wan l2tp mode");
	e = document.getElementById("wL2tpServer");
	e.innerHTML = _("inet server");
	e = document.getElementById("wL2tpUser");
	e.innerHTML = _("inet user");
	e = document.getElementById("wL2tpPassword");
	e.innerHTML = _("inet password");
	e = document.getElementById("wL2tpAddrMode");
	e.innerHTML = _("wan address mode");
	e = document.getElementById("wL2tpAddrModeS");
	e.innerHTML = _("wan address mode static");
	e = document.getElementById("wL2tpAddrModeD");
	e.innerHTML = _("wan address mode dynamic");
	e = document.getElementById("wL2tpIp");
	e.innerHTML = _("inet ip");
	e = document.getElementById("wL2tpNetmask");
	e.innerHTML = _("inet netmask");
	e = document.getElementById("wL2tpGateway");
	e.innerHTML = _("inet gateway");
	e = document.getElementById("wL2tpOPMode");
	e.innerHTML = _("wan protocol opmode");
	e = document.getElementById("wL2tpKeepAlive");
	e.innerHTML = _("wan protocol opmode keepalive");
	e = document.getElementById("wL2tpOnDemand");
	e.innerHTML = _("wan protocol opmode ondemand");
	e = document.getElementById("wL2tpManual");
	e.innerHTML = _("wan protocol opmode manual");

	e = document.getElementById("wPptpMode");
	e.innerHTML = _("wan pptp mode");
	e = document.getElementById("wPptpServer");
	e.innerHTML = _("inet server");
	e = document.getElementById("wPptpUser");
	e.innerHTML = _("inet user");
	e = document.getElementById("wPptpPassword");
	e.innerHTML = _("inet password");
	e = document.getElementById("wPptpAddrMode");
	e.innerHTML = _("wan address mode");
	e = document.getElementById("wPptpAddrModeS");
	e.innerHTML = _("wan address mode static");
	e = document.getElementById("wPptpAddrModeD");
	e.innerHTML = _("wan address mode dynamic");
	e = document.getElementById("wPptpIp");
	e.innerHTML = _("inet ip");
	e = document.getElementById("wPptpNetmask");
	e.innerHTML = _("inet netmask");
	e = document.getElementById("wPptpGateway");
	e.innerHTML = _("inet gateway");
	e = document.getElementById("wPptpOPMode");
	e.innerHTML = _("wan protocol opmode");
	e = document.getElementById("wPptpKeepAlive");
	e.innerHTML = _("wan protocol opmode keepalive");
	e = document.getElementById("wPptpOnDemand");
	e.innerHTML = _("wan protocol opmode ondemand");
	e = document.getElementById("wPptpManual");
	e.innerHTML = _("wan protocol opmode manual");

	e = document.getElementById("wMacClone");
	e.innerHTML = _("wan mac clone");
	e = document.getElementById("wMacCloneD");
	e.innerHTML = _("inet disable");
	e = document.getElementById("wMacCloneE");
	e.innerHTML = _("inet enable");
	e = document.getElementById("wMacCloneAddr");
	e.innerHTML = _("inet mac");

	e = document.getElementById("wApply");
	e.value = _("inet apply");
	e = document.getElementById("wCancel");
	e.value = _("inet cancel");
}

function initValue()
{
	var mode = "<% getCfgGeneral(1, "wanConnectionMode"); %>";
	var pptpMode = <% getCfgZero(1, "wan_pptp_mode"); %>;
	var clone = <% getCfgZero(1, "macCloneEnabled"); %>;

	initTranslation();
	if (mode == "STATIC") {
		document.wanCfg.connectionType.options.selectedIndex = 0;
	}
	else if (mode == "DHCP") {
		document.wanCfg.connectionType.options.selectedIndex = 1;
	}
	else if (mode == "PPPOE") {
		document.wanCfg.connectionType.options.selectedIndex = 2;
	}
	else if (mode == "L2TP") {
		var l2tp_opmode = "<% getCfgGeneral(1, "wan_l2tp_opmode"); %>";
		var l2tp_optime = "<% getCfgGeneral(1, "wan_l2tp_optime"); %>";
		
		document.wanCfg.connectionType.options.selectedIndex = 3;
		if (l2tp_opmode == "Manual")
		{
			document.wanCfg.l2tpOPMode.options.selectedIndex = 2;
		}
		else if (l2tp_opmode == "OnDemand")
		{
			document.wanCfg.l2tpOPMode.options.selectedIndex = 1;
			if (l2tp_optime != "")
				document.wanCfg.l2tpIdleTime.value = l2tp_optime;
		}
		else if (l2tp_opmode == "KeepAlive")
		{
			document.wanCfg.l2tpOPMode.options.selectedIndex = 0;
			if (l2tp_optime != "")
				document.wanCfg.l2tpRedialPeriod.value = l2tp_optime;
		}
		l2tpOPModeSwitch();
	}
	else if (mode == "PPTP") {
		var pptp_opmode = "<% getCfgGeneral(1, "wan_pptp_opmode"); %>";
		var pptp_optime = "<% getCfgGeneral(1, "wan_pptp_optime"); %>";

		document.wanCfg.connectionType.options.selectedIndex = 4;
		document.wanCfg.pptpMode.options.selectedIndex = 1*pptpMode;
		pptpModeSwitch();
		if (pptp_opmode == "Manual")
		{
			document.wanCfg.pptpOPMode.options.selectedIndex = 2;
			if (pptp_optime != "")
				document.wanCfg.pptpIdleTime.value = pptp_optime;
		}
		if (pptp_opmode == "OnDemand")
		{
			document.wanCfg.pptpOPMode.options.selectedIndex = 1;
			if (pptp_optime != "")
				document.wanCfg.pptpIdleTime.value = pptp_optime;
		}
		else if (pptp_opmode == "KeepAlive")
		{
			document.wanCfg.pptpOPMode.options.selectedIndex = 0;
			if (pptp_optime != "")
				document.wanCfg.pptpRedialPeriod.value = pptp_optime;
		}
		pptpOPModeSwitch();
	}
	else {
		document.wanCfg.connectionType.options.selectedIndex = 0;
	}
	connectionTypeSwitch();

	if (clone == 1)
		document.wanCfg.macCloneEnbl.options.selectedIndex = 1;
	else
		document.wanCfg.macCloneEnbl.options.selectedIndex = 0;
	macCloneSwitch();
}
</script>
</head>

<body onLoad="initValue()">
<div align="center">
 <center>
<table class="body"><tr><td>

<h1 id="wTitle"></h1>
<p id="wIntroduction"></p>
<hr />

<form method=post name="wanCfg" action="/goform/setWan" onSubmit="return CheckValue()">
<table width="95%" cellpadding="2" cellspacing="1">
<tr align="center">
  <td><b id="wConnectionType"></b>&nbsp;&nbsp;&nbsp;&nbsp;</td>
  <td>
    <select name="connectionType" size="1" onChange="connectionTypeSwitch();">
      <option value="STATIC" id="wConnTypeStatic">Static Mode (fixed IP)</option>
      <option value="DHCP" id="wConnTypeDhcp">DHCP (Auto Config)</option>
      <option value="PPPOE" id="wConnTypePppoe">PPPOE (ADSL)</option>
      <option value="L2TP" id="wConnTypeL2tp">L2TP</option>
      <option value="PPTP" id="wConnTypePptp">PPTP</option>
    </select>
  </td>
</tr>
</table>

<!-- ================= STATIC Mode ================= -->
<table id="static" width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="wStaticMode">Static Mode</td>
</tr>
<tr>
  <td class="head" id="wStaticIp">IP Address</td>
  <td><input name="staticIp" maxlength=15 value="<% getWanIp(); %>"></td>
</tr>
<tr>
  <td class="head" id="wStaticNetmask">Subnet Mask</td>
  <td><input name="staticNetmask" maxlength=15 value="<% getWanNetmask(); %>">
  </td>
</tr>
<tr>
  <td class="head" id="wStaticGateway">Default Gateway</td>
  <td><input name="staticGateway" maxlength=15 value="<% getWanGateway(); %>">
  </td>
</tr>
<tr>
  <td class="head" id="wStaticPriDns">Primary DNS Server</td>
  <td><input name="staticPriDns" maxlength=15 value="<% getDns(1); %>"></td>
</tr>
<tr>
  <td class="head" id="wStaticSecDns">Secondary DNS Server</td>
  <td><input name="staticSecDns" maxlength=15 value="<% getDns(2); %>"></td>
</tr>
</table>

<!-- ================= DHCP Mode ================= -->
<table id="dhcp" width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="wDhcpMode">DHCP Mode</td>
</tr>
<tr>
  <td class="head"><div id="wDhcpHost">Host Name</div> (optional)</td>
  <td><input type=text name="hostname" size=28 maxlength=32 value=""></td>
</tr>
</table>

<!-- ================= PPPOE Mode ================= -->
<table id="pppoe" width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="wPppoeMode">PPPoE Mode</td>
</tr>
<tr>
  <td class="head" id="wPppoeUser">User Name</td>
  <td><input name="pppoeUser" maxlength=64 size=64
             value="<% getCfgGeneral(1, "wan_pppoe_user"); %>"></td>
</tr>
<tr>
  <td class="head" id="wPppoePassword">Password</td>
  <td><input type="password" name="pppoePass" maxlength=32 size=32
             value="<% getCfgGeneral(1, "wan_pppoe_pass"); %>"></td>
</tr>
<tr>
  <td class="head" id="wPppoePass2">Verify Password</td>
  <td><input type="password" name="pppoePass2" maxlength=32 size=32
             value="<% getCfgGeneral(1, "wan_pppoe_pass"); %>"></td>
</tr>
</table>

<!-- ================= L2TP Mode ================= -->
<table id="l2tp" width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="wL2tpMode">L2TP Mode</td>
</tr>
<tr>
  <td class="head" id="wL2tpServer">L2TP Server IP Address</td>
  <td><input name="l2tpServer" maxlength="15" size=15 value="<%
       getCfgGeneral(1, "wan_l2tp_server"); %>"></td>
</tr>
<tr>
  <td class="head" id="wL2tpUser">User Name</td>
  <td><input name="l2tpUser" maxlength="20" size=20 value="<%
       getCfgGeneral(1, "wan_l2tp_user"); %>"></td>
</tr>
<tr>
  <td class="head" id="wL2tpPassword">Password</td>
  <td><input type="password" name="l2tpPass" maxlength="32" size=32 value="<%
       getCfgGeneral(1, "wan_l2tp_pass"); %>"></td>
</tr>
<tr>
  <td class="head" id="wL2tpAddrMode">Address Mode</td>
  <td>
    <select name="l2tpMode" size="1" onChange="l2tpModeSwitch()">
      <option value="0" id="wL2tpAddrModeS">Static</option>
      <option value="1" id="wL2tpAddrModeD">Dynamic</option>
    </select>
  </td>
</tr>
<tr id="l2tpIp">
  <td class="head" id="wL2tpIp">IP Address</td>
  <td><input name="l2tpIp" maxlength=15 size=15 value="<% getCfgGeneral(1, "wan_l2tp_ip"); %>"></td>
</tr>
<tr id="l2tpNetmask">
  <td class="head" id="wL2tpNetmask">Subnet Mask</td>
  <td><input name="l2tpNetmask" maxlength=15 size=15 value="<% getCfgGeneral(1, "wan_l2tp_netmask"); %>">
  </td>
</tr>
<tr id="l2tpGateway">
  <td class="head" id="wL2tpGateway">Default Gateway</td>
  <td><input name="l2tpGateway" maxlength=15 size=15 value="<% getCfgGeneral(1, "wan_l2tp_gateway"); %>">
  </td>
</tr>
<tr>
  <td class="head" rowspan="3" id="wL2tpOPMode">Operation Mode</td>
  <td>
    <select name="l2tpOPMode" size="1" onChange="l2tpOPModeSwitch()">
      <option value="KeepAlive" id="wL2tpKeepAlive">Keep Alive</option>
      <option value="OnDemand" id="wL2tpOnDemand">On Demand</option>
      <option value="Manual" id="wL2tpManual">Manual</option>
    </select>
  </td>
</tr>
<tr>
  <td>
    Keep Alive Mode: Redial Period
    <input type="text" name="l2tpRedialPeriod" maxlength="5" size="3" value="60">
    senconds
    <br />
    On demand Mode:  Idle Time
    <input type="text" name="l2tpIdleTime" maxlength="3" size="2" value="5">
    minutes
  </td>
</tr>
</table>

<!-- ================= PPTP Mode ================= -->
<table id="pptp" width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="wPptpMode">PPTP Mode</td>
</tr>
<tr>
  <td class="head" id="wPptpServer">PPTP Server IP Address</td>
  <td><input name="pptpServer" maxlength="15" size=15 value="<%
       getCfgGeneral(1, "wan_pptp_server"); %>"></td>
</tr>
<tr>
  <td class="head" id="wPptpUser">User Name</td>
  <td><input name="pptpUser" maxlength="20" size=20 value="<%
       getCfgGeneral(1, "wan_pptp_user"); %>"></td>
</tr>
<tr>
  <td class="head" id="wPptpPassword">Password</td>
  <td><input type="password" name="pptpPass" maxlength="32" size=32 value="<%
       getCfgGeneral(1, "wan_pptp_pass"); %>"></td>
</tr>
<tr>
  <td class="head" id="wPptpAddrMode">Address Mode</td>
  <td>
    <select name="pptpMode" size="1" onChange="pptpModeSwitch()">
      <option value="0" id="wPptpAddrModeS">Static</option>
      <option value="1" id="wPptpAddrModeD">Dynamic</option>
    </select>
  </td>
</tr>
<tr id="pptpIp">
  <td class="head" id="wPptpIp">IP Address</td>
  <td><input name="pptpIp" maxlength=15 size=15 value="<% getCfgGeneral(1, "wan_pptp_ip"); %>"></td>
</tr>
<tr id="pptpNetmask">
  <td class="head" id="wPptpNetmask">Subnet Mask</td>
  <td><input name="pptpNetmask" maxlength=15 size=15 value="<% getCfgGeneral(1, "wan_pptp_netmask"); %>">
  </td>
</tr>
<tr id="pptpGateway">
  <td class="head" id="wPptpGateway">Default Gateway</td>
  <td><input name="pptpGateway" maxlength=15 size=15 value="<% getCfgGeneral(1, "wan_pptp_gateway"); %>">
  </td>
</tr>
<tr>
  <td class="head" rowspan="3" id="wPptpOPMode">Operation Mode</td>
  <td>
    <select name="pptpOPMode" size="1" onChange="pptpOPModeSwitch()">
      <option value="KeepAlive" id="wPptpKeepAlive">Keep Alive</option>
      <option value="OnDemand" id="wPptpOnDemand">On Demand</option>
      <option value="Manual" id="wPptpManual">Manual</option>
    </select>
  </td>
</tr>
<tr>
  <td>
    Keep Alive Mode: Redial Period
    <input type="text" name="pptpRedialPeriod" maxlength="5" size="3" value="60">
    senconds
    <br />
    On demand Mode:  Idle Time
    <input type="text" name="pptpIdleTime" maxlength="3" size="2" value="5">
    minutes
  </td>
</tr>
</table>

<!-- =========== MAC Clone =========== -->
<table width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="wMacClone">MAC Address Clone</td>
</tr>
<tr>
  <td class="head">Enabled</td>
  <td>
    <select name="macCloneEnbl" size="1" onChange="macCloneSwitch()">
      <option value="0" id="wMacCloneD">Disable</option>
      <option value="1" id="wMacCloneE">Enable</option>
    </select>
  </td>
</tr>
<tr id="macCloneMacRow">
  <td class="head" id="wMacCloneAddr">MAC Address</td>
  <td>
	<input name="macCloneMac" id="macCloneMac" maxlength=17 value="<% getCfgGeneral(1, "macCloneMac"); %>">
	<input type="button" name="macCloneMacFill" id="macCloneMacFill" value="Fill my MAC" onclick="macCloneMacFillSubmit();" >
 </td>
</tr>
</table>

<table width="540" cellpadding="2" cellspacing="1">
<tr align="center">
  <td>
    <input type=submit style="{width:120px;}" value="Apply" id="wApply">&nbsp;&nbsp;
    <input type=reset  style="{width:120px;}" value="Cancel" id="wCancel" onClick="window.location.reload()">
  </td>
</tr>
</table>
</form>

</td></tr></table>
 </center>
</div>
</body>
</html>

