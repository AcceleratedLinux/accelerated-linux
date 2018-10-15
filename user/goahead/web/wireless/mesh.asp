<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>Basic Wireless Settings</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

var PhyMode  = '<% getCfgZero(1, "WirelessMode"); %>';
var meshenable = '<% getCfgZero(1, "MeshEnabled"); %>';

function style_display_on()
{
	if (window.ActiveXObject)  // IE
	{
		return "block";
	}
	else if (window.XMLHttpRequest)  // Mozilla, Safari,...
	{
		return "table-row";
	}
}

function checkHex(str){
	var len = str.length;

	for (var i=0; i<str.length; i++) 
	{
		if ((str.charAt(i) >= '0' && str.charAt(i) <= '9') ||
		    (str.charAt(i) >= 'a' && str.charAt(i) <= 'f') ||
		    (str.charAt(i) >= 'A' && str.charAt(i) <= 'F') )
		{
				continue;
		}
		else
	        	return false;
	}

	return true;
}

function checkInjection(str)
{
	var len = str.length;
	for (var i=0; i<str.length; i++) {
		if ( str.charAt(i) == '\r' || str.charAt(i) == '\n')
		{
				return false;
		}
		else
			continue;
	}
    return true;
}

function hiddenBasicWeb()
{
	document.wireless_mesh.MeshID.disabled = true;
	document.wireless_mesh.HostName.disabled = true;
	document.wireless_mesh.AutoLinkEnable.disabled = true;
	document.getElementById("div_mesh_settings").style.visibility = "hidden";
	document.getElementById("div_mesh_settings").style.display = "none";
	document.getElementById("manual_link").style.visibility = "hidden";
	document.getElementById("manual_link").style.display = "none";
	document.mesh_manual_link.mpmac.disabled = true;
}

function hiddenSecureWeb()
{
	document.getElementById("div_open_secure_mode").style.visibility = "hidden";
	document.getElementById("div_open_secure_mode").style.display = "none";
	document.wireless_mesh.open_encrypt_type.disabled= true;
	document.getElementById("div_wpa_algorithms").style.visibility = "hidden";
	document.getElementById("div_wpa_algorithms").style.display = "none";
	document.wireless_mesh.wpa_cipher[0].disabled = true;
	document.wireless_mesh.wpa_cipher[1].disabled = true;
	document.getElementById("div_wep").style.visibility = "hidden";
	document.getElementById("div_wep").style.display = "none";
	document.wireless_mesh.wep_key.disabled = true;
	document.wireless_mesh.wep_select.disabled = true;
	document.getElementById("div_wpa").style.visibility = "hidden";
	document.getElementById("div_wpa").style.display = "none";
	document.wireless_mesh.passphrase.disabled = true;
}

