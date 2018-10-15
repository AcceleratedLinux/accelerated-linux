<html>
<head>
<title>Local Area Network (LAN) Settings</title>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script type="text/javascript" src="/common.js"></script>

<script language="JavaScript" type="text/javascript">
restartPage_init();

var sbuttonMax=1;

Butterlate.setTextDomain("internet");
var lan2 = '<% getCfgZero(1, "Lan2Enabled"); %>';

var secs
var timerID = null
var timerRunning = false
function StartTheTimer(){
	if (secs==0){
		TimeoutReload(5);
		//window.location.reload();
		window.location.href=window.location.href;	//reload page
    }else{
        self.status = secs
        secs = secs - 1
        timerRunning = true
        timerID = self.setTimeout("StartTheTimer()", 1000)
    }
}

function TimeoutReload(timeout)
{
	secs = timeout;
	if(timerRunning)
		clearTimeout(timerID)
	timerRunning = false
	StartTheTimer();	
}

function display_on()
{
	if (window.ActiveXObject) { // IE
		return "block";
	}
	else if (window.XMLHttpRequest) { // Mozilla, Firefox, Safari,...
		return "table-row";
	}
}

function dhcpTypeSwitch()
{
	var a_lease_time = "<% getCfgGeneral(1, "dhcpLease"); %>";
	
	document.getElementById("start").style.visibility = "hidden";
	document.getElementById("start").style.display = "none";
	document.lanCfg.dhcpStart.disabled = true;
	document.getElementById("end").style.visibility = "hidden";
	document.getElementById("end").style.display = "none";
	document.lanCfg.dhcpEnd.disabled = true;
	document.getElementById("mask").style.visibility = "hidden";
	document.getElementById("mask").style.display = "none";
	document.lanCfg.dhcpMask.disabled = true;
	document.getElementById("pridns").style.visibility = "hidden";
	document.getElementById("pridns").style.display = "none";
	document.lanCfg.dhcpPriDns.disabled = true;
	document.getElementById("secdns").style.visibility = "hidden";
	document.getElementById("secdns").style.display = "none";
	document.lanCfg.dhcpSecDns.disabled = true;
	document.getElementById("gateway").style.visibility = "hidden";
	document.getElementById("gateway").style.display = "none";
	document.lanCfg.dhcpGateway.disabled = true;
	document.getElementById("lease").style.visibility = "hidden";
	document.getElementById("lease").style.display = "none";
	document.lanCfg.dhcpLease.disabled = true;
	document.getElementById("staticlease1").style.visibility = "hidden";
	document.getElementById("staticlease1").style.display = "none";
	document.lanCfg.dhcpStatic1Mac.disabled = true;
	document.lanCfg.dhcpStatic1Ip.disabled = true;
	document.getElementById("staticlease2").style.visibility = "hidden";
	document.getElementById("staticlease2").style.display = "none";
	document.lanCfg.dhcpStatic2Mac.disabled = true;
	document.lanCfg.dhcpStatic2Ip.disabled = true;
	document.getElementById("staticlease3").style.visibility = "hidden";
	document.getElementById("staticlease3").style.display = "none";
	document.lanCfg.dhcpStatic3Mac.disabled = true;
	document.lanCfg.dhcpStatic3Ip.disabled = true;

/* Disable LAN2 - Start [Yian,2009/06/17] */
	document.getElementById("lLan2").style.visibility = "hidden";
	document.getElementById("lLan2").style.display = "none";

	document.getElementById("lLan2_Radio").style.visibility = "hidden";
	document.getElementById("lLan2_Radio").style.display = "none";

	document.getElementById("lLan2Ip").style.visibility = "hidden";
	document.getElementById("lLan2Ip").style.display = "none";

	document.getElementById("lLan2Ip_input").style.visibility = "hidden";
	document.getElementById("lLan2Ip_input").style.display = "none";

	document.getElementById("lLan2Netmask").style.visibility = "hidden";
	document.getElementById("lLan2Netmask").style.display = "none";

	document.getElementById("lLan2Netmask_input").style.visibility = "hidden";
	document.getElementById("lLan2Netmask_input").style.display = "none";

/* Disable LAN2 - End [Yian,2009/06/17] */

	if (document.lanCfg.lanDhcpType.options.selectedIndex == 1)
	{
		document.getElementById("start").style.visibility = "visible";
		document.getElementById("start").style.display = display_on();
		document.lanCfg.dhcpStart.disabled = false;
		document.getElementById("end").style.visibility = "visible";
		document.getElementById("end").style.display = display_on();
		document.lanCfg.dhcpEnd.disabled = false;
		/*document.getElementById("mask").style.visibility = "visible";
		document.getElementById("mask").style.display = display_on();*/
		document.lanCfg.dhcpMask.disabled = false;
		
		if(	!document.lanCfg.dnspEnbl.options.selectedIndex ){
			document.getElementById("pridns").style.visibility = "visible";
			document.getElementById("pridns").style.display = display_on();
			document.lanCfg.dhcpPriDns.disabled = false;
			document.getElementById("secdns").style.visibility = "visible";
			document.getElementById("secdns").style.display = display_on();
			document.lanCfg.dhcpSecDns.disabled = false;
		}
		/*document.getElementById("gateway").style.visibility = "visible";
		document.getElementById("gateway").style.display = display_on();*/
		document.lanCfg.dhcpGateway.disabled = false;
		document.getElementById("lease").style.visibility = "visible";
		document.getElementById("lease").style.display = display_on();
		document.lanCfg.dhcpLease.disabled = false;
		/*document.getElementById("staticlease1").style.visibility = "visible";
		document.getElementById("staticlease1").style.display = display_on();
		document.lanCfg.dhcpStatic1Mac.disabled = false;
		document.lanCfg.dhcpStatic1Ip.disabled = false;
		document.getElementById("staticlease2").style.visibility = "visible";
		document.getElementById("staticlease2").style.display = display_on();
		document.lanCfg.dhcpStatic2Mac.disabled = false;
		document.lanCfg.dhcpStatic2Ip.disabled = false;
		document.getElementById("staticlease3").style.visibility = "visible";
		document.getElementById("staticlease3").style.display = display_on();
		document.lanCfg.dhcpStatic3Mac.disabled = false;
		document.lanCfg.dhcpStatic3Ip.disabled = false;*/
		
		document.lanCfg.dhcpLease.options[0].selected =  ( a_lease_time =="1800");
	  	document.lanCfg.dhcpLease.options[1].selected =  ( a_lease_time =="3600");
  		document.lanCfg.dhcpLease.options[2].selected =  ( a_lease_time =="7200");
	  	document.lanCfg.dhcpLease.options[3].selected =  ( a_lease_time =="43200");
	  	document.lanCfg.dhcpLease.options[4].selected =  ( a_lease_time =="86400");
	  	document.lanCfg.dhcpLease.options[5].selected =  ( a_lease_time =="172800");
		document.lanCfg.dhcpLease.options[6].selected =  ( a_lease_time =="604800");
		document.lanCfg.dhcpLease.options[7].selected =  ( a_lease_time =="1209600");
		document.lanCfg.dhcpLease.options[8].selected =  ( a_lease_time =="62208000");
	}
}

