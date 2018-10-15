<html><head><title>Static Routing Settings</title>

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script type="text/javascript" src="/common.js"></script>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("internet");

restartPage_init();

var sbuttonMax=3;

var opmode = "<% getCfgZero(1, "OperationMode"); %>";

var destination = new Array();
var gateway = new Array();
var netmask = new Array();
var flags = new Array();
var metric = new Array();
var ref = new Array();
var use = new Array();
var true_interface = new Array();
var category = new Array();
var interface = new Array();
var idle = new Array();
var comment = new Array();


/* checkDuplicateRoute, [2009/07/14] */
function checkDuplicateRoute(ip_s, netmask_s, gateway_s)
{
	var i;
	var entries = new Array();
	var all_str = <% getRoutingTable(); %>;

	entries = all_str.split(";");
	for(i=0; i<entries.length; i++){
		var one_entry = entries[i].split(",");

		true_interface[i] = one_entry[0];
		destination[i] = one_entry[1];
		gateway[i] = one_entry[2];
		netmask[i] = one_entry[3];
		flags[i] = one_entry[4];
		ref[i] = one_entry[5];
		use[i] = one_entry[6];
		metric[i] = one_entry[7];
		category[i] = parseInt(one_entry[8]);
		interface[i] = one_entry[9];
		idle[i] = parseInt(one_entry[10]);
		comment[i] = one_entry[11];
		if(comment[i] == " " || comment[i] == "")
			comment[i] = "&nbsp";
	}

	for(i=0; i<entries.length; i++)
	{
		if ((destination[i]==ip_s)&&(netmask[i]==netmask_s))
			return false;
	}
	return true;
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
	if(cols.length != 4){
        field.focus();
        return false;
	}
    if(field.value == ""){
        field.focus();
        return false;
    }

    if ( isAllNum(field.value) == 0) {
        field.focus();
        return false;
    }

    if( (!checkRange(field.value,1,0,255)) ||
        (!checkRange(field.value,2,0,255)) ||
        (!checkRange(field.value,3,0,255)) ||
        (!checkRange(field.value,4,0,255)) ){
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

function formCheck()
{
	if( document.addrouting.dest.value != "" && !checkIpAddr(document.addrouting.dest )){
		alert(_('routing destination has wrong format'));
		document.addrouting.dest.focus();
		return false;
	}
	if( document.addrouting.netmask.value != "" && !checkIpAddr(document.addrouting.netmask )){
		alert(_('routing netmask has wrong format'));
		document.addrouting.netmask.focus();
		return false;
	}
	if( document.addrouting.gateway.value != "" && !checkIpAddr(document.addrouting.gateway)){
		alert(_('routing gateway has wrong format'));
		document.addrouting.gateway.focus();
		return false;
	}

	if(	document.addrouting.dest.value == ""){
		alert(_('routing please input the destination'));
		document.addrouting.dest.focus();
		return false;
	}

    if( document.addrouting.hostnet.selectedIndex == 1 &&
		document.addrouting.netmask.value == ""){
		alert(_('routing please input the netmask'));
		document.addrouting.netmask.focus();
        return false;
    }

	/*if(document.addrouting.interface.value == "Custom" &&
		document.addrouting.custom_interface.value == ""){
		alert("please input custom interface name.");
		return false;
	}*/

	if (checkDuplicateRoute(document.addrouting.dest.value,document.addrouting.netmask.value,document.addrouting.gateway.value)==false)
	{
		alert(_('routing Incorrect Rule Alive'));
		document.addrouting.dest.focus();
		return false;
	} /* [Yian,2009/07/14] */
		
	if (!is_valid_text(document.addrouting.comment.value, false))
	{
		alert(_('routing Illegal characters are now allowable'));
		document.addrouting.comment.focus();
		return false;
	}
	
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
	var e;
	e = document.getElementById("routingTitle");
	e.innerHTML = _("routing title");
	e = document.getElementById("routingIntroduction");
	e.innerHTML = _("routing Introduction")+"<br>";
	e = document.getElementById("routingAddRule");
	e.innerHTML = _("routing add rule");
	e = document.getElementById("routingDest");
	e.innerHTML = _("routing routing dest");
	e = document.getElementById("routingRange");
	e.innerHTML = _("routing range");
	e = document.getElementById("routingNetmask");
	e.innerHTML = _("routing netmask");
	e = document.getElementById("routingGateway");
	e.innerHTML = _("routing gateway");
	e = document.getElementById("routingInterface");
	e.innerHTML = _("routing interface");
	/*e = document.getElementById("routingCustom");
	e.innerHTML = _("routing custom");*/
	e = document.getElementById("routingComment");
	e.innerHTML = _("routing comment");
	e = document.getElementById("routingSubmit");
	e.value = _("routing submit");
	e = document.getElementById("routingReset");
	e.value = _("routing reset");
	e = document.getElementById("routingCurrentRoutingTableRules");
	e.innerHTML = _("routing del title");
	e = document.getElementById("routingNo");
	e.innerHTML = _("routing Number");
	e = document.getElementById("routingDelDest");
	e.innerHTML = _("routing del dest");
	e = document.getElementById("routingDelNetmask");
	e.innerHTML = _("routing del netmask");
	e = document.getElementById("routingDelGateway");
	e.innerHTML = _("routing del gateway");
	e = document.getElementById("routingDelFlags");
	e.innerHTML = _("routing del flags");
	e = document.getElementById("routingDelMetric");
	e.innerHTML = _("routing del metric");
	e = document.getElementById("routingDelRef");
	e.innerHTML = _("routing del ref");
	e = document.getElementById("routingDelUse");
	e.innerHTML = _("routing del use");
	e = document.getElementById("routingDelInterface");
	e.innerHTML = _("routing del interface");
	e = document.getElementById("routingDelComment");
	e.innerHTML = _("routing del comment");
	e = document.getElementById("routingDel");
	e.value = _("routing del");
	e = document.getElementById("routingDelReset");
	e.value = _("routing del reset");
	e = document.getElementById("routing host");
	e.innerHTML = _("routing host");
	e = document.getElementById("routing net");
	e.innerHTML = _("routing net");
	e = document.getElementById("routing LAN");
	e.innerHTML = _("routing LAN");
	if(document.getElementById("routing WAN")){
		e = document.getElementById("routing WAN");
		e.innerHTML = _("routing WAN");
	}
	/*e = document.getElementById("dynamicRoutingTitle");
	e.innerHTML = _("routing dynamic Title");*/
	e = document.getElementById("dynamicRoutingTitle2");
	e.innerHTML = _("routing dynamic Title2");
	e = document.getElementById("RIPDisable");
	e.innerHTML = _("routing dynamic rip disable");
	/*e = document.getElementById("RIPEnable");
	e.innerHTML = _("routing dynamic rip enable");*/
	e = document.getElementById("dynamicRoutingApply");
	e.value = _("routing dynamic rip apply");
	e = document.getElementById("dynamicRoutingReset");
	e.value = _("routing dynamic rip reset");
}

function onInit()
{
	initTranslation();

	document.addrouting.hostnet.selectedIndex = 0;

	document.addrouting.netmask.readOnly = true;
	document.getElementById("routingNetmaskRow").style.visibility = "hidden";
	document.getElementById("routingNetmaskRow").style.display = "none";

	document.addrouting.interface.selectedIndex = 0;
	/*document.addrouting.custom_interface.value = "";
	document.addrouting.custom_interface.readOnly = true;*/

	document.dynamicRouting.RIPSelect.selectedIndex = <% getCfgZero(1, "RIPEnable"); %>;

	mydiv = document.getElementById("dynamicRoutingDiv");
	if(! <% getDynamicRoutingBuilt(); %>){
		mydiv.style.display = "none";
		mydiv.style.visibility = "hidden";
	}

}

function wrapDel(str, idle)
{
	if(idle == 1){
		document.write("<del>" + str + "</del>");
	}else
		document.write(str);
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

function hostnetChange()
{
	if(document.addrouting.hostnet.selectedIndex == 1){
		document.getElementById("routingNetmaskRow").style.visibility = "visible";
		document.getElementById("routingNetmaskRow").style.display = style_display_on();
		document.addrouting.netmask.readOnly = false;
		document.addrouting.netmask.focus();

	}else{
		document.addrouting.netmask.value = "";
		document.addrouting.netmask.readOnly = true;
		document.getElementById("routingNetmaskRow").style.visibility = "hidden";
		document.getElementById("routingNetmaskRow").style.display = "none";
	}
}

function interfaceChange()
{
	if(document.addrouting.interface.selectedIndex == 2){
		/*document.addrouting.custom_interface.readOnly = false;
		document.addrouting.custom_interface.focus();*/
	}else{
		/*document.addrouting.custom_interface.value = "";
		document.addrouting.custom_interface.readOnly = true;*/
	}
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

function onClicksbuttonMax()
{
	if (formCheck())
	{
		sbutton_disable(sbuttonMax);
		document.addrouting.submit();
		restartPage_block();
	}
}
</script><!--     body      --></head><body onload="onInit()" bgcolor="#FFFFFF">
<div align="center">
 <center>
<table class="body"><tbody><tr><td>

<table border="1" cellpadding="2" cellspacing="1" width="550">

<tr>
  <td class="title" colspan="2" id="routingTitle">Static Routing  Settings </td>
</tr>
<tr><td colspan="2">
<p class="head" id="routingIntroduction"> You may add or remote Internet routing rules here.</p>
</td>
</tr>

</table>

<br>

<form method="post" name="addrouting" action="/goform/addRouting">
<table border="1" cellpadding="2" cellspacing="1" width="550">
<tbody><tr>
  <td class="title" colspan="2" id="routingAddRule">Add a routing rule</td>
</tr>

<tr>
	<td class="head" id="routingDest">
		Destination
	</td>
	<td>
  		<input size="16" name="dest" type="text">
	</td>
</tr>

<tr>
	<td class="head" id="routingRange">
		Host/Net
	</td>
	<td>
		<select name="hostnet" onChange="hostnetChange()">
		<option select="" value="host" id="routing host">Host</option>
		<option value="net"  id="routing net">Net</option>
		</select>
	</td>
</tr>

<tr id="routingNetmaskRow">
	<td class="head" id="routingNetmask">
		Sub Netmask
	</td>
	<td>
  		<input size="16" name="netmask" type="text">
	</td>
</tr>

<tr>
	<td class="head" id="routingGateway">
		Gateway
	</td>
	<td>
  		<input size="16" name="gateway" type="text">
	</td>
</tr>

<tr>
	<td class="head" id="routingInterface">
		Interface
	</td>
	<td>
		<select name="interface" onChange="interfaceChange()">
		<option select="" value="LAN" id="routing LAN">LAN</option>

		<script language="JavaScript" type="text/javascript">
			if(opmode == "1")
				document.write("<option value=\"WAN\" id=\"routing WAN\">WAN</option>");
		</script>

		<!--<option value="Custom" id="routingCustom">Custom</option>-->
		</select>
		<!--<input alias="right" size="16" name="custom_interface" type="text">-->
	</td>
</tr>

<tr>
	<td class="head" id="routingComment">
		Comment
	</td>
	<td>
		<input name="comment" size="16" maxlength="32" type="text">
	</td>
</tr>
</tbody></table>

<table border="0" width="550">
<tr id="sbutton0"><td>
<p align="center">
	<input value="Apply" id="routingSubmit" name="addFilterPort" onclick="onClicksbuttonMax();" type="button"> &nbsp;&nbsp;
	<input value="Reset" id="routingReset" name="reset" type="reset">
</p>
</td></tr>
</table>

</form>

<br>

<!--  delete rules -->
<form action="/goform/delRouting" method="post" name="delRouting">

<table border="1" cellpadding="2" cellspacing="1" width="550">	
	<tbody><tr>
		<td class="title" colspan="10" id="routingCurrentRoutingTableRules">Current Routing table in the system: </td>
	</tr>

	<tr>
		<td class="head" id="routingNo"> No.</td>
		<td class="head" id="routingDelDest" align="center"> Destination </td>
		<td class="head" id="routingDelNetmask" align="center"> Netmask</td>
		<td class="head" id="routingDelGateway" align="center"> Gateway</td>
		<td class="head" id="routingDelFlags" align="center"> Flags</td>
		<td class="head" id="routingDelMetric" align="center"> Metric</td>
		<td class="head" id="routingDelRef" align="center"> Ref</td>
		<td class="head" id="routingDelUse" align="center"> Use</td>
		<td class="head" id="routingDelInterface" align="center"> Interface</td>
		<td class="head" id="routingDelComment" align="center"> Comment</td>
	</tr>

	<script language="JavaScript" type="text/javascript">
	var i;
	var entries = new Array();
	var all_str = <% getRoutingTable(); %>;

	entries = all_str.split(";");
	for(i=0; i<entries.length; i++){
		var one_entry = entries[i].split(",");


		true_interface[i] = one_entry[0];
		destination[i] = one_entry[1];
		gateway[i] = one_entry[2];
		netmask[i] = one_entry[3];
		flags[i] = one_entry[4];
		ref[i] = one_entry[5];
		use[i] = one_entry[6];
		metric[i] = one_entry[7];
		category[i] = parseInt(one_entry[8]);
		interface[i] = one_entry[9];
		idle[i] = parseInt(one_entry[10]);
		comment[i] = one_entry[11];
		if(comment[i] == " " || comment[i] == "")
			comment[i] = "&nbsp";
	}

	for(i=0; i<entries.length; i++){
		if(category[i] > -1){
			document.write("<tr bgcolor=#DBE2EC>");
			document.write("<td>");
			document.write(i+1);
			document.write("<input type=checkbox name=DR"+ category[i] + 
				" value=\""+ destination[i] + " " + netmask[i] + " " + true_interface[i] +"\">");
			document.write("</td>");
		}else{
			document.write("<tr>");
			document.write("<td>"); 	document.write(i+1);			 	document.write("</td>");
		}

		document.write("<td>"); 	wrapDel(destination[i], idle[i]); 	document.write("</td>");
		document.write("<td>"); 	wrapDel(netmask[i], idle[i]);		document.write("</td>");
		document.write("<td>"); 	wrapDel(gateway[i], idle[i]); 		document.write("</td>");
		document.write("<td>"); 	wrapDel(flags[i], idle[i]);			document.write("</td>");
		document.write("<td>"); 	wrapDel(metric[i], idle[i]);		document.write("</td>");
		document.write("<td>"); 	wrapDel(ref[i], idle[i]);			document.write("</td>");
		document.write("<td>"); 	wrapDel(use[i], idle[i]);			document.write("</td>");

		if(interface[i] == "LAN")
			interface[i] = _("routing LAN");
		else if(interface[i] == "WAN")
			interface[i] = _("routing WAN");
		else if(interface[i] == "Custom")
			interface[i] = _("routing custom");

		document.write("<td>"); 	wrapDel(interface[i] + "(" +true_interface[i] + ")", idle[i]);		document.write("</td>");
		document.write("<td>"); 	wrapDel(comment[i], idle[i]);		document.write("</td>");
		document.write("</tr>\n");
	}
	  </script>

</tbody></table>
<br>
<table border="0" width="550">	
<tr id="sbutton1"><td width="550">
<p align="center">
<input value="Delete Selected" id="routingDel" name="deleteSelPortForward" onclick="sbutton_disable(sbuttonMax); restartPage_block(); return deleteClick();" type="submit">&nbsp;&nbsp;
<input value="Reset" id="routingDelReset" name="reset" type="reset">
</td></tr>
</table>

</form>
<br>

<div id=dynamicRoutingDiv>
<!-- <h1 id="dynamicRoutingTitle">Dynamic Routing Settings </h1> -->
<form method=post name="dynamicRouting" action="/goform/dynamicRouting">
<table width="550" border="1" cellpadding="2" cellspacing="1">
<tr>
	<td class="title" colspan="2" id="dynamicRoutingTitle2" width="540">Dynamic routing</td>
</tr>
<tr>
	<td class="head" id="RIP" width="156">
		RIP
	</td>

	<td width="377">
	<select name="RIPSelect" size="1">
	<option value=0 id="RIPDisable">Disable</option>
	<option value=1 id="RIPV1Enable">v1</option>
	<option value=2 id="RIPV1Enable">v2</option>
	</select>
	</td>
</tr>
</table>

<table width="550" border="0">
<tr id="sbutton2"><td width="550">
<p align="center">
	<input type="submit" value="Apply" id="dynamicRoutingApply" name="dynamicRoutingApply" onclick="sbutton_disable(sbuttonMax); restartPage_block();"> &nbsp;&nbsp;
	<input type="reset" value="Reset" id="dynamicRoutingReset" name="dynamicRoutingReset" onClick="window.location.reload()">
</p>
</td></tr>
</table>

</form>
</div>


</td></tr></tbody></table>
 </center>
</div>
</body></html>