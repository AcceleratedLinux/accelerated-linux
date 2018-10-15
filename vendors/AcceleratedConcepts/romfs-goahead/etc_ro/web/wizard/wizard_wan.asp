<html>
<head>
    <title>Wide Area Network (WAN) Settings     
    </title>    
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
	<meta http-equiv="CACHE-CONTROL" content="NO-CACHE">
	
<script type="text/javascript" src="/lang/b28n.js"></script>
<script type="text/javascript" src="/common.js"></script>
<script language="JavaScript" type="text/javascript">

var http_request = false;
var sbuttonMax=1;
var checked =  '<% getWan_3gpincode_protect1(); %>';
var clone = '<% getCfgZero(1, "macCloneEnabled"); %>';

restartPage_init();
Butterlate.setTextDomain("internet");
/*
var sta = 10;
setInterval("getUSBVendorIDbyForm();", 5000);
setInterval("show_usb_sta();", 1000);

function show_usb_sta(){
	var e1 = document.getElementById("Usb_sta");
	e1.color="red";
	
	if(sta == 10){
		e1.innerHTML = "No USB Devices.";
		sta = 11;
	}else if(sta == 11){
		e1.innerHTML = "No USB Devices..";
		sta = 12;
	}else if(sta == 12){
		e1.innerHTML = "No USB Devices...";
		sta = 13;
	}else if(sta == 13){
		e1.innerHTML = "No USB Devices....";
		sta = 14;
	}else if(sta == 14){
		e1.innerHTML = "No USB Devices.....";
		sta = 10;
	}
}

function getUSBVendorIDbyForm()
{
    var http_req = false;
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_req = new XMLHttpRequest();
        if (http_req.overrideMimeType) {
            http_req.overrideMimeType('text/xml');
        }
    }else if (window.ActiveXObject) { // IE
        try {
            http_req = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_req = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_req) {
        return false;
    }
	http_request.onreadystatechange = getUSBVendorID;
    http_req.open('POST', "/goform/getUSBVendorID", true);
    http_req.send('null');
}
function getUSBVendorID(){
	var e1 = document.getElementById("Usb_sta");

	if (http_req.readyState == 4) {
        if (http_req.status == 200) {
			e1.innerHTML = http_req.responseText;
			e1.color="blue";
			sta = 100;
        }else {
            sta = 10;
        }
    }
}
*/
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
        alert(_("wan not create XMLHTTP instance"));
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
			alert(_("wan not get mac"));
		}
	}
}
function macCloneSwitch()
{
		if (document.wanCfg.wan_macClone_enable.checked)
	{
		document.getElementById("wan_macClone_enable").value = 1;
		document.wanCfg.wan_macClone_enable.checked = true;
		document.getElementById("wMacCloneAddrDis").style.visibility = "visible";
		document.getElementById("wMacCloneAddrDis").style.display = style_display_on();
	}
	else
	{
		document.getElementById("wan_macClone_enable").value = 0;
		document.wanCfg.wan_macClone_enable.checked = false;
		document.getElementById("wMacCloneAddrDis").style.visibility = "hidden";
		document.getElementById("wMacCloneAddrDis").style.display = "none";
	}
}
function connectionTypeSwitch()
{
	if(document.wanCfg.connectionType.value != "99"){
		document.wanCfg.wan_macClone_enable.disabled = false;
		document.wanCfg.wan_macClone_enable.value = 1;
		
		if('<% getCfgZero(1, "macCloneEnabled"); %>' == 1){
			document.wanCfg.wan_macClone_enable.checked = true;
			document.getElementById("wMacCloneAddrDis").style.visibility = "visible";
			document.getElementById("wMacCloneAddrDis").style.display = style_display_on();
		}else{
			document.wanCfg.wan_macClone_enable.checked = false;
			document.getElementById("wMacCloneAddrDis").style.visibility = "hidden";
			document.getElementById("wMacCloneAddrDis").style.display = "none";
		}
		
	}else{
		document.wanCfg.wan_macClone_enable.disabled = true;
		document.wanCfg.wan_macClone_enable.value = 0;
		document.wanCfg.wan_macClone_enable.checked = false;
		document.getElementById("wMacCloneAddrDis").style.visibility = "hidden";
		document.getElementById("wMacCloneAddrDis").style.display = "none";
	}
	
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
	document.getElementById("3G").style.visibility = "hidden";
	document.getElementById("3G").style.display = "none";
	document.getElementById("dual_wan_mode").style.visibility = "hidden";
	document.getElementById("dual_wan_mode").style.display = "none";
/*Added dns setting (custom) [2009/08/15] - Start*/			
	document.getElementById("dns_customer").style.visibility = "hidden";
	document.getElementById("dns_customer").style.display = "none";
/*Added dns setting (custom) [2009/08/15] - End*/			
	if (document.wanCfg.connectionType.options.selectedIndex == 0) {
		document.getElementById("static").style.visibility = "visible";
		document.getElementById("static").style.display = "block";
	}
	else if (document.wanCfg.connectionType.options.selectedIndex == 1) {
		document.getElementById("dhcp").style.visibility = "visible";
		document.getElementById("dhcp").style.display = "block";
/*Added dns setting (custom) [2009/08/15] - Start*/	
		document.getElementById("dns_customer").style.visibility = "visible";
		document.getElementById("dns_customer").style.display = "block";
/*Added dns setting (custom) [2009/08/15] - End*/		
	}
	else if (document.wanCfg.connectionType.options.selectedIndex == 2) {
		document.getElementById("pppoe").style.visibility = "visible";
		document.getElementById("pppoe").style.display = "block";
		pppoeOPModeSwitch();
	}
	else if (document.wanCfg.connectionType.options.selectedIndex == 3) {
		document.getElementById("pptp").style.visibility = "visible";
		document.getElementById("pptp").style.display = "block";
/*Added dns setting (custom) [2009/08/15] - Start*/			
		document.getElementById("dns_customer").style.visibility = "visible";
		document.getElementById("dns_customer").style.display = "block";
/*Added dns setting (custom) [2009/08/15] - End*/			
		pptpOPModeSwitch();
	}
	else if (document.wanCfg.connectionType.options.selectedIndex == 4) {
		document.getElementById("l2tp").style.visibility = "visible";
		document.getElementById("l2tp").style.display = "block";
/*Added dns setting (custom) [2009/08/15] - Start*/			
		document.getElementById("dns_customer").style.visibility = "visible";
		document.getElementById("dns_customer").style.display = "block";
/*Added dns setting (custom) [2009/08/15] - End*/	
		l2tpOPModeSwitch();
	}
/*Marked by Maddux [2009/09/17]
	else if (document.wanCfg.connectionType.options.selectedIndex == 5) {
		document.getElementById("3G").style.visibility = "visible";
		document.getElementById("3G").style.display = "block";
	}
	else {
		document.getElementById("static").style.visibility = "visible";
		document.getElementById("static").style.display = "block";
	}
*/
/*Added by Maddux [2009/09/17] - Start*/
/*
	if (document.wanCfg.connectionType1.options.selectedIndex != 0 && document.wanCfg.connectionType.options.selectedIndex != 5) 
	{
		document.getElementById("dual_wan_mode").style.visibility = "visible";
		document.getElementById("dual_wan_mode").style.display = "block";
	}
*/
                                                                   
	if(document.wanCfg.connectionType1.options.selectedIndex == 0 )
	{
		document.getElementById("3G").style.visibility = "visible";
		document.getElementById("3G").style.display = "block";
		
		//if("<% getCfgGeneral(1, "USB_Port_Status"); %>" != "USB_Device_Inserted")
			//setTimeout("alert(\"Please keep USB 3G Card.\")" ,5000);
//		  alert("Please keep USB 3G Card.");
}

	if(document.wanCfg.connectionType.value != "99" && document.wanCfg.connectionType1.value != "99") // dual wan
	{
		document.getElementById("dual_wan_mode").style.visibility = "visible";
		document.getElementById("dual_wan_mode").style.display = "block";
		id_visible("wan2_option_radios");
		id_invisible("3g_conn_type");

		document.wanCfg.wan_3g_connecttype.value="0"; // Keep Alive
	}else // single wan or no wan
	{
		id_invisible("wan2_option_radios");
		//id_visible("3g_conn_type");
		id_invisible("3g_conn_type");
	}
/*Added by Maddux [2009/09/17] - End*/
	wan_3g_connecttype_change(document.wanCfg);
	wan_3g_policy_alert_change(document.wanCfg);
	wan_3g_budget_control_enable_change(document.wanCfg);
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
		
		/*document.getElementById("dns_customer").style.visibility = "hidden";
		document.getElementById("dns_customer").style.display = "none";*/
	}
	else {
		document.getElementById("l2tpIp").style.visibility = "hidden";
		document.getElementById("l2tpIp").style.display = "none";
		document.getElementById("l2tpNetmask").style.visibility = "hidden";
		document.getElementById("l2tpNetmask").style.display = "none";
		document.getElementById("l2tpGateway").style.visibility = "hidden";
		document.getElementById("l2tpGateway").style.display = "none";
		
		/*document.getElementById("dns_customer").style.visibility = "visible";
		document.getElementById("dns_customer").style.display = "block";*/
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
	
		/*document.getElementById("dns_customer").style.visibility = "hidden";
		document.getElementById("dns_customer").style.display = "none";*/
	}
	else {
		document.getElementById("pptpIp").style.visibility = "hidden";
		document.getElementById("pptpIp").style.display = "none";
		document.getElementById("pptpNetmask").style.visibility = "hidden";
		document.getElementById("pptpNetmask").style.display = "none";
		document.getElementById("pptpGateway").style.visibility = "hidden";
		document.getElementById("pptpGateway").style.display = "none";
		
		/*document.getElementById("dns_customer").style.visibility = "visible";
		document.getElementById("dns_customer").style.display = "block";*/
	}
}
function pppoeOPModeSwitch()
{
	document.wanCfg.pppoeRedialPeriod.disabled = true;
	document.wanCfg.pppoeIdleTime.disabled = true;
	if (document.wanCfg.pppoeOPMode.options.selectedIndex == 0) 
		document.wanCfg.pppoeRedialPeriod.disabled = false;
	else if (document.wanCfg.pppoeOPMode.options.selectedIndex == 1)
		document.wanCfg.pppoeIdleTime.disabled = false;
}
function l2tpOPModeSwitch()
{
	document.wanCfg.l2tpMode.options.selectedIndex = '<% getCfgZero(1, "wan_l2tp_mode"); %>'; 
	l2tpModeSwitch();
	
	document.wanCfg.l2tpRedialPeriod.disabled = true;
	// document.wanCfg.l2tpIdleTime.disabled = true;
	if (document.wanCfg.l2tpOPMode.options.selectedIndex == 0) 
		document.wanCfg.l2tpRedialPeriod.disabled = false;
	/*
	else if (document.wanCfg.l2tpOPMode.options.selectedIndex == 1)
		document.wanCfg.l2tpIdleTime.disabled = false;
	*/
}
function pptpOPModeSwitch()
{
	document.wanCfg.pptpMode.options.selectedIndex = '<% getCfgZero(1, "wan_pptp_mode"); %>';
	pptpModeSwitch();
	
	document.wanCfg.pptpRedialPeriod.disabled = true;
	// document.wanCfg.pptpIdleTime.disabled = true;
	if (document.wanCfg.pptpOPMode.options.selectedIndex == 0) 
		document.wanCfg.pptpRedialPeriod.disabled = false;
	/*
	else if (document.wanCfg.pptpOPMode.options.selectedIndex == 1)
		document.wanCfg.pptpIdleTime.disabled = false;
	*/
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
function checkDigit(str)
{
        for(var i=0; i<str.length; i++)
        {
                if(str.charAt(i) >= '0' && str.charAt(i) <= '9')
                        continue;
                return 0;
        }
        return 1;
}
function checkIllegalChar(str)
{
  var flag = 0;
        for(var i=0; i<str.length; i++)
        {
                if((i > 0) && ( i < (str.length - 1) )) {
                        if( (str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) >= 'a' && str.charAt(i) <= 'z') ||
                                (str.charAt(i) >= 'A' && str.charAt(i) <= 'Z') || str.charAt(i) == '@' || str.charAt(i) == '.' ||
                                str.charAt(i) == '_' || str.charAt(i) == '-' ){
          if(   str.charAt(i) == '@')
                                    flag += 1;
          if( flag > 1)
            return 0;
          if( (str.charAt(i) == '@' || str.charAt(i) == '.' ||  str.charAt(i) == '_' || str.charAt(i) == '-') &&
              (str.charAt(i+1) == '@' || str.charAt(i+1) == '.' ||      str.charAt(i+1) == '_' || str.charAt(i+1) == '-' ) ){
              return 0;
          }
                            continue;
      }
                        return 0;
                }else{
                        if( (str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) >= 'a' && str.charAt(i) <= 'z') ||
                                (str.charAt(i) >= 'A' && str.charAt(i) <= 'Z') )
                                continue;
                        return 0;
                }
        }
        return 1;
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
function is_email(str)
{
    var validRegExp = /^\S+@\S+\.\S+$/ ;
    var data = str.match(validRegExp);    
    if (!data) 
        return false;
	if (str.indexOf("..") > 0)
		return false;
    return true;
}  
function checkServerIpAddr(field, ismask)
{
  var type_is_dn = 0;
  var str = field.value;
  for(var i=0; i<str.length; i++) {
    if ( (str.charAt(i) >= 'a' && str.charAt(i) <= 'z') || (str.charAt(i) >= 'A' && str.charAt(i) <= 'Z') )
      type_is_dn = 1;
  }
  if(type_is_dn == 0) //server type is IP
    return checkIpAddr(field, false);
  else{ //server type is domain name
    for(i=0 ; i<str.length; i++) {
      if( ((i==0) || (i==str.length - 1)) && str.charAt(i) == '.' ){
        alert( _('lan ip incorrect format') );
        return false;
      }
      if( str.charAt(i) == '.' && str.charAt(i+1) == '.' ){
        alert( _('lan ip incorrect format') );
        return false;
      }
    }
    return true;
  }
}
function checkIpAddr(field, ismask)
{
	var cols = field.value.split('.');
	if (cols.length != 4) {
		alert(_("wan ip invalid"));
		field.focus();
		return false;
	}
	if (field.value == "") {
		alert(_("wan ip empty"));
		field.value = field.defaultValue;
		field.focus();
		return false;
	}
	if (isAllNum(field.value) == 0) {
		alert(_("lan ip incorrect value"));
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
			alert(_("lan ip incorrect format"));
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
			alert(_("lan ip incorrect format"));
			field.value = field.defaultValue;
			field.focus();
			return false;
		}
	}
	return true;
}

