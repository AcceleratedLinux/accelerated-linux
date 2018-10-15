<html>
<head>
<title>WPS STA</title>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script type="text/javascript" src="/wps/wps_timer.js"></script>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

var http_request = false;
function makeRequest(url, content, sync) {
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
	http_request.onreadystatechange = alertContents;

	http_request.open('POST', url, (sync == 1) ? false : true);
	http_request.send(content);
}

function isSafeForShell(str){

    for(i=0; i < str.length; i++){
		if(str.charAt(i) == ',' ) return false;
		if(str.charAt(i) == '\\') return false;
		if(str.charAt(i) == ';' ) return false;
		if(str.charAt(i) == '\'') return false;
		if(str.charAt(i) == '\n') return false;
		if(str.charAt(i) == '`' ) return false;
		if(str.charAt(i) == '\"') return false;
	}
	return true;
}

function alertContents() {
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			WPSStaUpdateStatus( http_request.responseText);
		} else {
//			alert('There was a problem with the request.');
		}
	}
}

function updateWPSStaStatus(){
	makeRequest("/goform/updateWPSStaStatus", "n/a");
}

function WPSStaUpdateStatus(str)
{
	document.getElementById("WPSInfo").value = str;
}

var WPSAPListStr = "<% getStaWPSBSSIDListASP(0); %>";
var WPSAPList = new Array();
WPSAPList = WPSAPListStr.split("\n\n");


var need_submit_registrar_setting_flag = 0;
var PINCode =  "<% getWPSSTAPINCodeASP(); %>";
var RegistrarSSID = "<% getWPSSTARegSSIDASP(); %>";
var RegistrarAuth = "<% getWPSSTARegAuthASP(); %>";
var RegistrarEncry = "<% getWPSSTARegEncryASP(); %>";
var RegistrarKeyType =  "<% getWPSSTARegKeyTypeASP(); %>";
var RegistrarKeyIndex = "<% getWPSSTARegKeyIndexASP(); %>";
var RegistrarKey =  "<% getWPSSTARegKeyASP(); %>";

function InitRegistrarSetting()
{
	enableSubmitButton(false);

	//SSID
	document.getElementById("SSID").value = RegistrarSSID;

	if(RegistrarAuth == ""){
		RegistrarAuth = "WPAPSK";
		RegistrarEncry = "TKIP";
		RegistrarKeyType = "1";
		RegistrarKeyIndex = "1";
		RegistrarKey = "12345678";
		need_submit_registrar_setting_flag = 1;
		enableSubmitButton(true);
	}

	//Auth
	if(RegistrarAuth == "OPEN"){
		document.getElementById("Authenication").options.selectedIndex = 0;
		AuthChange();
		if(RegistrarEncry == "NONE"){
			document.getElementById("EncryptTypeSelect").options.selectedIndex = 0;
		}else if(RegistrarEncry == "WEP"){
			document.getElementById("EncryptTypeSelect").options.selectedIndex = 1;

			// wep key type
			if(RegistrarKeyType == "0")
				document.getElementById("KeyTypeSelect").options.selectedIndex = 0;
			else if(RegistrarKeyType == "1")
				document.getElementById("KeyTypeSelect").options.selectedIndex = 1;
			else
				return;

			// wep key index
			if(RegistrarKeyIndex == "1")
				document.getElementById("KeyIndexSelect").options.selectedIndex = 0;
			else if(RegistrarKeyIndex == "2")
				document.getElementById("KeyIndexSelect").options.selectedIndex = 1;
			else if(RegistrarKeyIndex == "3")
				document.getElementById("KeyIndexSelect").options.selectedIndex = 2;
			else if(RegistrarKeyIndex == "4")
				document.getElementById("KeyIndexSelect").options.selectedIndex = 3;

			document.getElementById("Key").value = RegistrarKey;
		}else{
			alert("internal error 1");
			return;
		}
	}else if(RegistrarAuth == "WPAPSK"){
		document.getElementById("Authenication").options.selectedIndex = 1;
		AuthChange();
		if(RegistrarEncry == "TKIP"){
			document.getElementById("EncryptTypeSelect").options.selectedIndex = 0;
		}else if(RegistrarEncry == "AES"){
			document.getElementById("EncryptTypeSelect").options.selectedIndex = 1;
		}else{
			alert("internal error 2");
			return;
		}
		document.getElementById("Key").value = RegistrarKey;
//		document.getElementById("SSID").value = RegistrarKey;
	}else if(RegistrarAuth == "WPA2PSK"){
		document.getElementById("Authenication").options.selectedIndex = 2;	
		AuthChange();
		if(RegistrarEncry == "TKIP"){
			document.getElementById("EncryptTypeSelect").options.selectedIndex = 0;
		}else if(RegistrarEncry == "AES"){
			document.getElementById("EncryptTypeSelect").options.selectedIndex = 1;
		}else{
			alert("internal error 2");
			return;
		}
		document.getElementById("Key").value = RegistrarKey;
	}else{
		alert("internal error 3;");
		return;
	}

	EncryChange();
}

