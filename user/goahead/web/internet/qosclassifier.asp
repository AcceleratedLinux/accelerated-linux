<!-- Copyright (c), Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<script type="text/javascript" src="/common.js"></script>

<title> </title>
<script language="JavaScript" type="text/javascript">

restartPage_init();

Butterlate.setTextDomain("internet");

var layer7_all = "<% getLayer7FiltersASP(); %>";
var layer7_filter_entries = new Array();
var layer7_filter_item = new Array();
var layer7_filename = new Array();
var layer7_name = new Array();
var layer7_intro = new Array();

layer7_filter_entries = layer7_all.split(";");
for(i=0; i<layer7_filter_entries.length; i++){
	layer7_filter_item = layer7_filter_entries[i].split("#");
	layer7_filename[i] = layer7_filter_item[0];
	layer7_name[i] = layer7_filter_item[1];
	layer7_intro[i] = layer7_filter_item[2];
}






var AF1Name = "<% getCfgGeneral(1, "QoSAF1Name"); %>";
var AF2Name = "<% getCfgGeneral(1, "QoSAF2Name"); %>";
var AF3Name = "<% getCfgGeneral(1, "QoSAF3Name"); %>";
var AF4Name = "<% getCfgGeneral(1, "QoSAF4Name"); %>";
var AF5Name = "<% getCfgGeneral(1, "QoSAF5Name"); %>";
var AF6Name = "<% getCfgGeneral(1, "QoSAF6Name"); %>";
AF1Name = (AF1Name == "") ? "NoName1" : AF1Name;
AF2Name = (AF2Name == "") ? "NoName2" : AF2Name;
AF3Name = (AF3Name == "") ? "NoName3" : AF3Name;
AF4Name = (AF4Name == "") ? "NoName4" : AF4Name;
AF5Name = (AF5Name == "") ? "NoName5" : AF5Name;
AF6Name = (AF6Name == "") ? "Default" : AF6Name;


function style_display_on()
{
	if (window.ActiveXObject) { // IE
		return "block";
	}else if (window.XMLHttpRequest) { // Mozilla, Safari,...
		return "table-row";
	}
}

function hiddenAll()
{
	document.getElementById("portRangeTR1").style.visibility = "hidden";
	document.getElementById("portRangeTR1").style.display = "none";
	document.getElementById("portRangeTR2").style.visibility = "hidden";
	document.getElementById("portRangeTR2").style.display = "none";
	document.getElementById("appTR").style.visibility = "hidden";
	document.getElementById("appTR").style.display = "none";
}

function layer7Change()
{
	var index = document.classifier.layer7.options.selectedIndex;
	var intro = document.getElementById("Layer7Intro");
	intro.innerHTML = layer7_intro[index];
}

function protocolChange()
{
	hiddenAll();
	document.classifier.dFromPort.disabled = true;
	document.classifier.dToPort.disabled = true;
	document.classifier.sFromPort.disabled = true;
	document.classifier.sToPort.disabled = true;

	document.classifier.dFromPort.value = 
		document.classifier.dToPort.value = 
		document.classifier.sFromPort.value = 
		document.classifier.sToPort.value = "";

	if( document.classifier.protocol.options.selectedIndex == 0){
		return;
	}else if( document.classifier.protocol.options.selectedIndex == 1 ||
		document.classifier.protocol.options.selectedIndex == 2){
		document.classifier.dFromPort.disabled = false;
		document.classifier.dToPort.disabled = false;
		document.classifier.sFromPort.disabled = false;
		document.classifier.sToPort.disabled = false;

		document.getElementById("portRangeTR1").style.visibility = "visible";
		document.getElementById("portRangeTR1").style.display = style_display_on();
		document.getElementById("portRangeTR2").style.visibility = "visible";
		document.getElementById("portRangeTR2").style.display = style_display_on();
	}else if(document.classifier.protocol.options.selectedIndex == 3){
		return;
	}else if(document.classifier.protocol.options.selectedIndex == 4){
		document.getElementById("appTR").style.visibility = "visible";
		document.getElementById("appTR").style.display = style_display_on();
	}
}