function is_valid_hostname(hostname)
{
	var num = "0123456789";
	var ascii = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	var extra = "1234567890-_."; /* Added "-" , [Yian,2009/07/10] */
    var i;
    var valid_string = num + ascii + extra;
    if (hostname.length > 64 || hostname.length < 0)
    {
        alert(_("host name incorrect"));
        return false;
    }
    for (i = 0; i < hostname.length; i++)
    {
        /*if (!is_contained_char(hostname[i], valid_string))*/
        if (!is_contained_char(hostname.substr(i, 1), valid_string))        
        {
            alert(_("host name format illegal"));
            return false;
        }
    }
    return true;
}
function CheckValue()
{
	f1=document.wanCfg.connectionType;
	f2=document.wanCfg.connectionType1;
	if (f1.value == "99" && f2.value == "99") {  //no wan, return
		alert( _("wan Dual Wan error no wan selected") );
		return false;
	}
	
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
	}
	else if (document.wanCfg.connectionType.selectedIndex == 1) { //DHCP
		if (document.wanCfg.hostname.value == "")
		{
				alert(_("host name not specify"));
				document.wanCfg.hostname.focus();
				return false;
		}
		if (!is_valid_hostname(document.wanCfg.hostname.value))
		{
			document.wanCfg.hostname.focus();
			return false;
		}
	}
	else if (document.wanCfg.connectionType.selectedIndex == 2) { //PPPOE
		if (document.wanCfg.pppoeUser.value =="" ) {
			alert(_("wan pppoe not specify ussername"));
			return false;
		}
		/*if (!is_email(document.wanCfg.pppoeUser.value)) {
			alert("PPPoE user name are supposed to be a email address.");
			document.wanCfg.pppoeUser.focus();
			return false;
		}*/
		if ( (document.wanCfg.pppoePass.value == "")||(document.wanCfg.pppoePass2.value == "") ) {
			alert(_("wan pppoe not specify password"));
			document.wanCfg.pppoePass.focus();
			return false;
		}
		if (document.wanCfg.pppoePass.value != document.wanCfg.pppoePass2.value) {
			alert(_("wan pppoe password mismatched"));
			document.wanCfg.pppoePass.focus();
			return false;
		}
		if (document.wanCfg.pppoeOPMode.options.selectedIndex == 0)
		{
			if (document.wanCfg.pppoeRedialPeriod.value == "")
			{
				alert(_("wan pppoe redial period error"));
				document.wanCfg.pppoeRedialPeriod.focus();
				document.wanCfg.pppoeRedialPeriod.select();
				return false;
			}
			/*if (isNaN(document.wanCfg.pppoeRedialPeriod.value) ||
				document.wanCfg.pppoeRedialPeriod.value.indexOf('.') != -1)*/
			if ( (isNaN(document.wanCfg.pppoeRedialPeriod.value)) || 
				(document.wanCfg.pppoeRedialPeriod.value.indexOf('-') != -1) ||
				(document.wanCfg.pppoeRedialPeriod.value.indexOf('.') != -1) )
			{
				alert(_("wan pppoe not a valid integer"));
				document.wanCfg.pppoeRedialPeriod.focus();
				return false;
			}
			if ( atoi(document.wanCfg.pppoeRedialPeriod.value,1) == 0 )
			{
				alert(_("wan pppoe not a valid integer zero"));
				document.wanCfg.pppoeRedialPeriod.focus();
				return false;
			}
		}
		else if (document.wanCfg.pppoeOPMode.options.selectedIndex == 1);
		{
			if (document.wanCfg.pppoeIdleTime.value == "")
			{
				alert(_("wan pppoe not specify idle time"));
				document.wanCfg.pppoeIdleTime.focus();
				document.wanCfg.pppoeIdleTime.select();
				return false;
			}
			/*if (isNaN(document.wanCfg.pppoeIdleTime.value) ||
				document.wanCfg.pppoeIdleTime.value.indexOf('.') != -1)*/
			if ( (isNaN(document.wanCfg.pppoeIdleTime.value)) ||
				(document.wanCfg.pppoeIdleTime.value.indexOf('-') != -1) ||
				(document.wanCfg.pppoeIdleTime.value.indexOf('.') != -1) )
			{
				alert(_("wan pppoe not a valid idle time integer"));
				document.wanCfg.pppoeIdleTime.focus();
				return false;
			}
			if ( atoi(document.wanCfg.pppoeIdleTime.value,1) == 0 )
			{
				alert(_("wan pppoe not a valid idle time integer zero"));
				document.wanCfg.pppoeIdleTime.focus();
				return false;
			}
		}
	}
	else if (document.wanCfg.connectionType.selectedIndex == 3) { //PPTP
		if (document.wanCfg.pptpUser.value == "") {
			alert(_("wan pptp not specify ussername"));
			return false;
		}
		if (document.wanCfg.pptpPass.value == "") {
			alert(_("wan pptp not specify password"));
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
				alert(_("wan pptp redial period error"));
				document.wanCfg.pptpRedialPeriod.focus();
				document.wanCfg.pptpRedialPeriod.select();
				return false;
			}
			
			if ( (isNaN(document.wanCfg.pptpRedialPeriod.value)) || 
				(document.wanCfg.pptpRedialPeriod.value.indexOf('-') != -1) ||
				(document.wanCfg.pptpRedialPeriod.value.indexOf('.') != -1) )
			{
				alert(_("wan pptp not a valid integer"));
				document.wanCfg.pptpRedialPeriod.focus();
				return false;
			}
			if ( atoi(document.wanCfg.pptpRedialPeriod.value,1) == 0 )
			{
				alert(_("wan pptp not a valid integer zero"));
				document.wanCfg.pptpRedialPeriod.focus();
				return false;
			}
		}
		
		else if(document.wanCfg.pptpOPMode.options.selectedIndex == 1)
		{
		/*	if (document.wanCfg.pptpIdleTime.value == "")
			{
				alert(_("wan pptp not specify idle time"));
				document.wanCfg.pptpIdleTime.focus();
				document.wanCfg.pptpIdleTime.select();
				return false;
			}*/
		}
	}
	else if (document.wanCfg.connectionType.selectedIndex == 4) { //L2TP
		if (document.wanCfg.l2tpUser.value == "") {
			alert(_("wan l2tp not specify ussername"));
			return false;
		}
		if (document.wanCfg.l2tpPass.value == "") {
			alert(_("wan l2tp not specify password"));
			return false;
		}
		if (!checkIpAddr(document.wanCfg.l2tpServer, false))
			return false;
		if (document.wanCfg.l2tpMode.selectedIndex == 0) {
			if (!checkIpAddr(document.wanCfg.l2tpIp, false))
				return false;
			if (!checkIpAddr(document.wanCfg.l2tpNetmask, true))
				return false;
			if (!checkIpAddr(document.wanCfg.l2tpGateway, false))
				return false;
		}
		if (document.wanCfg.l2tpOPMode.options.selectedIndex == 0)
		{
			if (document.wanCfg.l2tpRedialPeriod.value == "")
			{
				alert(_("wan l2tp redial period error"));
				document.wanCfg.l2tpRedialPeriod.focus();
				document.wanCfg.l2tpRedialPeriod.select();
				return false;
			}
		}
		/*
		else if (document.wanCfg.l2tpOPMode.options.selectedIndex == 1)
		{
			if (document.wanCfg.l2tpIdleTime.value == "")
			{
				alert(_("wan l2tp not specify idle time"));
				document.wanCfg.l2tpIdleTime.focus();
				document.wanCfg.l2tpIdleTime.select();
				return false;
			}
		}
		*/
	}
	
	if (document.wanCfg.connectionType.selectedIndex == 5 || document.wanCfg.connectionType1.selectedIndex == 0) //3G
	{
		/*if(document.wanCfg.wan_3g_pincode_protect.checked)
		{
			if( document.wanCfg.wan_3g_pincode.value == "" || document.wanCfg.wan_3g_pincode.value.length<4
				|| document.wanCfg.wan_3g_pincode.value.length>8)
			{
			        alert( _('wan 3g pincode invalid') );
			        document.wanCfg.wan_3g_pincode.focus();
			        document.wanCfg.wan_3g_pincode.select();
			        return false;
			}
			if(!checkDigit(document.wanCfg.wan_3g_pincode.value))
			{
			        alert( _('wan 3g pincode digits') );
			        document.wanCfg.wan_3g_pincode.focus();
			        document.wanCfg.wan_3g_pincode.select();
			        return false;
			}
		}
		
		if( document.wanCfg.wan_3g_connecttype.value=="1" && !checkDigit(document.wanCfg.wan_3g_maxidletime.value))
		{
		        alert( _('wan 3g maxidletime digits') );
		        document.wanCfg.wan_3g_maxidletime.focus();
		        document.wanCfg.wan_3g_maxidletime.select();
		        return false;
		}*/
		
		if(document.wanCfg.wan_3g_time_budget_enable.checked) // check time budget limit field
		{
		        if(!checkDigit(document.wanCfg.wan_3g_max_online_monthly.value))
		        {
		                alert( _('wan 3g max online monthly digits') );
		                document.wanCfg.wan_3g_max_online_monthly.focus();
		                document.wanCfg.wan_3g_max_online_monthly.select();
		                return false;
		        }
				/* to check user wan_3g_max_online_monthly range:1~999, minglin, 2009/12/28 *start*/
				if( (document.wanCfg.wan_3g_max_online_monthly.value*1 < 1 ) || (document.wanCfg.wan_3g_max_online_monthly.value*1 > 999 ))
				{
					alert( _('wan 3g max online monthly invalid') );
					document.wanCfg.wan_3g_max_online_monthly.focus();
					document.wanCfg.wan_3g_max_online_monthly.select();
					return false;
				}
				/* to check user wan_3g_max_online_monthly range:1~999, minglin, 2009/12/28 *end*/
		        if(!checkDigit(document.wanCfg.wan_3g_time_limit.value))
		        {
		                alert( _('wan 3g max time limit digits') );
		                document.wanCfg.wan_3g_time_limit.focus();
		                document.wanCfg.wan_3g_time_limit.select();
		                return false;
		        }
		        if( (document.wanCfg.wan_3g_time_limit.value*1 <= 0 ) || (document.wanCfg.wan_3g_time_limit.value*1 >= 100 ))
		        {
		                alert( _('wan 3g max time limit invalid') );
		                document.wanCfg.wan_3g_time_limit.focus();
		                document.wanCfg.wan_3g_time_limit.select();
		                return false;
		        }
		}
		
		if(document.wanCfg.wan_3g_data_budget_enable.checked) // check data budget limit field
		{
		        if(!checkDigit(document.wanCfg.wan_3g_max_xfer_monthly.value))
		        {
		                alert( _('wan 3g max xfer monthly digits') );
		                document.wanCfg.wan_3g_max_xfer_monthly.focus();
		                document.wanCfg.wan_3g_max_xfer_monthly.select();
		                return false;
		        }
			// 20090915 modified by Carl to check user wan_3g_max_xfer_monthly range: 3~4000
			if( (document.wanCfg.wan_3g_max_xfer_monthly.value*1 < 3 ) || (document.wanCfg.wan_3g_max_xfer_monthly.value*1 > 4000 ))
			{
				alert( _('wan 3g max xfer monthly invalid') );
				document.wanCfg.wan_3g_max_xfer_monthly.focus();
				document.wanCfg.wan_3g_max_xfer_monthly.select();
				return false;
			}
			//+20090915 modified by Carl to check user wan_3g_max_xfer_monthly range: 3~4000
		        if(!checkDigit(document.wanCfg.wan_3g_data_limit.value))
		        {
		                alert( _('wan 3g max data limit digits') );
		                document.wanCfg.wan_3g_data_limit.focus();
		                document.wanCfg.wan_3g_data_limit.select();
		        return false;
		        }
		        if( (document.wanCfg.wan_3g_data_limit.value*1 <= 0 ) || (document.wanCfg.wan_3g_data_limit.value*1 >= 100 ))
		        {
		                alert( _('wan 3g max data limit invalid') );
		                document.wanCfg.wan_3g_data_limit.focus();
		                document.wanCfg.wan_3g_data_limit.select();
		                return false;
		        }
		}
		/*fix bug and add alert field check, minglin, 2009/12/28 *start*/
		//if(!checkDigit(document.wanCfg.wan_3g_policy_alert.checked))
		if(document.wanCfg.wan_3g_policy_alert.checked) // enable budget control alert method
		{
			if(!checkDigit(document.wanCfg.wan_3g_mail_sent_period.value)) // check E-mail Alert re-send period value
					{
				alert( _('wan 3g mail sent period digits') );
				document.wanCfg.wan_3g_mail_sent_period.focus();
				document.wanCfg.wan_3g_mail_sent_period.select();
						return false;
					}
			if(document.wanCfg.wan_3g_mail_sent_period.value == "" ) // check E-mail Alert re-send period value
		        {
		                alert( _('wan 3g mail sent period digits') );
		                document.wanCfg.wan_3g_mail_sent_period.focus();
		                document.wanCfg.wan_3g_mail_sent_period.select();
		                return false;
		        }
			
			if(document.wanCfg.wan_3g_smtp_username.value == "" && 
				document.wanCfg.wan_3g_smtp_auth_enable.selectedIndex != 2 ){ // check E-mail alert user name is null?
				
				alert( _('chk Mail User Name') );
				document.wanCfg.wan_3g_smtp_username.focus();
				document.wanCfg.wan_3g_smtp_username.select();
				return false;
			}
			if(document.wanCfg.wan_3g_mail_server.value == ""){ // check E-mail alert Server is null?
				alert( _('chk Mail Server') );
				document.wanCfg.wan_3g_mail_server.focus();
				document.wanCfg.wan_3g_mail_server.select();
				return false;
			}
			if(document.wanCfg.wan_3g_mail_sender.value == ""){ // check E-mail alert Sender is null?
				alert( _('chk Mail Sender') );
				document.wanCfg.wan_3g_mail_sender.focus();
				document.wanCfg.wan_3g_mail_sender.select();
				return false;
		}
			if(document.wanCfg.wan_3g_mail_recipient.value == ""){ // check E-mail alert Recipient is null?
				alert( _('chk Mail Recipient') );
				document.wanCfg.wan_3g_mail_recipient.focus();
				document.wanCfg.wan_3g_mail_recipient.select();
				return false;
			}
		}
		/*fix bug and add alert field check, minglin, 2009/12/28 *end*/
	}
		
	if (f1.value != "99" && f2.value != "99") { //daulwan
		if (!checkServerIpAddr(document.wanCfg.masterDetectIp, false)) {
		        document.wanCfg.masterDetectIp.focus();
		        document.wanCfg.masterDetectIp.select();
		        return false;
		}
		if(!checkIllegalChar(document.wanCfg.masterDetectIp.value))
		{
		        alert( _('wan illegal characters in masterDetectIp') );
		        document.wanCfg.masterDetectIp.focus();
		        document.wanCfg.masterDetectIp.select();
		        return false;
		}
		if (!checkServerIpAddr(document.wanCfg.backupDetectIp, false)) {
		        document.wanCfg.backupDetectIp.focus();
		        document.wanCfg.backupDetectIp.select();
		        return false;
		}
		if(!checkIllegalChar(document.wanCfg.backupDetectIp.value))
		{
		        alert( _('wan illegal characters in backupDetectIp') );
		        document.wanCfg.backupDetectIp.focus();
		        document.wanCfg.backupDetectIp.select();
		        return false;
		}
		if (document.wanCfg.detectTimeout.value*1 < 1 || document.wanCfg.detectTimeout.value*1 > 5) {
		        alert( _("wan Detect Timeout range error"));
		        return false;
		}
		if(!checkDigit(document.wanCfg.detectTimeout.value))
		{
		        alert( _('wan Detect Timeout digits') );
		        document.wanCfg.detectTimeout.focus();
		        document.wanCfg.detectTimeout.select();
		        return false;
		}
	}
	
	if (document.wanCfg.wan_macClone_enable.checked == "checked") {
		var re = /[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}/;
		if (document.wanCfg.macCloneMac.value.length == 0) {
			alert(_("wan mac empty"));
			document.wanCfg.macCloneMac.focus();
			return false;
		}
		if (!re.test(document.wanCfg.macCloneMac.value)) {
			alert(_("wan fill incorrect mac01"));
			document.wanCfg.macCloneMac.focus();
			return false;
		}
		if (document.wanCfg.macCloneMac.value == "FF:FF:FF:FF:FF:FF") {
			alert(_("wan fill incorrect mac02"));
			document.wanCfg.macCloneMac.focus();
			return false;
		}
	}
	return true;
}
function wan_3g_budget_control_enable_change(f)
{
	if (f.wan_3g_budget_control_enable.checked)
	{	// enabled
		id_visible("Budget_ctrl_table");
		id_visible("Budget_ctrl_policy");
		var time_budget_enable="<% getWan_3g_time_budget_enable1();%>";
		var data_budget_enable="<% getWan_3g_data_budget_enable1();%>";
		/* modify, minglin, 2009/12/27 * Start */
		if ( time_budget_enable == "checked" ){
			document.wanCfg.wan_3g_time_budget_enable.checked=true;
			disen_chg('wan_3g_time_budget_enable', 'wan_3g_time_limit'); 
			disen_chg('wan_3g_time_budget_enable', 'wan_3g_max_online_monthly');
		}
		
		if ( data_budget_enable == "checked" ){
			document.wanCfg.wan_3g_data_budget_enable.checked=true;
			disen_chg('wan_3g_data_budget_enable', 'wan_3g_data_limit'); 
			disen_chg('wan_3g_data_budget_enable', 'wan_3g_max_xfer_monthly');  
			disen_chg('wan_3g_data_budget_enable', 'wan_3g_up_down_stream_limit');
		}
		
		chk_disen_chg(); // check time budget and data budget both disable, then disable both Drop Current Connection and Disallow New Connection click box.
		/* modify, minglin, 2009/12/27 * end */
/*
		var policy_drop="<% getWan_3g_policy_drop1();%>";
		var policy_disallow="<% getWan_3g_policy_disallow1();%>";
		if ( policy_drop == "checked" )
		{
			document.wanCfg.wan_3g_policy_drop.checked=true;
			//document.wanCfg.wan_3g_policy_drop.disabled=true;
			document.wanCfg.wan_3g_policy_drop.value = 1;
		}else{
			document.wanCfg.wan_3g_policy_drop.checked=false;
			document.wanCfg.wan_3g_policy_drop.value = 0;
		}
		if ( policy_disallow == "checked" )
		{
			document.wanCfg.wan_3g_policy_disallow.checked=true;
			//document.wanCfg.wan_3g_policy_disallow.disabled=true;
			document.wanCfg.wan_3g_policy_disallow.value = 1;
		}else{
			document.wanCfg.wan_3g_policy_disallow.checked=false;
			document.wanCfg.wan_3g_policy_disallow.value = 0;
		}
*/
		id_visible("Budget_ctrl_counter");
		//id_visible("manuallyBudgetSet");
		if (document.getElementById("wan_3g_policy_alert").checked)
		{	// enabled email
			id_visible("id_eamil_auth");
		}
		else
		{
			id_invisible("id_eamil_auth");
		}
/**/
	}
	else
	{
  		id_invisible("Budget_ctrl_table");
  		id_invisible("Budget_ctrl_policy");
  		/**/id_invisible("Budget_ctrl_counter");
		/**/id_invisible("id_eamil_auth");
		//id_invisible("manuallyBudgetSet");
	}
}
function wan_detection_userdefined(f) // 20091002 added by Carl
{
	if (f.wan_detection.checked)
	{// wan_detection_userdefined
		id_visible("MasterDetectCol");
		id_visible("BackupDetectCol");
		id_visible("DetectTimeoutCol");
	}
	else
	{// default
		id_invisible("MasterDetectCol");
		id_invisible("BackupDetectCol");
		id_invisible("DetectTimeoutCol");
	}
}

