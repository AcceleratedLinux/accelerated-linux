<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<script type="text/javascript" src="/common.js"></script>
<title>AP Client Feature</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

restartPage_init();

var sbuttonMax=1;

var ssid = '<% getCfgGeneral(1, "ApCliSsid"); %>';
var bssid = '<% getCfgGeneral(1, "ApCliBssid"); %>';
var mode = '<% getCfgGeneral(1, "ApCliAuthMode"); %>';
var enc = '<% getCfgGeneral(1, "ApCliEncrypType"); %>';
var wpapsk = '<% getCfgGeneral(1, "ApCliWPAPSK"); %>';
var keyid = '<% getCfgGeneral(1, "ApCliDefaultKeyID"); %>';
var key1 = '<% getCfgGeneral(1, "ApCliKey1"); %>';
var key2 = '<% getCfgGeneral(1, "ApCliKey2"); %>';
var key3 = '<% getCfgGeneral(1, "ApCliKey3"); %>';
var key4 = '<% getCfgGeneral(1, "ApCliKey4"); %>';

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

function KeyTypeSwitch(id)
{
	var keytype = eval("document.wireless_apcli.apcli_key"+id+"type.options.selectedIndex");
	document.wireless_apcli.apcli_key1type.options.selectedIndex = keytype;
	document.wireless_apcli.apcli_key2type.options.selectedIndex = keytype;
	document.wireless_apcli.apcli_key3type.options.selectedIndex = keytype;
	document.wireless_apcli.apcli_key4type.options.selectedIndex = keytype;
}

function SecurityModeSwitch()
{
	document.getElementById("div_apcli_default_key").style.visibility = "hidden";
	document.getElementById("div_apcli_default_key").style.display = "none";
	document.wireless_apcli.apcli_default_key.disabled = true;
	document.getElementById("div_apcli_key1").style.visibility = "hidden";
	document.getElementById("div_apcli_key1").style.display = "none";
	document.wireless_apcli.apcli_key1.disabled = true;
	document.getElementById("div_apcli_key2").style.visibility = "hidden";
	document.getElementById("div_apcli_key2").style.display = "none";
	document.wireless_apcli.apcli_key2.disabled = true;
	document.getElementById("div_apcli_key3").style.visibility = "hidden";
	document.getElementById("div_apcli_key3").style.display = "none";
	document.wireless_apcli.apcli_key3.disabled = true;
	document.getElementById("div_apcli_key4").style.visibility = "hidden";
	document.getElementById("div_apcli_key4").style.display = "none";
	document.wireless_apcli.apcli_key4.disabled = true;
	document.getElementById("div_apcli_wpapsk").style.visibility = "hidden";
	document.getElementById("div_apcli_wpapsk").style.display = "none";
	document.wireless_apcli.apcli_wpapsk.disabled = true;

	var mysel = document.wireless_apcli.apcli_enc;
	mysel.options.length = 0;
	if (document.wireless_apcli.apcli_mode.options.selectedIndex == 0) {
		mysel.options[0] = new Option("None", "NONE");
		mysel.options[1] = new Option("Wep", "WEP");
		document.getElementById("div_apcli_default_key").style.visibility = "visible";
		document.getElementById("div_apcli_default_key").style.display = style_display_on();
		document.wireless_apcli.apcli_default_key.disabled = false;
		document.getElementById("div_apcli_key1").style.visibility = "visible";
		document.getElementById("div_apcli_key1").style.display = style_display_on();
		document.wireless_apcli.apcli_key1.disabled = false;
		document.getElementById("div_apcli_key2").style.visibility = "visible";
		document.getElementById("div_apcli_key2").style.display = style_display_on();
		document.wireless_apcli.apcli_key2.disabled = false;
		document.getElementById("div_apcli_key3").style.visibility = "visible";
		document.getElementById("div_apcli_key3").style.display = style_display_on();
		document.wireless_apcli.apcli_key3.disabled = false;
		document.getElementById("div_apcli_key4").style.visibility = "visible";
		document.getElementById("div_apcli_key4").style.display = style_display_on();
		document.wireless_apcli.apcli_key4.disabled = false;
	}
	else if (document.wireless_apcli.apcli_mode.options.selectedIndex == 1) {
		mysel.options[0] = new Option("None", "NONE");
		mysel.options[1] = new Option("Wep", "WEP");
		document.getElementById("div_apcli_default_key").style.visibility = "visible";
		document.getElementById("div_apcli_default_key").style.display = style_display_on();
		document.wireless_apcli.apcli_default_key.disabled = false;
		document.getElementById("div_apcli_key1").style.visibility = "visible";
		document.getElementById("div_apcli_key1").style.display = style_display_on();
		document.wireless_apcli.apcli_key1.disabled = false;
		document.getElementById("div_apcli_key2").style.visibility = "visible";
		document.getElementById("div_apcli_key2").style.display = style_display_on();
		document.wireless_apcli.apcli_key2.disabled = false;
		document.getElementById("div_apcli_key3").style.visibility = "visible";
		document.getElementById("div_apcli_key3").style.display = style_display_on();
		document.wireless_apcli.apcli_key3.disabled = false;
		document.getElementById("div_apcli_key4").style.visibility = "visible";
		document.getElementById("div_apcli_key4").style.display = style_display_on();
		document.wireless_apcli.apcli_key4.disabled = false;
	}
	else if (document.wireless_apcli.apcli_mode.options.selectedIndex == 2) {
		mysel.options[0] = new Option("TKIP", "TKIP");
		mysel.options[1] = new Option("AES", "AES");
		document.getElementById("div_apcli_wpapsk").style.visibility = "visible";
		document.getElementById("div_apcli_wpapsk").style.display = style_display_on();
		document.wireless_apcli.apcli_wpapsk.disabled = false;
	}
	else if (document.wireless_apcli.apcli_mode.options.selectedIndex == 3) {
		mysel.options[0] = new Option("TKIP", "TKIP");
		mysel.options[1] = new Option("AES", "AES");
		document.getElementById("div_apcli_wpapsk").style.visibility = "visible";
		document.getElementById("div_apcli_wpapsk").style.display = style_display_on();
		document.wireless_apcli.apcli_wpapsk.disabled = false;
	}
}

