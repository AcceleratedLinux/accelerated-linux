<html><head><title>WPS</title>

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script type="text/javascript" src="/common.js"></script>
<script type="text/javascript" src="/wps/wps_timer.js"></script>

<script language="JavaScript" type="text/javascript">

Butterlate.setTextDomain("wireless");
restartPage_init();

var sbuttonMax=3;

function style_display_on(){
    if(window.ActiveXObject) { // IE
        return "block";
    } else if (window.XMLHttpRequest) { // Mozilla, Safari,...
        return "table-row";
    }
}

var http_request = false;
function makeRequest(url, content) {
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
		alert(_("alert giving up:cannot create an xmlhttp instance"));
		return false;
	}
	http_request.onreadystatechange = alertContents;
	http_request.open('POST', url, true);
	http_request.send(content);
}

function alertContents() {
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			WPSUpdateHTML( http_request.responseText);
		} else {
			alert(_("alert a problem with the request"));
		}
	}
}

function WPSUpdateHTML(str)
{
	var all_str = new Array();
	all_str = str.split("\n");

	wpsconfigured = document.getElementById("WPSConfigured");
	if(all_str[0] == "1" || all_str[0] == "0")
		wpsconfigured.innerHTML = "No";
	else if(all_str[0] == "2")
		wpsconfigured.innerHTML = "Yes";
	else
		wpsconfigured.innerHTML = "Unknown";
	
	wpsssid = document.getElementById("WPSSSID");
	wpsssid.innerHTML = all_str[1];

	wpsauthmode = document.getElementById("WPSAuthMode");
	wpsauthmode.innerHTML = all_str[2];

	wpsencryptype = document.getElementById("WPSEncryptype");
	wpsencryptype.innerHTML = all_str[3];

	wpsdefaultkeyindex = document.getElementById("WPSDefaultKeyIndex");
	wpsdefaultkeyindex.innerHTML = all_str[4];

	wpskeytype = document.getElementById("WPSKeyType");
	if(all_str[3] == "WEP"){
		wpskeytype.innerHTML = "WPS Key (Hex value)";
	}else
		wpskeytype.innerHTML = "WPS Key (ASCII)";

	wpswpakey = document.getElementById("WPSWPAKey");
	wpswpakey.innerHTML = all_str[5];

	wpsstatus = document.getElementById("WPSCurrentStatus");
	wpsstatus.innerHTML = all_str[6];


	/*if(all_str[7] == "-1")
		document.getElementById("WPSInfo").value = "WSC failed";
	else if(all_str[7] == "0"){
		document.getElementById("WPSInfo").value = "WSC:" + all_str[6];
	}else if(all_str[7] == "1")
		document.getElementById("WPSInfo").value = "WSC Success";*/
		
	if(all_str[7] == "-1")
		document.getElementById("WPSInfo").value = "Failed";
	else if(all_str[7] == "0"){
		document.getElementById("WPSInfo").value = all_str[6];
	}else if(all_str[7] == "1")
		document.getElementById("WPSInfo").value = "Success";
		
}

function updateWPS(){
	makeRequest("/goform/updateWPS", "something");
}

function enableTextField (field)
{
  if(document.all || document.getElementById)
    field.disabled = false;
  else {
    field.onfocus = field.oldOnFocus;
  }
}


function disableTextField (field)
{
  if(document.all || document.getElementById)
    field.disabled = true;
  else {
    field.oldOnFocus = field.onfocus;
    field.onfocus = skip;
  }
}

function ValidateChecksum(PIN)
{
    var accum = 0;

    accum += 3 * (parseInt(PIN / 10000000) % 10);
    accum += 1 * (parseInt(PIN / 1000000) % 10);
    accum += 3 * (parseInt(PIN / 100000) % 10);
    accum += 1 * (parseInt(PIN / 10000) % 10);
    accum += 3 * (parseInt(PIN / 1000) % 10);
    accum += 1 * (parseInt(PIN / 100) % 10);
    accum += 3 * (parseInt(PIN / 10) % 10);
    accum += 1 * (parseInt(PIN / 1) % 10);

    return ((accum % 10) == 0);
}