function wan_3g_pincode_protect_enable_change(f)
{
	if (f.wan_3g_pincode_protect.checked)
	{// enabled
		alert( _("wan 3g pincode warring") ); // 20090915 added by Carl to Warring user pincode proctect
	}
}

function wan_3g_smtp_auth_change(f)
{
	if(document.getElementById("wan_3g_policy_alert").checked && document.getElementById("wan_3g_smtp_auth_enable").selectedIndex== 2 )
	{
           id_invisible("row_wan_3g_smtp_username");
           id_invisible("row_wan_3g_smtp_password");
	} 
	else 
	{
           id_visible("row_wan_3g_smtp_username");
           id_visible("row_wan_3g_smtp_password");
	}
}

function wan_3g_policy_alert_change(f)
{
	if (document.getElementById("wan_3g_policy_alert").checked)
	{	// enabled
		id_visible("id_eamil_auth");
	} 
	else
	{
		id_invisible("id_eamil_auth");
	}
	wan_3g_smtp_auth_change(f);
}
function init_3G_setting(f) //f:document.wanCfg.connectionType1 or document.wanCfg.connectionType2
{
        var var_3g_connectType = "<% getWan_3g_connecttype(); %>";
        var var_3g_updownst = "<% getWan_3g_up_down_stream_limit0(); %>";
        var var_3g_cycledate = "<% getCfgGeneral(1, "wan_3g_cycle_date"); %>";
        var var_3g_smtpauth  = "<% getCfgGeneral(1, "wan_3g_smtp_auth_enable"); %>";
        var var_3g_start_mode  = "<% getCfgZero(1, "wan_3g_start_mode"); %>";
        f.value = "G3G";
        if (var_3g_connectType == 1)
                document.wanCfg.wan_3g_connecttype.options.selectedIndex = 1;
        else
                document.wanCfg.wan_3g_connecttype.options.selectedIndex = 0;
        if (var_3g_updownst == 0)
                document.wanCfg.wan_3g_up_down_stream_limit.options.selectedIndex = 0;
        else if (var_3g_updownst == 1)
                document.wanCfg.wan_3g_up_down_stream_limit.options.selectedIndex = 1;
        else if (var_3g_updownst == 2)
                document.wanCfg.wan_3g_up_down_stream_limit.options.selectedIndex = 2;
		document.wanCfg.wan_3g_cycle_date.options.selectedIndex = 1*(var_3g_cycledate) ;
        document.wanCfg.wan_3g_smtp_auth_enable.options.selectedIndex = 1*(var_3g_smtpauth) ;
		document.getElementById("wan_3g_smtp_auth_enable").selectedIndex = 1*(var_3g_smtpauth);
        wan_3g_connecttype_change(document.wanCfg);
		wan_3g_policy_alert_change(document.wanCfg);
        wan_3g_budget_control_enable_change(document.wanCfg);
        //wan_3gConnModeSwitch();
}
function init_NONE_setting(f) //f:document.wanCfg.connectionType1 or document.wanCfg.connectionType2
{
        f.value = "99";
}
function initTranslation()
{
	var get_msg;
	var e = document.getElementById("wTitle");
	e.innerHTML = _("wan title");
	e = document.getElementById("wIntroduction");
	e.innerHTML = _("wan introduction")+"<br>";
	e = document.getElementById("wDaulwanIntroduction");
	e.innerHTML = _("wan dual wan introduction");
	e = document.getElementById("w3GIntroduction");
	e.innerHTML = _("wan 3g introduction");
/*
	e = document.getElementById("progressinner");
	e.innerHTML = _("admin saving");
*/
	e = document.getElementById("wConnectionType");
	e.innerHTML = _("wan connection type");
	e = document.getElementById("wconnectionType1");
	e.innerHTML = _("wan connection type of ethernet");
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
//	e = document.getElementById("wConnTypebpond1");
//	e.innerHTML = _("wan connection type bpond");
//	e = document.getElementById("wConnType3g1");
//	e.innerHTML = _("wan connection type 3g");
	e = document.getElementById("wConnTypeNone");
	e.innerHTML = _("wan connection type none");
	e = document.getElementById("wConnTypeNone1");
	e.innerHTML = _("wan connection type none");
	e = document.getElementById("wconnectionType2");
	e.innerHTML = _("wan connection type of usb");
//	e = document.getElementById("wConnTypeStatic2");
//	e.innerHTML = _("wan connection type static");
//	e = document.getElementById("wConnTypeDhcp2");
//	e.innerHTML = _("wan connection type dhcp");
//	e = document.getElementById("wConnTypePppoe2");
//	e.innerHTML = _("wan connection type pppoe");
//	e = document.getElementById("wConnTypePptp2");
//	e.innerHTML = _("wan connection type pptp");
//	e = document.getElementById("wConnTypebpond2");
//	e.innerHTML = _("wan connection type bpond");
	e = document.getElementById("wConnType3g2");
	e.innerHTML = _("wan connection type 3g");
//	e = document.getElementById("wanUsbStatus");
//	e.innerHTML = _("wan 3g Device status");
	e = document.getElementById("wan2_option_master");
	e.innerHTML = _("wan Master");
	e = document.getElementById("wan2_option_backup");
	e.innerHTML = _("wan Backup");
	e = document.getElementById("fallback_wan_option");
	e.innerHTML = _("wan fallback option");
	e = document.getElementById("fallback_Enable");
	e.innerHTML = _("inet enable");
	e = document.getElementById("fallback_Disable");
	e.innerHTML = _("inet disable");
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
	e = document.getElementById("wPppoeOPMode");
	e.innerHTML = _("wan protocol opmode");
	e = document.getElementById("wPppoeKeepAlive");
	e.innerHTML = _("wan protocol opmode keepalive");
	e = document.getElementById("wPppoeOnDemand");
	e.innerHTML = _("wan protocol opmode ondemand");
	e = document.getElementById("wPppoeManual");
	e.innerHTML = _("wan protocol opmode manual");

	e = document.getElementById("wppoeKeepAliveMode");
	e.innerHTML = _("wan protocol keep alive mode");
	e = document.getElementById("wppoeKeepseconds");
	e.innerHTML = _("wan protocol keep seconds");
	e = document.getElementById("wppoeOnDemandMode");
	e.innerHTML = _("wan protocol on demand mode");
	e = document.getElementById("wppoeOnDemandMinutes");
	e.innerHTML = _("wan protocol on demand minutes");
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
	/*
	e = document.getElementById("wL2tpOnDemand");
	e.innerHTML = _("wan protocol opmode ondemand");
	*/
	e = document.getElementById("wL2tpManual");
	e.innerHTML = _("wan protocol opmode manual");

	e = document.getElementById("wl2tpKeepAliveMode");
	e.innerHTML = _("wan protocol keep alive mode");
	e = document.getElementById("wl2tpKeepseconds");
	e.innerHTML = _("wan protocol keep seconds");
/*
	e = document.getElementById("wl2tpOnDemandMode");
	e.innerHTML = _("wan protocol on demand mode");
	e = document.getElementById("wl2tpOnDemandMinutes");
	e.innerHTML = _("wan protocol on demand minutes");
*/
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
	/*
	e = document.getElementById("wPptpOnDemand");
	e.innerHTML = _("wan protocol opmode ondemand");
	*/
	e = document.getElementById("wPptpManual");
	e.innerHTML = _("wan protocol opmode manual");

	e = document.getElementById("wpptpKeepAliveMode");
	e.innerHTML = _("wan protocol keep alive mode");
	e = document.getElementById("wpptpKeepseconds");
	e.innerHTML = _("wan protocol keep seconds");
/*
	e = document.getElementById("wpptpOnDemandMode");
	e.innerHTML = _("wan protocol on demand mode");
	e = document.getElementById("wpptpOnDemandMinutes");
	e.innerHTML = _("wan protocol on demand minutes");
*/
/*
	e = document.getElementById("w3GMode");
	e.innerHTML = _("wan 3G mode");
	e = document.getElementById("w3GDev");
	e.innerHTML = _("wan 3G modem");
*/
	e = document.getElementById("wdns_customer");
	e.innerHTML = _("inet wc dns");
	e = document.getElementById("wDhcpPriDns");
	e.innerHTML = _("inet wc pri dns");
	e = document.getElementById("wDhcpSecDns");
	e.innerHTML = _("inet wc sec dns");
	
	e = document.getElementById("wanDaulWanMode");
	e.innerHTML = _("wan Dual Wan Mode");
	e = document.getElementById("MasterDetectIp");
	e.innerHTML = _("wan Master Detect IP");
	e = document.getElementById("BackupDetectIp");
	e.innerHTML = _("wan Backup Detect IP");
	e = document.getElementById("DetectTimeout");
	e.innerHTML = _("wan Detect Timeout");
	
//	e = document.getElementById("row_wan_3g_box_buton");
//	e.innerHTML = _("3G Start Mode");
//	e = document.getElementById("data_Automatically");
//	e.innerHTML = _("3G Automatically");
//	e = document.getElementById("data_Manually");
//	e.innerHTML = _("3G Manually");
	e = document.getElementById("row_wan_3g_pincode_protect");
	e.innerHTML = _("3G Pin Code Protect");
	e = document.getElementById("data_enable");
	e.innerHTML = _("inet enable");
	e = document.getElementById("row_wan_3g_dialnumber");
	e.innerHTML = _("3G Dial Code");
	e = document.getElementById("row_wan_3g_apn");
	e.innerHTML = _("3G APN Service");
	e = document.getElementById("row_wan_3g_username");
	e.innerHTML = _("inet user");
	e = document.getElementById("row_wan_3g_password");
	e.innerHTML = _("inet password");
	e = document.getElementById("row_wan_3g_connecttype");
	e.innerHTML = _("3G Connect Type");
	e = document.getElementById("data_Keep_Alive");
	e.innerHTML = _("wan protocol opmode keepalive");
	e = document.getElementById("data_Auto_Connect");
	e.innerHTML = _("wan protocol on demand mode");
	e = document.getElementById("data_wan_3g_maxidletime");
	e.innerHTML = _("3G Max Idle Time");
	e = document.getElementById("data_seconds");
	e.innerHTML = _("3G Idle");
	e = document.getElementById("row_wan_3G_budget_control_enable");
	e.innerHTML = _("Budget Control");
	e = document.getElementById("wan_3g_budget_control_enable_text");
	e.innerHTML = _("Budget Control Enable");
	e = document.getElementById("row_wan_3g_Budget_Scheme");
	e.innerHTML = _("Budget Criterion");
	e = document.getElementById("data_Time_budget");
	e.innerHTML = _("3G Time Budget");
	e = document.getElementById("data_hours_PM_budget");
	e.innerHTML = _("hours per month");
	e = document.getElementById("data_Data_Budge");
	e.innerHTML = _("3G Data Budget");
	e = document.getElementById("data_MByte_PM");
	e.innerHTML = _("MBytes per month");
	/*e = document.getElementById("data_MByte_range");
	e.innerHTML = _("MBytes per month range");*/
	e = document.getElementById("data_download");
	e.innerHTML = _("inet Download");
	e = document.getElementById("data_upload");
	e.innerHTML = _("inet Upload");
	e = document.getElementById("data_UDload");
	e.innerHTML = _("inet Download Upload");
//	e = document.getElementById("per_month");
//	e.innerHTML = _("per_month");
	e = document.getElementById("row_wan_3g_budget_policy");
	e.innerHTML = _("Budget Policy");
    
    e = document.getElementById("data_over_bdg");
    e.innerHTML = _("Actions if Over Budget");
    get_msg = "<% getWan_3g_over_time_budget_msg(); %>";
    e = document.getElementById("data_over_bdg_time_msg");
    if ( get_msg == "TIME_Over_Budget" ){
            e.innerHTML =  _("TIME Budget OK");
            e.color="blue";
    } else if (  get_msg == "TIME_Over_Budget"  ){
            e.innerHTML =  _("TIME Over Budget");
            e.color="red";
    } else if (  get_msg == "" ){
            e.innerHTML = "";
            e.color="red";
    }
    
    get_msg= "<% getWan_3g_over_data_budget_msg(); %>";
    e = document.getElementById("data_over_bdg_data_msg");
    if ( get_msg == "DATA_Budget_OK" ){
            e.innerHTML =  _("DATA Budget OK");
            e.color="blue";
    } else if ( get_msg == "DATA_Over_Budget"  ){
            e.innerHTML =  _("DATA Over Budget");
            e.color="red";
    } else if ( get_msg == "" ){
            e.innerHTML ="";
            e.color="red";
    }
    
	e = document.getElementById("data_drop_con_text");
	e.innerHTML = _("Drop Current Connection");
	e = document.getElementById("data_disallow_text");
	e.innerHTML = _("Disallow New Connection");
	e = document.getElementById("data_trigger_by_limit_msg"); ////
	e.innerHTML = _("Trigger by Limit Budget");
    get_msg = "<% getWan_3g_prelimit_time_budget_msg(); %>";
    e = document.getElementById("prelimit_time_budget_msg");
    if ( get_msg == "TIME_Pre-Limit_OK" ){
            e.innerHTML =  _("TIME Pre Limit OK");
            e.color="blue";
    } else if ( get_msg == "TIME_Reach_Pre-Limit" ){
            e.innerHTML =  _("TIME Reach Pre Limit");
            e.color="red";
    } else if ( get_msg == "" ){
            e.innerHTML = "";
            e.color="red";
    }////
    
    get_msg = "<% getWan_3g_prelimit_data_budget_msg(); %>"; ///
    e = document.getElementById("prelimit_data_budget_msg");
    if ( get_msg == "DATA_Pre-Limit_OK"){
            e.innerHTML =  _("DATA Pre Limit OK");
            e.color="blue";
    } else if ( get_msg == "DATA_Reach_Pre-Limit" ){
            e.innerHTML =  _("DATA Reach Pre Limit");
            e.color="red";
    } else if ( get_msg == "" ){
            e.innerHTML = "";
            e.color="red";
    }
	e = document.getElementById("data_of_the_time_bgt_text");
	e.innerHTML = _("% of the time budget");
	e = document.getElementById("data_of_the_data_bgt_text");
	e.innerHTML = _("% of the data budget"); 
	e = document.getElementById("email_alert_head");
	e.innerHTML = _("E-mail Alert");
	e = document.getElementById("email_alert_body_text");
	e.innerHTML = _("will recur every");
	e = document.getElementById("email_alert_unit");
	e.innerHTML = _("minutes");
	e = document.getElementById("row_wan_3g_cycle_date");
	e.innerHTML = _("Budget Counter");
	e = document.getElementById("data_reset_on");
	e.innerHTML = _("Reset on");
	/* minglin, 2010/01/14 *Start*/
	e = document.getElementById("data_reset_on_msg");
	e.innerHTML = _("Reset on message");
	/* minglin, 2010/01/14 *end*/
	e = document.getElementById("data_id1_0");
	e.innerHTML = _("dayid 1");
	e = document.getElementById("data_id1_1");
	e.innerHTML = _("dayid 2");
	e = document.getElementById("data_id1_2");
	e.innerHTML = _("dayid 3");
	e = document.getElementById("data_id1_3");
	e.innerHTML = _("dayid 4");
	e = document.getElementById("data_id1_4");
	e.innerHTML = _("dayid 5");
	e = document.getElementById("data_id1_5");
	e.innerHTML = _("dayid 6");
	e = document.getElementById("data_id1_6");
	e.innerHTML = _("dayid 7");
	e = document.getElementById("data_id1_7");
	e.innerHTML = _("dayid 8");
	e = document.getElementById("data_id1_8");
	e.innerHTML = _("dayid 9");
	e = document.getElementById("data_id1_9");
	e.innerHTML = _("dayid 10");
	e = document.getElementById("data_id1_10");
	e.innerHTML = _("dayid 11");
	e = document.getElementById("data_id1_11");
	e.innerHTML = _("dayid 12");
	e = document.getElementById("data_id1_12");
	e.innerHTML = _("dayid 13");
	e = document.getElementById("data_id1_13");
	e.innerHTML = _("dayid 14");
	e = document.getElementById("data_id1_14");
	e.innerHTML = _("dayid 15");
	e = document.getElementById("data_id1_15");
	e.innerHTML = _("dayid 16");
	e = document.getElementById("data_id1_16");
	e.innerHTML = _("dayid 17");
	e = document.getElementById("data_id1_17");
	e.innerHTML = _("dayid 18");
	e = document.getElementById("data_id1_18");
	e.innerHTML = _("dayid 19");
	e = document.getElementById("data_id1_19");
	e.innerHTML = _("dayid 20");
	e = document.getElementById("data_id1_20");
	e.innerHTML = _("dayid 21");
	e = document.getElementById("data_id1_21");
	e.innerHTML = _("dayid 22");
	e = document.getElementById("data_id1_22");
	e.innerHTML = _("dayid 23");
	e = document.getElementById("data_id1_23");
	e.innerHTML = _("dayid 24");
	e = document.getElementById("data_id1_24");
	e.innerHTML = _("dayid 25");
	e = document.getElementById("data_id1_25");
	e.innerHTML = _("dayid 26");
	e = document.getElementById("data_id1_26");
	e.innerHTML = _("dayid 27");
	e = document.getElementById("data_id1_27");
	e.innerHTML = _("dayid 28");
	e = document.getElementById("data_id1_28");
	e.innerHTML = _("dayid 29");
	e = document.getElementById("data_id1_29");
	e.innerHTML = _("dayid 30");
	e = document.getElementById("data_id1_30");
	e.innerHTML = _("dayid 31");
	e = document.getElementById("row_wan_3g_budget_mail");
	e.innerHTML = _("Mail Settings");
	e = document.getElementById("data_smtp_authen");
 	e.innerHTML = _("Mail SMTP Authentication");
	e = document.getElementById("data_plain");
	e.innerHTML = _("Mail PLAIN");
	e = document.getElementById("data_login");
	e.innerHTML = _("Mail LOGIN");
	e = document.getElementById("data_disabled");

//	e = document.getElementById("wMacClone");
//	e.innerHTML = _("wan mac clone");	e.innerHTML = _("inet disable");
	e = document.getElementById("wan_macClone_enable_text");
	e.innerHTML = _("wan mac clone");
	e = document.getElementById("row_wan_3g_mail_username");
	e.innerHTML = _("inet user");
	e = document.getElementById("row_wan_3g_mail_password");
	e.innerHTML = _("inet password");
	e = document.getElementById("row_wan_3g_mail_server");
	e.innerHTML = _("Mail Server");
	e = document.getElementById("row_wan_3g_mail_sender");
	e.innerHTML = _("Mail Sender");
	e = document.getElementById("row_wan_3g_mail_recipient");
	e.innerHTML = _("Mail Recipient");
//	e = document.getElementById("wMacCloneD");
//	e.innerHTML = _("inet disable");
//	e = document.getElementById("wMacCloneE");
//	e.innerHTML = _("inet enable");
	e = document.getElementById("wMacCloneAddr");
	e.innerHTML = _("inet mac");
	e = document.getElementById("wApply");
	e.value = _("inet next");
	e = document.getElementById("wCancel");
	e.value = _("inet cancel");
     	e = document.getElementById("wPrevious");
	e.value = _("w Previous");
	
	get_msg = "<% getWan_3g_setPincode_Error_Msg(); %>";
    e = document.getElementById("pincode_authentication");
    if ( get_msg == "Not_dial_yet" ){
//			e.innerHTML = _("Not dial yet");
            e.innerHTML = _("");
            e.color="red";
    }
    else if ( get_msg == "Auth_successfully" ){
            e.innerHTML =  _("Auth successfully");
            e.color="blue";
    }
    else if ( get_msg == "Auth_failure" ){
            e.innerHTML =  _("Auth failure");
            e.color="red";
    }
    e = document.getElementById("DetectTimeoutRange");
    e.innerHTML = _("wan Detect Timeout range");
}
function initValue()
{
/*	var mode = "<% getCfgGeneral(1, "wanConnectionMode"); %>";*/
	var mode = "<% getCfgGeneral(1, "wan1"); %>";
	var wan2 = "<% getCfgGeneral(1, "wan2"); %>";
	var pptpMode = <% getCfgZero(1, "wan_pptp_mode"); %>;
	var l2tpMode = <% getCfgZero(1, "wan_l2tp_mode"); %>;

	if (mode == "STATIC") {
		document.wanCfg.connectionType.options.selectedIndex = 0;
	}
	else if (mode == "DHCP") {
		document.wanCfg.connectionType.options.selectedIndex = 1;
	}
	else if (mode == "PPPOE") {
		var pppoe_opmode = "<% getCfgGeneral(1, "wan_pppoe_opmode"); %>";
		var pppoe_optime = "<% getCfgGeneral(1, "wan_pppoe_optime"); %>";
		document.wanCfg.connectionType.options.selectedIndex = 2;
		if (pppoe_opmode == "Manual")
		{
			document.wanCfg.pppoeOPMode.options.selectedIndex = 2;
		}
		else if (pppoe_opmode == "OnDemand")
		{
			document.wanCfg.pppoeOPMode.options.selectedIndex = 1;
			if (pppoe_optime != "")
				document.wanCfg.pppoeIdleTime.value = pppoe_optime;
		}
		else if (pppoe_opmode == "KeepAlive")
		{
			document.wanCfg.pppoeOPMode.options.selectedIndex = 0;
			if (pppoe_optime != "")
				document.wanCfg.pppoeRedialPeriod.value = pppoe_optime;
		}
		pppoeOPModeSwitch();
	}
	else if (mode == "PPTP") {
		var pptp_opmode = "<% getCfgGeneral(1, "wan_pptp_opmode"); %>";
		var pptp_optime = "<% getCfgGeneral(1, "wan_pptp_optime"); %>";
		document.wanCfg.connectionType.options.selectedIndex = 3;
		document.wanCfg.pptpMode.options.selectedIndex = pptpMode;
		pptpModeSwitch();
		if (pptp_opmode == "Manual")
		{
			document.wanCfg.pptpOPMode.options.selectedIndex = 1;//2;
			//if (pptp_optime != "")
				//document.wanCfg.pptpIdleTime.value = pptp_optime;
		}
		/*
		else if (pptp_opmode == "OnDemand")
		{
			document.wanCfg.pptpOPMode.options.selectedIndex = 1;
			if (pptp_optime != "")
				document.wanCfg.pptpIdleTime.value = pptp_optime;
		}
		*/
		else if (pptp_opmode == "KeepAlive")
		{
			document.wanCfg.pptpOPMode.options.selectedIndex = 0;
			if (pptp_optime != "")
				document.wanCfg.pptpRedialPeriod.value = pptp_optime;
		}
		pptpOPModeSwitch();
	}	
	else if (mode == "L2TP") {
		var l2tp_opmode = "<% getCfgGeneral(1, "wan_l2tp_opmode"); %>";
		var l2tp_optime = "<% getCfgGeneral(1, "wan_l2tp_optime"); %>";
		
		document.wanCfg.connectionType.options.selectedIndex = 4;
		document.wanCfg.l2tpMode.options.selectedIndex = l2tpMode;
		l2tpModeSwitch();
		if (l2tp_opmode == "Manual")
		{
			/*document.wanCfg.l2tpOPMode.options.selectedIndex = 2;*/
			document.wanCfg.l2tpOPMode.options.selectedIndex = 1;
		}
		/*
		else if (l2tp_opmode == "OnDemand")
		{
			document.wanCfg.l2tpOPMode.options.selectedIndex = 1;
			if (l2tp_optime != "")
				document.wanCfg.l2tpIdleTime.value = l2tp_optime;
		}
		*/
		else if (l2tp_opmode == "KeepAlive")
		{
			document.wanCfg.l2tpOPMode.options.selectedIndex = 0;
			if (l2tp_optime != "")
				document.wanCfg.l2tpRedialPeriod.value = l2tp_optime;
		}
		l2tpOPModeSwitch();
	}
/*
	else if (mode == "3G") {
		var dev_3g = "<% getCfgGeneral(1, "wan_3g_dev"); %>";
		document.wanCfg.connectionType.options.selectedIndex = 5;
		if (dev_3g == "HUAWEI-E169")
			document.wanCfg.Dev3G.options.selectedIndex = 1;
		else if (dev_3g == "BandLuxe-C270")
			document.wanCfg.Dev3G.options.selectedIndex = 2;
		else if (dev_3g == "OPTION-ICON225")
			document.wanCfg.Dev3G.options.selectedIndex = 3;
		else
			document.wanCfg.Dev3G.options.selectedIndex = 0;
	}
*/
	else if (mode == "99"){
		document.wanCfg.connectionType.options.selectedIndex = 5;
	}

	if (wan2 == "G3G")
		init_3G_setting(document.wanCfg.connectionType1);
	else
		init_NONE_setting(document.wanCfg.connectionType1);
	
	// initial dual_wan bluck
    //wan2 mode: 0--backup wan, 1 -- master wan(or only wan2); dafauld = 0
    var wan2_mode = "<% getCfgGeneral(1, "wan2_option_mode"); %>";
    var master_wan_detect_ip = "<% getCfgGeneral(1, "wan_dual_wan_detect_ip"); %>";
    var backup_wan_detect_ip = "<% getCfgGeneral(1, "wan_dual_backup_wan_detect_ip"); %>";
    var fallback_enable = "<% getCfgGeneral(1, "wan_dual_wan_fallback_enable"); %>";
    var fallback_detect_timeout = "<% getCfgGeneral(1, "wan_dual_wan_detect_timeout"); %>";
    document.wanCfg.wan2_option_mode.value = 1*wan2_mode;
    document.wanCfg.fallback_option_mode.value = 1*fallback_enable;
    document.wanCfg.masterDetectIp.value = master_wan_detect_ip;
    document.wanCfg.backupDetectIp.value = backup_wan_detect_ip;
    document.wanCfg.detectTimeout.value = 1*fallback_detect_timeout
	
	if('<% getCfgZero(1, "macCloneEnabled"); %>' == 1)
		document.wanCfg.wan_macClone_enable.checked = true;
		
	connectionTypeSwitch();
	
	// change mac visibility 
	macCloneSwitch();
	// init hidden input
	disen_chg('wan_3g_time_budget_enable', 'wan_3g_time_limit'); 
	disen_chg('wan_3g_time_budget_enable', 'wan_3g_max_online_monthly');

	disen_chg('wan_3g_data_budget_enable', 'wan_3g_data_limit'); 
	disen_chg('wan_3g_data_budget_enable', 'wan_3g_max_xfer_monthly');  
	disen_chg('wan_3g_data_budget_enable', 'wan_3g_up_down_stream_limit');

	disen_chg('wan_3g_policy_alert', 'wan_3g_mail_sent_period');
	chk_disen_chg();
	initTranslation();
}
function id_invisible(id)
{
        document.getElementById(id).style.visibility = "hidden";
        document.getElementById(id).style.display = "none";
}
function id_visible(id)
{
        document.getElementById(id).style.visibility = "visible";
        document.getElementById(id).style.display = style_display_on();
}
function wan_3g_connecttype_change(f)
{
        if (f.wan_3g_connecttype.selectedIndex=="0")
                id_invisible("row_wan_3g_maxidletime");
        else
                id_visible("row_wan_3g_maxidletime");
}
function onClicksbuttonMax()
{
	if (CheckValue() && prompmsg())
	{
		window.scrollBy(0, -1000);
		sbutton_disable(sbuttonMax);
		document.wanCfg.submit();
		restartPage_block();
	}
}
function onPinCodeChecked(){
	
  if (checked == 0)
{
	  checked = 1;
      if(navigator.appName == "Microsoft Internet Explorer"){ 
			//alert("Please allow Internet Explorer Information Bar! \r\n Please input your Pin Code(length=4~8).");
			document.wanCfg.wan_3g_pincode_protect.checked = "";
		}else{ 
			alert("Please input your Pin Code(length=4~8)."); 
		}
      inputPinCode();
   }else if(document.wanCfg.wan_3g_pincode_protect.checked != "checked"){
      document.wanCfg.wan_3g_pincode_protect.checked = "";
      document.wanCfg.wan_3g_pincode.value = "";
	  checked = 0;
   }
}
function inputPinCode(){
  document.wanCfg.wan_3g_pincode_protect.checked = "";
  document.wanCfg.wan_3g_pincode.value = ""; 
  checked = 0;
  var pinCode = "";//prompt("Please input your Pin Code(length=4). \n Example : 0000");
  var npinCode = false;//check_pincode(pinCode);
  
  if(navigator.appName == "Microsoft Internet Explorer"){ 
		alert("Please allow Internet Explorer Information Bar! \n Please input your Pin Code(length=4~8).");
	}
	
  while(npinCode == false){
    document.wanCfg.wan_3g_pincode_protect.checked = "";
    document.wanCfg.wan_3g_pincode.value = "";
    checked = 0; 
    pinCode = prompt("Please Input your Pin Code. \n Example : 0000(length=4~8)");
    npinCode = check_pincode(pinCode);
  }
  
  if(npinCode == true){
    document.wanCfg.wan_3g_pincode_protect.checked = "checked";
    document.wanCfg.wan_3g_pincode.value = pinCode; 
	checked = 1;
/*  }else{ // repleace PinCode in flash data 
    document.wanCfg.wan_3g_pincode.value = ; */  
  }      
}
function check_pincode(str){
  // check input length, if too short
  if(str.length < 4){
    alert("The Length of Pin Code too short.\n Please try again(length=4~8)");
    return false;
  }
  // check input length, if too long
  if(str.length > 8){
    alert("The Length of Pin Code too long.\n Please try again(length=4~8)");
    return false;
  }
  // check input word
  for(var i = 0 ; i < str.length ; i++){
    if(!(str.substr(i, 1) >= '0' && str.substr(i, 1) <= '9')){ 
      alert("Pin Code Check Error... \n Please try again(Example:0000).");
      return false;
    }
  }
  return true;
} 
/* add confirm func. , minglin, 2009/12/25 *Start*/
function prompmsg(){
	if(!document.getElementById("connectionType1").selectedIndex)// if 3G is Enabled.
	{
	var msg = _('alt budget warning') +"\r\n"+ _('alt budget warning message')+"\r\n"; //"Warning!!\r\n budget control now is disable.\r\n";
	if( document.getElementById("wan_3g_budget_control_enable").checked ){
		msg = _('alt budget warning message2') +"\r\n"+ _('alt budget budget criterion') +"\r\n"; //"Please make sure your settings:\r\n======== Budget Criterion ============== \r\n ";
		if(document.getElementById('wan_3g_time_budget_enable').checked){ // Time Budget
			msg += _('3G Time Budget') + " --> "+
			document.getElementById("wan_3g_max_online_monthly").value
			+ _('hours per month') + "\r\n";// " hour(s)/month \r\n";
		}else{
			msg += _('3G Time Budget') + " --> " + _('inet disable') + "\r\n";	
		}
		if(document.getElementById("wan_3g_data_budget_enable").checked){ // Data Budget
			msg += _('3G Data Budget') + " --> "+
			document.getElementById("wan_3g_max_xfer_monthly").value
			+ _('MBytes per month') + " (" +
				((document.getElementById("wan_3g_up_down_stream_limit").selectedIndex == 0)? _('inet Download') : 
				((document.getElementById("wan_3g_up_down_stream_limit").selectedIndex == 1)? _('inet Upload'): _('inet Download Upload')))			   
			+")\r\n";
		}else{
			msg += _('3G Data Budget') + " --> " + _('inet disable') + "\r\n";
		}
		
		msg += _('alt budget budget policy') + "\r\n"; //"======== Budget Policy ================ \r\n";
		
		if(document.getElementById("data_drop_con").checked){ // Drop current connection
			msg += _('Actions if Over Budget') + " --> " + _('Drop Current Connection') + "\r\n";
		}
		if(document.getElementById("data_disallow").checked){ // Disallow New Connection
			msg += _('Actions if Over Budget') + " --> " + _('Disallow New Connection') + "\r\n";
		}
		if(document.getElementById("wan_3g_time_budget_enable").checked){ // Trigger by limit budget --> time
			msg += _('Trigger by Limit Budget') + " --> "
				+ document.getElementById("wan_3g_time_limit").value
				+ _('% of the time budget') + "\r\n";
		}
		if(document.getElementById("wan_3g_data_budget_enable").checked){ // Trigger by limit budget --> Date
			msg += _('Trigger by Limit Budget') + " --> "
				+ document.getElementById("wan_3g_data_limit").value
				+ _('% of the data budget') + "\r\n";
		}
		if(document.getElementById("wan_3g_policy_alert").checked){ // Email Alert --> time
			msg +=  _('E-mail Alert') + " --> " + _('inet enable') + "(" + _('will recur every') + " " 
				+ document.getElementById("wan_3g_mail_sent_period").value + " "
				+ _('minutes') + ") \r\n";
		}else{
			msg += _('E-mail Alert') + " --> " + _('inet disable') + "\r\n";
		}
		msg += _('Budget Counter') +" --> "+ _('Reset on') +" "+ _("dayid " + (document.getElementById('wan_3g_cycle_date').selectedIndex + 1)) +"\r\n";
		msg += "================================= \r\n";
	}
	return confirmMSG = confirm(msg);
	}else{ // else if 3g is disable, then allow setting.
	return true;
	}
}
/* add confirm func. , minglin, 2009/12/25 *end*/
/* disable/enable input field, minglin, 2009/12/25 *start*/
function disen_chg(con_Elemnt, becon_Elemnt){
	if(document.getElementById(con_Elemnt).checked){
		document.getElementById(becon_Elemnt).disabled = false;
	}else{
		document.getElementById(becon_Elemnt).disabled = true;
	}
}
/* disable/enable input field, minglin, 2009/12/25 *start*/
/* check disable/enable when select change, minglin 2009/12/27 *Start*/
function chk_disen_chg(){
	if(!document.getElementById("wan_3g_time_budget_enable").checked && 
		!document.getElementById("wan_3g_data_budget_enable").checked ){ // time budget and data budget --> disable
		document.getElementById('data_drop_con').disabled = true; // disable Drop current conntection.
		document.getElementById('data_drop_con').checked = false; // unclicked Drop current conntection.
		document.getElementById('data_disallow').disabled = true;
		document.getElementById('data_disallow').checked = false; 
		document.getElementById('wan_3g_policy_alert').disabled = true;
		document.getElementById('wan_3g_policy_alert').checked = false;
		document.getElementById('wan_3g_mail_sent_period').disabled = true; 
		wan_3g_policy_alert_change(document.wanCfg); // hidden e-mail input field
	}else{ // time budget and data budget --> enable
		document.getElementById('data_drop_con').disabled = false;
		document.getElementById('data_disallow').disabled = false;
		document.getElementById('wan_3g_policy_alert').disabled = false;
	}
}
/* check disable/enable when select change, minglin 2009/12/27 *End*/
function setLocation()
{
	window.location.href = "/wizard/wizard_timeset.asp";
}
</script>
</head>
<body onLoad="initValue(); parent.fnDispWizard(3);" bgcolor="#FFFFFF">
<div align="center">
 <center>
        <table class="body">          
          <tr><td>              
              <!--
                            <table width="620" border="1" cellpadding="2" cellspacing="1">