function initTranslation()
{
	var e;

	e = document.getElementById("QoSClassifierTitleStr");
	e.innerHTML = _("qos classifier title");
	e = document.getElementById("QoSClassifierNameStr");
	e.innerHTML = _("qos classifier name");
	e = document.getElementById("QoSClassifierGroupStr");
	e.innerHTML = _("qos classifier group");
	e = document.getElementById("QoSClassifierMacStr");
	e.innerHTML = _("qos classifier mac");
/*	e = document.getElementById("QoSClassifierInIf");
	e.innerHTML = _("qos classifier ingress if");  */
	e = document.getElementById("QoSClassifierDIPStr");
	e.innerHTML = _("qos classifier dip");
	e = document.getElementById("QoSClassifierSIPStr");
	e.innerHTML = _("qos classifier sip");
	e = document.getElementById("QoSClassifierPktLenStr");
	e.innerHTML = _("qos classifier pktlen");

	e = document.getElementById("QoSClassifierDSCPStr");
	e.innerHTML = _("qos classifier dscp");
	e = document.getElementById("QoSClassifierProtoStr");
	e.innerHTML = _("qos classifier proto");
	e = document.getElementById("QoSClassifierDPortStr");
	e.innerHTML = _("qos classifier dport");
	e = document.getElementById("QoSClassifierDPortStr");
	e.innerHTML = _("qos classifier sport");
	e = document.getElementById("QoSClassifierL7Str");
	e.innerHTML = _("qos classifier l7");
	e = document.getElementById("QoSClassifierRemarkStr");
	e.innerHTML = _("qos classifier remark");
	e = document.getElementById("QoSClassifierAutoStr");
	e.innerHTML = _("qos classifier auto");

	e = document.getElementById("QoSClassifierNewStr")
	e.value = _("qos rule add");
}
	
function initValue()
{
	initTranslation();

	hiddenAll();
}

function checkPortNum(num)
{
	if( num > 65536 || num < 0)
		return false;
	return true;
}