function PINPBCFormCheck()
{
	if(	document.WPS.PINPBCRadio[0].checked){
		// PIN
		if(document.WPS.PIN.value != ""){
			if( document.WPS.PIN.value.length < 4 )
			{
				alert(_("wps alert pin number length less than 4"));
				return false;
			}
			
			if( ! ValidateChecksum( document.WPS.PIN.value)  ){
				alert(_("wps alert pin number validation failed"));
				return false;
			}
			
			sbutton_disable(sbuttonMax); 
			restartPage_block();

			return true;
		}
		else
		{
				alert(_("wps alert null pin number"));
				return false;
		}
	}else{
	
		sbutton_disable(sbuttonMax); 
		restartPage_block();
		// PBC
		return true;
	}
}

function initTranslation()
{
	var e = document.getElementById("wpsTitle_text");
	e.innerHTML = _("wps title");
	e = document.getElementById("wpsIntroduction_text");
	e.innerHTML = _("wps introduction");
	//=====================================================================
	//e = document.getElementById("wpsIntroduction_text2");
	//e.innerHTML = _("wps introduction2");
	
	e = document.getElementById("wpsConfig_text");
	e.innerHTML = _("wps config");
	e = document.getElementById("wpsWPS_text");
	e.innerHTML = _("wps wps");
	e = document.getElementById("wpsDisable_text");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("wpsEnable_text");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("wpsConfigApply_text");
	e.value = _("wireless apply");

	e = document.getElementById("wpsSummary_text");
	e.innerHTML = _("wps summary");
	e = document.getElementById("wpsCurrentStatus_text");
	e.innerHTML = _("wps current status");
	e = document.getElementById("wpsConfigured_text");
	e.innerHTML = _("wps configured");
	e = document.getElementById("wpsSSID_text");
	e.innerHTML = _("wps ssid");
	e = document.getElementById("wpsAuthMode_text");
	e.innerHTML = _("wps auth mode");
	e = document.getElementById("wpsEncrypType_text");
	e.innerHTML = _("wps encryp type");
	e = document.getElementById("wpsDefaultKeyIndex_text");
	e.innerHTML = _("wps default key index");
	e = document.getElementById("wpsAPPIN_text");
	e.innerHTML = _("wps ap pin");
	e = document.getElementById("wpsResetOOB_text");
	e.value = _("wps reset oob");

	e = document.getElementById("wpsProgress_text");
	e.innerHTML = _("wps progress");
	e = document.getElementById("wpsMode_text");
	e.innerHTML = _("wps mode");
	e = document.getElementById("wpsPINMode_text");
	e.innerHTML = _("wps pin mode");
	e = document.getElementById("wpsPBCMode_text");
	e.innerHTML = _("wps pbc mode");
	e = document.getElementById("wpsPINNum_text");
	e.innerHTML = _("wps pin num");
	e = document.getElementById("wpsPINApply_text");
	e.value = _("wireless apply");
	
	e = document.getElementById("wpsStatus_text");
	e.innerHTML = _("wps status");

}