function initValue()
{
	document.wireless_apcli.apcli_ssid.value = ssid;
	document.wireless_apcli.apcli_bssid.value = bssid;
	if (mode == "OPEN")
		document.wireless_apcli.apcli_mode.options.selectedIndex = 0;
	else if (mode == "SHARED")
		document.wireless_apcli.apcli_mode.options.selectedIndex = 1;
	else if (mode == "WPAPSK")
		document.wireless_apcli.apcli_mode.options.selectedIndex = 2;
	else if (mode == "WPA2PSK")
		document.wireless_apcli.apcli_mode.options.selectedIndex = 3;
	SecurityModeSwitch();
}

function CheckHex(str)
{
	var len = str.length;

	for (var i = 0; i < str.length; i++) {
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
				(str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
				(str.charAt(i) >= 'A' && str.charAt(i) <= 'F')) {
			continue;
		}
		else
			return false;
	}
	return true;
}

function CheckInjection(str)
{
	var len = str.length;
	for (var i = 0; i < str.length; i++) {
		if (str.charAt(i) == ';' || str.charAt(i) == ',' ||
				str.charAt(i) == '\r' || str.charAt(i) == '\n') {
			return false;
		}
		else
			continue;
	}
	return true;
}

function CheckWep()
{
	var defaultid = 1*document.wireless_apcli.apcli_default_key.value;
	if (document.wireless_apcli.apcli_enc.value == "WEP") {
		if (eval("document.wireless_apcli.apcli_key"+defaultid).value.length == 0) {
			alert('Please input wep key'+defaultid+'!');
			eval("document.wireless_apcli.apcli_key"+defaultid).focus();
			return false;
		}
	}

	if (document.wireless_apcli.apcli_key1type.options.selectedIndex == 0) {
		for (var i = 1; i < 5; i++) {
			var mykey = eval("document.wireless_apcli.apcli_key"+i);
			if (mykey.value.length != 0 && mykey.value.length != 5 && mykey.value.length != 13) {
				alert('Please input 5 or 13 characters of wep key'+i);
				mykey.focus();
				return false;
			}
			if (!CheckInjection(mykey.value)) {
				alert('Wep key'+i+' contains invalid characters!');
				mykey.focus();
				return false;
			}
		}
	}
	else {
		for (var i = 1; i < 5; i++) {
			var mykey = eval("document.wireless_apcli.apcli_key"+i);
			if (mykey.value.length != 0 && mykey.value.length != 10 && mykey.value.length != 26) {
				alert('Please input 10 or 26 characters of wep key'+i);
				mykey.focus();
				return false;
			}
			if (!CheckHex(wireless_apcli.apcli_key1.value)) {
				alert('Invalid Wep key'+i+' format!');
				mykey.focus();
				return false;
			}
		}
	}
	
	sbutton_disable(sbuttonMax); 
	restartPage_block(); 
	
	return true;
}

function CheckWpa()
{
	if (document.wireless_apcli.apcli_wpapsk.value.length < 8) {
		alert("Pass Phrase length should be larger than 8!");
		document.wireless_apcli.apcli_wpapsk.focus();
		return false;
	}
	if (!CheckInjection(document.wireless_apcli.apcli_wpapsk.value)) {
		alert('Invalid characters in Pass Phrase.');
		document.wireless_apcli.apcli_wpapsk.focus();
		return false;
	}
	
	sbutton_disable(sbuttonMax); 
	restartPage_block(); 
	
	return true;
}