function initTranslation()
{
	var e = document.getElementById("lTitle");
	e.innerHTML = _("lan title");
	e = document.getElementById("lIntroduction");
	e.innerHTML = _("lan introduction")+"<br>";

	e = document.getElementById("lSetup");
	e.innerHTML = _("lan setup");
	e = document.getElementById("lHostname");
	e.innerHTML = _("inet hostname");
	e = document.getElementById("lIp");
	e.innerHTML = _("inet ip");
	e = document.getElementById("lNetmask");
	e.innerHTML = _("inet netmask");
	e = document.getElementById("lLan2");
	e.innerHTML = _("inet lan2");
	e = document.getElementById("lLan2Enable");
	e.innerHTML = _("inet enable");
	e = document.getElementById("lLan2Disable");
	e.innerHTML = _("inet disable");
	e = document.getElementById("lLan2Ip");
	e.innerHTML = _("inet lan2 ip");
	e = document.getElementById("lLan2Netmask");
	e.innerHTML = _("inet lan2 netmask");
	e = document.getElementById("lGateway");
	e.innerHTML = _("inet gateway");
	e = document.getElementById("lPriDns");
	e.innerHTML = _("inet pri dns");
	e = document.getElementById("lSecDns");
	e.innerHTML = _("inet sec dns");
	e = document.getElementById("lMac");
	e.innerHTML = _("inet mac");

	e = document.getElementById("lDhcpSetup");
	e.innerHTML = _("lan dhcp setup");
	e = document.getElementById("lDhcpType");
	e.innerHTML = _("lan dhcp type");
	e = document.getElementById("lDhcpTypeD");
	e.innerHTML = _("inet disable");
	e = document.getElementById("lDhcpTypeS");
	e.innerHTML = _("lan dhcp type server");
	e = document.getElementById("lDhcpStart");
	e.innerHTML = _("lan dhcp start");
	e = document.getElementById("lDhcpEnd");
	e.innerHTML = _("lan dhcp end");
	e = document.getElementById("lDhcpNetmask");
	e.innerHTML = _("inet netmask");
	e = document.getElementById("lDhcpPriDns");
	e.innerHTML = _("inet pri dns");
	e = document.getElementById("lDhcpSecDns");
	e.innerHTML = _("inet sec dns");
	e = document.getElementById("lDhcpGateway");
	e.innerHTML = _("inet gateway");
	e = document.getElementById("lDhcpLease");
	e.innerHTML = _("lan dhcp lease");
	e = document.getElementById("lDhcpStatic1");
	e.innerHTML = _("lan dhcp static");
	e = document.getElementById("lDhcpStatic2");
	e.innerHTML = _("lan dhcp static");
	e = document.getElementById("lDhcpStatic3");
	e.innerHTML = _("lan dhcp static");
	//minglin, 2010/01/04
	e = document.getElementById("dhcpLease").options[0].innerHTML = _("dhcpLease Half hour");
	e = document.getElementById("dhcpLease").options[1].innerHTML = _("dhcpLease One hour");
	e = document.getElementById("dhcpLease").options[2].innerHTML = _("dhcpLease Two hours");
	e = document.getElementById("dhcpLease").options[3].innerHTML = _("dhcpLease Half day");
	e = document.getElementById("dhcpLease").options[4].innerHTML = _("dhcpLease One day");
	e = document.getElementById("dhcpLease").options[5].innerHTML = _("dhcpLease Two days");
	e = document.getElementById("dhcpLease").options[6].innerHTML = _("dhcpLease One week");
	e = document.getElementById("dhcpLease").options[7].innerHTML = _("dhcpLease Two weeks");
	e = document.getElementById("dhcpLease").options[8].innerHTML = _("dhcpLease Forever");
	//minglin, 2010/01/04
	e = document.getElementById("lDhcpOtherSetup");
	e.innerHTML = _("lan dhcp other setup");
	
	e = document.getElementById("lStp");
	e.innerHTML = _("lan stp");
	e = document.getElementById("lStpD");
	e.innerHTML = _("inet disable");
	e = document.getElementById("lStpE");
	e.innerHTML = _("inet enable");

	e = document.getElementById("lLltd");
	e.innerHTML = _("lan lltd");
	e = document.getElementById("lLltdD");
	e.innerHTML = _("inet disable");
	e = document.getElementById("lLltdE");
	e.innerHTML = _("inet enable");

	e = document.getElementById("lIgmpp");
	e.innerHTML = _("lan igmpp");
	e = document.getElementById("lIgmppD");
	e.innerHTML = _("inet disable");
	e = document.getElementById("lIgmppE");
	e.innerHTML = _("inet enable");

	e = document.getElementById("lUpnp");
	e.innerHTML = _("lan upnp");
	e = document.getElementById("lUpnpD");
	e.innerHTML = _("inet disable");
	e = document.getElementById("lUpnpE");
	e.innerHTML = _("inet enable");

	e = document.getElementById("lRadvd");
	e.innerHTML = _("lan radvd");
	e = document.getElementById("lRadvdD");
	e.innerHTML = _("inet disable");
	e = document.getElementById("lRadvdE");
	e.innerHTML = _("inet enable");

	e = document.getElementById("lPppoer");
	e.innerHTML = _("lan pppoer");
	e = document.getElementById("lPppoerD");
	e.innerHTML = _("inet disable");
	e = document.getElementById("lPppoerE");
	e.innerHTML = _("inet enable");

	e = document.getElementById("lDnsp");
	e.innerHTML = _("lan dnsp");
	e = document.getElementById("lDnspD");
	e.innerHTML = _("inet disable");
	e = document.getElementById("lDnspE");
	e.innerHTML = _("inet enable");

	e = document.getElementById("lApply");
	e.value = _("inet apply");
	e = document.getElementById("lCancel");
	e.value = _("inet cancel");
}

