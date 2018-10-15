<html>
<head>
<title>Access Point Status</title>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("admin");

var opmode = 1* <% getCfgZero(1, "OperationMode"); %>;
var show = 0;
setTimeout( "makeRequest();",10 * 1000);

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

function checkStatus()
{
	var refresh_counter = <% getRefreshCounter(); %>;	

	if(atoi(refresh_counter, 1))
		window.location.replace("./status.asp");
}

function showOpMode()
{
	var opmode = 1* <% getCfgZero(1, "OperationMode"); %>;
	if (opmode == 0)
		document.write(_('status bridge mode'));
	else if (opmode == 1)
		document.write(_('status router mode'));
	else if (opmode == 2)
		document.write(_('status ethernet converter mode'));
	else if (opmode == 3)
		document.write(_('status ap client mode'));
	else
		document.write(_('status unknown'));
}

function check_otg_sta() {
	
    if (http_reflash_page.readyState == 4) {
        if (http_reflash_page.status == 200) {
			// refresh
			window.location.replace("http://" + location.hostname + "/adm/status.asp");
        } else {
        }
    }
}
var http_reflash_page = false;
function makeRequest() {
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_reflash_page = new XMLHttpRequest();
        if (http_reflash_page.overrideMimeType) {
            http_reflash_page.overrideMimeType('text/xml');
        }
    } else if (window.ActiveXObject) { // IE
        try {
            http_reflash_page = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_reflash_page = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_reflash_page) {
        //alert('Giving up :( Cannot create an XMLHTTP instance');
        return false;
    }
    http_reflash_page.onreadystatechange = check_otg_sta;
    http_reflash_page.open('GET', "http://" + location.hostname + "/adm/status.asp", false);
    http_reflash_page.send(null);
}
function showPortStatus()
{<!--
	var str = "<% getPortStatus(); %>";
	var all = new Array();

	if(str == "-1"){
		document.write("not support");
		return ;
	}

	all = str.split(",");
	for(i=0; i< all.length-1; i+=3){
		document.write("<td>");
		if(all[i] == "1"){
			if(all[i+1] == "10")
				document.write("<img src=/graphics/10.gif> ");
			else if(all[i+1] == "100")
				document.write("<img src=/graphics/100.gif> ");

//			if(all[i+2] == "F")
//				document.write("Full ");
//			else(all[i+2] == "H")
//				document.write("Half ");
		}else if(all[i] == "0"){
				document.write("<img src=/graphics/empty.gif> ");
		}
		document.write("</td>");
	}
//-->
}

function initTranslation()
{
	var e = document.getElementById("statusTitle");
	e.innerHTML = _("status title");
	e = document.getElementById("statusIntroduction");
	e.innerHTML = _("status introduction");
	
	e = document.getElementById("statusSysInfo");
	e.innerHTML = _("status system information");
	e = document.getElementById("statusSDKVersion");
	e.innerHTML = _("status sdk version");
	e = document.getElementById("statusSysUpTime");
	e.innerHTML = _("status system up time");
	e = document.getElementById("statusOPMode");
	e.innerHTML = _("status operate mode");

	e = document.getElementById("statusInternetConfig");
	e.innerHTML = _("status internet config");
	e = document.getElementById("statusConnectedType");
	e.innerHTML = _("status connect type");
	e = document.getElementById("statusConnectedStatus");
	e.innerHTML = _("status connect status");
	e = document.getElementById("statusWANIPAddr");
	e.innerHTML = _("status wan ipaddr");
	e = document.getElementById("statusSubnetMask");
	e.innerHTML = _("status subnet mask");
	e = document.getElementById("statusDefaultGW");
	e.innerHTML = _("status default gateway");
	e = document.getElementById("statusPrimaryDNS");
	e.innerHTML = _("status primary dns");
	e = document.getElementById("statusSecondaryDNS");
	e.innerHTML = _("status secondary dns");
	e = document.getElementById("statusWANMAC");
	e.innerHTML = _("status mac");

	e = document.getElementById("statusLocalNet");
	e.innerHTML = _("status local network");
	e = document.getElementById("statusLANIPAddr");
	e.innerHTML = _("status lan ipaddr");
	e = document.getElementById("statusLocalNetmask");
	e.innerHTML = _("status local netmask");
	e = document.getElementById("statusLANMAC");
	e.innerHTML = _("status mac");

	e = document.getElementById("statusEthPortStatus");
	e.innerHTML = _("status ethernet port status");
}

function PageInit()
{
	parent.web_page_selected('6', '4');
	var ethtoolb = "<% getETHTOOLBuilt(); %>";
	initTranslation();

	if (0)//(ethtoolb == "1") // hidded this, minglin
	{
		document.getElementById("statusEthPortStatus").style.visibility = "visible";
		document.getElementById("statusEthPortStatus").style.display = style_display_on();
		document.getElementById("div_ethtool").style.visibility = "visible";
		document.getElementById("div_ethtool").style.display = style_display_on();
	}
	else
	{
		document.getElementById("statusEthPortStatus").style.visibility = "hidden";
		document.getElementById("statusEthPortStatus").style.display = "none";
		document.getElementById("div_ethtool").style.visibility = "hidden";
		document.getElementById("div_ethtool").style.display = "none";
	}
	
	if ( opmode == 0){ // hidden Internet Configuretion.
		id_invisible("internetConfig");	
	}
	/** 2009/5/7 simon_ho **/
	/* refresh 60 times every 10secs */
	//setTimeout("checkStatus()", 10*1000 );
	
	show_conn_info();
}