<p>
<tr>
  <td class="title" colspan="2" id="wTitle">Wide Area Network (WAN) Settings</td>
</tr>
<tr>
<td colspan="2">
 <p class="head" id="wIntroduction">You may choose different connection type suitable for your environment. Besides, 
you may also configure parameters according to the selected connection type.</p>
                            -->              
              <!-- <p class="head" id="wDaulwanIntroduction">When Dual WAN has been selected, only Keep Alive mode will be supported.</p>-->              
              <!--
</td>
</tr>
</p>
</table>
                            -->              
              <h1 id="wTitle"></h1>              
              <p id="wIntroduction">              
              </p>              
              <p id="wDaulwanIntroduction">              
              </p>              
              <p>                
                <font id="w3GIntroduction" color="#ff0000">                
                </font>              
              </p>              
              <hr />              
<br>
              <form method="post" name="wanCfg" action="/goform/setWan_wizard">                
                <table width="620" border="1" cellpadding="2" cellspacing="1">                  
                  <tr>                       
                    <td class="title" colspan="2" id="wConnectionType">WAN Connections</td>                     
                  </tr>                     
                  <tr id="primary_wan_option_col">                    
                    <td  width="170" class="head" id="wconnectionType1">Via Ethernet Port&nbsp;&nbsp;&nbsp;&nbsp;</td>                       
                    <td width="400">                           
    <select name="connectionType" size="1" onChange="connectionTypeSwitch();">
                        <option value="STATIC" id="wConnTypeStatic">Static (fixed IP)                         
                        </option>                               
                        <option value="DHCP" id="wConnTypeDhcp">DHCP (Auto Config)                         
                        </option>                               
                        <option value="PPPOE" id="wConnTypePppoe">PPPOE (ADSL)                         
                        </option>                               
                        <option value="PPTP" id="wConnTypePptp">PPTP                         
                        </option>                               
                        <option value="L2TP" id="wConnTypeL2tp">L2TP                         
                        </option>                               
      <!--<option value="3G" id="wConnType3G">3G</option>-->
                        <option value="99" id="wConnTypeNone">Disable                         
                        </option>                           
                      </select>
					  <!-- =========== MAC Clone =========== -->   
					<input type="checkbox" id="wan_macClone_enable" name="wan_macClone_enable" value="" onClick="macCloneSwitch();" >
					<font id="wan_macClone_enable_text" >Enable Mac Clone</font>
					<center><div id="wMacCloneAddrDis"><hr>
					<font id="wMacCloneAddr"></font>&nbsp;&nbsp;
					<input name="macCloneMac" id="macCloneMac" maxlength=17 value="<% getCfgGeneral(1, "macCloneMac"); %>" size="20"> 	                       
                    <input type="button" name="macCloneMacFill" id="macCloneMacFill" value="Clone Your PC's MAC Address" onClick="macCloneMacFillSubmit();" >
					</div></center>
				</td> 	                 
                  </tr>                  
                  <tr id="second_wan_option_col" class="data_invisible">	                     
                    <td width="170" class="head" id="wconnectionType2">Via 3G USB Port&nbsp;&nbsp;&nbsp;&nbsp;</td>	                     
                    <td width="400">  		                       
                      <select name="connectionType1" id="connectionType1" size="1" onChange="connectionTypeSwitch();">        	                         
                        <option value="G3G" id="wConnType3g2">3G                         
                        </option>        	                         
                        <option value="99" id="wConnTypeNone1">Disable                         
                        </option>    	                       
                      </select>&nbsp;&nbsp;
					  <font id="Usb_sta"> 
					  </font>
                    <div id="wan2_option_radios">      		                         
                        <input type="radio" name="wan2_option_mode" value="1" <% getWan2_option_mode1(); %> >                         
                        <span id='wan2_option_master'>Master WAN                         
                        </span>      		                         
                        <input type="radio" name="wan2_option_mode" value="0" <% getWan2_option_mode0(); %> >                         
                        <span id='wan2_option_backup'>Backup WAN                         
                        </span>    	                       
                     </div>  
  </td>