function checkInjection(str)
{
	var len = str.length;
	for (var i=0; i<str.length; i++) {
		if ( str.charAt(i) == '\r' || str.charAt(i) == '\n' || str.charAt(i) == ';' || str.charAt(i) == ','){
			return false;
		}else
			continue;
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

function isAllNumAndSlash(str)
{
	for (var i=0; i<str.length; i++){
		if( (str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.') || (str.charAt(i) == '/'))
			continue;
		return 0;
	}
	return 1;
}

function checkIpAddr(str)
{
	if(str == "")
		return false;

	if (isAllNumAndSlash(str) == 0)
		return false;

	var ip_pair = new Array();
	ip_pair = str.split("/");

	if(ip_pair.length > 2){
		return false;
	}

	if(ip_pair.length == 2){
		// sub mask
		if(!ip_pair[1].length)
			return false;
		if(!isNumOnly(ip_pair[1])){
			return false;
		}
		tmp = parseInt(ip_pair[1], 10);
		if(tmp < 0 || tmp > 32){
			return false;
		}
	}

    if( (!checkRange(ip_pair[0],1,0,255)) ||
		(!checkRange(ip_pair[0],2,0,255)) ||
		(!checkRange(ip_pair[0],3,0,255)) ||
		(!checkRange(ip_pair[0],4,0,254)) ){
		return false;
    }
	return true;
}


function checkForm()
{
	if(document.classifier.comment.value == ""){
		alert("Please input a name.");
		return false;
	}

	if(!checkInjection(document.classifier.comment.value)){
		alert("There is a illegal character in the name.");
		return false;
	}

	if(document.classifier.mac_address.value != ""){
		if(!checkMac(document.classifier.mac_address.value)){
			alert("The MAC address format is invalid.");
			return false;
		}
	}

	if(document.classifier.dip_address.value != ""){
		if(! checkIpAddr(document.classifier.dip_address.value) ){
			alert("The destination ip address format is invalid.");
			return false;
		}
	}

	if(document.classifier.sip_address.value != ""){
		if(! checkIpAddr(document.classifier.sip_address.value) ){
			alert("The source ip address format is invalid.");
			return false;
		}
	}

	// packet length
	if(	(document.classifier.pktlenfrom.value != "" && document.classifier.pktlento.value == "" ) || 
		(document.classifier.pktlento.value != "" && document.classifier.pktlenfrom.value == "" ) ){
		alert("Please input a range for packet length. (ex: 0-128 64-1024)");
		return false;
	}
	if( document.classifier.pktlenfrom.value != "" ){
		pktlenfrom = parseInt(document.classifier.pktlenfrom.value);
		pktlento = parseInt(document.classifier.pktlento.value);
		if(pktlenfrom > 2048 ||  pktlento > 2048){
			alert("The packet length is too big.");
			return false;
		}
		if(pktlenfrom < 0 ||  pktlento < 0){
			alert("The packet length is too small.");
			return false;
		}
		if(pktlento < pktlenfrom){
			alert("The packet length range is invalid.");
			return false;
		}
	}



	if(	document.classifier.mac_address.value == "" && 
		document.classifier.dip_address.value == "" && 
		document.classifier.sip_address.value == "" &&
		document.classifier.pktlenfrom.value == "" && 
		document.classifier.dscp.value == "" && 
		document.classifier.protocol.value == "" &&
		document.classifier.layer7.value == ""){
		alert("Please input the classifications.");
		return false;
	}

	if(document.classifier.procotol.value == "TCP" || document.classifier.procotol.value == "UDP"){
		if(document.classifier.dFromPort.value == "" && document.classifier.sFromPort.value == ""){
			alert("Please fill the port number");
			return false;
		}

		var dFromPort = parseInt(document.classifier.dFromPort.value);
		var dToPort = parseInt(document.classifier.dToPort.value);
		var sFromPort = parseInt(document.classifier.sFromPort.value);
		var sToPort = parseInt(document.classifier.sToPort.value);
		if(document.classifier.dFromPort.value != "" && !checkPortNum(dFromPort) ){
			alert("The destination port range is invalid.");
			return false;
		}
		if(document.classifier.dToPort.value != "" && !checkPortNum(dToPort) ){
			alert("The destination port range is invalid.");
			return false;
		}
		if(document.classifier.sFromPort.value != "" && !checkPortNum(sFromPort) ){
			alert("The source port range is invalid.");
			return false;
		}
		if(document.classifier.sToPort.value != "" && !checkPortNum(sToPort) ){
			alert("The source port range is invalid.");
			return false;
		}

		if(dToPort && (dToPort <= dFromPort)){
			alert("The destination port range is invalid.");
			return false;
		}
		if(sToPort && (sToPort <= sFromPort)){
			alert("The source port range is invalid.");
			return false;
		}
	}

	restartPage_block();

	return true;
}


</script>
</head>

<body onLoad="initValue()">
<table class="body"><tr><td>


<form method=post name="classifier" action="/goform/qosClassifier">
<table width="500" border="1" cellpadding="2" cellspacing="1">
<tr>
	<td class="title" colspan="4" id="QoSClassifierTitleStr">Classifier Settings</td>
</tr>

<tr>
	<td class="head" colspan="2" id="QoSClassifierNameStr">
		Name
	</td>
	<td colspan="2">
		<input type="text" name="comment" id="comment" size="16" maxlength="32">
	</td>
</tr>

<!--
		<input type=hidden name="af_index" id="af_index" value="">
-->
		<input type=hidden name="dp_index" id="dp_index" value="1">
<tr>
	<td class="head" colspan="2" id="QoSClassifierGroupStr">
		Group
	</td>
	<td colspan="2">
		<select name="af_index" id="af_index">
		<option value="5"> <script> document.write(AF5Name) </script> </option>
		<option value="2"> <script> document.write(AF2Name) </script> </option>
		<option value="6"> <script> document.write(AF6Name) </script> </option>
		<option value="1"> <script> document.write(AF1Name) </script> </option>
		</select>&nbsp;&nbsp;
	</td>
</tr>
</table>
<br>
<table width="500" border="1" cellpadding="2" cellspacing="1">
<tr>
	<td class="head" colspan="2" id="QoSClassifierMacStr">
		Mac address
	</td>
	<td colspan="2">
		 <input type="text" size="18" name="mac_address" id="mac_address">
	</td>
</tr>

<!-- 
<tr>
	<td class="head" colspan="2" id="QoSClassifierInIf">
		Ingress interface
	</td>
	<td colspan="2">
		<select name="ingress_if" id="ingress_if">
		<option value="None">None</option>
		<option value="TCP"></option>
		<option value="UDP">UDP</option>
		<option value="ICMP">ICMP</option>
		<option value="Application">Application</option>
		</select>&nbsp;&nbsp;

	</td>
</tr>
-->

<tr>
	<td class="head" colspan="2" id="QoSClassifierDIPStr">
		Dest IP Address
	</td>
	<td colspan="2">
		<input type="text" size="16" name="dip_address" id="dip_address">
		<!-- we dont support ip range in kernel 2.4.30 
		-<input type="text" size="16" name="dip_address2">
		-->
	</td>
</tr>

<tr>
	<td class="head" colspan="2" id="QoSClassifierSIPStr">
		Source IP Address
	</td>
	<td colspan="2">
  		<input type="text" size="16" name="sip_address" id="sip_address">
		<!-- we dont support ip range in kernel 2.4.30 
		-<input type="text" size="16" name="sip_address2">
		-->
	</td>
</tr>

<tr>
	<td class="head" colspan="2" id="QoSClassifierPktLenStr">
		Packet Length
	</td>
	<td colspan="2">
  		<input type="text" size="4" name="pktlenfrom" id="pktlenfrom"> -
		<input type="text" size="4" name="pktlento" id="pktlento">
		<font color="#808080" id="">(ex: 0-128 for small packets)</font>
	</td>
</tr>

<tr>
	<td class="head" colspan="2" id="QoSClassifierDSCPStr">
		DSCP
	</td>
	<td colspan="2">
		<select name="dscp" id="dscp">
		<option value=""></option>
		<option value="BE">BE (Default)</option>
		<option value="AF11">AF11</option>
		<option value="AF12">AF12</option>
		<option value="AF13">AF13</option>
		<option value="AF21">AF21</option>
		<option value="AF22">AF22</option>
		<option value="AF23">AF23</option>
		<option value="AF31">AF31</option>
		<option value="AF32">AF32</option>
		<option value="AF33">AF33</option>
		<option value="AF41">AF41</option>
		<option value="AF42">AF42</option>
		<option value="AF43">AF43</option>
		<option value="EF">EF</option>
		</select>&nbsp;&nbsp;
	</td>
</tr>

<tr>
	<td class="head" colspan="2" id="QoSClassifierProtoStr">
		Protocol
	</td>
	<td colspan="2">
		<select onChange="protocolChange()" name="protocol" id="procotol">
		<option value=""></option>
		<option value="TCP">TCP</option>
		<option value="UDP">UDP</option>
		<option value="ICMP">ICMP</option>
		<option value="Application">Application</option>
		</select>&nbsp;&nbsp;
	</td>
</tr>

<tr id="portRangeTR1" style="visibility: hidden;">
	<td class="head" colspan="2" id="QoSClassifierDPortStr">
		Dest. Port Range
	</td>
	<td colspan="2">
  		<input type="text" size="5" name="dFromPort" id="dFromPort">-
		<input type="text" size="5" name="dToPort" id="dToPort">
	</td>
</tr>

<tr id="portRangeTR2" style="visibility: hidden;">
	<td class="head" colspan="2" id="QoSClassifierSPortStr">
		Src Port Range
	</td>
	<td colspan="2">
  		<input type="text" size="5" name="sFromPort" id="sFromPort">-
		<input type="text" size="5" name="sToPort" id="sToPort">
	</td>
</tr>

<tr id="appTR" style="visibility: hidden;">
	<td class="head" colspan="2" id="QoSClassifierL7Str">
		Application
	</td>
	<td colspan="2">
		<select name="layer7" id="layer7" size="15" onChange="layer7Change()" >
		<script type="text/javascript">
			for(i=0; i<layer7_filename.length; i++){
				document.write("<option value=\""+layer7_filename[i]+"\">"+layer7_name[i]+"</option>");
			}
		</script>
		</select>&nbsp;&nbsp;<br>
		<span id="Layer7Intro"> </span>
		<br>
	</td>
</tr>
</table>
<br>
<table width="500" border="1" cellpadding="2" cellspacing="1">
<tr>
	<td class="head" colspan="2" id="QoSClassifierRemarkStr">
		Remark DSCP as:
	</td>
	<td colspan="2">
		<select name="remark_dscp" id="remark_dscp">
		<option value="Auto" id="QoSClassifierAutoStr">Auto</option>
		<option value="BE">BE (000000)</option>
		<option value="AF11">AF11</option>
		<option value="AF12">AF12</option>
		<option value="AF13">AF13</option>
		<option value="AF21">AF21</option>
		<option value="AF22">AF22</option>
		<option value="AF23">AF23</option>
		<option value="AF31">AF31</option>
		<option value="AF32">AF32</option>
		<option value="AF33">AF33</option>
		<option value="AF41">AF41</option>
		<option value="AF42">AF42</option>
		<option value="AF43">AF43</option>
		<option value="EF">EF (101110)</option>
		</select>&nbsp;&nbsp;
	</td>
</tr>
</table>
<input type="submit" name="modify" value="new" id="QoSClassifierNewStr" onClick="return checkForm()">

</form>

</td></tr></table>
 </center>
</div>
</body>
</html>