function GTLTConvert(str){
	var i;
	var result = "";
	for(i=0; i < str.length; i++){
		if(str.charAt(i) == '>')
			result = result + "&gt;";
		else if(str.charAt(i) == '<')
			result = result + "&lt;";
		else
			result = result + str.charAt(i);
	}
	return result;
}

function VersionTranslate(num)
{
	if(num == 16)
		return "1.0";
	else if(num == 17)
		return "1.1";
	else
		return "?";
}

function StateTranslate(num)
{
	if(num == 1)
		return "Unconf.";
	else if(num ==2)
		return "Conf.";
	else
		return "?";
}

function WPSAPClick(num)
{
	document.getElementById("APInfo").value = "";

	var WPSAPInfo = new Array;
	WPSAPInfo = WPSAPList[num].split("\n");	

	var i;
	for(i=8; i< WPSAPInfo.length; i++)
		document.getElementById("APInfo").value += (WPSAPInfo[i] + "\n") ;
}

function RegistrarTableShow(show)
{
	if(show){
		document.getElementById("registrarSettingTable").style.visibility = "visible";
		if (window.ActiveXObject) { // IE
			document.getElementById("registrarSettingTable").style.display = "block";
		}else if (window.XMLHttpRequest) { // Mozilla, Safari...
			document.getElementById("registrarSettingTable").style.display = "table";
		}
	}else{
		document.getElementById("registrarSettingTable").style.visibility = "hidden";	
		document.getElementById("registrarSettingTable").style.display = "none";	
	}
}

function initTranslation()
{
	var e = document.getElementById("stawpsTitle");
	e.innerHTML = _("stawps title");
	e = document.getElementById("stawpsIntroduction");
	e.innerHTML = _("stawps introduction");

	e = document.getElementById("stawpsAPScan");
	e.innerHTML = _("stawps scan");
	e = document.getElementById("stawpsRefresh");
	e.value = _("stawps refresh");
	e = document.getElementById("stawpsMode_txt");
	e.innerHTML = _("stawps mode");
	e = document.getElementById("stawpsEnrollee");
	e.innerHTML = _("stawps enrollee");
	e = document.getElementById("stawpsRegistrar");
	e.innerHTML = _("stawps registrar");
	e = document.getElementById("stawpsPIN");
	e.innerHTML = _("wps pin mode");
	e = document.getElementById("stawpsPINStart");
	e.value = _("stawps pin start");
	e = document.getElementById("stawpsPBCStart");
	e.value = _("stawps pbc start");
	e = document.getElementById("stawpsCancel");
	e.value = _("wireless cancel");
	e = document.getElementById("gen_new_pin");
	e.value = _("stawps renew pin");

	e = document.getElementById("stawpsRegistrarSet");
	e.innerHTML = _("stawps registrar set");
	e = document.getElementById("stawpsSSID");
	e.innerHTML = _("station ssid");
	e = document.getElementById("stawpsAuth");
	e.innerHTML = _("station auth");
	e = document.getElementById("stawpsEncrypType");
	e.innerHTML = _("secure encryp type");
	e = document.getElementById("stawpsWEPKeyType");
	e.innerHTML = _("stawps wep key type");
	e = document.getElementById("stawpsWEPKeyIndex");
	e.innerHTML = _("stawps wep key index");
	e = document.getElementById("stawpsKey");
	e.innerHTML = _("stawps key");
	e = document.getElementById("submitButton");
	e.value = _("stawps submit");

	e = document.getElementById("stawpsStatus");
	e.innerHTML = _("wps status");
}