</tr>
</table>
                <table width="620" border="1" cellpadding="2" cellspacing="1" id="dual_wan_mode">                           
                  <tr>                               
                    <td class="title" colspan="2" id="wanDaulWanMode">Dual WAN Mode</td>                           
                  </tr>                           
                  <tr id="fallback_wan_option_col" class="data_invisible">                               
                    <td class="head" id="fallback_wan_option">Fallback Option</td>                               
                    <td class="opt_value">                                     
                      <input type="radio" name="fallback_option_mode" value="1"  <% getFallback_option_mode1(); %> >                       
                      <span id="fallback_Enable">Enable                       
                      </span>                                     
                      <input type="radio" name="fallback_option_mode" value="0"  <% getFallback_option_mode0(); %> >                       
                      <span id="fallback_Disable">Disable                       
                      </span>          </td>                           
                  </tr>                           
                  <tr>                               
                    <td width="170" class="head" id="MasterDetectIp">Detect IP Address of Master WAN</td>                               
                    <td width="400">                      
                      <input name="masterDetectIp" value="<% getCfgGeneral(1, "wan_dual_wan_detect_ip"); %>" style="font-family: Arial; width:190px"></td>                           
                  </tr>                           
                  <tr>                               
                    <td width="170" class="head" id="BackupDetectIp">Detect IP Address of Backup WAN</td>                               
                    <td width="400">                      
                      <input name="backupDetectIp" value="<% getCfgGeneral(1, "wan_dual_backup_wan_detect_ip"); %>" style="font-family: Arial; width:190px"></td>                           
                  </tr>                           
                  <tr>                               
                    <td width="170" class="head" id="DetectTimeout">Detect Timeout</td>                               
                    <td width="400">                      
                      <input name="detectTimeout" maxlength=2 value="<% getCfgGeneral(1, "wan_dual_wan_detect_timeout"); %>" style="font-family: Arial; width:190px">                       
                      <br>                                       
                      <font id="DetectTimeoutRange">(1 ~ 5 seconds)                       
                      </font></td>                           
                  </tr>                
                </table>                