function initValue()
{
	var opmode = "<% getCfgZero(1, "OperationMode"); %>";
	var dhcp = <% getCfgZero(1, "dhcpEnabled"); %>;
	var stp = <% getCfgZero(1, "stpEnabled"); %>;
	var lltd = <% getCfgZero(1, "lltdEnabled"); %>;
	var igmp = <% getCfgZero(1, "igmpEnabled"); %>;
	var upnp = <% getCfgZero(1, "upnpEnabled"); %>;
	var radvd = <% getCfgZero(1, "radvdEnabled"); %>;
	var pppoe = <% getCfgZero(1, "pppoeREnabled"); %>;
	var dns = <% getCfgZero(1, "dnsPEnabled"); %>;
	var wan = "<% getCfgZero(1, "wanConnectionMode"); %>";
	var lltdb = "<% getLltdBuilt(); %>";
	var igmpb = "<% getIgmpProxyBuilt(); %>";
	var upnpb = "<% getUpnpBuilt(); %>";
	var radvdb = "<% getRadvdBuilt(); %>";
	var pppoeb = "<% getPppoeRelayBuilt(); %>";
	var dnsp = "<% getDnsmasqBuilt(); %>";

	initTranslation();

	if (lan2 == "1")
	{
		var lan2_ip = '<% getCfgGeneral(1, "lan2_ipaddr"); %>';
		var lan2_nm = '<% getCfgGeneral(1, "lan2_netmask"); %>';

		document.lanCfg.lan2enabled[0].checked = true;
		document.lanCfg.lan2Ip.disabled = false;
		document.lanCfg.lan2Ip.value = lan2_ip;
		document.lanCfg.lan2Netmask.disabled = false;
		document.lanCfg.lan2Netmask.value = lan2_nm;
	}
	else
	{
		document.lanCfg.lan2enabled[1].checked = true;
		document.lanCfg.lan2Ip.disabled = true;
		document.lanCfg.lan2Netmask.disabled = true;
	}


	document.lanCfg.lanDhcpType.options.selectedIndex = 1*dhcp;
	dhcpTypeSwitch();
	document.lanCfg.stpEnbl.options.selectedIndex = 1*stp;
	document.lanCfg.lltdEnbl.options.selectedIndex = 1*lltd;
	document.lanCfg.igmpEnbl.options.selectedIndex = 1*igmp;
	document.lanCfg.upnpEnbl.options.selectedIndex = 1*upnp;
	document.lanCfg.radvdEnbl.options.selectedIndex = 1*radvd;
	document.lanCfg.pppoeREnbl.options.selectedIndex = 1*pppoe;
	document.lanCfg.dnspEnbl.options.selectedIndex = 1*dns;

	//gateway, dns only allow to configure at bridge mode
	if (opmode != "0") {
		document.getElementById("brGateway").style.visibility = "hidden";
		document.getElementById("brGateway").style.display = "none";
		document.getElementById("brPriDns").style.visibility = "hidden";
		document.getElementById("brPriDns").style.display = "none";
		document.getElementById("brSecDns").style.visibility = "hidden";
		document.getElementById("brSecDns").style.display = "none";
	}
	if( opmode == 0 ) // opmode == 0
	{	//hidden igmp Proxy select box
		document.getElementById("igmpProxy").style.visibility = "hidden";
		document.getElementById("igmpProxy").style.display = "none";
		//hidden radvd select box
		document.getElementById("radvd").style.visibility = "hidden";
		document.getElementById("radvd").style.display = "none";
		//hidden pppoe relay select box
		document.getElementById("pppoerelay").style.visibility = "hidden";
		document.getElementById("pppoerelay").style.display = "none";
		//hidden dns Proxy select box
		document.getElementById("dnsproxy").style.visibility = "hidden";
		document.getElementById("dnsproxy").style.display = "none";
	}
	/* ppp0 is not a disabled interface anymore..
	if (wan == "PPPOE" || wan == "L2TP" || wan == "PPTP") {
		document.getElementById("igmpProxy").style.visibility = "hidden";
		document.getElementById("igmpProxy").style.display = "none";
	}
	else {
		document.getElementById("igmpProxy").style.visibility = "visible";
		document.getElementById("igmpProxy").style.display = display_on();
	}
	*/

	if (lltdb == "0") {
		document.getElementById("lltd").style.visibility = "hidden";
		document.getElementById("lltd").style.display = "none";
		document.lanCfg.lltdEnbl.options.selectedIndex = 0;
	}
	if (igmpb == "0") {
		document.getElementById("igmpProxy").style.visibility = "hidden";
		document.getElementById("igmpProxy").style.display = "none";
		document.lanCfg.igmpEnbl.options.selectedIndex = 0;
	}
	if (upnpb == "0") {
		document.getElementById("upnp").style.visibility = "hidden";
		document.getElementById("upnp").style.display = "none";
		document.lanCfg.upnpEnbl.options.selectedIndex = 0;
	}
	if (radvdb == "0") {
		document.getElementById("radvd").style.visibility = "hidden";
		document.getElementById("radvd").style.display = "none";
		document.lanCfg.radvdEnbl.options.selectedIndex = 0;
	}
	if (pppoeb == "0") {
		document.getElementById("pppoerelay").style.visibility = "hidden";
		document.getElementById("pppoerelay").style.display = "none";
		document.lanCfg.pppoeREnbl.options.selectedIndex = 0;
	}
	if (dnsp == "0") {
		document.getElementById("dnsproxy").style.visibility = "hidden";
		document.getElementById("dnsproxy").style.display = "none";
		document.lanCfg.dnspEnbl.options.selectedIndex = 0;
	}
	
	if (dns == "0")
	{
		document.getElementById("pridns").style.visibility = "visible";
		document.getElementById("pridns").style.display = display_on();
		document.lanCfg.dhcpPriDns.disabled = false;
		document.getElementById("secdns").style.visibility = "visible";
		document.getElementById("secdns").style.display = display_on();
		document.lanCfg.dhcpSecDns.disabled = false;
	}
	else
	{
		document.getElementById("pridns").style.visibility = "hidden";
		document.getElementById("pridns").style.display = "none";
		/*document.lanCfg.dhcpPriDns.disabled = true;*/
		document.getElementById("secdns").style.visibility = "hidden";
		document.getElementById("secdns").style.display = "none";
		/*document.lanCfg.dhcpSecDns.disabled = true;*/
	}
	/* hidden dns proxy and Router Adm.(radvd), minglin, 2010/01/14 *Start*/
	document.getElementById("radvd").style.visibility = "hidden";
	document.getElementById("radvd").style.display = "none";
	document.getElementById("dnsproxy").style.visibility = "hidden";
	document.getElementById("dnsproxy").style.display = "none";
	/* hidden dns proxy and Router Adm.(radvd), minglin, 2010/01/14 *end*/
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

function checkIpAddr(field, ismask)
{
	var cols = field.value.split('.');
	if (cols.length != 4) {
		alert(_("lan Error Not a legal IP address"));
		field.focus();
		return false;
	}

	if (field.value == "") {
		alert(_("lan Error IP address is empty"));
		field.value = field.defaultValue;
		field.focus();
		return false;
	}

	if (isAllNum(field.value) == 0) {
		alert(_('lan It should be a [0-9] number'));
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
			alert(_('lan IP adress format error'));
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
			alert(_('lan IP adress format error'));
			field.value = field.defaultValue;
			field.focus();
			return false;
		}
	}
	return true;
}