function pageInit()
{
	initTranslation();
	// hide tables first
	document.getElementById("div_wps_status").style.visibility = "hidden";
	document.getElementById("div_wps_status").style.display = "none";
	document.getElementById("div_wps_status_submit").style.visibility = "hidden";
	document.getElementById("div_wps_status_submit").style.display = "none";
	
	document.getElementById("div_wps").style.visibility = "hidden";
	document.getElementById("div_wps").style.display = "none";
	document.getElementById("div_wps_submit").style.visibility = "hidden";
	document.getElementById("div_wps_submit").style.display = "none";	
	document.getElementById("div_wps_info").style.visibility = "hidden";
	document.getElementById("div_wps_info").style.display = "none";

	wpsenable = <% getWPSModeASP(); %>;
	if(! wpsenable){
		// disable WPS
		document.getElementById("WPSEnable").options.selectedIndex = 0;
	}else{
		// enable WPS
		document.getElementById("WPSEnable").options.selectedIndex = 1;

		document.getElementById("div_wps_status").style.visibility = "hidden";
		document.getElementById("div_wps_status").style.display = "none";
		document.getElementById("div_wps_status_submit").style.visibility = "hidden";
		document.getElementById("div_wps_status_submit").style.display = "none";
	
		document.getElementById("div_wps").style.visibility = "hidden";
		document.getElementById("div_wps").style.display = "none";
		document.getElementById("div_wps_submit").style.visibility = "hidden";
		document.getElementById("div_wps_submit").style.display = "none";

		document.getElementById("div_wps_info").style.visibility = "hidden";
		document.getElementById("div_wps_info").style.display = "none";

		document.getElementById("div_wps_status").style.visibility = "visible";
		document.getElementById("div_wps_status_submit").style.visibility = "visible";
		document.getElementById("div_wps").style.visibility = "visible";
		document.getElementById("div_wps_submit").style.visibility = "visible";
		document.getElementById("div_wps_info").style.visibility = "visible";


		// show WPS-related tables
		if (window.ActiveXObject) { // IE
			document.getElementById("div_wps_status").style.display = "block";
			document.getElementById("div_wps_status_submit").style.display = "block";			
			document.getElementById("div_wps").style.display = "block";
			document.getElementById("div_wps_submit").style.display = "block";
			document.getElementById("div_wps_info").style.display = "block";
		}else if (window.XMLHttpRequest) { // Mozilla, Safari...
			document.getElementById("div_wps_status").style.display = "table";
			document.getElementById("div_wps_status_submit").style.display = "table";
			document.getElementById("div_wps").style.display = "table";
			document.getElementById("div_wps_submit").style.display = "table";
			document.getElementById("div_wps_info").style.display = "table";
 		}

		updateWPS();
		InitializeTimer(3);
	}
}


function onPINPBCRadioClick(value)
{
	if(value == 1){
		// PIN selected
		document.getElementById("PINRow").style.visibility = "visible";
		document.getElementById("PINRow").style.display = style_display_on();
	}else{
		// PBC selected
		document.getElementById("PINRow").style.visibility = "hidden";
		document.getElementById("PINRow").style.display = "none";

	}
}

</script>

</head>
<body onload="pageInit()" bgcolor="#FFFFFF">

<div align="center">
 <center>

<table class="body"><tr><td>

<table width="95%" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="wpsTitle_text">Wi-Fi Protected Setup</td>
</tr>
<tr>
<td colspan="1"  bgcolor="#DBE2EC">
<p class="head" id="wpsIntroduction_text"> You could setup security easily by choosing PIN or PBC method to do Wi-Fi Protected Setup.</p>
<!--<p class="head" id="wpsIntroduction_text2"> if you have enabled WEP and wish to enable WPS as well, WEP settings will automatically be changed to WPA. </p>-->
</td>
</tr>

</table>

<br>

<form method="post" name ="WPSConfig" action="/goform/WPSSetup">
<table border="1" cellpadding="2" cellspacing="1" width="95%">
<tbody>
<!-- ==================  WPS Config  ================== -->
<tr>
  <td class="title" colspan="2" id="wpsConfig_text">WPS Config</td>
</tr>

<tr>
  <td class="head" id="wpsWPS_text">WPS: </td>
  <td>	<select id="WPSEnable" name="WPSEnable" size="1">
			<option value=0 id="wpsDisable_text">Disable</option>
			<option value=1 id="wpsEnable_text">Enable</option>
		</select>
  </td>
</tr>
</tbody>
</table>

<table border="0" cellpadding="2" cellspacing="1" width="95%">
<tr id="sbutton0">
<td colspan="2" border="0"> <input type="submit" value="Apply" id="wpsConfigApply_text" name="submitWPSEnable" align="left" onClick="sbutton_disable(sbuttonMax); restartPage_block();"> </td>
</tr>
</table>
</form>

<br>

<table id="div_wps_status" name=="div_wps_status" border="1" cellpadding="2" cellspacing="1" width="95%" style="visibility: hidden;">
<tbody>

<!-- =================  WPS Summary  ================= -->
<tr>
  <td class="title" colspan="2" id="wpsSummary_text">WPS Summary</td>