<!-- ================= STATIC Mode ================= -->
                <table id="static" width="620" border="1" cellpadding="2" cellspacing="1">                  
<tr>
                    <td class="title" colspan="2" id="wStaticMode">Static Mode</td>                  
</tr>
<tr>
                    <td width="170" class="head" id="wStaticIp">IP Address</td>                       
                    <td width="400">                         
                      <input name="staticIp" maxlength="15" value="<% getCfgGeneral(1, "wan_ipaddr"); %>" size="25" style="font-family: Arial; width:190px">   </td>                  
</tr>
<tr>
                    <td class="head" id="wStaticNetmask">Subnet Mask</td>  <td>                         
                      <input name="staticNetmask" maxlength="15" value="<% getCfgGeneral(1, "wan_netmask"); %>" size="25" style="font-family: Arial; width:190px">   </td>                  
</tr>
<tr>
                    <td class="head" id="wStaticGateway">Default Gateway</td>  <td>                         
                      <input name="staticGateway" maxlength="15" value="<% getCfgGeneral(1, "wan_gateway"); %>" size="25" style="font-family: Arial; width:190px">   </td>                  
</tr>
<tr>
                    <td class="head" id="wStaticPriDns">Primary DNS Server</td>  <td>                         
                      <input name="staticPriDns" maxlength="15" value="<% getCfgGeneral(1, "wan_primary_dns"); %>" size="25" style="font-family: Arial; width:190px">   </td>                  
</tr>
<tr>
                    <td class="head" id="wStaticSecDns">Secondary DNS Server</td>  <td>                         
                      <input name="staticSecDns" maxlength="15" value="<% getCfgGeneral(1, "wan_secondary_dns"); %>" size="25" style="font-family: Arial; width:190px">   </td>                  
</tr>
</table>
<!-- ================= DHCP Mode ================= -->
                <table id="dhcp" width="620" border="1" cellpadding="2" cellspacing="1">                  
<tr>
                    <td class="title" colspan="2" id="wDhcpMode">DHCP Mode</td>                  
</tr>
<tr>
                    <td class="head" width="170">                      
                      <div id="wDhcpHost">Host Name                       
                      </div></td>                       
                    <td width="400">                      
                      <input type=text name="hostname" size="25" maxlength="32" value="<% getCfgGeneral(1, "HostName"); %>" style="font-family: Arial; width:190px"></td>                  
</tr>
</table>
<!-- ================= PPPOE Mode ================= -->
                <table id="pppoe" width="620" border="1" cellpadding="2" cellspacing="1">                  
<tr>
                    <td class="title" colspan="2" id="wPppoeMode">PPPoE Mode</td>                  
</tr>
<tr>
                    <td class="head" id="wPppoeUser" width="170">User Name</td>                       
                    <td width="400">                      
                      <input name="pppoeUser" maxlength="32" size="25"              value="<% getCfgGeneral(1, "wan_pppoe_user"); %>" style="font-family: Arial; width:190px"></td>                  
</tr>
<tr>
                    <td class="head" id="wPppoePassword">Password</td>  <td>                      
                      <input type="password" name="pppoePass" maxlength="32" size="25"              value="<% getCfgGeneral(1, "wan_pppoe_pass"); %>" style="font-family: Arial; width:190px"></td>                  
</tr>
<tr>
                    <td class="head" id="wPppoePass2">Verify Password</td>  <td>                      
                      <input type="password" name="pppoePass2" maxlength="32" size="25"              value="<% getCfgGeneral(1, "wan_pppoe_pass"); %>" style="font-family: Arial; width:190px"></td>                  
</tr>
<tr>
                    <td class="head" rowspan="2" id="wPppoeOPMode">Operation Mode</td>  
					<td>                           
    <select name="pppoeOPMode" size="1" onChange="pppoeOPModeSwitch()">
                        <option value="KeepAlive" id="wPppoeKeepAlive">Keep Alive                         
                        </option>                               
                        <option value="OnDemand" id="wPppoeOnDemand">On Demand                         
                        </option>                               
                        <option value="Manual" id="wPppoeManual">Manual                         
                        </option>                           
                      </select>  
					 </td>                  
</tr>
                  <tr>
					<td>                           
                      <font id="wppoeKeepAliveMode">Keep Alive Mode: Redial Period                       
                      </font>                           
    <input type="text" name="pppoeRedialPeriod" maxlength="5" size="3" value="60">
                      <font id="wppoeKeepseconds">seconds                       
                      </font>                        
                      <font id="wppoeOnDemandMode">On demand Mode:  Idle Time                       
                      </font>                           
                      <input type="text" name="pppoeIdleTime" maxlength="3" size="3" value="5">                           
                      <font id="wppoeOnDemandMinutes">minutes                       
                      </font>
					</td>                  
</tr>
</table>
<!-- ================= PPTP Mode ================= -->
                <table id="pptp" width="620" border="1" cellpadding="2" cellspacing="1">                  
<tr>
                    <td class="title" colspan="2" id="wPptpMode">PPTP Mode</td>                  
</tr>
<tr>
                    <td class="head" id="wPptpServer" width="170">PPTP Server IP Address</td>                       
                    <td width="400">                      
                      <input name="pptpServer" maxlength="15" size="25" value="<% getCfgGeneral(1, "wan_pptp_server"); %>" style="font-family: Arial; width:190px"></td>                  
</tr>
<tr>
                    <td class="head" id="wPptpUser">User Name</td>  <td>                      
                      <input name="pptpUser" maxlength="20" size="25" value="<% getCfgGeneral(1, "wan_pptp_user"); %>" style="font-family: Arial; width:190px"></td>                  
</tr>
<tr>
                    <td class="head" id="wPptpPassword">Password</td>  <td>                      
                      <input type="password" name="pptpPass" maxlength="32" size="25" value="<% getCfgGeneral(1, "wan_pptp_pass"); %>" style="font-family: Arial; width:190px"></td>                  
</tr>
<tr>
                    <td class="head" id="wPptpAddrMode">Address Mode</td>  <td>                           
    <select name="pptpMode" size="1" onChange="pptpModeSwitch()">
                        <option value="0" id="wPptpAddrModeS">Static                         
                        </option>                               
                        <option value="1" id="wPptpAddrModeD">Dynamic                         
                        </option>                           
                      </select>  </td>                  