function FormatNum(Source,Length){  
	var strTemp="";  
	for(i=1;i<=Length-Source.length;i++){  
		strTemp+="0";  
	}  
	return strTemp+Source;  
}

function CheckValue()
{
	var wanipaddr = "<% getCfgZero(1, "wan_ipaddr"); %>";
	var wanip_get_from_f = "<% getWanIp(); %>";
	var opmode = "<% getCfgZero(1, "OperationMode"); %>";

	if (document.lanCfg.hostname.value.indexOf(" ") >= 0)
	{
		alert(_('lan Dont enter Blank Space in this feild'));
		document.lanCfg.hostname.focus();
		document.lanCfg.hostname.select();
		return false;
	}
	if (!checkIpAddr(document.lanCfg.lanIp, false))
		return false;

	if (document.lanCfg.lanIp.value != "")
	{
		if ( wanip_get_from_f != "" )
		{
			if (opmode!= "0")
			{
			if (document.lanCfg.lanIp.value == wanip_get_from_f)
			{
				alert(_('lan Incorrect Lan IP = Wan IP'));
				document.lanCfg.lanIp.focus();
				return false;
			}
			}
			
			if (checkIPAddr_sub(document.lanCfg.lanIp.value)==false)
			{	
				alert(_('lan IP adress format error'));
				document.lanCfg.lanIp.focus();
				return false;
			}
		}
		else if ( wanipaddr != "" )
		{
			if (opmode!= "0")
			{
		
			if (document.lanCfg.lanIp.value == wanipaddr)
			{
				alert(_('lan Incorrect Lan IP = Wan IP'));
				document.lanCfg.lanIp.focus();
				return false;
			}
			}			
			
			if (checkIPAddr_sub(document.lanCfg.lanIp.value)==false)
			{
				alert(_('lan IP adress format error'));
				document.lanCfg.lanIp.focus();
				return false;
			}
		}
	}

	if (!checkIpAddr(document.lanCfg.lanNetmask, true))
		return false;
		
	if (document.lanCfg.lanNetmask.value != "")
	{
		if (checkIPAddr_sub(document.lanCfg.lanNetmask.value)==false)
		{	
			alert(_('lan Netmask format error'));
			document.lanCfg.lanNetmask.focus();
			return false;
		}
	}			
	
	if (document.lanCfg.lan2enabled[0].checked == true)
	{
		if (!checkIpAddr(document.lanCfg.lan2Ip, false))
			return false;
		if (!checkIpAddr(document.lanCfg.lan2Netmask, true))
			return false;
	}
	
	if (!checkIPwithSubnetMask(document.lanCfg.lanIp.value,document.lanCfg.lanNetmask.value)) {
		alert(_('lan Incorrect Lan IP NOT in subnet'));
		return false;
	} /* [Yian,2009/07/11] */
	
	if (document.lanCfg.lanGateway.value != "")
	{
		if (!checkIpAddr(document.lanCfg.lanGateway, false))
			return false;
			
		if (checkIPAddr_sub(document.lanCfg.lanGateway.value)==false)
		{	
			alert(_('lan Netmask format error'));
			document.lanCfg.lanGateway.focus();
			return false;
		}
	}
	if (document.lanCfg.lanPriDns.value != "")
		if (!checkIpAddr(document.lanCfg.lanPriDns, false))
			return false;
	if (document.lanCfg.lanSecDns.value != "")
		if (!checkIpAddr(document.lanCfg.lanSecDns, false))
			return false;
	if (document.lanCfg.lanDhcpType.options.selectedIndex == 1) {
		if (!checkIpAddr(document.lanCfg.dhcpStart, false))
			return false;
		if (!checkIpAddr(document.lanCfg.dhcpEnd, false))
			return false;
		if (!checkIpAddr(document.lanCfg.dhcpMask, true))
			return false;
		if (document.lanCfg.dhcpPriDns.value != "")
			if (!checkIpAddr(document.lanCfg.dhcpPriDns, false))
				return false;
		if (document.lanCfg.dhcpSecDns.value != "")
			if (!checkIpAddr(document.lanCfg.dhcpSecDns, false))
				return false;
		if (!checkIpAddr(document.lanCfg.dhcpGateway, false))
			return false;

		if (!checkDhcpStartEnd(document.lanCfg.dhcpStart.value, document.lanCfg.dhcpEnd.value)) {
			alert(_('lan Incorrect dhcpStart IP dhcpEnd IP'));
			document.lanCfg.dhcpStart.focus();
			return false;
		} 
		
		if ( document.lanCfg.dhcpStart.value == document.lanCfg.lanIp.value)
		{
			alert(_('lan Incorrect dhcpStart IP Lan IP'));
			document.lanCfg.dhcpStart.focus();
			return false;	
		}
		if ( document.lanCfg.dhcpEnd.value == document.lanCfg.lanIp.value)
		{
			alert(_('lan Incorrect dhcpStart IP Lan IP'));
			document.lanCfg.dhcpEnd.focus();			
			return false;	
		}
		
		if (checkIPAddr_sub(document.lanCfg.dhcpStart.value)==false)
		{	
			alert(_('lan IP address format error'));
			document.lanCfg.dhcpStart.focus();
			return false;
		}
		
		if (checkIPAddr_sub(document.lanCfg.dhcpEnd.value)==false)
		{	
			alert(_('lan IP address format error'));
			document.lanCfg.dhcpEnd.focus();
			return false;
		}
		
		if (isSameClass(document.lanCfg.dhcpStart.value,document.lanCfg.lanIp.value,document.lanCfg.lanNetmask.value)==false || 
			isSameClass(document.lanCfg.dhcpEnd.value,document.lanCfg.lanIp.value,document.lanCfg.lanNetmask.value)==false) {
			alert(_('lan Incorrect DHCP Range NOT in Subnet'));
			return false;	
		} /* [Yian, 2009/07/14] */	
		
		if (!checkIPwithSubnetMask(document.lanCfg.dhcpStart.value,document.lanCfg.lanNetmask.value) || 
			!checkIPwithSubnetMask(document.lanCfg.dhcpEnd.value,document.lanCfg.lanNetmask.value)) {
			return false;	
		}
	
		if (document.lanCfg.dhcpStatic1Mac.value != "") {
			var re = /[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}/;
			if (!re.test(document.lanCfg.dhcpStatic1Mac.value)) {
				alert(_('lan Please fill the MAC Address in correct format'));
				document.lanCfg.dhcpStatic1Mac.focus();
				return false;
			}
			if (!checkIpAddr(document.lanCfg.dhcpStatic1Ip, false))
				return false;
			document.lanCfg.dhcpStatic1.value = document.lanCfg.dhcpStatic1Mac.value + ';' + document.lanCfg.dhcpStatic1Ip.value;
		}
		if (document.lanCfg.dhcpStatic2Mac.value != "") {
			var re = /[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}/;
			if (!re.test(document.lanCfg.dhcpStatic2Mac.value)) {
				alert(_('lan Please fill the MAC Address in correct format'));
				document.lanCfg.dhcpStatic2Mac.focus();
				return false;
			}
			if (!checkIpAddr(document.lanCfg.dhcpStatic2Ip, false))
				return false;
			document.lanCfg.dhcpStatic2.value = document.lanCfg.dhcpStatic2Mac.value + ';' + document.lanCfg.dhcpStatic2Ip.value;
		}
		if (document.lanCfg.dhcpStatic3Mac.value != "") {
			var re = /[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}/;
			if (!re.test(document.lanCfg.dhcpStatic3Mac.value)) {
				alert(_('lan Please fill the MAC Address in correct format'));
				document.lanCfg.dhcpStatic3Mac.focus();
				return false;
			}
			if (!checkIpAddr(document.lanCfg.dhcpStatic3Ip, false))
				return false;
			document.lanCfg.dhcpStatic3.value = document.lanCfg.dhcpStatic3Mac.value + ';' + document.lanCfg.dhcpStatic3Ip.value;
		}
		
		/* Lan ip in DHCP lease range [Yian,2009/07/11] - Start */
		var LanIP = document.lanCfg.lanIp.value;
		var ipstart = document.lanCfg.dhcpStart.value;
		var ipend = document.lanCfg.dhcpEnd.value;
		var ns = eval(FormatNum(""+atoi(ipstart,1),3)+FormatNum(""+atoi(ipstart,2),3)+FormatNum(""+atoi(ipstart,3),3)+FormatNum(""+atoi(ipstart,4),3));
		var ne = eval(FormatNum(""+atoi(ipend,1),3)+FormatNum(""+atoi(ipend,2),3)+FormatNum(""+atoi(ipend,3),3)+FormatNum(""+atoi(ipend,4),3));
		var n = eval(FormatNum(""+atoi(LanIP,1),3)+FormatNum(""+atoi(LanIP,2),3)+FormatNum(""+atoi(LanIP,3),3)+FormatNum(""+atoi(LanIP,4),3));
		if (ns <= ne && n >= ns && n <= ne) {
			alert(_('lan Incorrect Lan ip in DHCP lease range'));
			document.lanCfg.lanIp.focus();
			return false;
		}
		/* Lan ip in DHCP lease range [Yian,2009/07/11] - End */

	}
	
	document.lanCfg.dhcpMask.value = document.lanCfg.lanNetmask.value;
	
	if	(document.lanCfg.dnspEnbl.options.selectedIndex == 1)
	{
	document.lanCfg.dhcpPriDns.value = document.lanCfg.lanIp.value;
	document.lanCfg.dhcpSecDns.value = document.lanCfg.lanIp.value;
	}
	
	document.lanCfg.dhcpGateway.value = document.lanCfg.lanIp.value;
	
	return true;
}

