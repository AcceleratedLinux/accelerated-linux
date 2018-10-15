<html>
<head>
<title>IP/Port Filtering Settings</title>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script type="text/javascript" src="/common.js"></script>

<script language="JavaScript" type="text/javascript">
restartPage_init();
var rulelist;
var sbuttonMax=3;

Butterlate.setTextDomain("firewall");

var MAX_RULES = 32;

var secs;
var timerID = null;
var timerRunning = false;
var timeout = 3;
var delay = 1000;

var rules_num = <% getIPPortRuleNumsASP(); %>;

function InitializeTimer(){
	// Set the length of the timer, in seconds
	secs = timeout
	StopTheClock()
	StartTheTimer()
}

function StopTheClock(){
	if(timerRunning)
		clearTimeout(timerID)
	timerRunning = false
}

function StartTheTimer(){
	if (secs==0){
		StopTheClock()

		timerHandler();

		secs = timeout
		StartTheTimer()
    }else{
		self.status = secs
		secs = secs - 1
		timerRunning = true
		timerID = self.setTimeout("StartTheTimer()", delay)
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
			//updatePacketCount( http_request.responseText);
		} else {
			//alert('There was a problem with the request.');
		}
	}
}

function updatePacketCount(str){
	var pc = new Array();
	pc = str.split(" ");
	for(i=0; i < pc.length; i++){
		e = document.getElementById("pktCnt" + i);
		e.innerHTML = pc[i];
	}
}

function timerHandler(){
	makeRequest("/goform/getRulesPacketCount", "n/a");
}