</tr>
<tr id="pptpIp">
                    <td class="head" id="wPptpIp">IP Address</td>  <td>                      
                      <input name="pptpIp" maxlength="15" size="25" value="<% getCfgGeneral(1, "wan_pptp_ip"); %>" style="font-family: Arial; width:190px"></td>                  
</tr>
<tr id="pptpNetmask">
                    <td class="head" id="wPptpNetmask">Subnet Mask</td>  <td>                      
                      <input name="pptpNetmask" maxlength="15" size="25" value="<% getCfgGeneral(1, "wan_pptp_netmask"); %>" style="font-family: Arial; width:190px">   </td>                  
</tr>
<tr id="pptpGateway">
                    <td class="head" id="wPptpGateway">Default Gateway</td>  <td>                      
                      <input name="pptpGateway" maxlength="15" size="25" value="<% getCfgGeneral(1, "wan_pptp_gateway"); %>" style="font-family: Arial; width:190px">   </td>                  
</tr>
<tr>
                    <td class="head" rowspan="3" id="wPptpOPMode">Operation Mode</td>  <td>                           
    <select name="pptpOPMode" size="1" onChange="pptpOPModeSwitch()">
                        <option value="KeepAlive" id="wPptpKeepAlive">Keep Alive                         
                        </option>                               
      <!--
      <option value="OnDemand" id="wPptpOnDemand">On Demand</option>
      -->
                        <option value="Manual" id="wPptpManual">Manual                         
                        </option>                           
                      </select>  </td>                  
</tr>
                  <tr>  <td>                           
                      <font id="wpptpKeepAliveMode">Keep Alive Mode: Redial Period                       
                      </font>                           
    <input type="text" name="pptpRedialPeriod" maxlength="5" size="3" value="60">
                      <font id="wpptpKeepseconds">seconds                       
                      </font>                           
    <!--
    <br />
                                                <font id="wpptpOnDemandMode">On demand Mode:  Idle Time</font>
    <input type="text" name="pptpIdleTime" maxlength="3" size="2" value="5">
                                                <font id="wpptpOnDemandMinutes">minutes</font>
                                                -->   </td>                  
</tr>
</table>
<!-- ================= L2TP Mode ================= -->
                <table id="l2tp" width="620" border="1" cellpadding="2" cellspacing="1">                  
<tr>
                    <td class="title" colspan="2" width="620" id="wL2tpMode">L2TP Mode</td>                  
</tr>
<tr>
                    <td class="head" id="wL2tpServer" width="170">L2TP Server IP Address</td>                       
                    <td width="400">                      
                      <input name="l2tpServer" maxlength="15" size="25" value="<% getCfgGeneral(1, "wan_l2tp_server"); %>" style="font-family: Arial; width:190px"></td>                  
</tr>
<tr>
                    <td class="head" id="wL2tpUser">User Name</td>  <td>                      
                      <input name="l2tpUser" maxlength="20" size="25" value="<% getCfgGeneral(1, "wan_l2tp_user"); %>" style="font-family: Arial; width:190px"></td>                  
</tr>
<tr>
                    <td class="head" id="wL2tpPassword">Password</td>  <td>                      
                      <input type="password" name="l2tpPass" maxlength="32" size="25" value="<% getCfgGeneral(1, "wan_l2tp_pass"); %>" style="font-family: Arial; width:190px"></td>                  
</tr>
<tr>
                    <td class="head" id="wL2tpAddrMode">Address Mode</td>  <td>                           
    <select name="l2tpMode" size="1" onChange="l2tpModeSwitch()">
                        <option value="0" id="wL2tpAddrModeS">Static                         
                        </option>                               
                        <option value="1" id="wL2tpAddrModeD">Dynamic                         
                        </option>                           
                      </select>  </td>                  
</tr>
<tr id="l2tpIp">
                    <td class="head" id="wL2tpIp">IP Address</td>  <td>                      
                      <input name="l2tpIp" maxlength="15" size="25" value="<% getCfgGeneral(1, "wan_l2tp_ip"); %>" style="font-family: Arial; width:190px"></td>                  
</tr>
<tr id="l2tpNetmask">
                    <td class="head" id="wL2tpNetmask">Subnet Mask</td>  <td>                      
                      <input name="l2tpNetmask" maxlength="15" size="25" value="<% getCfgGeneral(1, "wan_l2tp_netmask"); %>" style="font-family: Arial; width:190px">   </td>                  
</tr>
<tr id="l2tpGateway">
                    <td class="head" id="wL2tpGateway">Default Gateway</td>  <td>                      
                      <input name="l2tpGateway" maxlength="15" size="25" value="<% getCfgGeneral(1, "wan_l2tp_gateway"); %>" style="font-family: Arial; width:190px">   </td>                  
</tr>
<tr>
                    <td class="head" rowspan="3" id="wL2tpOPMode">Operation Mode</td>  <td>                           
    <select name="l2tpOPMode" size="1" onChange="l2tpOPModeSwitch()">
                        <option value="KeepAlive" id="wL2tpKeepAlive">Keep Alive                         
                        </option>                               
      <!--
      <option value="OnDemand" id="wL2tpOnDemand">On Demand</option>
      -->
                        <option value="Manual" id="wL2tpManual">Manual                         
                        </option>                           
                      </select>  </td>                  
</tr>
                  <tr>  <td>                           
                      <font id="wl2tpKeepAliveMode">Keep Alive Mode: Redial Period                       
                      </font>                           
    <input type="text" name="l2tpRedialPeriod" maxlength="5" size="3" value="60">
                      <font id="wl2tpKeepseconds">seconds                       
                      </font>                           
    <!--
    <br />
                                                <font id="wl2tpOnDemandMode">On demand Mode:  Idle Time</font>
    <input type="text" name="l2tpIdleTime" maxlength="3" size="2" value="5">
                                                <font id="wl2tpOnDemandMinutes">minutes</font>
                                                -->   </td>                  
</tr>
</table>
<!-- =========== 3G Modular =========== -->
                <!-- Marked by Maddux, 2009/09/17 -->                
                <!--
                                <table id="3G" width="620" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="w3GMode">3G Mode</td>
</tr>
<tr>
  <td class="head" rowspan="3" id="w3GDev">USB 3G modem</td>
  <td>
    <select name="Dev3G" size="1">
      <option value="MU-Q101" id="MU-Q101">NU MU-Q101</option>
      <option value="HUAWEI-E169" id="E169">HUAWEI E169</option>
      <option value="BandLuxe-C270" id="C270">BandLuxe C270</option>
      <option value="OPTION-ICON225" id="ICON225">OPTION ICON 225</option>
    </select>
  </td>