function DHCPDnsOnChange()
{
	if (document.lanCfg.dnspEnbl.options.selectedIndex == 0)
	{
		document.getElementById("pridns").style.visibility = "visible";
		document.getElementById("pridns").style.display = display_on();
		document.lanCfg.dhcpPriDns.disabled = false;
		document.getElementById("secdns").style.visibility = "visible";
		document.getElementById("secdns").style.display = display_on();
		document.lanCfg.dhcpSecDns.disabled = false;
		document.lanCfg.dhcpSecDns.readonly = 1;
	}
	else
	{
		document.getElementById("pridns").style.visibility = "hidden";
		document.getElementById("pridns").style.display = "none";
		/*document.lanCfg.dhcpPriDns.disabled = true;*/
		document.getElementById("secdns").style.visibility = "hidden";
		document.getElementById("secdns").style.display = "none";
		/*document.lanCfg.dhcpSecDns.disabled = true;*/
	}
}


function lan2_enable_switch()
{
	if (document.lanCfg.lan2enabled[1].checked == true)
	{
		document.lanCfg.lan2Ip.disabled = true;
		document.lanCfg.lan2Netmask.disabled = true;
	}
	else
	{
		document.lanCfg.lan2Ip.disabled = false;
		document.lanCfg.lan2Netmask.disabled = false;
	}
}