function CheckValue()
{
	if (document.wireless_apcli.apcli_ssid.value == '') {
		alert('empty SSID!');
		document.wireless_apcli.apcli_ssid.focus(); 
		return false;
	}

	if (document.wireless_apcli.apcli_bssid.value != '') {
		var re = /[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}/;
		if (!re.test(document.wireless_apcli.apcli_bssid.value)) {
			alert("BSSID format error!");
			document.wireless_apcli.apcli_bssid.focus(); 
			return false;
		}
	}

	if (document.wireless_apcli.apcli_mode.options.selectedIndex == 0)
		return CheckWep();
	else if (document.wireless_apcli.apcli_mode.options.selectedIndex == 1)
		return CheckWep();
	else if (document.wireless_apcli.apcli_mode.options.selectedIndex == 2)
		return CheckWpa();
	else if (document.wireless_apcli.apcli_mode.options.selectedIndex == 3)
		return CheckWpa();
	else {
		alert('Unknown Security Mode!');
		return false;
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
  <td class="title" colspan="2" id="apcliTitle">AP Client Feature</td>
</tr>
<tr>

<tr>
<td colspan="2">
<p class="head" id="apcliIntroduction"> You could configure AP Client parameters here.</p>
</td>
<tr>
</table>

<br>

<form method=post name=wireless_apcli action="/goform/wirelessApcli" onSubmit="return CheckValue()">
<table width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="3">AP Client Parameters</td>
  </tr>
  <tr> 
    <td class="head" colspan="2">SSID</td>
    <td><input type=text name="apcli_ssid" size=20 maxlength=32></td>
  </tr>
  <tr> 
    <td class="head" colspan="2">MAC Address (Optional)</td>
    <td><input type=text name="apcli_bssid" size=20 maxlength=17></td>
  </tr>
  <tr> 
    <td class="head" colspan="2">Security Mode</td>
    <td>
      <select name="apcli_mode" id="apcli_mode" size="1" onchange="SecurityModeSwitch()">
	<option value="OPEN">Open</option>
	<option value="SHARED">Shared</option>
	<option value="WPAPSK">WPAPSK</option>
	<option value="WPA2PSK">WPA2PSK</option>
      </select>
    </td>
  </tr>
  <tr id="div_apcli_enc">
    <td class="head" colspan="2">Encryption Type</td>
    <td>
      <select name="apcli_enc" id="apcli_enc" size="1">
      </select>
    </td>
  </tr>
  <tr id="div_apcli_default_key">
    <td class="head" colspan="2">WEP Default Key</td>
    <td>
      <select name="apcli_default_key" id="apcli_default_key" size="1">
	<option value="1">Key 1</option>
	<option value="2">Key 2</option>
	<option value="3">Key 3</option>
	<option value="4">Key 4</option>
      </select>
    </td>
  </tr>
  <tr id="div_apcli_key1">
    <td class="head1" rowspan="4">WEP Keys</td>
    <td class="head2">WEP Key 1 :</td>
    <td>
      <input name="apcli_key1" id="apcli_key1" maxlength="26" size="20" value="<% getCfgGeneral(1, "ApCliKey1"); %>">
      <select id="apcli_key1type" name="apcli_key1type" onchange="KeyTypeSwitch(1)"> 
        <option value="1">ASCII</option>
	<option value="0">Hex</option>
      </select>
    </td>
  </tr>
  <tr id="div_apcli_key2">
    <td class="head2">WEP Key 2 :</td>
    <td>
      <input name="apcli_key2" id="apcli_key2" maxlength="26" size="20" value="<% getCfgGeneral(1, "ApCliKey2"); %>">
      <select id="apcli_key2type" name="apcli_key2type" onchange="KeyTypeSwitch(2)"> 
        <option value="1">ASCII</option>
	<option value="0">Hex</option>
      </select>
    </td>
  </tr>
  <tr id="div_apcli_key3">
    <td class="head2">WEP Key 3 :</td>
    <td>
      <input name="apcli_key3" id="apcli_key3" maxlength="26" size="20" value="<% getCfgGeneral(1, "ApCliKey3"); %>">
      <select id="apcli_key3type" name="apcli_key3type" onchange="KeyTypeSwitch(3)"> 
        <option value="1">ASCII</option>
	<option value="0">Hex</option>
      </select>
    </td>
  </tr>
  <tr id="div_apcli_key4">
    <td class="head2">WEP Key 4 :</td>
    <td>
      <input name="apcli_key4" id="apcli_key4" maxlength="26" size="20" value="<% getCfgGeneral(1, "ApCliKey4"); %>">
      <select id="apcli_key4type" name="apcli_key4type" onchange="KeyTypeSwitch(4)"> 
        <option value="1">ASCII</option>
	<option value="0">Hex</option>
      </select>
    </td>
  </tr>
  <tr id="div_apcli_wpapsk"> 
    <td class="head" colspan="2">Pass Phrase</td>
    <td><input type=text name="apcli_wpapsk" size=20 maxlength=64 value="<% getCfgGeneral(1, "ApCliWPAPSK"); %>"></td>
  </tr>
</table>

<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr id="sbutton0" align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>

</form>  

</td></tr></table>
 </center>
</div>
</body>
</html>