function initTranslation()
{
	var e = document.getElementById("meshTitle");
	e.innerHTML = _("mesh title");
	e = document.getElementById("meshIntroduction");
	e.innerHTML = _("mesh introduction");

	e = document.getElementById("meshMeshCapacity");
	e.innerHTML = _("mesh capacity");
	e = document.getElementById("meshMeshEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("meshMeshDisable");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("meshMeshSettings");
	e.innerHTML = _("mesh settings");
	e = document.getElementById("meshMID");
	e.innerHTML = _("mesh mid");
	e = document.getElementById("meshHostName");
	e.innerHTML = _("mesh hostname");

	e = document.getElementById("meshAutoLink");
	e.innerHTML = _("mesh autolink");
	e = document.getElementById("meshAutoLinkEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("meshAutoLinkDisable");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("meshSecurePolicy");
	e.innerHTML = _("secure security policy");
	e = document.getElementById("meshSecureMode");
	e.innerHTML = _("secure security mode");
	e = document.getElementById("meshSecureEncrypType");
	e.innerHTML = _("secure encryp type");
	e = document.getElementById("meshSecureWPAAlgorithm");
	e.innerHTML = _("secure wpa algorithm");
	e = document.getElementById("meshSecureWEP");
	e.innerHTML = _("secure wep");
	e = document.getElementById("meshSecureWEPKey");
	e.innerHTML = _("secure wep key");
	e = document.getElementById("meshSecreWPA");
	e.innerHTML = _("secure wpa");
	e = document.getElementById("meshSecureWPAPassPhrase");
	e.innerHTML = _("secure wpa pass phrase");

	e = document.getElementById("meshApply");
	e.value = _("wireless apply");
	e = document.getElementById("meshCancel");
	e.value = _("wireless cancel");

	e = document.getElementById("meshMLink");
	e.innerHTML = _("mesh manual");
	e = document.getElementById("meshMPMAC");
	e.innerHTML = _("mesh mpmac");

	e = document.getElementById("meshAddLink");
	e.value = _("station add");
	e = document.getElementById("meshDelLink");
	e.value = _("station del");

	e = document.getElementById("meshMeshInfo");
	e.innerHTML = _("mesh info");

	e = document.getElementById("meshReresh");
	e.value = _("stawps refresh");

	e = document.getElementById("meshNbrMacAddr");
	e.innerHTML = _("mesh nbrmacaddr");
	e = document.getElementById("meshNbrRSSI");
	e.innerHTML = _("scan rssi");
	e = document.getElementById("meshNbrMID");
	e.innerHTML = _("mesh mid");
	e = document.getElementById("meshNbrHostName");
	e.innerHTML = _("mesh hostname");
	e = document.getElementById("meshNbrChannel");
	e.innerHTML = _("basic frequency");
	e = document.getElementById("meshNbrEncrypType");
	e.innerHTML = _("secure encryp type");
}

function initValue()
{
	var AuthMode = '<% getCfgGeneral(1,"MeshAuthMode"); %>';
	var EncrypType = '<% getCfgGeneral(1,"MeshEncrypType"); %>';

	initTranslation();

	if (meshenable == "1")
	{
		document.wireless_mesh.MeshEnable.options.selectedIndex = 1;
		var autolink = '<% getCfgZero(1, "MeshAutoLink"); %>';
		if (autolink == "1")
		{
			document.wireless_mesh.AutoLinkEnable[1].checked = true;
		}
		else
		{
			document.wireless_mesh.AutoLinkEnable[0].checked = true;
		}
		if (AuthMode == "OPEN" || AuthMode == "")
		{
			document.wireless_mesh.security_mode.options.selectedIndex = 0;
			if (EncrypType == "NONE")
			{
				document.wireless_mesh.open_encrypt_type.options.selectedIndex = 0;
			}
			else if (EncrypType == "WEP")
			{
				var WEPKeyType = '<% getCfgGeneral(1,"MeshWEPKEYType"); %>';

				document.wireless_mesh.open_encrypt_type.options.selectedIndex = 1;
				document.wireless_mesh.wep_select.options.selectedIndex = (WEPKeyType == "0" ? 1 : 0);
			}
		}
		else if (AuthMode == "WPANONE")
		{
			document.wireless_mesh.security_mode.options.selectedIndex = 1;
			if (EncrypType == "TKIP")
			{
				document.wireless_mesh.wpa_cipher[0].checked = true;
			}
			else if (EncrypType == "AES")
			{
				document.wireless_mesh.wpa_cipher[1].checked = true;
			}
		}
	}
	else
	{
		document.wireless_mesh.MeshEnable.options.selectedIndex = 0;
	}
	switch_mesh_capacity();
}

function switch_mesh_capacity()
{
	hiddenBasicWeb();
	hiddenSecureWeb();
	if (document.wireless_mesh.MeshEnable.options.selectedIndex == 1)
	{
		document.getElementById("div_mesh_settings").style.visibility = "visible";
		if (window.ActiveXObject)    // IE
		{  
			document.getElementById("div_mesh_settings").style.display = "block";
		}
		else if (window.XMLHttpRequest) // Mozilla, Safari...
		{
			document.getElementById("div_mesh_settings").style.display = "table";
		}
		document.wireless_mesh.MeshID.disabled = false;
		document.wireless_mesh.HostName.disabled = false;
		document.wireless_mesh.AutoLinkEnable.disabled = false;
		switch_autolink();
		switch_security_mode();
	}
}

function switch_autolink()
{
	if (document.wireless_mesh.AutoLinkEnable[0].checked == true)
	{
		document.getElementById("manual_link").style.visibility = "visible";
			document.getElementById("manual_link").style.display = style_display_on();
		document.mesh_manual_link.mpmac.disabled = false;
	}
	else
	{
		document.getElementById("manual_link").style.visibility = "hidden";
		document.getElementById("manual_link").style.display = "none";
		document.mesh_manual_link.mpmac.disabled = true;
	}
}

function switch_security_mode()
{
	hiddenSecureWeb();

	if (document.wireless_mesh.security_mode.options.selectedIndex == 1)
	{
		document.getElementById("div_wpa_algorithms").style.visibility = "visible";
		document.getElementById("div_wpa_algorithms").style.display = style_display_on();
		document.wireless_mesh.wpa_cipher[0].disabled = false;
		document.wireless_mesh.wpa_cipher[1].disabled = false;
		document.getElementById("div_wpa").style.visibility = "visible";
		if (window.ActiveXObject)  // IE 
		{
			document.getElementById("div_wpa").style.display = "block";
		}
		else if (window.XMLHttpRequest)  // Mozilla, Safari...
		{
			document.getElementById("div_wpa").style.display = "table";
		}
		document.wireless_mesh.passphrase.disabled = false;
	}
	else
	{
		document.getElementById("div_open_secure_mode").style.visibility = "visible";
		document.getElementById("div_open_secure_mode").style.display = style_display_on();
		document.wireless_mesh.open_encrypt_type.disabled= false;
		switch_open_encrypt();
	}
}

function switch_open_encrypt()
{
	if (document.wireless_mesh.open_encrypt_type.options.selectedIndex == 1)
	{
		document.getElementById("div_wep").style.visibility = "visible";
		if (window.ActiveXObject)  // IE 
		{
			document.getElementById("div_wep").style.display = "block";
		}
		else if (window.XMLHttpRequest)  // Mozilla, Safari...
		{
			document.getElementById("div_wep").style.display = "table";
		}
		document.wireless_mesh.wep_key.disabled = false;
		document.wireless_mesh.wep_select.disabled = false;
	}
	else
	{
		document.getElementById("div_wep").style.visibility = "hidden";
		document.getElementById("div_wep").style.display = "none";
		document.wireless_mesh.wep_key.disabled = true;
		document.wireless_mesh.wep_select.disabled = true;
	}
}

function CheckValue()
{
	if (document.wireless_mesh.MeshEnable.options.selectedIndex == 1)
	{
		if (document.wireless_mesh.MeshID.value == "")
		{
			alert("Please enter Mesh ID!");
			document.wireless_mesh.MeshID.focus();
			document.wireless_mesh.MeshID.select();
			return false;
		}
		if (document.wireless_mesh.security_mode.options.selectedIndex == 0)
		{
			var encrypt_type = document.wireless_mesh.open_encrypt_type.options.selectedIndex;
			if (encrypt_type == 1 && !check_wep())
				return false;
		}
		else if (document.wireless_mesh.security_mode.options.selectedIndex == 1)
		{
			if (!check_wpa())
				return false;
		}

	}

	return true;
}

function check_wep()
{
	var keylength = document.wireless_mesh.wep_key.value.length;

	if (keylength == 0)
	{
		alert('Please input wep key!');
		return false;
	}
	if (keylength != 0)
	{
		if (document.wireless_mesh.wep_select.options.selectedIndex == 0)
		{
			if (keylength != 5 && keylength != 13) 
			{
				alert('Please input 5 or 13 characters of wep key!');
				return false;
			}
			if (checkInjection(document.wireless_mesh.wep_key.value) == false)
			{
				alert('Wep key contains invalid characters.');
				return false;
			}
		}
		if (document.wireless_mesh.wep_select.options.selectedIndex == 1)
		{
			if(keylength != 10 && keylength != 26) 
			{
				alert('Please input 10 or 26 characters of wep key!');
				return false;
			}
			if(checkHex(document.wireless_mesh.wep_key.value) == false)
			{
				alert('Invalid Wep key format!');
				return false;
			}
		}
	}

	return true;
}

function check_wpa()
{
	var keyvalue = document.wireless_mesh.passphrase.value;

	if (document.wireless_mesh.wpa_cipher[0].checked != true && 
	    document.wireless_mesh.wpa_cipher[1].checked != true)
	{
		alert('Please choose a WPA Algorithms.');
		return false;
	}
	if (keyvalue.length == 0)
	{
		alert('Please input wpapsk key!');
		return false;
	}

	if (keyvalue.length < 8)
	{
		alert('Please input at least 8 character of wpapsk key!');
		return false;
	}

	if(checkInjection(document.wireless_mesh.passphrase.value) == false)
	{
		alert('Invalid characters in Pass Phrase.');
		return false;
	}

	return true;
}

function checkData()
{
 	var reg = new RegExp("[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}");
		
	if (document.mesh_manual_link.mpmac.value.length == 0)
	{
		alert('Please input Mesh Point MAC Address!');
		document.mesh_manual_link.mpmac.focus();
		document.mesh_manual_link.mpmac.select();

		return false;
	}
	if (reg.test(document.mesh_manual_link.mpmac.value) == false)
	{
		alert("Please fill Mesh Point MAC Address in correct format! (XX:XX:XX:XX:XX:XX)");
		document.mesh_manual_link.mpmac.focus();
		document.mesh_manual_link.mpmac.select();

		return false;
	}
	
	return true;
}

function mesh_link_submit(parm)
{
	if (checkData())
	{
		document.mesh_manual_link.link_action.value = parm;
		document.mesh_manual_link.submit();
	}
}

function web_refresh()
{
	window.location.reload();
}
</script>
</head>

<body onLoad="initValue()">
<table class="body"><tr><td>

<h1 id="meshTitle">Mesh Network</h1>
<p id="meshIntroduction">A Mesh network is an IEEE 802 LAN comprised of IEEE 802.11 links and control elements to forward
frames among the network members.</p>
<hr />
<form method="post" name="wireless_mesh" action="/goform/wirelessMesh" onSubmit="return CheckValue()">
<table width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" id="meshMeshCapacity">Mesh Capacity</td>
  </tr>
  <tr>
    <td class="head">Mesh</td>
    <td>
      <select name="MeshEnable" onChange="switch_mesh_capacity()">
        <option value="0" checked id="meshMeshDisable">Disable</option>
	<option value="1" id="meshMeshEnable">Enable</option>
    </td>
  </tr>
</table>

<br />

<table width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD" id="div_mesh_settings">
  <tr> 
    <td class="title" colspan="2" id="meshMeshSettings">Mesh Settings</td>
  </tr>
  <tr> 
    <td class="head" id="meshMID">Mesh ID (MID)</td>
    <td><input type="text" name="MeshID" size="20" maxlength="32" value="<% getCfgGeneral(1, "MeshId"); %>"></td>
  </tr>
  <tr> 
    <td class="head" id="meshHostName">Host Name</td>
    <td><input type="text" name="HostName" size="20" maxlength="32" value="<% getCfgGeneral(1, "MeshHostName"); %>"></td>
  </tr>
  <tr>
    <td class="head" id="meshAutoLink">Auto Link</td>
    <td>
      <input type="radio" name="AutoLinkEnable" value="0" onClick="switch_autolink()" checked><font id="meshAutoLinkDisable">Disable&nbsp;</font>
      <input type="radio" name="AutoLinkEnable" value="1" onClick="switch_autolink()"><font id="meshAutoLinkEnable">Enable</font>
    </td>
  </tr>
</table>

<br />

<table border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540">
  <tr>
    <td class="title" colspan="2" id="meshSecurePolicy"> Security Policy </td>
  </tr>
  <tr> 
    <td class="head" id="meshSecureMode">Security Mode</td>
    <td>
      <select name="security_mode" size="1" onchange="switch_security_mode()">
        <option value="OPEN" selected>OPEN</option>
        <option value="WPANONE">WPANONE</option>
      </select>
    </td>
  </tr>
  <tr id="div_open_secure_mode" name="div_open_secure_mode"> 
    <td class="head" id="meshSecureEncrypType">Encrypt Type</td>
    <td>
      <select name="open_encrypt_type" size="1" onchange="switch_open_encrypt()">
	<option value="NONE" selected>None</option>
	<option value="WEP">WEP</option>
      </select>
    </td>
  </tr>
  <tr id="div_wpa_algorithms" name="div_wpa_algorithms"> 
    <td class="head" id="meshSecureWPAAlgorithm">WPA Algorithms</td>
    <td>
      <input name="wpa_cipher" value="TKIP" type="radio" checked>TKIP &nbsp;
      <input name="wpa_cipher" value="AES" type="radio">AES &nbsp;
    </td>
  </tr>
</table>
<table id="div_wep" name="div_wep" border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540">
  <tr> 
    <td class="title" colspan="3" id="meshSecureWEP">Wire Equivalence Protection (WEP)</td>
  </tr>
  <tr> 
    <td class="head" id="meshSecureWEPKey">WEP Key:</td>
    <td><input name="wep_key" id="WEP" maxlength="26" value="<% getCfgGeneral(1, "MeshWEPKEY"); %>"></td>
    <td>
      <select name="wep_select" size="1"> 
        <option value="1">ASCII</option>
	<option value="0" selected>Hex</option>
      </select>
    </td>
  </tr>
</table>
<table id="div_wpa" name="div_wpa" border="1" bordercolor="#9babbd" cellpadding="3" cellspacing="1" hspace="2" vspace="2" width="540">
  <tr>
    <td class="title" colspan="2" id="meshSecreWPA">WPA</td>
  </tr>
  <tr>
    <td class="head" id="meshSecureWPAPassPhrase">Pass Phrase</td>
    <td>
      <input name="passphrase" size="28" maxlength="64" value="<% getCfgGeneral(1,"MeshWPAKEY"); %>">
    </td>
  </tr>
</table>

<br />

<table width ="540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type="submit" style="{width:120px;}" value="Apply" id="meshApply"> &nbsp; &nbsp;
      <input type="reset"  style="{width:120px;}" value="Cancel" id="meshCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>  

<br />

<form method="post" name="mesh_manual_link" action="/goform/meshManualLink">
<input type="hidden" name="link_action" value="">
<table width="540" id="manual_link" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="3" id="meshMLink">Manual Mesh Link</td>
  </tr>
  <tr>
    <td class="head" id="meshMPMAC">Mesh Point MAC Address</td>
    <td><input type="text" name="mpmac" size=20 maxlength=17 value=""></td>
    <td>
      <input type="button" style="{width:120px;}" value="ADD" id="meshAddLink" onClick="mesh_link_submit('add')">&nbsp;
      <input type="button" style="{width:120px;}" value="DEL" id="meshDelLink" onClick="mesh_link_submit('del')">
    </td>
  </tr>
</table>
</form>  

<br />

<table width="540" id="mesh_info" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="7">
      <font id="meshMeshInfo">Mesh Network Infomation&nbsp;&nbsp;&nbsp;</font>
    </td>
  </tr>
  <tr align="center">
    <td class="head">&nbsp;&nbsp;</td>
    <td class="head" id="meshNbrMacAddr">Neighbor MAC Address</td>
    <td class="head" id="meshNbrRSSI">RSSI</td>
    <td class="head" id="meshNbrMID">Mesh ID</td>
    <td class="head" id="meshNbrHostName">Host Name</td>
    <td class="head" id="meshNbrChannel">Channel</td>
    <td class="head" id="meshNbrEncrypType">Encrypt Type</td>
  </tr>
  <% ShowMeshState(); %>
</table>
<table width ="540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="right">
    <td>
      <input type="button" style="{width:120px;}" name="refresh" value="Refresh" id="meshReresh" onClick="web_refresh()">
    </td>
  </tr>
</table>
</td></tr></table>
</body>
</html>