var oldIp;
function recIpCfg()
{
	oldIp = document.lanCfg.lanIp.value;
}

/*
 * Try to modify dhcp server configurations:
 *   dhcp start/end ip address to the same as new lan ip address
 */
function modDhcpCfg()
{
	var i, j;
	var mask = document.lanCfg.lanNetmask.value;
	var newNet = document.lanCfg.lanIp.value;

	//support simple subnet mask only
	if (mask == "255.255.255.0")
		mask = 3;
	else if (mask == "255.255.0.0")
		mask = 2;
	else if (mask == "255.0.0.0")
		mask = 1;
	else
		return;

	//get the old subnet
	for (i=0, j=0; i<oldIp.length; i++) {
		if (oldIp.charAt(i) == '.') {
			j++;
			if (j != mask)
				continue;
			oldIp = oldIp.substring(0, i);
			break;
		}
	}

	//get the new subnet
	for (i=0, j=0; i<newNet.length; i++) {
		if (newNet.charAt(i) == '.') {
			j++;
			if (j != mask)
				continue;
			newNet = newNet.substring(0, i);
			break;
		}
	}

	document.lanCfg.dhcpStart.value = document.lanCfg.dhcpStart.value.replace(oldIp, newNet);
	document.lanCfg.dhcpEnd.value = document.lanCfg.dhcpEnd.value.replace(oldIp, newNet);
}