function deleteClick()
{
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
	var cols;
	cols = field.value.split('.');
	if (cols.length != 4) {
		return false;
	}

	if(field.value == "")
		return false;

	if (isAllNumAndSlash(field.value) == 0){
		return false;
	}

	var ip_pair = new Array();
	ip_pair = field.value.split("/");

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

function update_rulelist()
{
	var tbl = document.getElementById("rulelist");
	var i, ip, port, protocol, text;
	var lines_no = tbl.rows.length;
	var browser = navigator.appName;

	rulelist = Array();
	//kent_chang,2010-01-18, IE not support textContent
	if(browser=="Microsoft Internet Explorer"){
		for (i = 2; i < lines_no - 1; i++) {
			text = '' + tbl.rows[i].cells[1].innerText + 
			tbl.rows[i].cells[2].innerText +
			tbl.rows[i].cells[3].innerText +
			tbl.rows[i].cells[4].innerText +
			tbl.rows[i].cells[5].innerText +
			tbl.rows[i].cells[6].innerText +
			tbl.rows[i].cells[7].innerText;
			text = text.replace(/ /g, '');
			//translate to English before rule comparison  
			text = text.replace(_("port filter action accept"), "Accept");
			text = text.replace(_("port filter action drop"), "Drop");
			rulelist.push(text);
		}
	}else{
		for (i = 2; i < lines_no - 1; i++) {
			text = '' + tbl.rows[i].cells[1].textContent + 
			tbl.rows[i].cells[2].textContent +
			tbl.rows[i].cells[3].textContent +
			tbl.rows[i].cells[4].textContent +
			tbl.rows[i].cells[5].textContent +
			tbl.rows[i].cells[6].textContent +
			tbl.rows[i].cells[7].textContent;
			text = text.replace(/ /g, '');
			//translate to English before rule comparison 
			text = text.replace(_("port filter action accept"), "Accept");
			text = text.replace(_("port filter action drop"), "Drop");
			rulelist.push(text);
		}
	}
	return;
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

function isAllNum(str)
{
	for (var i=0; i<str.length; i++){
	    if((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.'))
			continue;
		return 0;
	}
	return 1;
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



function isNumOnly(str)
{
	for (var i=0; i<str.length; i++){
	    if((str.charAt(i) >= '0' && str.charAt(i) <= '9') )
			continue;
		return 0;
	}
	return 1;
}

function is_valid_text(text, is_extra)
{
	var num = "0123456789";
	var ascii = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	var extra = "~!@#$%^&*)_+{};<>[]:,.";
	var extra;
    var i;
    var valid_string = num + ascii + extra;
	
	if (!is_extra) extra = "";
    if (text.length > 32 || text.length < 0)
        return false;
    for (i = 0; i < text.length; i++)
	{
        if (!is_contained_char(text.substr(i, 1), valid_string))
            return false;
	}
    return true;
}

function ipportFormCheck()
{
	if(rules_num >= (MAX_RULES) ){
		alert(_("port filter alert the rule number is exceeded")+ MAX_RULES +".");
		return false;
	}
	
	if( document.ipportFilter.sip_address.value == "" && 
		document.ipportFilter.dip_address.value == "" &&
		document.ipportFilter.sFromPort.value == "" &&
		document.ipportFilter.dFromPort.value == "" &&
		document.ipportFilter.mac_address.value == ""){
		alert(_("port filter alert please input any ip or/and port value"));
		return false;
	}
	
	if (document.ipportFilter.mac_address.value!="")
	{
		var re = /[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}/;
		//if (!re.test(document.ipportFilter.mac_address.value)) {
		if ((!re.test(document.ipportFilter.mac_address.value))||(document.ipportFilter.mac_address.value.indexOf(' ') >= 0)) {
			alert(_("port filter alert please fill the mac address in correct format"));
			document.ipportFilter.mac_address.focus();
			return false;
		}
		else
			document.ipportFilter.mac_address.value=document.ipportFilter.mac_address.value.toLowerCase(); // Fixed case, [Yian,2009/07/22]
	}
	//kent_chang, 2010-01-21,fixed check port range
	if(document.ipportFilter.protocol.value=="TCP" || document.ipportFilter.protocol.value=="UDP" ){

		if(document.ipportFilter.sFromPort.value != ""){
			d1 = atoi(document.ipportFilter.sFromPort.value, 1);
			if(isNumOnly( document.ipportFilter.sFromPort.value ) == 0){
				alert(_("port filter alert invalid source port number: from"));
				document.ipportFilter.sFromPort.focus();
				return false;
			}
			if(d1 > 65535 || d1 < 1){
				alert(_("port filter alert invalid source port number: from"));
				document.ipportFilter.sFromPort.focus();
				return false;
			}
			
			if(document.ipportFilter.sToPort.value != ""){
				if(isNumOnly( document.ipportFilter.sToPort.value ) == 0){
					alert(_("port filter alert invalid source port number: to"));
					return false;
				}		
				d2 = atoi(document.ipportFilter.sToPort.value, 1);
				if(d2 > 65535 || d2 < 1){
					alert(_("port filter alert invalid source port number: to"));
					return false;
				}
				if(d1 > d2){
				alert(_("port filter alert invalid source port range setting"));
				return false;
				}
			}
		}else{
			if(document.ipportFilter.sToPort.value != ""){
				alert(_("port filter alert invalid source port number: from"));
			}
		}

		if(document.ipportFilter.dFromPort.value != ""){
			d1 = atoi(document.ipportFilter.dFromPort.value, 1);
			if(isNumOnly( document.ipportFilter.dFromPort.value ) == 0){
				alert(_("port filter alert invalid dest port number: from"));
				return false;
			}
			if(d1 > 65535 || d1 < 1){
				alert(_("port filter alert invalid dest port number: from"));
				return false;
			}
			
			if(document.ipportFilter.dToPort.value != ""){
				if(isNumOnly( document.ipportFilter.dToPort.value ) == 0){
					alert(_("port filter alert invalid dest port number: to"));
					return false;
				}

				d2 = atoi(document.ipportFilter.dToPort.value, 1);
				if(d2 > 65535 || d2 < 1){
					alert(_("port filter alert invalid dest port number: to"));
					return false;
				}
				if(d1 > d2){
				alert(_("port filter alert invalid dest port range setting"));
				return false;
				}
			}
		}else{
			if(document.ipportFilter.dToPort.value != ""){
				alert(_("port filter alert invalid dest port number: from"));
				return false;
			}
		}
		
		if(document.ipportFilter.sFromPort.value == "" && document.ipportFilter.dFromPort.value == ""){
			alert(_("port filter alert please input src or dest from port number"));
			return false;
		}

	}

	// check ip address format
	if(document.ipportFilter.sip_address.value != ""){
		if(! checkIpAddr(document.ipportFilter.sip_address) ){
			alert(_("port filter alert source ip address format error"));
			return false;
		}
    	}
	
	if(document.ipportFilter.dip_address.value != ""){
		if(! checkIpAddr(document.ipportFilter.dip_address) ){
			alert(_("port filter alert dest ip address format error"));
			return false;
		}
    	}
    	/*--------------------------------------------------------------------
	 kent_chang, 2010-01-19, fixed the same rule check
         t = MAC/SIP/DIP/protocol/dfport(-dtport)/sfrport(-stport)/action
         it will replace with '-' when item not exist.
	 ex1:--192.168.10.1TCP--Accept ; ex2:---TCP25-66-Accept
        ---------------------------------------------------------------------*/
	var dest = (document.ipportFilter.dToPort.value == "") ? document.ipportFilter.dFromPort.value : 
                   document.ipportFilter.dFromPort.value + document.ipportFilter.dToPort.value;

	var src = (document.ipportFilter.sToPort.value == "") ? document.ipportFilter.sFromPort.value : 
                   document.ipportFilter.sFromPort.value + document.ipportFilter.sToPort.value;

   	var input = '' + document.ipportFilter.mac_address.value + 
   			 document.ipportFilter.dip_address.value + 
			 document.ipportFilter.sip_address.value + 
	                 ((document.ipportFilter.protocol.value == "None") ? '' : 
                         document.ipportFilter.protocol.value + dest + src) +
	                 document.ipportFilter.action.value;
   	var t;

   	for (i = 0; i < rulelist.length; i++) {
   		t = '' + rulelist[i];
		t = t.replace(/-/g,'');
   		if (t == input) {
			alert(_("port filter alert the same rule has existed"));
			return false;
		}
   	}
   
	if(document.ipportFilter.comment.value != "")
	{
   		if (!is_valid_text(document.ipportFilter.comment.value, false))
		{
			alert(_("port filter alert illegal characters are not allowable"));
			document.ipportFilter.comment.focus();
			return false;
		}
	}
	
    	sbutton_disable(sbuttonMax); 
    	restartPage_block(); 

	return true;
}


function display_on()
{
  if(window.XMLHttpRequest){ // Mozilla, Firefox, Safari,...
    return "table-row";
  } else if(window.ActiveXObject){ // IE
    return "block";
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

function enableTextField (field)
{
  if(document.all || document.getElementById)
    field.disabled = false;
  else {
    field.onfocus = field.oldOnFocus;
  }
}

function initTranslation()
{
	var e = document.getElementById("portTitle");
	e.innerHTML = _("port title");
	e = document.getElementById("portIntroduction");
	e.innerHTML = _("port introduction");

	e = document.getElementById("portBasicSet");
	e.innerHTML = _("port basic setting");
	e = document.getElementById("portBasicFilter");
	e.innerHTML = _("port basic filter");
	e = document.getElementById("portBasicDisable");
	e.innerHTML = _("firewall disable");
	e = document.getElementById("portBasicEnable");
	e.innerHTML = _("firewall enable");
	e = document.getElementById("portBasicDefaultPolicy");
	e.innerHTML = _("port basic default policy");
	e = document.getElementById("portBasicDefaultPolicyAccept");
	e.innerHTML = _("port basic default policy accepted");
	e = document.getElementById("portBasicDefaultPolicyDrop");
	e.innerHTML = _("port basic default policy dropped");
	e = document.getElementById("portBasicApply");
	e.value = _("firewall apply");
	e = document.getElementById("portBasicReset");
	e.value = _("firewall reset");

	e = document.getElementById("portFilterSet");
	e.innerHTML = _("port filter setting");
	e = document.getElementById("portFilterMac");
	e.innerHTML = _("port filter macaddr");
	e = document.getElementById("portFilterSIPAddr");
	e.innerHTML = _("port filter source ipaddr");
	e = document.getElementById("portFilterSPortRange");
	e.innerHTML = _("port filter source port range");
	e = document.getElementById("portFilterDIPAddr");
	e.innerHTML = _("port filter dest ipaddr");
	e = document.getElementById("portFilterDPortRange");
	e.innerHTML = _("port filter dest port range");
	e = document.getElementById("portFilterProtocol");
	e.innerHTML = _("firewall protocol");
	e = document.getElementById("portFilterAction");
	e.innerHTML = _("port filter action");
	e = document.getElementById("portFilterActionDrop");
	e.innerHTML = _("port filter action drop");
	e = document.getElementById("portFilterActionAccept");
	e.innerHTML = _("port filter action accept");
	e = document.getElementById("portFilterComment");
	e.innerHTML = _("firewall comment");
	e = document.getElementById("portFilterApply");
	e.value = _("firewall apply");
	e = document.getElementById("portFilterReset");
	e.value = _("firewall reset");

	e = document.getElementById("portCurrentFilter");
	e.innerHTML = _("port current filter");
	e = document.getElementById("portCurrentFilterNo");
	e.innerHTML = _("firewall no");
	e = document.getElementById("portCurrentFilterSIP");
	e.innerHTML = _("port filter current source ipaddr");
	e = document.getElementById("portCurrentFilterSPort");
	e.innerHTML = _("port filter current source port range");
	e = document.getElementById("portCurrentFilterDIP");
	e.innerHTML = _("port filter current dest ipaddr");
	e = document.getElementById("portCurrentFilterMac");
	e.innerHTML = _("port filter macaddr");
	e = document.getElementById("portCurrentFilterDPort");
	e.innerHTML = _("port filter current dest port range");
	e = document.getElementById("portCurrentFilterProtocol");
	e.innerHTML = _("firewall protocol");
	e = document.getElementById("portCurrentFilterAction");
	e.innerHTML = _("port filter action");
	/*e = document.getElementById("portCurrentFilterPacketCount");
	e.innerHTML = _("port filter packetcount");*/
	e = document.getElementById("portCurrentFilterComment");
	e.innerHTML = _("firewall comment");
	e = document.getElementById("portCurrentFilterDel");
	e.value = _("firewall del select");
	e = document.getElementById("portCurrentFilterReset");
	e.value = _("firewall reset");

	if(document.getElementById("portCurrentFilterDefaultDrop")){
		e = document.getElementById("portCurrentFilterDefaultDrop");
		e.innerHTML = _("firewall default drop");
	}
	if(document.getElementById("portCurrentFilterDefaultAccept")){
		e = document.getElementById("portCurrentFilterDefaultAccept");
		e.innerHTML = _("firewall default accept");
	}

	var i=0;
	while( document.getElementById("portFilterActionDrop"+i) ||
			document.getElementById("portFilterActionAccept"+i) ){
		if(document.getElementById("portFilterActionDrop"+i)){
			e = document.getElementById("portFilterActionDrop"+i);
			e.innerHTML = _("port filter action drop");
		}

		if(document.getElementById("portFilterActionAccept"+i)){
			e = document.getElementById("portFilterActionAccept"+i);
			e.innerHTML = _("port filter action accept");
		}

		i++;
	}
}


function defaultPolicyChanged()
{
	if( document.BasicSettings.defaultFirewallPolicy.options.selectedIndex == 0){
		document.ipportFilter.action.options.selectedIndex = 0;
	}else
		document.ipportFilter.action.options.selectedIndex = 1;
}
	
function updateState()
{
	initTranslation();
    if(! rules_num ){
 		disableTextField(document.ipportFilterDelete.deleteSelFilterPort);
 		disableTextField(document.ipportFilterDelete.reset);
	}else{
        enableTextField(document.ipportFilterDelete.deleteSelFilterPort);
        enableTextField(document.ipportFilterDelete.reset);
	}

	if( document.BasicSettings.defaultFirewallPolicy.options.selectedIndex == 0){
		document.ipportFilter.action.options.selectedIndex = 0;
	}else
		document.ipportFilter.action.options.selectedIndex = 1;

	protocolChange();
	//kent_chang,2010-01-21, Hide ipportFilter form when disable port filtering
	if('<% getIPPortFilterEnableASP(1); %>'== "selected"){
		//InitializeTimer();	// update packet count
		document.ipportFilter.style.visibility = "visible";
		document.ipportFilter.style.display = display_on();	
		document.getElementById("deletetable").style.visibility = "visible";
		document.getElementById("deletetable").style.display = display_on();	
	}else{
		document.ipportFilter.style.visibility = "hidden";
		document.ipportFilter.style.display = "none";	
		document.getElementById("deletetable").style.visibility = "hidden";
		document.getElementById("deletetable").style.display = "none";	
	}
	setTimeout("update_rulelist()", 600);
}

function actionChanged()
{
	if( document.BasicSettings.defaultFirewallPolicy.options.selectedIndex != 
		document.ipportFilter.action.options.selectedIndex)
		alert(_("port filter alert the action of this rule would be the same with default policy"));
}

function protocolChange()
{
	if( document.ipportFilter.protocol.options.selectedIndex == 1 ||
		document.ipportFilter.protocol.options.selectedIndex == 2){
		document.ipportFilter.dFromPort.disabled = false;
		document.ipportFilter.dToPort.disabled = false;
		document.ipportFilter.sFromPort.disabled = false;
		document.ipportFilter.sToPort.disabled = false;
	}else{
		document.ipportFilter.dFromPort.disabled = true;
		document.ipportFilter.dToPort.disabled = true;
		document.ipportFilter.sFromPort.disabled = true;
		document.ipportFilter.sToPort.disabled = true;

		document.ipportFilter.dFromPort.value = 
			document.ipportFilter.dToPort.value = 
			document.ipportFilter.sFromPort.value = 
			document.ipportFilter.sToPort.value = "";
	}
}

</script>
</head>

                         <!--     body      -->
<body onload="updateState()" bgcolor="#FFFFFF">
<div align="center">
 <center>

<table class="body"><tr><td>

<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="portTitle">Filtering Settings </td>
 <% checkIfUnderBridgeModeASP(); %>
</tr>
<tr>
<td colspan="2">
<p class="head" id="portIntroduction"> You may setup firewall rules to protect your network from virus,worm and malicious activity on the Internet.</p>
</td>
</tr>

</table>

<br>

<!-- ====================   BASIC  form  ==================== -->
<form method=post name="BasicSettings" action=/goform/BasicSettings>
<table width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
	<td class="title" colspan="2" id="portBasicSet">Basic Settings</td>
</tr>

<tr>
	<td class="head" id="portBasicFilter">
		MAC/IP/Port Filtering
	</td>
	<td>
	<select onChange="updateState()" name="portFilterEnabled" size="1">
	<option value=0 <% getIPPortFilterEnableASP(0); %> id="portBasicDisable">Disable</option>
    <option value=1 <% getIPPortFilterEnableASP(1); %> id="portBasicEnable">Enable</option>
    </select>
    </td>
</tr>

<tr>
	<td class="head" id="portBasicDefaultPolicy">
		Default Policy -- The packet that don't match with any rules would be:
	</td>
	<td>
		<select onChange="defaultPolicyChanged()" name="defaultFirewallPolicy">
		<option value=0 <% getDefaultFirewallPolicyASP(0); %> id="portBasicDefaultPolicyAccept">Accepted.</option>
		<option value=1 <% getDefaultFirewallPolicyASP(1); %> id="portBasicDefaultPolicyDrop">Dropped.</option>
		</select>
	</td>
</tr>
</table>

<table width="540" border="0" cellpadding="2" cellspacing="1">
<tr id="sbutton0" align="center">
	<td>
		<input type="submit" value="Apply" id="portBasicApply" name="addDMZ" onClick="sbutton_disable(sbuttonMax); restartPage_block();"> &nbsp;&nbsp;
		<input type="reset" value="Reset" id="portBasicReset" name="reset">
	</td>
</tr>
</table>

</form>

<br>

<!-- ====================   MAC/IP/Port form   ==================== -->
<form method=post name="ipportFilter" action=/goform/ipportFilter>
<table width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
	<td class="title" colspan="4" id="portFilterSet">IP/Port Filter Settings</td>
</tr>

<tr>
	<td class="head" colspan="2" id="portFilterMac">
		Mac address
	</td>
	<td colspan="2">
		 <input type="text" size="18" name="mac_address">
	</td>
</tr>

<tr>
	<td class="head" colspan="2" id="portFilterDIPAddr">
		Dest IP Address
	</td>
	<td colspan="2">
		<input type="text" size="16" name="dip_address">
		<!-- we dont support ip range in kernel 2.4.30 
		-<input type="text" size="16" name="dip_address2">
		-->
	</td>
</tr>

<tr>
	<td class="head" colspan="2" id="portFilterSIPAddr">
		Source IP Address
	</td>
	<td colspan="2">
  		<input type="text" size="16" name="sip_address">
		<!-- we dont support ip range in kernel 2.4.30 
		-<input type="text" size="16" name="sip_address2">
		-->
	</td>
</tr>

<tr>
	<td class="head" colspan="2" id="portFilterProtocol">
		Protocol
	</td>
	<td colspan="2">
		<select onChange="protocolChange()" name="protocol" id="procotol">
		<option value="None">None</option>
		<option value="TCP">TCP</option>
		<option value="UDP">UDP</option>
		<option value="ICMP">ICMP</option>
		</select>&nbsp;&nbsp;
	</td>
</tr>

<tr>
	<td class="head" colspan="2" id="portFilterDPortRange">
		Dest. Port Range
	</td>
	<td colspan="2">
  		<input type="text" size="5" name="dFromPort" id="dFromPort">-
		<input type="text" size="5" name="dToPort" id="dToPort">
	</td>
</tr>


<tr>
	<td class="head" colspan="2" id="portFilterSPortRange">
		Src Port Range
	</td>
	<td colspan="2">
  		<input type="text" size="5" name="sFromPort" id="sFromPort">-
		<input type="text" size="5" name="sToPort" id="sToPort">
	</td>
</tr>

<tr>
	<td class="head" colspan="2" id="portFilterAction">
		Action
	</td>
	<td colspan="2">
		<select onChange="actionChanged()" name="action">
   		<option value="Drop" id="portFilterActionDrop">Drop</option>
		<option value="Accept" id="portFilterActionAccept">Accept</option>
   		</select>
	</td>
</tr>

<tr>
	<td class="head" colspan="2" id="portFilterComment">
		Comment
	</td>
	<td colspan="2">
		<input type="text" name="comment" size="16" maxlength="32">
	</td>
</tr>
</table>
<script language="JavaScript" type="text/javascript">
	var strbuf = _("content filter message");
	document.write( "(" + strbuf + " " + MAX_RULES +".)");
</script>


<br>

<table width="540" border="0" cellpadding="2" cellspacing="1">
<tr id="sbutton1" align="center">
	<td>
	<input type="submit" value="Apply" id="portFilterApply" name="addFilterPort" onClick="return ipportFormCheck();"> &nbsp;&nbsp;
	<input type="reset" value="Reset" id="portFilterReset" name="reset">
	</td>
</tr>
</table>

</form>

<br>

<!-- =========================  delete rules  ========================= -->
<form action=/goform/ipportFilterDelete method=POST name="ipportFilterDelete">

<table width="540" border="1" cellpadding="2" cellspacing="1" id="rulelist">	
	<tr>
		<td class="title" colspan="10" id="portCurrentFilter" width="530" dir="ltr">Current IP/Port filtering rules in system </td>
	</tr>

	<tr>
		<td class="head" id="portCurrentFilterNo" width="25"> No.</td>
		<td class="head" align=center id="portCurrentFilterMac" width="57"> Mac Address </td>
		<td class="head" align=center id="portCurrentFilterDIP" width="62"> Dest IP Address </td>
		<td class="head" align=center id="portCurrentFilterSIP" width="66"> Source IP Address </td>
		<td class="head" align=center id="portCurrentFilterProtocol" width="52"> Protocol</td>
		<td class="head" align=center id="portCurrentFilterDPort" width="52"> Dest Port Range</td>
		<td class="head" align=center id="portCurrentFilterSPort" width="59"> Source Port Range</td>
		<td class="head" align=center id="portCurrentFilterAction" width="40"> Action</td>
		<td class="head" align=center id="portCurrentFilterComment" width="60"> Comment</td>
		<!--<td class="head" align=center id="portCurrentFilterPacketCount"> PktCnt</td>-->
	</tr>

	<% showIPPortFilterRulesASP(); %>
</table>
<br>

<table width="540" border="0" cellpadding="2" cellspacing="1" id="deletetable">
<tr id="sbutton2" align="center">
	<td>
		<p align="center">
		<input type="submit" value="Delete Selected" id="portCurrentFilterDel" name="deleteSelFilterPort" onClick="sbutton_disable(sbuttonMax); restartPage_block(); return deleteClick();">&nbsp;&nbsp;
		<input type="reset" value="Reset" id="portCurrentFilterReset" name="reset">
	</td>
</tr>
</table>

</form>

</td></tr></table>
 </center>
</div>
</body>
</html>