</tr>
</table>
                                -->                
                <!-- Maddux added on 2009/09/17 -->                
                <table width="620" border="1" cellpadding="2" cellspacing="1" id="3G">                  
                  <!--    <tr id="row_title_wan_3g" class="data_invisible"> -->                           
                  <tr>                               
                    <td width="620" colspan="2" class="title">3G</td>                           
                  </tr>                  
                  <!--
                                            <tr  class="data_invisible">
                                            <tr id="3g_conn_start_mode">
                                              <td class="head" id="row_wan_3g_box_buton" width="170">
                                                Start Mode
                                              </td>
                                              <td id="dynamic_wan_3g_box_button" width="400">
                                                  <select name="wan_3g_start_mode" size="1" onChange="wan_3gConnModeSwitch()">
                                                     <option value="0" id="data_Automatically">Automatically</option>
                                                     <option value="1" id="data_Manually">Manually</option>
                                                  </select>
                                              </td>
                                            </tr>
                                    -->                           
                  <tr class="data_invisible">                               
                    <td class="head" id="row_wan_3g_pincode_protect" width="170">              Pin Code Protect           </td>                               
                    <td width="400">                                     
                      <input type="checkbox" id="wan_3g_pincode_protect" name="wan_3g_pincode_protect" onClick="onPinCodeChecked()" value="1" <% getWan_3gpincode_protect1(); %> >                       
                      <span id="data_enable">Enable                       
                      </span>&nbsp; 			                         
                      <input type="password" id="wan_3g_pincode" name="wan_3g_pincode" size="10" readOnly="true" maxlength="8" onClick="inputPinCode();" onKeyDown="inputPinCode();" value="<% getCfgGeneral(1, "wan_3g_pincode"); %>">&nbsp;                                      
                      <font id="pincode_authentication">                        
                      </font>                      
                      
                                                          <!--  <% getWan_3g_setPincode_Error_Msg(); %>
                                                          <font id="pincode_authentication">3G pincode authenticated information</font>-->
                                                     </td>                           
                  </tr>                           
                  <tr class="data_invisible">                               
                    <td class="head" id="row_wan_3g_dialnumber" width="170">            Dial Code           </td>                               
                    <td class="opt_value">                                   
                      <input type="text" name="wan_3g_dialnumber" size="25" maxlength="64" value="<% getCfgGeneral(1, "wan_3g_dialnumber"); %>" style="font-family: Arial; width:190px">           </td>                           
                  </tr>                           
                  <tr  class="data_invisible">                               
                    <td class="head" id="row_wan_3g_apn" width="170">            APN Service           </td>                               
                    <td class="opt_value">                                   
                      <input type="text" name="wan_3g_apn" size="25" maxlength="64" value="<% getCfgGeneral(1, "wan_3g_apn"); %>" style="font-family: Arial; width:190px">           </td>                           
                  </tr>                           
                  <tr class="data_invisible">                               
                    <td class="head" id="row_wan_3g_username" width="170">            3G Username           </td>                               
                    <td class="opt_value">                                   
                      <input type="text" name="wan_3g_username" size="25" maxlength="64" value="<% getCfgGeneral(1, "wan_3g_username"); %>" style="font-family: Arial; width:190px">           </td>                           
                  </tr>                           
                  <tr class="data_invisible">                               
                    <td class="head"  id="row_wan_3g_password" width="170" >            3G Password           </td>                               
                    <td class="opt_value">                                   
                      <input type="password" name="wan_3g_password" size="25" maxlength="64" value="<% getCfgGeneral(1, "wan_3g_password"); %>" style="font-family: Arial; width:190px">           </td>                           
                  </tr>                           
                  <tr feature="3g_conn_mode_enable" class="data_invisible" id="3g_conn_type" style="visibility:hidden" >                               
                    <td class="head" id="row_wan_3g_connecttype" width="170" >            Connect Type           </td>                               
                    <td class="opt_value">                                 
                      <select size="1" name="wan_3g_connecttype" onChange="wan_3g_connecttype_change(document.wanCfg);">                                     
                        <option value="0" id="data_Keep_Alive">Keep Alive                         
                        </option>                                     
                        <!--<option value="1" id="data_Auto_Connect">Auto Connect</option>-->                                      
                        <option value="1" id="data_Auto_Connect">On demand Mode:  Idle Time                         
                        </option>                                 
                      </select>          </td>                           
                  </tr>                           
                  <tr feature="3g_conn_mode_enable" class="data_invisible" id="row_wan_3g_maxidletime" >                               
                    <td class="head"  id="data_wan_3g_maxidletime" width="170" >            3G Max Idle Time           </td>                               
                    <td class="opt_value">			                       
                      <input type="text" name="wan_3g_maxidletime" size="25" value="<% getCfgGeneral(1, "wan_3g_maxidletime"); %>" style="font-family: Arial; width:190px">                       
                      <span id="data_seconds"> seconds.                       
                      </span>          </td>                           
                  </tr>                           
                  <tr feature="3G_budget_control_enable"  class="data_invisible" >                               
                    <td class="head" id="row_wan_3G_budget_control_enable" width="170">            Budget Control           </td>          <td>                                   
                      <input type="checkbox" name="wan_3g_budget_control_enable" id="wan_3g_budget_control_enable" value="1" <% getWan_3g_budget_control_enable1();%>                        onclick="wan_3g_budget_control_enable_change(document.wanCfg);">                                    
                      <font id="wan_3g_budget_control_enable_text">Enable                       
                      </font>          </td>                           
                  </tr>                  
                  <!--  Budget Scheme -->                  
                  <!-- ======================================================================================================== -->                           
                  <tr class="data_invisible" id="Budget_ctrl_table">                               
                    <td class="head" id="row_wan_3g_Budget_Scheme" >            Budget Criterion           </td>          <td>                                         
                      <table>                        
                        <!--Time Budget-->                                             
                        <tr>                                                       
                          <td class="head">                                                           
                            <input type="checkbox" name="wan_3g_time_budget_enable" id="wan_3g_time_budget_enable"                                         value="<% getWan_3g_time_budget_enable1();%>" onClick="disen_chg('wan_3g_time_budget_enable', 'wan_3g_time_limit'); disen_chg('wan_3g_time_budget_enable', 'wan_3g_max_online_monthly'); chk_disen_chg(); " >                             
                            <span id="data_Time_budget">                                Time Budget                             
                            </span>(1~999)                            
						  </td>                                                       
                          <td class="opt_value">							                               
                            <input type="text" name="wan_3g_max_online_monthly" maxlength="3" size="4" id="wan_3g_max_online_monthly" value="<% getCfgGeneral(1, "wan_3g_max_online_monthly"); %>"  >                                                            
                            <font id="data_hours_PM_budget">hour(s) per month                             
                            </font>                            
						  </td>                       
                          <!--Data Budget-->                                             
                        </tr>                                             
                        <tr>                                                 
                          <td class="head">                                                     
                            <input type="checkbox" name="wan_3g_data_budget_enable" id="wan_3g_data_budget_enable" value="<% getWan_3g_data_budget_enable1(); %>" onClick="disen_chg('wan_3g_data_budget_enable', 'wan_3g_data_limit'); disen_chg('wan_3g_data_budget_enable', 'wan_3g_max_xfer_monthly');  disen_chg('wan_3g_data_budget_enable', 'wan_3g_up_down_stream_limit'); chk_disen_chg(); " >                             
                            <font id="data_Data_Budge">                          Data Budget                             
                            </font>(3~4000)                      
						  </td>                                                 
                          <td class="opt_value">                                                     
                            <input type="text" name="wan_3g_max_xfer_monthly" maxlength="4" size="4" id="wan_3g_max_xfer_monthly" value="<% getWan_3g_max_xfer_monthly(); %>" >
                            <span id="data_MByte_PM">MByte(s) per month</span><br>                            			                             
                            <!--<span id="data_MByte_range">(50~1000 MBytes)</span>-->
						<!--</td>                                               
                        <td class="opt_value"> -->                                                    
                            <select name="wan_3g_up_down_stream_limit" size="1" id="wan_3g_up_down_stream_limit">                                                       
                              <option value="0" id="data_download">Download</option>                                                       
                              <option value="1" id="data_upload">Upload</option>                                                       
                              <option value="2" id="data_UDload">Download/Upload</option>                                                     
                            </select>                            
                            <!--
                                                                                <font id="per_month">per month</font>
                                                        -->    			                             
						</td>                                             
                        </tr>                                         
                      </table>          </td>                           
                  </tr>                  
                  <!-- =====================================================Budget Policy=================================================== -->                           
                  <tr  class="data_invisible"  id="Budget_ctrl_policy" >                               
                    <td class="head" id="row_wan_3g_budget_policy">            Budget Policy           </td>          <td>                                   
                      <table>                                       
                        <tr>                                           
                          <td colspan="4" class="head">                                                     
                            <font id="data_over_bdg">Actions if Over Budget                             
                            </font>&nbsp;                                                      
                            <font id="data_over_bdg_time_msg">                            
                            </font>&nbsp;                                                      
                            <font id="data_over_bdg_data_msg">                            
                            </font>                            
                            
                                                                        <!--<td colspan="4" class="head" id="data_over_bdg">
                                                                          Actions if Over Budget&nbsp;
                                                                          <% getWan_3g_over_time_budget_msg(); %>   <% getWan_3g_over_data_budget_msg(); %>
                                                                       </td> -->                                       
                        </tr>                                       
                        <tr>                          
                          
                                                                   <!-- <td class="opt_value">
                                                    	          <input type="checkbox" name="wan_3g_policy_drop" size="10" value="1" defaultChecked="true" id="data_drop_con">
                                                    			&nbsp;<font id="data_drop_con_text">Drop Current Connection</font>
                                                    	        </td>      
                                                    	        <td class="opt_value">
                                                    	          <input type="checkbox" name="wan_3g_policy_disallow" size="10" value="1" defaultChecked="true" id="data_disallow">
                                                    			&nbsp;<font id="data_disallow_text">Disallow New Connection</font>
                                                    	        </td> -->
                                                    	                                   
                          <td class="opt_value">                                               
                            <input type="checkbox" name="wan_3g_policy_drop" size="3" value="1" <% getWan_3g_policy_drop1(); %>  id="data_drop_con">                         &nbsp;                             
                            <font id="data_drop_con_text">Drop Current Connection                             
                            </font>                </td>                                           
                          <td class="opt_value">                                               
                            <input type="checkbox" name="wan_3g_policy_disallow" size="3" value="1" <% getWan_3g_policy_disallow1(); %>  id="data_disallow">                         &nbsp;                             
                            <font id="data_disallow_text">Disallow  New Connection                             
                            </font>                </td>                                       
                        </tr>                                       
                        <tr>                                           
                          <td colspan="4" class="head" id="data_trigger_by_limit">                                                     
                            <font id="data_trigger_by_limit_msg">Trigger by Limit Budget                             
                            </font>&nbsp;                                                      
                            <font id="prelimit_time_budget_msg">                            
                            </font>&nbsp;                                                      
                            <font id="prelimit_data_budget_msg">                            
                            </font>                            
                           
                        <!--Trigger by Limit Budget&nbsp; -->
                                                                          <% getWan_3g_prelimit_time_budget_msg(); %> <% getWan_3g_prelimit_data_budget_msg(); %>
                                                                       </td>                                       
                        </tr>                                       
                        <tr>                                           
                          <td class="opt_value">					                               
		<input type="text" id="wan_3g_time_limit" name="wan_3g_time_limit" maxlength="2" size="2" value="<% getCfgGeneral(1, "wan_3g_time_limit"); %>" >                         &nbsp;                             
                            <font id="data_of_the_time_bgt_text">% of the time budget                             
                            </font>                </td>                                           
                          <td class="opt_value">                                               
		<input type="text" id="wan_3g_data_limit" name="wan_3g_data_limit" maxlength="2" size="2" value="<% getCfgGeneral(1, "wan_3g_data_limit"); %>" >                         &nbsp;                             
                            <font id="data_of_the_data_bgt_text">% of the data budget                             
                            </font>                </td>                                       
                        </tr>                        
                        
                                                              <tr>
                                                                <td class="opt_value">
                                                                  <input type="checkbox" name="wan_3g_policy_alert" value="1" <% getWan_3g_policy_alert1();%> id="wan_3g_policy_alert"
                                                                        onclick="wan_3g_policy_alert_change(document.wanCfg); disen_chg('wan_3g_policy_alert', 'wan_3g_mail_sent_period')">
                                                                        &nbsp;<font id="email_alert_head">E-mail Alert</font>
                                                                  <td class="opt_value">
                                                                        &nbsp;<font id="email_alert_body_text">will recur every</font>
                                                                      <input type="text" id="wan_3g_mail_sent_period" name="wan_3g_mail_sent_period" maxlength="4" size="2" value="<% getCfgGeneral(1, "wan_3g_mail_sent_period"); %>">
                                                                        &nbsp;<font id="email_alert_unit">minutes</font>
                                                                  </td>
                                                                </td>
                                                              </tr>
                                                                                   
                      </table>          </td>                           
                  </tr>                  
                  <!-- ======================================================================================================== -->                  
                  
                                            <tr class="data_invisible"  id="Budget_ctrl_counter">
                                              <td class="head" id="row_wan_3g_cycle_date" >
                                                Budget Counter
                                              </td>
                                              <td class="opt_value">
                                                <font id="data_reset_on">Reset on</font>
                                                        <select size="1" name="wan_3g_cycle_date" id="wan_3g_cycle_date" >
                                                        <option value="0"  id="data_id1_0">1st day per month</option>
                                                        <option value="1"  id="data_id1_1">2nd day per month</option>
                                                        <option value="2"  id="data_id1_2">3rd day per month</option>
                                                        <option value="3"  id="data_id1_3">4th day per month</option>
                                                        <option value="4"  id="data_id1_4">5th day per month</option>
                                                        <option value="5"  id="data_id1_5">6th day per month</option>
                                                        <option value="6"  id="data_id1_6">7th day per month</option>
                                                        <option value="7"  id="data_id1_7">8th day per month</option>
                                                        <option value="8"  id="data_id1_8">9th day per month</option>
                                                        <option value="9"  id="data_id1_9">10th day per month</option>
                                                        <option value="10" id="data_id1_10">11th day per month</option>
                                                        <option value="11" id="data_id1_11">12th day per month</option>
                                                        <option value="12" id="data_id1_12">13th day per month</option>
                                                        <option value="13" id="data_id1_13">14th day per month</option>
                                                        <option value="14" id="data_id1_14">15th day per month</option>
                                                        <option value="15" id="data_id1_15">16th day per month</option>
                                                        <option value="16" id="data_id1_16">17th day per month</option>
                                                        <option value="17" id="data_id1_17">18th day per month</option>
                                                        <option value="18" id="data_id1_18">19th day per month</option>
                                                        <option value="19" id="data_id1_19">20th day per month</option>
                                                        <option value="20" id="data_id1_20">21st day per month</option>
                                                        <option value="21" id="data_id1_21">22ed day per month</option>
                                                        <option value="22" id="data_id1_22">23rd day per month</option>
                                                        <option value="23" id="data_id1_23">24th day per month</option>
                                                        <option value="24" id="data_id1_24">25th day per month</option>
                                                        <option value="25" id="data_id1_25">26th day per month</option>
                                                        <option value="26" id="data_id1_26">27th day per month</option>
                                                        <option value="27" id="data_id1_27">28th day per month</option>
                                                        <option value="28" id="data_id1_28">29th day per month</option>
                                                        <option value="29" id="data_id1_29">30th day per month</option>
                                                        <option value="30" id="data_id1_30">31st day per month</option>
                                                      </select>
			&nbsp;<font id="data_reset_on_msg">If not over budget.</font>
                                              </td>
                                            </tr>
                                                     
                  <!-- =============================================================data_smtp_authen=========================================== -->                  
                  
                                            <tr class="data_invisible" id="id_eamil_auth">
                                              <td class="head" id="row_wan_3g_budget_mail" >
                                                E-mail Settings
                                              </td>
                                              <td>
                                                <table id="id_eamil_auth_table">
                                                        <tr id="row_wan_3g_smtp_auth_enable" class="data_invisible">
                                                          <td class="opt_value">
                                                              <font  id="data_smtp_authen">SMTP Authentication</font>
                                                                  <td class="opt_value">
                                                                    <select size="1" name="wan_3g_smtp_auth_enable" id="wan_3g_smtp_auth_enable" onChange="wan_3g_smtp_auth_change(document.wanCfg);" >
                                                                    <option value="0"  id="data_plain">PLAIN</option>
                                                                    <option value="1"  id="data_login">LOGIN</option>
                                                                    <option value="2"  id="data_disabled">Disabled</option>
                                                                    </select>
                                                                  </td>
                                                         </td>
                                                        </tr>
                                                        <tr class="data_invisible" id="row_wan_3g_smtp_username">
                                                          <td class="opt_name" id="row_wan_3g_mail_username">
                                                                    Username
                                                          </td>
                                                          <td class="opt_value">
					<input type="text" name="wan_3g_smtp_username" id="wan_3g_smtp_username"  value="<% getCfgGeneral(1, "wan_3g_smtp_username"); %>">
                                                          </td>
                                                        </tr>
                                                        <tr class="data_invisible" id="row_wan_3g_smtp_password">
                                                          <td class="opt_name" id="row_wan_3g_mail_password">
                                                            Password
                                                          </td>
                                                          <td class="opt_value">
					<input type="password" name="wan_3g_smtp_passwd" id="wan_3g_smtp_passwd"  value="<% getCfgGeneral(1, "wan_3g_smtp_passwd"); %>">
                                                          </td>
                                                        </tr>
                                                        <tr class="data_invisible">
                                                          <td class="opt_name" id="row_wan_3g_mail_server">
                                                            Mail Server
                                                          </td>
                                                          <td class="opt_value">
					<input type="text" name="wan_3g_mail_server" id="wan_3g_mail_server" maxlength="64" value="<% getCfgGeneral(1, "wan_3g_mail_server"); %>">
                                                          </td>
                                                        </tr>
                                                        <tr class="data_invisible">
                                                          <td class="opt_name" id="row_wan_3g_mail_sender">
                                                            Mail Sender
                                                          </td>
                                                          <td class="opt_value">
					<input type="text" name="wan_3g_mail_sender" id="wan_3g_mail_sender" value="<% getCfgGeneral(1, "wan_3g_mail_sender"); %>">
                                                          </td>
                                                        </tr>
                                                        <tr class="data_invisible">
                                                          <td class="opt_name" id="row_wan_3g_mail_recipient">
                                                            Mail Recipient
                                                          </td>
                                                          <td class="opt_value">
					<input type="text" name="wan_3g_mail_recipient" id="wan_3g_mail_recipient" value="<% getCfgGeneral(1, "wan_3g_mail_recipient"); %>">
                                                          </td>
                                                        </tr>
                                                </table>
                                              </td>
                                            </tr>
                                                    
                </table>                
<!-- =========== DNS setting by customer =========== -->
                <table id="dns_customer" width="620" border="1" cellpadding="2" cellspacing="1">                  
<tr>
                    <td class="title" colspan="2" id="wdns_customer">DNS Settings (Optional)</td>                  
</tr>
<tr>
                    <td class="head" id="wDhcpPriDns" width="170" >Primary DNS Server</td>                       
                    <td width="400">                         
                      <input name="DhcpPriDns" maxlength="15" value="<% getCfgGeneral(1, "wan_primary_dns"); %>" size="20"></td>                  
</tr>
<tr>
                    <td class="head" id="wDhcpSecDns">Secondary DNS Server</td>  <td>                         
                      <input name="DhcpSecDns" maxlength="15" value="<% getCfgGeneral(1, "wan_secondary_dns"); %>" size="20"></td>                  
</tr>
</table>
                                
<br>
                <table width="620" cellpadding="2" cellspacing="1">                  
                  <tr id="sbutton0" align="center">  <td>                           
	    <input type="hidden" name="goform_next" value="/wizard/wizard_security.asp">
        <input type="button" style="{width:120px;}" value="Previous" id="wPrevious" onClick="setLocation(); parent.web_page_selected(1, 2);"> &nbsp; &nbsp;
    <input type="button" style="{width:120px;}" value="Apply" id="wApply" onClick="onClicksbuttonMax();">&nbsp;&nbsp;
                      <input type="reset"  style="{width:120px;}" value="Cancel" id="wCancel" onClick="window.location.reload();">  </td>                  
                  </tr>                
                </table>              
              </form></td>          
</tr>
</table>
 </center>
</div>
</body>
</html>