function onClicksbuttonMax()
{
	if (CheckValue())
	{
		sbutton_disable(sbuttonMax);
		if( document.lanCfg.lanIp.value != "<% getLanIp(); %>"){
			parent.redirectebydiffip(document.lanCfg.lanIp.value);
		}
		window.scrollBy(0, -1000);
		document.lanCfg.submit();
		restartPage_block();
	}
}
</script>
</head>

<body onLoad="initValue()" bgcolor="#FFFFFF">

<div align="center">
 <center>

<table class="body"><tr><td>

<table width="95%" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="lTitle">Local Area Network (LAN) Settings</td>
</tr>
<tr>
<td colspan="2">
<p class="head" id="lIntroduction"></p>
</td>
</tr>

</table>

<br>

<form method=post name="lanCfg" action="/goform/setLan">
<table width="95%" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="lSetup">LAN Interface Setup</td>
</tr>
<tr>
  <td class="head" id="lMac">MAC Address</td>
  <td><% getLanMac(); %>&nbsp;</td>
</tr>
<tr <% var hashost = getHostSupp();
      if (hashost != "1") write("style=\"visibility:hidden;display:none\""); %>>
  <td class="head" id="lHostname">Hostname</td>
  <td>
  <input name="hostname" maxlength=16
              value="<% getCfgGeneral(1, "HostName"); %>" size="20"></td>
</tr>
<tr>
  <td class="head" id="lIp">IP Address</td>
  <td>
  <input name="lanIp" maxlength=15 value="<% getLanIp(); %>" onFocus="recIpCfg()" onBlur="modDhcpCfg()" size="20"></td>
</tr>
<tr>
  <td class="head" id="lNetmask">Subnet Mask</td>
  <td>
  <input name="lanNetmask" maxlength=15 value="<% getLanNetmask(); %>" size="20"></td>
</tr>
<tr id="lLan2_Radio">
  <td class="head" id="lLan2">LAN2</td>
  <td >
    <input type="radio" name="lan2enabled" value="1" onClick="lan2_enable_switch()"><font id="lLan2Enable">Enablei</font>&nbsp;
    <input type="radio" name="lan2enabled" value="0" onClick="lan2_enable_switch()" checked><font id="lLan2Disable">Disable</font>
  </td>
</tr>
<tr id="lLan2Ip_input">
  <td class="head" id="lLan2Ip">LAN2 IP Address</td>
  <td ><input name="lan2Ip" maxlength=15 value="" size="20"></td>
</tr>
<tr id="lLan2Netmask_input">
  <td class="head" id="lLan2Netmask">LAN2 Subnet Mask</td>
  <td ><input name="lan2Netmask" maxlength=15 value="" size="20"></td>
</tr>
<tr id="brGateway">
  <td class="head" id="lGateway">Default Gateway</td>
  <td>
  <input name="lanGateway" maxlength=15 value="<% getWanGateway(); %>" size="20"></td>
</tr>
<tr id="brPriDns">
  <td class="head" id="lPriDns">Primary DNS Server</td>
  <td><input name="lanPriDns" maxlength=15 value="<% getDns(1); %>" size="20"></td>
</tr>
<tr id="brSecDns">
  <td class="head" id="lSecDns">Secondary DNS Server</td>
  <td><input name="lanSecDns" maxlength=15 value="<% getDns(2); %>" size="20"></td>
</tr>
</table>

<br>

<table width="95%" border="1" cellpadding="2" cellspacing="1">
<tr><td class="title" colspan="2" id="lDhcpSetup">DHCP Setup</td></tr>
<tr>
  <td class="head" id="lDhcpType">DHCP Type</td>
  <td>
    <select name="lanDhcpType" size="1" onChange="dhcpTypeSwitch();">
      <option value="DISABLE" id="lDhcpTypeD">Disable</option>
      <option value="SERVER" id="lDhcpTypeS">Server</option>
    </select>
  </td>
</tr>
<tr id="start">
  <td class="head" id="lDhcpStart" align="left">
  <p align="left">DHCP Start IP</td>
  <td align="left">
  <input name="dhcpStart" maxlength=15
             value="<% getCfgGeneral(1, "dhcpStart"); %>" size="20"></td>
</tr>
<tr id="end">
  <td class="head" id="lDhcpEnd" align="left">
  <p align="left">DHCP End IP</td>
  <td align="left">
  <input name="dhcpEnd" maxlength=15
             value="<% getCfgGeneral(1, "dhcpEnd"); %>" size="20"></td>
</tr>
<tr id="mask">
  <td class="head" id="lDhcpNetmask" align="left">
  <p align="left">DHCP Subnet Mask</td>
  <td align="left">
  <input name="dhcpMask" maxlength=15
             value="<% getCfgGeneral(1, "dhcpMask"); %>" size="20"></td>
</tr>
<tr id="pridns">
  <td class="head" id="lDhcpPriDns" align="left">
  <p align="left">DHCP Primary DNS</td>
  <td align="left">
  <input name="dhcpPriDns" maxlength=15
             value="<% getCfgGeneral(1, "dhcpPriDns"); %>" size="20"></td>
</tr>
<tr id="secdns">
  <td class="head" id="lDhcpSecDns" align="left">
  <p align="left">DHCP Secondary DNS</td>
  <td align="left">
  <input name="dhcpSecDns" maxlength=15
             value="<% getCfgGeneral(1, "dhcpSecDns"); %>" size="20"></td>
</tr>
<tr id="gateway">
  <td class="head" id="lDhcpGateway" align="right">
  <p align="left">DHCP Default Gateway</td>
  <td>
  <input name="dhcpGateway" maxlength=15
             value="<% getCfgGeneral(1, "dhcpGateway"); %>" size="20"></td>