var STAMode = <% getWPSSTAModeASP(); %>;
function Init()
{
	initTranslation();
	if(WPSAPList.length)
		WPSAPClick(0);

	STATimerFlag = 1;
	InitializeTimer(3);

	InitRegistrarSetting();

	document.getElementById("STAWPSMode").options.selectedIndex = STAMode ? 1: 0;
	STAWPSModeChange();
}

function STAWPSModeChange()
{
	if( document.getElementById("STAWPSMode").value == "Registrar"){
		STAMode = 1;
		document.getElementById("PIN").readOnly = false;
		document.getElementById("PIN").value = "";
		RegistrarTableShow(true);
		hideGenNewPIN();

		// make flash keep this status
		makeRequest("/goform/WPSSTAMode", "1");
		document.getElementById("stawpsPBCStart").disabled = true;
	}else{
		STAMode = 0;
		document.getElementById("PIN").readOnly = true;
		document.getElementById("PIN").value = PINCode;
		RegistrarTableShow(false);
		showGenNewPIN();

		// make flash keep this status
		makeRequest("/goform/WPSSTAMode", "0");
		document.getElementById("stawpsPBCStart").disabled = false;
	}
}


function getSelectedSSID()
{
	var i;
	for(i=0; i< document.wpssta_form.WPSAPSelect.length ; i++){
		if(document.wpssta_form.WPSAPSelect[i].checked == true)
			break;
	}

	if(i == document.wpssta_form.WPSAPSelect.length){
		alert("Please select a WPS AP to process.");
		return;
	}

	var WPSAP;
	WPSAP = new Array();
	WPSAP = WPSAPList[i].split("\n");
	return WPSAP[0];
}


function PINStart()
{
	if(STAMode == 0){ // enrollee
		makeRequest("/goform/WPSSTAPINEnr", getSelectedSSID());
	}else if(STAMode == 1){	// Registrar

		if(!checkPIN(document.getElementById("PIN").value)){
			return;
		}

		if(need_submit_registrar_setting_flag){
			alert("This is the first time you running WPS registrar mode and \n the registrar settings need to be submitted.");
			return;
		}
		
		if(document.getElementById("submitButton").disabled == false){
			ret = confirm("You have changed the registrar settings but did NOT submit them.\nDo you really want running WPS without these settings?");
			if(!ret)
				return;
		}

		makeRequest("/goform/WPSSTAPINReg", document.getElementById("PIN").value + " " + getSelectedSSID());
	} 
}

function PBCStart()
{
	if(STAMode == 0){ // enrollee
		makeRequest("/goform/WPSSTAPBCEnr", "n/a1");
	}else if(STAMode == 1){	// Registrar
		makeRequest("/goform/WPSSTAPBCReg", "n/a2");
	}
}

function CancelSelected()
{
	makeRequest("/goform/WPSSTAStop", "n/a3");
}

function genNewPin()
{
	makeRequest("/goform/WPSSTAGenNewPIN", "n/a4", 1);
	//update PIN
	PINCode = http_request.responseText;	
	document.getElementById("PIN").value = PINCode;
}

