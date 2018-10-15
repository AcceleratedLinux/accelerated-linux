<html>
<head>
<title>MAC Filtering Settings</title>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script type="text/javascript" src="/common.js"></script>

<script language="JavaScript" type="text/javascript">

restartPage_init();

var sbuttonMax=1;

Butterlate.setTextDomain("firewall");

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


/*function checkIPAddr(field)
{
	var ans;
		ans = is_ip(field.value);
		if (ans == false)
		{
			field.focus();
			field.select();
			return false;
		}
   return true;
}*/

function isNumOnly(str)
{
	for (var i=0; i<str.length; i++){
	    if((str.charAt(i) >= '0' && str.charAt(i) <= '9') )
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

function checkIPAddr(field)
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

function formCheck()
{
	var alanip = "<% getCfgZero(1, "lan_ipaddr"); %>";
	var ip_a=new Array();
	var ip_b=new Array();


	if(!document.DMZ.DMZEnabled.options.selectedIndex){
		// user choose disable
		return true;
	}

	if(document.DMZ.DMZIPAddress.value == ""){
		alert(_("dmz alert not set a ip address"));
		document.DMZ.DMZIPAddress.focus();
		return false;
	}

	if(! checkIPAddr(document.DMZ.DMZIPAddress) ){
		alert(_("dmz alert ip address format error"));
		document.DMZ.DMZIPAddress.focus();
		return false;
	}
	
	if (checkIPAddr_sub(document.DMZ.DMZIPAddress.value)==false)
	{
		alert(_("dmz alert ip address format error"));
		document.DMZ.DMZIPAddress.focus();
		return false;
	}

	if (alanip == document.DMZ.DMZIPAddress.value){
		alert(_("dmz alert incorrect! ip address is lan ip"));
		document.DMZ.DMZIPAddress.focus();
		return false;
	}

	/*ip_a=document.DMZ.DMZIPAddress.value;
	ip_b=new Array(atoi(ip_a,1).atoi(ip_a,2).atoi(ip_a,3).atoi(ip_a,4));
	document.DMZ.DMZIPAddress.value=ip_b;*/
	
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
	var e = document.getElementById("dmzTitle");
	e.innerHTML = _("dmz title");
	e = document.getElementById("dmzIntroduction");
	e.innerHTML = _("dmz introduction");

	e = document.getElementById("dmzSetting");
	e.innerHTML = _("dmz setting");
	e = document.getElementById("dmzSet");
	e.innerHTML = _("dmz setting");
	e = document.getElementById("dmzDisable");
	e.innerHTML = _("firewall disable");
	e = document.getElementById("dmzEnable");
	e.innerHTML = _("firewall enable");
	e = document.getElementById("dmzIPAddr");
	e.innerHTML = _("dmz ipaddr");
	e = document.getElementById("dmzApply");
	e.value = _("firewall apply");
	e = document.getElementById("dmzReset");
	e.value = _("firewall reset");
}

function updateState()
{
	initTranslation();
	if(document.DMZ.DMZEnabled.options.selectedIndex == 1){
		enableTextField(document.DMZ.DMZIPAddress);
	}else{
		disableTextField(document.DMZ.DMZIPAddress);
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
  <td class="title" colspan="2" id="dmzTitle">DMZ Settings</td>
  <% checkIfUnderBridgeModeASP(); %>
</tr>
<tr>
<td colspan="2">
<p class="head" id="dmzIntroduction"> You may setup a De-militarized Zone(DMZ) to separate internal network and Internet.</p>
</td>
</tr>

</table>

<br>

<form method=post name="DMZ" action=/goform/DMZ>
<table width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="dmzSetting">DMZ Settings</td>
</tr>
<tr>
	<td class="head" id="dmzSet">
		DMZ Settings
	</td>
	<td>
	<select onChange="updateState()" name="DMZEnabled" size="1">
	<option value=0 <% getDMZEnableASP(0); %> id="dmzDisable">Disable</option>
    <option value=1 <% getDMZEnableASP(1); %> id="dmzEnable">Enable</option>
    </select>
    </td>
</tr>

<tr>
	<td class="head" id="dmzIPAddr">
		DMZ IP Address
	</td>
	<td>
  		<input type="text" size="24" name="DMZIPAddress" value=<% showDMZIPAddressASP(); %> >
	</td>
</tr>
</table>
<br>

<table width="540" border="0" cellpadding="2" cellspacing="1">
<tr id="sbutton0" align="center">
	<td>
		<input type="submit" value="Apply" id="dmzApply" name="addDMZ" onClick="return formCheck();"> &nbsp;&nbsp;
		<input type="reset" value="Reset" id="dmzReset" name="reset">
	</td>
</tr>
</table>
</form>



</td></tr></table>
 </center>
</div>
</body>
</html>