</tr>
<tr id="lease">
  <td class="head" id="lDhcpLease" align="left">
  <p align="left">DHCP Lease Time</td>
  <td align="left">
  <!--<input name="dhcpLease" maxlength=8
             value="<% getCfgGeneral(1, "dhcpLease"); %>" size="20"></td>-->
   <select name="dhcpLease" id='dhcpLease' size="1"> 
		<option value="1800"> Half hour </option> 
		<option value="3600"> One hour </option>
		<option value="7200"> Two hours </option>
		<option value="43200"> Half day </option>
		<option value="86400"> One day </option>
		<option value="172800"> Two days </option>
		<option value="604800"> One week </option>
		<option value="1209600"> Two weeks </option>
		<option value="62208000"> Forever </option>
	</select>
</tr>
<tr id="staticlease1">
  <td class="head" id="lDhcpStatic1" align="right">
  <p align="left">Statically Assigned</td>
  <td><input type=hidden name=dhcpStatic1 value="">
      MAC: 
  <input name="dhcpStatic1Mac" maxlength=17
             value="<% getCfgNthGeneral(1, "dhcpStatic1", 0); %>" size="20"><br />
      IP: 
      <input name="dhcpStatic1Ip" maxlength=15
             value="<% getCfgNthGeneral(1, "dhcpStatic1", 1); %>" size="20"></td>
</tr>
<tr id="staticlease2">
  <td class="head" id="lDhcpStatic2" align="right">
  <p align="left">Statically Assigned</td>
  <td><input type=hidden name=dhcpStatic2 value="">
      MAC: 
  <input name="dhcpStatic2Mac" maxlength=17
             value="<% getCfgNthGeneral(1, "dhcpStatic2", 0); %>" size="20"><br />
      IP: 
      <input name="dhcpStatic2Ip" maxlength=15
             value="<% getCfgNthGeneral(1, "dhcpStatic2", 1); %>" size="20"></td>
</tr>
<tr id="staticlease3">
  <td class="head" id="lDhcpStatic3" align="right">
  <p align="left">Statically Assigned</td>
  <td><input type=hidden name=dhcpStatic3 value="">
      MAC: 
  <input name="dhcpStatic3Mac" maxlength=17
             value="<% getCfgNthGeneral(1, "dhcpStatic3", 0); %>" size="20"><br />
      IP: 
      <input name="dhcpStatic3Ip" maxlength=15
             value="<% getCfgNthGeneral(1, "dhcpStatic3", 1); %>" size="20"></td>
</tr>
</table>

<br>

<table width="95%" border="1" cellpadding="2" cellspacing="1">
<tr><td class="title" colspan="2" id="lDhcpOtherSetup">Other Setup</td></tr>

<tr style="display:none">
  <td class="head" id="lStp">802.1d Spanning Tree</td>
  <td>
    <select name="stpEnbl" size="1">
      <option value="0" id="lStpD">Disable</option>
      <option value="1" id="lStpE">Enable</option>
    </select>
  </td>
</tr>
<tr id="lltd">
  <td class="head" id="lLltd">LLTD</td>
  <td>
    <select name="lltdEnbl" size="1">
      <option value="0" id="lLltdD">Disable</option>
      <option value="1" id="lLltdE">Enable</option>
    </select>
  </td>
</tr>
<tr id="igmpProxy">
  <td class="head" id="lIgmpp">IGMP proxy</td>
  <td>
    <select name="igmpEnbl" size="1">
      <option value="0" id="lIgmppD">Disable</option>
      <option value="1" id="lIgmppE">Enable</option>
    </select>
  </td>
</tr>
<tr id="upnp">
  <td class="head" id="lUpnp">UPNP</td>
  <td>
    <select name="upnpEnbl" size="1">
      <option value="0" id="lUpnpD">Disable</option>
      <option value="1" id="lUpnpE">Enable</option>
    </select>
  </td>
</tr>
<tr id="radvd" style="visibility:hidden" style="display:none" >
  <td class="head" id="lRadvd">Router Advertisement</td>
  <td>
    <select name="radvdEnbl" size="1">
      <option value="0" id="lRadvdD">Disable</option>
      <option value="1" id="lRadvdE">Enable</option>
    </select>
  </td>
</tr>
<tr id="pppoerelay">
  <td class="head" id="lPppoer">PPPOE relay</td>
  <td>
    <select name="pppoeREnbl" size="1">
      <option value="0" id="lPppoerD">Disable</option>
      <option value="1" id="lPppoerE">Enable</option>
    </select>
  </td>
</tr>
<tr id="dnsproxy" style="visibility:hidden" style="display:none" >
  <td class="head" id="lDnsp">DNS proxy</td>
  <td>
    <select name="dnspEnbl" size="1" onChange="DHCPDnsOnChange()">
      <option value="0" id="lDnspD">Disable</option>
      <option value="1" id="lDnspE">Enable</option>
    </select>
  </td>
</tr>
</table>
<br>
<table width="95%" cellpadding="2" cellspacing="1">
<tr id="sbutton0" align="center">
  <td>
    <input type="button" style="{width:120px;}" value="Apply" id="lApply"  onClick="onClicksbuttonMax();">&nbsp;&nbsp;
    <input type=reset  style="{width:120px;}" value="Cancel" id="lCancel" onClick="window.location.reload()">
  </td>
</tr>
</table>
</form>

</td></tr></table>

 </center>
</div>

</body>
</html>