function show_conn_info(){
	var conn_Info = document.getElementById("conn_status");

	if ( "<% getActWanIp(); %>" != "" ){
		conn_Info.innerHTML = _("status connected");
		conn_Info.color="blue";
		show = 1;
	}else{
		if(!show){
			conn_Info.innerHTML = _("status disconnected");
			conn_Info.color="red";
			show = 1;
		}else{
			conn_Info.innerHTML = "";
			show = 0;
		}
	}
	setTimeout("show_conn_info();", 500);
}
</script>
</head>

<body onLoad="PageInit()" bgcolor="#FFFFFF">
<div align="center">
 <center>

<table class="body"><tr><td>

<table width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="statusTitle">Status</td>
</tr>
<tr>
<td colspan="2">
<p class="head" id="statusIntroduction">Status</p>
</td>
</tr>

</table>

<br>

<table width="540" border="1" cellpadding="2" cellspacing="1">
<!-- ================= System Info ================= -->
<tr>
  <td class="title" colspan="2" id="statusSysInfo">System Info</td>
</tr>
<tr>
  <td class="head" id="statusSDKVersion">SDK Version</td>
  <td><% getAWBVersion(); %> (<% getSysBuildTime(); %>)</td>
</tr>
<tr>
  <td class="head" id="statusSysUpTime">System Time</td>
  <td><% getSysUptime(); %>&nbsp;</td>
</tr>
<tr>
  <td class="head" id="statusOPMode">Operation Mode</td>
  <td><script type="text/javascript">showOpMode();</script>&nbsp;</td>
</tr>
</table>
<!-- ================= Internet Configurations ================= -->
<table width="540" border="1" cellpadding="2" cellspacing="1" id="internetConfig" >
<tr>
  <td class="title" colspan="2" id="statusInternetConfig">Internet Configurations</td>
</tr>
<tr>
  <td class="head" id="statusConnectedType">Connected Type</td>
  <td> 
      <script language="JavaScript" type="text/javascript">
      if("<% getActWan(); %>" == "G3G")
          document.write("3G"); 
      else
          document.write("<% getActWan(); %>");
      </script>&nbsp;</td>
</tr>
<tr>
  <td class="head" id="statusConnectedStatus">Connection Status</td>
  <td><font id="conn_status"></font>&nbsp;</td>
</tr>
<tr>
  <td class="head" id="statusWANIPAddr">WAN IP Address</td>
  <td id="statusWanIPAddr"><% getActWanIp(); %>&nbsp;</td>
</tr>
<tr>
  <td class="head" id="statusSubnetMask">Subnet Mask</td>
  <td><% getActWanNetmask(); %>&nbsp;</td>
</tr>
<tr>
  <td class="head" id="statusDefaultGW">Default Gateway</td>
  <td><% getActWanGateway(); %>&nbsp;</td>
</tr>
<tr>
  <td class="head" id="statusPrimaryDNS">Primary Domain Name Server</td>
  <td>
      <script language="JavaScript" type="text/javascript">
        if("<% getActWanIp(); %>" != "")
          document.write("<% getDns(1); %>");
      </script>
    &nbsp;</td>
</tr>
<tr>
  <td class="head" id="statusSecondaryDNS">Secondary Domain Name Server</td>
  <td>
      <script language="JavaScript" type="text/javascript">
        if("<% getActWanIp(); %>" != "")
          document.write("<% getDns(2); %>");
      </script>
      &nbsp;</td>
</tr>
<tr>
  <td class="head" id="statusWANMAC">MAC Address</td>
  <td><% getActWanMac(); %>&nbsp;</td>
</tr>
</table>
<!-- ================= Local Network ================= -->
<table width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="statusLocalNet">Local Network</td>
</tr>
<tr>
  <td class="head" id="statusLANIPAddr">Local IP Address</td>
  <td><% getLanIp(); %>&nbsp;</td>
</tr>
<tr>
  <td class="head" id="statusLocalNetmask">Local Netmask</td>
  <td><% getLanNetmask(); %>&nbsp;</td>
</tr>
<tr>
  <td class="head" id="statusLANMAC">MAC Address</td>
  <td><% getLanMac(); %>&nbsp;</td>
</tr>
<!-- ================= Other Information ================= -->
</table>


<table border="0" id="div_ethtool" >
<tr>
  <td>
    <H1 id="statusEthPortStatus">Ethernet Port Status</H1>
  </td>
</tr>
<tr>
  <td>
    <script type="text/javascript">showPortStatus();</script>
  </td>
</tr>
</table>

</td></tr></table>

 </center>
</div>

</body>
</html>