function checkPIN(wsc_pin_code)
{
	if(wsc_pin_code == ""){
		alert("Please input the enrollee's PIN number");
		return false;
	}
	accum = 0;
	accum += 3 * (parseInt(wsc_pin_code / 10000000) % 10);
	accum += 1 * (parseInt(wsc_pin_code / 1000000) % 10);
	accum += 3 * (parseInt(wsc_pin_code / 100000) % 10);
	accum += 1 * (parseInt(wsc_pin_code / 10000) % 10);
	accum += 3 * (parseInt(wsc_pin_code / 1000) % 10);
	accum += 1 * (parseInt(wsc_pin_code / 100) % 10);
	accum += 3 * (parseInt(wsc_pin_code / 10) % 10);
	accum += 1 * (parseInt(wsc_pin_code / 1) % 10);
			
	if ((accum % 10) != 0){
		alert('ValidateChecksum failed, please try again !!');
		return false;
	}	
	return true;
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

function RegistrarSettingSubmit()
{
	var ret;
	if(!isSafeForShell(document.getElementById("SSID").value)){
		alert("The SSID contains dangerous characters\n");
		return false;
	}

	if(!isSafeForShell(document.getElementById("Key").value)){
		alert("The Key contains dangerous characters\n");
		return false;
	}

	if(document.getElementById("Authenication").value == "OPEN"){
		if(document.getElementById("EncryptTypeSelect").value == "NONE"){
			ret = confirm("No any security settings are selected. Are you sure?");
			if(!ret)
				return false;
		}else if(document.getElementById("EncryptTypeSelect").value == "WEP"){
			if(document.getElementById("KeyTypeSelect").value == "1"){
				if(	document.getElementById("Key").value.length != 5 &&
					document.getElementById("Key").value.length != 13){
						alert("WEP key invalid.");
						return false;
				}
			}else if(document.getElementById("KeyTypeSelect").value == "0"){
				if(	document.getElementById("Key").value.length != 10 &&
					document.getElementById("Key").value.length != 26){
						alert("WEP key invalid.");
						return false;
				}
				if( !checkHex(document.getElementById("Key").value)){
					alert("WEP key format invalid.");
					return false;
				}
			}
		}
	}else if(document.getElementById("Authenication").value == "WPAPSK" || document.getElementById("Authenication").value == "WPA2PSK" ){
		if(	document.getElementById("Key").value.length < 8 ||
			document.getElementById("Key").value.length > 64){
				alert("Key length invalid.");
				return false;
		}
	}else{
		return false;
	}

	enableSubmitButton(false);
	need_submit_registrar_setting_flag = 0;
	document.getElementById("hiddenPIN").value = document.getElementById("PIN").value;

	makeRequest("/goform/WPSSTARegistrarSetupSSID", document.getElementById("SSID").value);
	makeRequest("/goform/WPSSTARegistrarSetupRest",	document.getElementById("Authenication").value + " " +
												document.getElementById("EncryptTypeSelect").value + " " +
												document.getElementById("KeyTypeSelect").value + " " +
												document.getElementById("KeyIndexSelect").value);
	makeRequest("/goform/WPSSTARegistrarSetupKey", document.getElementById("Key").value);
	return true;
}

function enableSubmitButton(enable)
{
	document.getElementById("submitButton").disabled = (!enable);
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

function hideGenNewPIN()
{
		document.getElementById("gen_new_pin").style.visibility = "hidden";
		document.getElementById("gen_new_pin").style.display = "none";
}

function showGenNewPIN()
{
		document.getElementById("gen_new_pin").style.visibility = "visible";
		document.getElementById("gen_new_pin").style.display = style_display_on();
}

function hideKeyRow()
{
		document.getElementById("KeyRow").style.visibility = "hidden";
		document.getElementById("KeyRow").style.display = "none";
}

function showKeyRow()
{
		document.getElementById("KeyRow").style.visibility = "visible";
		document.getElementById("KeyRow").style.display = style_display_on();
}

function hideWEP()
{
	document.getElementById("keytype").style.visibility = "hidden";
	document.getElementById("keytype").style.display = "none";
	document.getElementById("keyindex").style.visibility = "hidden";
	document.getElementById("keyindex").style.display = "none";
}

function showWEP()
{
	document.getElementById("keytype").style.visibility = "visible";
	document.getElementById("keytype").style.display = style_display_on();
	document.getElementById("keyindex").style.visibility = "visible";
	document.getElementById("keyindex").style.display = style_display_on();
}

function AuthChange()
{
	encry_select = document.getElementById("EncryptTypeSelect");
	auth = document.getElementById("Authenication").value;
	
	hideKeyRow();
	hideWEP();

	if(auth == "OPEN"){
		encry_select.options.length = 0;
		encry_select.options[encry_select.length] = new Option("NONE", "NONE",	false, false);
		encry_select.options[encry_select.length] = new Option("WEP", "WEP", false, false);
	}else if(auth == "WPAPSK" || auth == "WPA2PSK"){
		encry_select.options.length = 0;
		encry_select.options[encry_select.length] = new Option("TKIP", "TKIP",	false, false);
		encry_select.options[encry_select.length] = new Option("AES", "AES", false, false);
	}else{
		return;
	}
	EncryChange();
}

function EncryChange()
{
	encry_select = document.getElementById("EncryptTypeSelect");
	auth = document.getElementById("Authenication").value;
	if(auth == "OPEN" && encry_select.value == "NONE" ){
		hideKeyRow();
		hideWEP();
	}else if(auth == "OPEN" && encry_select.value == "WEP" ){
		showKeyRow();
		showWEP();
	}else{
		showKeyRow();
		hideWEP();
	}
}

function RefreshClick()
{
	makeRequest("/goform/WPSSTABSSIDListReset", "n/a", 1);
	location.href=location.href;
}

</script>
</head>
<body onload="Init()">
<table class="body"><tbody><tr><td>
<h1 id="stawpsTitle">Wi-Fi Protected Setup (STA)</h1>
<p id="stawpsIntroduction"> You could setup security easily by choosing PIN or PBC method to do Wi-Fi Protected Setup.</p>

<form method="post" name="wpssta_form">
<table border="1" width="100%" border="1" cellpadding="2" cellspacing="1">
<tr class><td class="title" colspan="9" id="stawpsAPScan"> WPS AP site survey </td></tr>
<tbody>
<script type="text/javascript">
	document.write("<tr><td>No.</td><td>SSID</td><td>BSSID</td><td>RSSI</td><td>Ch.</td><td>Auth.</td><td>Encrypt</td><td>Ver.</td><td>Status</td></tr>");

	var i;
	for(i=0; i < WPSAPList.length -1; i++){
		var WPSAP;
		WPSAP = new Array();
		WPSAP = WPSAPList[i].split("\n");

		document.write("<tr>");
	    // ID: ???
		//document.write();

		//Radio
		document.write("<td>");
//		document.write(i+1);
		document.write("<input type=radio name=WPSAPSelect value=");
		document.write(i);
		document.write(" onClick=\"WPSAPClick(");
		document.write(i);
		document.write(");\"");
		if(i==0)
			document.write(" checked ");
		document.write("></td>");

	    // SSID
		document.write("<td>");
		document.write( GTLTConvert(  WPSAP[0] )  );
		document.write("</td>");

		//BSSID
		document.write("<td>");
		document.write(WPSAP[1]);
		document.write("</td>");

		//RSSI
		document.write("<td>");
		document.write(WPSAP[2]);
		document.write("%</td>");

	    // Channel
		document.write("<td>");
		document.write(WPSAP[3]);
		document.write("</td>");

		//Auth
		document.write("<td>");
		document.write(WPSAP[4]);
		document.write("</td>");

        //Encry
		document.write("<td>");
		document.write(WPSAP[5]);
		document.write("</td>");

        //Version
		document.write("<td>");
		document.write( VersionTranslate (WPSAP[6] ) );
		document.write("</td>");

        //wsc_state
		document.write("<td>");
		document.write( StateTranslate ( WPSAP[7] ) );
		document.write("</td>");

		// extend
		//document.write("<td>");
		//document.write(WPSAP[8]);
		//document.write("</td>");
		document.write("</tr>");
	}
</script>
</tbody></table>
</form>

<br>
<textarea name="APInfo" id="APInfo" cols="63" rows="5" wrap="off" readonly="1"></textarea>

<input value="Refresh" id="stawpsRefresh" onclick="RefreshClick();" type="button">
<font id="stawpsMode_txt">Mode</font>:<select name="STAWPSMode" id="STAWPSMode" size="1" onchange="STAWPSModeChange()">
		<option value=Enrollee id="stawpsEnrollee">Enrollee</option>
		<option value=Registrar id="stawpsRegistrar">Registrar</option>
</select>
<font id="stawpsPIN">PIN</font>:<input value="" name="PIN" id="PIN" size="6" maxlength="16" type="text">
<input value=" PIN Start" id="stawpsPINStart" onclick="PINStart();" type="button">
<input value=" PBC Start" id="stawpsPBCStart" onclick="PBCStart();" type="button">
<input value="Cancel" id="stawpsCancel" onclick="CancelSelected();" type="button">
&nbsp;&nbsp;&nbsp;
<input name="gen_new_pin" id="gen_new_pin" value="Renew PIN" onclick="genNewPin();" type="button">
<br>
<br>

<table border="1" cellpadding="2" cellspacing="1" width="400" id="registrarSettingTable">
<form method="post" name="registrar_form" action="/goform/WPSSTARegistrar">
<tr class><td class="title" colspan="2" id="stawpsRegistrarSet"> Registrar Settings</td></tr>
<tr class="head_filter"> <td id="stawpsSSID">SSID</td> <td> <input value="" name="SSID" id="SSID" size="20" maxlength="32" type="text" onKeyUp="enableSubmitButton(true);">  </td></tr>

<tr class="head_filter"> <td id="stawpsAuth">Authenication</td> <td>
<select name="Authenication" id="Authenication" size="1" onChange="AuthChange();enableSubmitButton(true);">
		<option value=OPEN>OPEN</option>
		<option value=WPAPSK>WPA-PSK</option>
		<option value=WPA2PSK>WPA2-PSK</option>
</select>
</td></tr>

<tr class="head_filter" id="encrypttype"> <td id="stawpsEncrypType">Encrypt Type</td> <td>
<select name="EncryptType" id="EncryptTypeSelect" size="1" onChange="EncryChange();enableSubmitButton(true);">
	<!-- Javascript would update this -->
</select>
</td></tr>

<tr class="head_filter" id="keytype"> <td id="stawpsWEPKeyType">WEP Key Type</td> <td>
<select name="KeyType" id="KeyTypeSelect" size="1" onChange="enableSubmitButton(true);">
		<option value="0">Hex</option>
		<option value="1">ASCII</option>
</select>
</td></tr>

<tr class="head_filter" id="keyindex"> <td id="stawpsWEPKeyIndex">WEP Key Index</td> <td>
<select name="KeyIndex" id="KeyIndexSelect" size="1" onChange="enableSubmitButton(true);">
		<option value=1>1</option>
		<option value=2>2</option>
		<option value=3>3</option>
		<option value=4>4</option>
</select>
</td></tr>

<tr class="head_filter" id="KeyRow"> <td id="stawpsKey">Key</td> <td> <input value="" name="Key" id="Key" size="32" maxlength="64" type="text" onKeyUp="enableSubmitButton(true);">  </td></tr>

<tr>
	<td colspan="2">
		<input type="hidden" name="hiddenPIN" id="hiddenPIN" value="">
		<input name="submitButton"  id="submitButton" value=" Submit " onclick="RegistrarSettingSubmit();" type="button"> 
	</td>
</tr>

</form>
</table>
<br>

<table border="1" cellpadding="1" cellspacing="1" width="100%">
<tbody><tr><td class="title" id="stawpsStatus">WPS Status</td></tr>
<tr>
	<td> 
		<textarea name="WPSInfo" id="WPSInfo" cols="63" rows="2" wrap="off" readonly="1"></textarea>
	</td>
</tr>
</tbody>
</table>

</td></tr></tbody></table>
</body></html>

