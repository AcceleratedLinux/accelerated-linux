<html>
<head>
<title>Virtual Settings</title>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script type="text/javascript" src="/common.js"></script>

<script language="JavaScript" type="text/javascript">
var rulelist = Array();
restartPage_init();

var sbuttonMax=3;

Butterlate.setTextDomain("firewall");

var MAX_RULES = 32;
var rules_num = <% getPortForwardRuleNumsASP(); %> ;

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
        alert(_("forward alert ip address format error"));
        field.value = field.defaultValue;
        field.focus();
	return false;
    }
    if(field.value == ""){
        alert(_("forward alert ip address is empty"));
        field.value = field.defaultValue;
        field.focus();
        return false;
    }

    if ( isAllNum(field.value) == 0) {
        alert(_("forward alert it should be a 0 to 9 number"));
        field.value = field.defaultValue;
        field.focus();
        return false;
    }

    if( (!checkRange(field.value,1,0,255)) ||
        (!checkRange(field.value,2,0,255)) ||
        (!checkRange(field.value,3,0,255)) ||
        (!checkRange(field.value,4,1,254)) ){
        alert(_("forward alert ip address range error"));
        field.value = field.defaultValue;
        field.focus();
        return false;
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

function isAllNum(str)
{
	for (var i=0; i<str.length; i++){
	    if((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.' ))
			continue;
		return 0;
	}
	return 1;
}

//kent_chang,2010-01-13, for exam port number
function isNumOnly(str)
{
	for (var i=0; i<str.length; i++){
	    if((str.charAt(i) >= '0' && str.charAt(i) <= '9'))
			continue;
		return 0;
	}
	return 1;
}



function is_valid_text(text)
{
	var num = "0123456789";
	var ascii = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	var extra = "~!@#$%^&*)_+{};<>[]:,.-";
    var i;
    var valid_string = num + ascii + extra;

    if (text.length > 32 || text.length < 0)
        return false;
    for (i = 0; i < text.length; i++)
        if (!is_contained_char(text.substr(i, 1), valid_string))
            return false;
    return true;
}

function formCheck()
{
	var i;
	if(rules_num >= (MAX_RULES) ){
		alert(_("forward alert the rule number is exceeded")+ MAX_RULES +".");
		return false;
	}
	/*
	if(!document.portForward.portForwardEnabled.options.selectedIndex){
		// user choose disable
		return true;
	}

	if(	document.portForward.ip_address.value == "" &&
		document.portForward.fromPort.value == "" &&
		document.portForward.toPort.value   == "" &&
		document.portForward.comment.value  == "")
		return true;
	*/
	// exam IP address
    if(document.portForward.ip_address.value == ""){
        alert(_("forward alert ip address is required"));
        document.portForward.ip_address.focus();
        return false;
    }

    if(! checkIpAddr(document.portForward.ip_address) ){
        document.portForward.ip_address.focus();
        return false;
    }

    if(isAllNum(document.portForward.ip_address.value) == 0){
        alert(_("forward alert invalid ip address"));
        document.portForward.ip_address.focus();
        return false;
    }

	// exam Port
	if(document.portForward.fromPort.value == ""){
		alert(_("forward alert please input fromport"));
		document.portForward.fromPort.focus();
		return false;
	}

	if(isNumOnly( document.portForward.fromPort.value ) == 0){
		alert(_("forward alert invalid port number"));
		document.portForward.fromPort.focus();
	return false;
	}

	d1 = atoi(document.portForward.fromPort.value, 1);
	if(d1 > 65535 || d1 < 1){
		alert(_("forward alert invalid port number"));
		document.portForward.fromPort.focus();
		return false;
	}
	
	if(document.portForward.toPort.value != ""){
		if(isNumOnly( document.portForward.toPort.value ) == 0){
			alert(_("forward alert invalid port number"));
			document.portForward.toPort.focus();
			return false;
		}
		d2 = atoi(document.portForward.toPort.value, 1);
		if(d2 > 65535 || d2 < 1){
			alert(_("forward alert invalid port number"));
			document.portForward.toPort.focus();
			return false;
		}
		if( (d2 == 8080)||( d2 == 23 ) ){
			alert(_("forward alert invalid port number"));
			document.portForward.toPort.focus();
			return false;
		}
		/*if(d1 > d2){
			alert("Invalid port range setting.");
			document.portForward.fromPort.focus();
			return false;
		}*/
		if(document.portForward.comment.value != ""){
			if (!is_valid_text(document.portForward.comment.value)){
				alert(_("forward alert illegal characters are not allowable"));
				document.portForward.comment.focus();
				return false;
			}
		}
   	} else {
		alert(_("forward alert please input toport"));
		document.portForward.toPort.focus();	
		return false;		
   	}

   var input = document.portForward.ip_address.value +  
   	document.portForward.fromPort.value + "-" + 
	document.portForward.toPort.value + 
	document.portForward.protocol.value.replace('&','+');
   var t;
   for (i = 0; i < rulelist.length; i++) {
   	t = '' + rulelist[i];
   	if (t == input) {
		alert(_("forward alert the same rule has existed"));
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
	var e = document.getElementById("forwardTitle");
	e.innerHTML = _("forward title");
	e = document.getElementById("forwardIntroduction");
	e.innerHTML = _("forward introduction");
	
	e = document.getElementById("forwardVirtualSrv");
	e.innerHTML = _("forward virtual server setting");
	/*e = document.getElementById("forwardVirtualSrvSet");
	e.innerHTML = _("forward virtual server setting");*/
	e = document.getElementById("forwardVirtualSrvOnOff");
	e.innerHTML = _("forward virtual server");
	e = document.getElementById("forwardVirtualSrvSetOnOff");
	e.innerHTML = _("forward virtual server");
	e = document.getElementById("forwardVirtualSrvDisable");
	e.innerHTML = _("firewall disable");
	e = document.getElementById("forwardVirtualSrvEnable");
	e.innerHTML = _("firewall enable");
	e = document.getElementById("forwardVirtualSrvIPAddr");
	e.innerHTML = _("forward virtual server ipaddr");
	e = document.getElementById("forwardVirtualSrvPortRange");
	e.innerHTML = _("forward virtual server port range");
	e = document.getElementById("forwardVirtualSrvPortRange2");
	e.innerHTML = _("forward virtual server port range2");
	e = document.getElementById("forwardVirtualSrvProtocol");
	e.innerHTML = _("firewall protocol");
	e = document.getElementById("forwardVirtualSrvComment");
	e.innerHTML = _("firewall comment");
	e = document.getElementById("forwardVirtualSrvApply");
	e.value = _("firewall apply");
	e = document.getElementById("forwardVirtualSrvReset");
	e.value = _("firewall reset");
	
	e = document.getElementById("forwardVirtualSrvApplyOnOff");
	e.value = _("firewall apply");

	e = document.getElementById("forwardCurrentVirtualSrv");
	e.innerHTML = _("forward current virtual server");
	e = document.getElementById("forwardCurrentVirtualSrvNo");
	e.innerHTML = _("firewall no");
	e = document.getElementById("forwardCurrentVirtualSrvIP");
	e.innerHTML = _("forward virtual server ipaddr");
	e = document.getElementById("forwardCurrentVirtualSrvPort");
	e.innerHTML = _("forward virtual server port mapping");
	e = document.getElementById("forwardCurrentVirtualSrvProtocol");
	e.innerHTML = _("firewall protocol");
	e = document.getElementById("forwardCurrentVirtualSrvComment");
	e.innerHTML = _("firewall comment");
	e = document.getElementById("forwardCurrentVirtualSrvDel");
	e.value = _("firewall del select");
	e = document.getElementById("forwardCurrentVirtualSrvReset");
	e.value = _("firewall reset");
}

function update_rulelist()
{
	var tbl = document.getElementById("rulelist");
	var i, text;
	var lines_no = tbl.rows.length;
	var browser = navigator.appName;	
	
	rulelist = Array();
	//kent_chang,2010-01-18, IE not support textContent
	if(browser=="Microsoft Internet Explorer"){
		 for (i = 2; i < lines_no; i++) {			
			text = '' + tbl.rows[i].cells[1].innerText + 
			tbl.rows[i].cells[2].innerText + 
			tbl.rows[i].cells[3].innerText;
			text = text.replace(/ /g, '');
			rulelist.push(text);
		}
	}else{
		for (i = 2; i < lines_no; i++) {
			text = '' + tbl.rows[i].cells[1].textContent + 
			tbl.rows[i].cells[2].textContent + 
			tbl.rows[i].cells[3].textContent;
			text = text.replace(/ /g, '');
			rulelist.push(text);
		}			
	}
	return;
}

function updateState()
{
	initTranslation();
    if(! rules_num ){
 		disableTextField(document.portForwardDelete.deleteSelPortForward);
 		disableTextField(document.portForwardDelete.reset);
	}else{
        enableTextField(document.portForwardDelete.deleteSelPortForward);
        enableTextField(document.portForwardDelete.reset);
	}
	/*if(document.portForwardOnOff.portForwardEnabled.options.selectedIndex == 1){
		enableTextField(document.portForward.ip_address);
		enableTextField(document.portForward.fromPort);
		enableTextField(document.portForward.toPort);
		enableTextField(document.portForward.protocol);  
		enableTextField(document.portForward.comment);
	}else{
		disableTextField(document.portForward.ip_address);
		disableTextField(document.portForward.fromPort);
		disableTextField(document.portForward.toPort);
		disableTextField(document.portForward.protocol);
		disableTextField(document.portForward.comment);
	}*//* minglin, 2010/01/11 *start*/
	if('<% getPortForwardEnableASP(1); %>' != "selected" ){// virtual server now is disabled
		document.portForward.style.visibility = "hidden";
		document.portForward.style.display =  "none";
		document.portForwardDelete.style.visibility = "hidden";
		document.portForwardDelete.style.display =  "none";
	}else{ // virtual server now is Enabled
		document.portForward.style.visibility = "visible";
		document.portForward.style.display = style_display_on();
		document.portForwardDelete.style.visibility = "visible";
		document.portForwardDelete.style.display = style_display_on();
	}
	/* minglin, 2010/01/11 *end*/
	update_rulelist();
}
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
</script>
</head>


                         <!--     body      -->
<body onload="updateState()" bgcolor="#FFFFFF">
<div align="center">
 <center>
<table class="body"><tr><td align="left">

<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>  
  <td class="title" colspan="2" id="forwardTitle">Virtual Server  Settings </td>
	<% checkIfUnderBridgeModeASP(); %>
</tr>
<tr>
<td colspan="2">
<p class="head" id="forwardIntroduction"> You may setup Virtual Servers to provide services on Internet.</p>
</td>
</tr>

</table>

<br>
<!-- Port Forward Enable/Disable-->
<form method="post" name="portForwardOnOff" action="/goform/portForwardOnOff" >
	<table width="540" border="1" cellpadding="2" cellspacing="1">
	<tr>
		<td class="title" colspan="2" id="forwardVirtualSrvOnOff">Virtual Server</td>
	</tr>
	<tr>
	<td class="head" id="forwardVirtualSrvSetOnOff">
		Virtual Server
	</td>
	<td>
	<select onChange="updateState()" name="portForwardEnabled" size="1">
		<option value=0 <% getPortForwardEnableASP(0); %> id="forwardVirtualSrvDisable">Disable</option>
		<option value=1 <% getPortForwardEnableASP(1); %> id="forwardVirtualSrvEnable">Enable</option>
    </select>
    </td>
</tr>
	</table>
	
	<table width="540" border="0" cellpadding="2" cellspacing="1">
<tr id="sbutton2" align="center">
	<td>
	<input type="submit" value="Apply" id="forwardVirtualSrvApplyOnOff" name="addFilterPortOnOff" onClick="sbutton_disable(sbuttonMax); restartPage_block();">
	</td>
</tr>
</table>
</form>

<!-- -->
<form method=post name="portForward" action=/goform/portForward>
<table width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="forwardVirtualSrv">Virtual Server Settings</td>
</tr>
<!--
<tr style="visibility:hidden" style="display:none" >
	<td class="head" id="forwardVirtualSrvSet">
		Virtual Server Settings
	</td>
	<td>
	<select onChange="updateState()" name="portForwardEnabled" size="1">
	<option value=0 <% getPortForwardEnableASP(0); %> id="forwardVirtualSrvDisable">Disable</option>
    <option value=1 <% getPortForwardEnableASP(1); %> id="forwardVirtualSrvEnable">Enable</option>
    </select>
    </td>
</tr>
-->

<tr>
	<td class="head" id="forwardVirtualSrvIPAddr">
		IP Address
	</td>
	<td>
  		<input type="text" size="16" name="ip_address">
	</td>
</tr>

<tr>
	<td class="head" id="forwardVirtualSrvPortRange">
		Private Port
	</td>
	<td>
  		<input type="text" size="5" name="fromPort">
	</td>
</tr>

<tr>
	<td class="head" id="forwardVirtualSrvPortRange2">
		Public Port
	</td>
	<td>
  		<input type="text" size="5" name="toPort">
	</td>
</tr>

<tr>
	<td class="head" id="forwardVirtualSrvProtocol">
		Protocol
	</td>
	<td>
		<select name="protocol">
   		<option select value="TCP&UDP">TCP&UDP</option>
		<option value="TCP">TCP</option>
   		<option value="UDP">UDP</option>
   		</select>&nbsp;&nbsp;
	</td>
</tr>

<tr>
	<td class="head" id="forwardVirtualSrvComment">
		Comment
	</td>
	<td>
		<input type="text" name="comment" size="16" maxlength="32">
	</td>
</tr>
</table>
<script>
    var strbuf = _("content filter message");
	document.write( "(" + strbuf + " " + MAX_RULES +".)");
</script>

<br>

<table width="540" border="0" cellpadding="2" cellspacing="1">
<tr id="sbutton0" align="center">
	<td>
	<input type="submit" value="Apply" id="forwardVirtualSrvApply" name="addFilterPort" onClick="return formCheck();"> &nbsp;&nbsp;
	<input type="reset" value="Reset" id="forwardVirtualSrvReset" name="reset">
	</td>
</tr>
</table>

</form>

<br>
<!--  delete rules -->

<form action=/goform/portForwardDelete method=POST name="portForwardDelete">

<table width="540" border="1" cellpadding="2" cellspacing="1" id="rulelist">	
	<tr>
		<td class="title" colspan="5" id="forwardCurrentVirtualSrv" width="530" dir="left">Current Virtual Servers in system</td>
	</tr>

	<tr>
		<td id="forwardCurrentVirtualSrvNo" width="27" bgcolor="#DBE2EC">No.</td>
		<td align=center id="forwardCurrentVirtualSrvIP" width="100" bgcolor="#DBE2EC">IP Address</td>
		<td align=center id="forwardCurrentVirtualSrvPort" width="100" bgcolor="#DBE2EC">Port Mapping</td>
		<td align=center id="forwardCurrentVirtualSrvProtocol" width="60" bgcolor="#DBE2EC">Protocol</td>
		<td align=center id="forwardCurrentVirtualSrvComment" width="60" bgcolor="#DBE2EC">Comment</td>
	</tr>

	<% showPortForwardRulesASP(); %>
</table>

<br>

<table width="539" border="0" cellpadding="2" cellspacing="1">
<tr id="sbutton1" align="center">
	<td width="533">
		<input type="submit" value="Delete Selected" id="forwardCurrentVirtualSrvDel" name="deleteSelPortForward" onClick="sbutton_disable(sbuttonMax); restartPage_block(); return deleteClick();">&nbsp;&nbsp;
		<input type="reset" value="Reset" id="forwardCurrentVirtualSrvReset" name="reset">
	</td>
</tr>
</table>

</form>

</td></tr></table>
 </center>
</div>
<p dir="ltr">&nbsp;</p>
</body>
</html>