</tr>

<tr>
  <td class="head" id="wpsCurrentStatus_text">WPS Current Status: </td>
  <td> <span id="WPSCurrentStatus"> </span> </td>
</tr>


<tr>
  <td class="head" id="wpsConfigured_text">WPS Configured: </td>
  <td> <span id="WPSConfigured"> </span> </td>
</tr>

<tr>
  <td class="head" id="wpsSSID_text">WPS SSID: </td>
  <td> <span id="WPSSSID"> </span> </td>
</tr>

<tr>
  <td class="head" id="wpsAuthMode_text">WPS Auth Mode: </td>
  <td> <span id="WPSAuthMode"> </span> </td>
</tr>

<tr>
  <td class="head" id="wpsEncrypType_text">WPS Encryp Type: </td>
  <td> <span id="WPSEncryptype"> </span> </td> 
</tr>

<tr>
  <td class="head" id="wpsDefaultKeyIndex_text">WPS Default Key Index: </td>
  <td> <span id="WPSDefaultKeyIndex"> </span> </td>
</tr>

<tr>
  <td class="head" > <span id="WPSKeyType"> </span></td>
  <td> <span id="WPSWPAKey"> </span> </td>
</tr>

<form method="post" name="SubmitGenPIN" action="/goform/GenPIN">
<tr>
  <td class="head" id="wpsAPPIN_text">AP PIN:
  </td>
  <td> <% getPINASP(); %> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type=submit value="Generate" name="GenPIN"></td>
</tr>
</form>
</tbody>
</table>

<table id="div_wps_status_submit" name=="div_wps_status_submit" border="0" width="95%" style="visibility: hidden;">
<tr id="sbutton1">
  <td colspan="2" border="0">
	<form method="post" name ="SubmitOOB" action="/goform/OOB">
		<input type="submit" value="Reset OOB" id="wpsResetOOB_text" name="submitResetOOB" align="left" onClick="sbutton_disable(sbuttonMax); restartPage_block();">
	</form>
  </td>
</tr>
</table>

<br>

<!-- ============================    WPS    ============================ -->
<form method="post" name ="WPS" action="/goform/WPS">
<table id="div_wps" name="div_wps"  border="1" cellpadding="2" cellspacing="1" width="95%" style="visibility: hidden;">
<tbody>
<tr>
  <td class="title" colspan="2" id="wpsProgress_text">WPS Progress</td>
</tr>

<tr>
	<td class="head" id="wpsMode_text">WPS mode</td>
	<td>
		<input name="PINPBCRadio" id="PINPBCRadio" value="1" type="radio" checked onClick="onPINPBCRadioClick(1)"><font id="wpsPINMode_text">PIN &nbsp;</font>
		<input name="PINPBCRadio" id="PINPBCRadio" value="2" type="radio" onClick="onPINPBCRadioClick(2)"><font id="wpsPBCMode_text">PBC &nbsp;</font>
	</td>
</tr>

<tr id="PINRow">
	<td class="head" id="wpsPINNum_text">PIN</td>
	<td>
		<input value="" name="PIN" id="PIN" size="8" maxlength="16" type="text">
	</td>
</tr>
</tbody>
</table>

<table id="div_wps_submit" name="div_wps_submit"  border="0" width="95%" style="visibility: hidden;">
<tr id="sbutton2">
	<td colspan="2" border="0">
		<input type="submit" value="Apply" id="wpsPINApply_text" name="submitWPS" align="left" onClick="return PINPBCFormCheck();">
	</td>
</tr>
</table>

</form>

<br>

<!-- =======================  WPS Info Bar  ======================= -->
<table id="div_wps_info" name="div_wps_info" border="1" cellpadding="1" cellspacing="1" width="95%" style="visibility: hidden;">
<tbody><tr><td class="title" id="wpsStatus_text">WPS Status</td></tr>
<tr><td> 
<textarea name="WPSInfo" id="WPSInfo" cols="63" rows="2" wrap="off" readonly="1"></textarea>
</td></tr>
</tbody></table>


<br>
</td></tr></table>
 </center>
</div>
</body></html>
