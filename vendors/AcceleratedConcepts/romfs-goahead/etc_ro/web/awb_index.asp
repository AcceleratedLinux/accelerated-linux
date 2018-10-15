<html>
<head>
<meta http-equiv="PRAGMA" content="NO-CACHE">
<meta http-equiv="CACHE-CONTROL" content="NO-CACHE">

<meta http-equiv="content-type" content="text/html; charset=UTF-8">
<script type="text/javascript" src="/lang/b28n.js"></script>


<style>
TD{font-size:10pt}
TD.list{Font-family:verdana; COLOR:#ffffff; TEXT-DECORATION:none; cursor:HAND; text-align:center}
TD.list a{display:#ffffff; border:0px solid #4F7599; background:#4F7599; padding:4px; margin:0px; color:#7793bb; text-decoration:none}
TD.list A:link{Font-family:verdana; COLOR:#ffffff; TEXT-DECORATION:none}
TD.list A:active{Font-family:verdana; COLOR:#ffffff; background:#7793bb; TEXT-DECORATION:none}
TD.list A:visited{Font-family:verdana; COLOR:#ffffff; TEXT-DECORATION:none}
TD.list A:hover{background:#7793bb; color:#ffffff; border:0px solid blue}

TD.sublist{Font-family:verdana; font-size:9pt; COLOR:#FFFFFF; TEXT-DECORATION:none; padding-left:2px; cursor:HAND}
TD.sublist a{display:#ffffff; border:0px solid #4F7599; background:#4F7599; padding:4px; margin:0px; color:#7793bb; text-decoration:none}
TD.sublist A:link{Font-family:verdana; COLOR:#ffffff; TEXT-DECORATION:none}
TD.sublist A:active{Font-family:verdana; COLOR:#ffffff; background:#7793bb; TEXT-DECORATION:none}
TD.sublist A:visited{Font-family:verdana; COLOR:#ffffff; TEXT-DECORATION:none}
TD.sublist A:hover{background:#7793bb; color:#ffffff; border:0px solid blue}
DIV.substyle{display:NONE; padding-left:2px; padding-bottom:5px}
</style>
<title></title>


<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("main");

var constMaxItem = parseInt("6");//No of MenuItems
var clickItem = 0;

var opmode = '<% getCfgZero(1, "OperationMode"); %>';
var dhcpen = '<% getCfgZero(1, "dhcpEnabled"); %>';
var dpbsta = '<% getDpbSta(); %>';
var vpnen = '<% getVPNBuilt(); %>';
var ethconv = '<% getCfgZero(1, "ethConvert"); %>';
var meshb = '<% getMeshBuilt(); %>';
var wdsb = '<% getWDSBuilt(); %>';
var wscb = '<% getWSCBuilt(); %>';
var usbb = '<% getUSBBuilt(); %>';
var storageb = '<% getStorageBuilt(); %>';
var ftpb = '<% getFtpBuilt(); %>';
var smbb = '<% getSmbBuilt(); %>';
var mediab = '<% getMediaBuilt(); %>';
var webcamb = '<% getWebCamBuilt(); %>';
var printersrvb = '<% getPrinterSrvBuilt(); %>';
var usbiNICb = '<% getUSBiNICBuilt(); %>';
var swqos = '<% getSWQoSBuilt(); %>';
var ad = '<% isAntennaDiversityBuilt(); %>';

var main_itemArray = Array("wizard list","operation mode","internet settings","wireless settings","firewall","administration");

var wizard_list_itemArray = Array("setup wizard");
var operation_mode_itemArray = Array("op mode");
var internet_itemArray = Array("wan","lan","dhcp clients","vpn passthrough","routing","qos");
var wireless_itemArray = Array("basic","advanced","security","wds","wps","ap client","station list","antenna diversity","mesh settings","profile","link status","site survey","site statistics","site advance","site qos","11n configurations","about","site wps");
var firewall_itemArray = Array("port filtering","port forwarding","dmz","system firewall","content filtering");
var administration_itemArray = Array("");
if (opmode != 0)
	administration_itemArray = Array("management","upload firmware","settings management","status","statistics","system log","3g budget status");
else
	administration_itemArray = Array("management","upload firmware","settings management","status","statistics","system log");

var menu_itemArray = Array(wizard_list_itemArray,operation_mode_itemArray,internet_itemArray,wireless_itemArray,firewall_itemArray,administration_itemArray,wizard_itemArray);
var wizard_itemArray = Array("wizard lang", "wizard time", "wizard wan", "wizard security", "wizard finish");
var main_current;

function fnDisPatch(value)
{
	for (i=1; i < (constMaxItem+1); i++)
	{
		if (i!=value)
			document.getElementById(main_itemArray[i-1]).style.background="#4F7599";
		else
			document.getElementById(main_itemArray[i-1]).style.background="#7793bb";
	}
}

function clear_subMenu(value)
{
	var field;
	var i;
	
	field=menu_itemArray[value-1];
		
	for (i=1;i<=field.length;i++)
			document.getElementById(field[i-1]).style.background="#4F7599";
}
	
function clicked_subMenu(main,value)
{
	var field;
	var i;
	
	field=menu_itemArray[main-1];
		
	for (i=1;i<=field.length;i++)
	{
		if (i==value)
			document.getElementById(field[i-1]).style.background="#7793bb";
		else
			document.getElementById(field[i-1]).style.background="#4F7599";
	}
}
    
function fnDispThis(active)
{
	
	fnDisPatch(active);
	
	for (i=1; i < (constMaxItem+1); i++)
	{
		if (i!=active){
			document.getElementById("sub"+i).style.display="NONE";
			if (active == 1){
				fnDispWizard(0);
			}
		}
		else
		{
			document.getElementById("sub"+i).style.display="BLOCK";
			/*window.open(urlArray[i-1],"mainFrame");*/
		}
	}
	/*clickItem = 0 ;*/ //click Main menu

	clear_subMenu(active);
}
function fnDispWizard(step){
	/*var s;

	for(s=1;s<=wizard_itemArray.length;s++){
		if(step == 0){
			document.getElementById(wizard_itemArray[s-1]).style.display="none";
			//document.getElementById(wizard_itemArray[s-1]).style.background="#4F7599";
			document.getElementById(wizard_itemArray[s-1]).style.visibility = "hidden";
		}else if(s < step){
			//document.getElementById(wizard_itemArray[s-1]).style.background="#7793bb";
			document.getElementById(wizard_itemArray[s-1]).style.display="BLOCK";
			document.getElementById(wizard_itemArray[s-1]).style.visibility = "visible";
		}else{
			document.getElementById(wizard_itemArray[s-1]).style.display="none";
			//document.getElementById(wizard_itemArray[s-1]).style.background="#4F7599";
			document.getElementById(wizard_itemArray[s-1]).style.visibility = "hidden";
		}
	}*/
}

function initTranslation()
{
	var e;

	e = document.getElementById("wizard list");
	e.innerHTML = _("treeapp setup wizard");
	
	e = document.getElementById("setup wizard");
	e.innerHTML =_("treeapp setup wizard");

	e = document.getElementById("operation mode");
	e.innerHTML = _("treeapp operation mode");
	
	e = document.getElementById("op mode");
	e.innerHTML = _("treeapp operation mode");

/* ------------ internet settings -------------*/
	
	e = document.getElementById("internet settings");
	e.innerHTML = _("treeapp internet settings");
	
	e = document.getElementById("wan");
	e.innerHTML = _("treeapp wan");

	e = document.getElementById("lan");
	e.innerHTML = _("treeapp lan");

	e = document.getElementById("dhcp clients");
	e.innerHTML = _("treeapp dhcp clients");
	
	e = document.getElementById("vpn passthrough");
	e.innerHTML = _("treeapp vpn passthrough");

	e = document.getElementById("routing");
	e.innerHTML = _("treeapp routing");

	e = document.getElementById("qos");
	e.innerHTML =_("treeapp qos");	

/* ------------ wireless settings -------------*/

	e = document.getElementById("wireless settings");
	e.innerHTML = _("treeapp wireless settings");

	e = document.getElementById("basic");
	e.innerHTML =_("treeapp basic");

	e = document.getElementById("advanced");
	e.innerHTML =_("treeapp advanced"); 

	e = document.getElementById("security");
	e.innerHTML =_("treeapp security");
	
	e = document.getElementById("wds");
	e.innerHTML =_("treeapp wds");

	e = document.getElementById("wps");
	e.innerHTML =_("treeapp wps");

	e = document.getElementById("ap client");
	e.innerHTML =_("treeapp ap client");

	e = document.getElementById("station list");
	e.innerHTML =_("treeapp station list");

	e = document.getElementById("mesh settings");
	e.innerHTML =_("treeapp mesh settings");				

/* --------------------------------------------*/

	e = document.getElementById("profile");
	e.innerHTML =_("treeapp profile");

	e = document.getElementById("link status");
	e.innerHTML =_("treeapp link status");

	e = document.getElementById("site survey");
	e.innerHTML =_("treeapp site survey");

	e = document.getElementById("site statistics");
	e.innerHTML =_("treeapp statistics");
	
	e = document.getElementById("site advance");
	e.innerHTML =_("treeapp advance");

	e = document.getElementById("site qos");
	e.innerHTML =_("treeapp qos");
	
	e = document.getElementById("11n configurations");
	e.innerHTML =_("treeapp 11n configurations");
	
	e = document.getElementById("about");
	e.innerHTML =_("treeapp about");
	
	e = document.getElementById("site wps");
	e.innerHTML =_("treeapp wps");
	
/* ----------------- firewall -----------------*/

	e = document.getElementById("firewall");
	e.innerHTML = _("treeapp firewall");

	e = document.getElementById("port filtering");
	e.innerHTML =_("treeapp ip/port filtering"); 

	e = document.getElementById("port forwarding");
	e.innerHTML =_("treeapp port forwarding"); 

	e = document.getElementById("dmz");
	e.innerHTML =_("treeapp dmz"); 

	e = document.getElementById("system firewall");
	e.innerHTML =_("treeapp system firewall"); 

	e = document.getElementById("content filtering");
	e.innerHTML =_("treeapp content filtering"); 

/* ------------- administration ----------------*/

	e = document.getElementById("administration");
	e.innerHTML = _("treeapp administration");

	e = document.getElementById("management");
	e.innerHTML =_("treeapp management");

	e = document.getElementById("upload firmware");
	e.innerHTML =_("treeapp upload firmware");

	e = document.getElementById("settings management");
	e.innerHTML =_("treeapp settings management");

	e = document.getElementById("status");
	e.innerHTML =_("treeapp status");

	e = document.getElementById("statistics");
	e.innerHTML =_("treeapp statistics");

	e = document.getElementById("system log");
	e.innerHTML =_("treeapp system log");	
	
	if( opmode != 0 )
		e = document.getElementById("3g budget status").innerHTML =_("treeapp 3g budget status");
	
/* ----------------wizard Step-------------------*/
	/*document.getElementById("wizard lang").innerHTML =_("treeapp wizard lang");
	document.getElementById("wizard time").innerHTML =_("treeapp wizard time");
	document.getElementById("wizard wan").innerHTML =_("treeapp wizard wan");
	document.getElementById("wizard security").innerHTML =_("treeapp wizard security");
	document.getElementById("wizard finish").innerHTML =_("treeapp wizard finish"); */
}

function initValue()
{

/* ----------------------------------------------------------------------------- */

	if (opmode == '0')
	{
		document.getElementById("wan").style.display="NONE";
		document.getElementById("wan_hr").style.display="NONE";

		document.getElementById("routing").style.display="NONE";
		document.getElementById("routing_hr").style.display="NONE";

		document.getElementById("firewall").style.display="NONE";
	}

	if (opmode != '3')
	{
		document.getElementById("ap client").style.display="NONE";
		document.getElementById("ap client_hr").style.display="NONE";
	}
/* ----------------------------------------------------------------------------- */
			
	if (dhcpen != "1")
	{
		document.getElementById("dhcp clients").style.display="NONE";
		document.getElementById("dhcp clients_hr").style.display="NONE";
	}
	if (vpnen != "1")
	{
		document.getElementById("vpn passthrough").style.display="NONE";
		document.getElementById("vpn passthrough_hr").style.display="NONE";
	}
	if (swqos == '1')
	{
		document.getElementById("qos").style.display="NONE";
		document.getElementById("qos_hr").style.display="NONE";
	}

/* ----------------------------------------------------------------------------- */

	if ((opmode == '0' && dpbsta == '1' && ethconv == '1') || opmode == '2')
	{
		document.getElementById("basic").style.display="NONE";
		document.getElementById("basic_hr").style.display="NONE";
		
		document.getElementById("advanced").style.display="NONE";
		document.getElementById("advanced_hr").style.display="NONE";

		document.getElementById("security").style.display="NONE";
		document.getElementById("security_hr").style.display="NONE";

		document.getElementById("wds").style.display="NONE";
		document.getElementById("wds_hr").style.display="NONE";

		document.getElementById("wps").style.display="NONE";
		document.getElementById("wps_hr").style.display="NONE";
		
		document.getElementById("ap client").style.display="NONE";
		document.getElementById("ap client_hr").style.display="NONE";
		
		document.getElementById("antenna diversity").style.display="NONE";
		document.getElementById("antenna diversity_hr").style.display="NONE";

		document.getElementById("mesh settings").style.display="NONE";
		document.getElementById("mesh settings_hr").style.display="NONE";

		/*document.getElementById("wds").style.display="NONE";
		document.getElementById("wds_hr").style.display="NONE";

		document.getElementById("wps").style.display="NONE";
		document.getElementById("wps_hr").style.display="NONE";

		document.getElementById("antenna diversity").style.display="NONE";
		document.getElementById("antenna diversity_hr").style.display="NONE";

		document.getElementById("mesh settings").style.display="NONE";			
		document.getElementById("mesh settings_hr").style.display="NONE";*/			

		if (wscb != "1")
		{
			document.getElementById("site wps").style.display="NONE";	
			document.getElementById("site wps_hr").style.display="NONE";
		}
	}
	else
	{
		document.getElementById("profile").style.display="NONE";
		document.getElementById("profile_hr").style.display="NONE";
		
		document.getElementById("link status").style.display="NONE";
		document.getElementById("link status_hr").style.display="NONE";

		document.getElementById("site survey").style.display="NONE";
		document.getElementById("site survey_hr").style.display="NONE";

		document.getElementById("site statistics").style.display="NONE";
		document.getElementById("site statistics_hr").style.display="NONE";
		
		document.getElementById("site advance").style.display="NONE";
		document.getElementById("site advance_hr").style.display="NONE";

		document.getElementById("site qos").style.display="NONE";
		document.getElementById("site qos_hr").style.display="NONE";

		document.getElementById("11n configurations").style.display="NONE";
		document.getElementById("11n configurations_hr").style.display="NONE";
		
		document.getElementById("about").style.display="NONE";
		document.getElementById("about_hr").style.display="NONE";
		
		document.getElementById("site wps").style.display="NONE";		
		document.getElementById("site wps_hr").style.display="NONE";		

		if (wdsb != "1")
		{
			document.getElementById("wds").style.display="NONE";
			document.getElementById("wds_hr").style.display="NONE";
		}
		if (wscb != "1")
		{
			document.getElementById("wps").style.display="NONE";
			document.getElementById("wps_hr").style.display="NONE";
		}
		if (ad != '1')
		{
			document.getElementById("antenna diversity").style.display="NONE";
			document.getElementById("antenna diversity_hr").style.display="NONE";
		}
		if (meshb != "1")
		{
			document.getElementById("mesh settings").style.display="NONE";
			document.getElementById("mesh settings_hr").style.display="NONE";
		}
	}
	
/* ----------------------------------------------------------------------------- */

	initTranslation();
}
	
function setMenu()
{
	initTranslation();
	fnDispThis(1);
}
function updateLang()
{
	self.initTranslation();
	//initTranslation();
	window.location.reload();
}
function web_page_selected(row, column){
	fnDispThis(row);
	subMenu(row); 
	clicked_subMenu(row,column);
}	
function redirectebydiffip(ip){
	new_ip = ip;
	var	 wanipAssi = "<% getCfgGeneral(1, "wan_ip_assignment"); %>";
	/* static, Dynamic, pppoe, 3G, pptp, l2tp */
	var  delayTime = Array("120", "120", "180", "120", "120", "120"); 
	setTimeout("top.location = \"http://\" + new_ip.toString() + \"/home.asp\";", delayTime[wanipAssi] * 1000);
}
var new_ip = "";
</SCRIPT>
<base target="main2Frame">
</head>

<body bottommargin="0px" onLoad="initValue()" link="#FFFFFF" vlink="#FFFFFF" alink="#FFFFFF">
<div align="center">
  <center>
<table width="876" border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse" >
  
  <tr>
    <td border="0" align="center" colspan="2" width="876" height="82" dir="ltr">
<img src="image/logo.jpg" align="center" /></td>
  </tr>
  <tr>
    <td border="0" align="center" colspan="2" width="876" height="1" dir="ltr">
<img src="image/top_hr.jpg" align="center" /></td>
  </tr>
  
  <tr>
    <td colspan="2" width="876" align="left"><div align="center">
    	<table width="876" border="0" cellspacing="0" cellpadding="0"  bgcolor="#4F7599" style="border-collapse: collapse">
      		<tr>
      			 <!--<td width="174" height="38" id="wizard list" class="list" nowrap="nowrap" onClick="fnDispThis(0)">
                 	<a href="wizard/wizard_langset.asp" target="mainFrame">Wizard</a></td>
                 </td>-->

      			 <td width="174" height="38" class="list" nowrap="nowrap" onClick="fnDispThis(0)"></td>
              
                	<td height="38" class="list" onClick="fnDispThis(1); clicked_subMenu('1', '1');" id="wizard list_td">
                 	<a href="awb_index_home.asp" target="mainFrame" id="wizard list">Setup Wizard</a></td>
                                               	
			<td height="38" class="list" onClick="fnDispThis(2); clicked_subMenu('2', '1');">
                	<a href="opmode.asp" target="mainFrame" id="operation mode">Operation</a></td>
                	
                 <!--<td width="130" height="38" class="list" nowrap="nowrap" onClick="fnDispThis(3)">
                	<a href="internet/wan.asp" target="mainFrame" id="internet settings" >Internet</a></td>-->

			    <td width="130" height="38" class="list" nowrap="nowrap" onClick="fnDispThis(3); clicked_subMenu('3', '1');">
			                    	
				<script language="JavaScript" type="text/javascript">	
				if (opmode == '0')
	            	document.write("<a href=\"internet/lan.asp\" target=\"mainFrame\" id=\"internet settings\" >Internet</a></td>");
	            else
	                document.write("<a href=\"internet/wan.asp\" target=\"mainFrame\" id=\"internet settings\" >Internet</a></td>");
				</script>
                	</td>
			    <!--<td height="38" class="list" onClick="fnDispThis(4)">
                	<a href="wireless/basic.asp" target="mainFrame" id="wireless settings" >Wireless</a></td>-->
                	
                <td height="38" class="list" onClick="fnDispThis(4); clicked_subMenu('4', '1');">
                
                <script language="JavaScript" type="text/javascript">
                if (opmode == '2')
	                document.write("<a href=\"station/site_survey.asp\" target=\"mainFrame\" id=\"wireless settings\" >Wireless</a></td>");
				else
                	document.write("<a href=\"wireless/basic.asp\" target=\"mainFrame\" id=\"wireless settings\" >Wireless</a></td>");
                </script></td>
                		
			    <td height="38" class="list" onClick="fnDispThis(5); clicked_subMenu('5', '1');">
                	<a href="firewall/port_filtering.asp" target="mainFrame" id="firewall">Firewall</a></td>
              	                	
			    <td width="130" height="38" class="list" nowrap="nowrap" onClick="fnDispThis(6); clicked_subMenu('6', '1');">
                	<a href="adm/management.asp" target="mainFrame" id="administration" >Admin</a></td>
   		  </tr>
    	</table>
	    </div></td>
  </tr>
  <tr>
  	<td colspan="2" width="876" align="left">
    	<div align="center">
          <center>
    	<table width="876" border="0" cellspacing="0" cellpadding="0" height="16"  bgcolor="#4F7599" style="border-collapse: collapse">
        	<tr>
            	<td height="545" class="sublist" width="313" valign="top">
            		
            		<!--<br>
				   	<DIV class="substyle" id="sub0" style="width: 173; height: 19">
						<p align="center">
						<a> <font color="#FFFFFF">[</font></a> <b>
						<a> <font color="#FFFFFF" id="null_sublist">aaaaaa</font></a></b>
						<a> <font color="#FFFFFF">]</font></a>
					</DIV>-->
						
					<br><br>
				    <DIV class="substyle" id="sub1">
					<font face="Verdana">
						<a href="awb_index_home.asp" target="mainFrame" onClick="subMenu('1'); clicked_subMenu('1','1');" id="setup wizard" style="text-decoration: none">Setup Wizard</a>
						<a id="setup wizard_hr"><hr></a>
						<!--<a id="wizard lang" style="text-decoration: none">Wizard Setup 1</a>
	                    <a id="wizard time" style="text-decoration: none">Wizard Setup 2</a>
						<a id="wizard wan" style="text-decoration: none">Wizard Setup 3</a>
	                    <a id="wizard security" style="text-decoration: none">Wizard Setup 4</a>
	                    <a id="wizard finish" style="text-decoration: none">Wizard Success</a> -->
					</font>
					</DIV>
							
            		<DIV class="substyle" id="sub2">
					<font face="Verdana">
						<a href="opmode.asp" target="mainFrame" onClick="subMenu('2'); clicked_subMenu('2','1');" id="op mode" style="text-decoration: none">Mode</a>
						<a id="op mode_hr"><hr></a>
					</font>
					</DIV>

                    <DIV class="substyle" id="sub3">
                    <font face="Verdana">
	                    <a href="internet/wan.asp" target="mainFrame" onClick="subMenu('3'); clicked_subMenu('3','1');" id="wan" style="text-decoration: none">Wan</a>
	                    <a id="wan_hr"><hr></a>
	                    <a href="internet/lan.asp" target="mainFrame" onClick="subMenu('3'); clicked_subMenu('3','2');" id="lan" style="text-decoration: none">Lan</a>
	                    <a id="lan_hr"><hr></a>
	                    <a href="internet/dhcpcliinfo.asp" target="mainFrame" onClick="subMenu('3'); clicked_subMenu('3','3');" id="dhcp clients" style="text-decoration: none">DHCP Clients </a>
	                    <a id="dhcp clients_hr"><hr></a>
	                    <a href="internet/vpnpass.asp" target="mainFrame" onClick="subMenu('3'); clicked_subMenu('3','4');" id="vpn passthrough" style="text-decoration: none">VPN PassThrough</a>
	                    <a id="vpn passthrough_hr"><hr></a>
	                    <a href="internet/routing.asp" target="mainFrame" onClick="subMenu('3'); clicked_subMenu('3','5');" id="routing" style="text-decoration: none" >Routing</a>
	                    <a id="routing_hr"><hr></a>
	                    <a href="internet/qos.asp" target="mainFrame" onClick="subMenu('3'); clicked_subMenu('3','6');" id="qos" style="text-decoration: none" >QOS</a>
	                    <a id="qos_hr"><hr></a>
                    </font>
					</DIV>
                    
					<DIV class="substyle" id="sub4">
					<font face="Verdana">
						<a href="wireless/basic.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','1');" id="basic" style="text-decoration: none">Basic </a>
							<a id="basic_hr"><hr></a>
        	            <a href="wireless/advanced.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','2');" id="advanced" style="text-decoration: none">Advanced</a>
							<a id="advanced_hr"><hr></a>
                    	<a href="wireless/security.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','3');" id="security" style="text-decoration: none">Security</a>
		                    <a id="security_hr"><hr></a>
						<a href="wireless/wds.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','4');" id="wds" style="text-decoration: none">WDS</a>
		                    <a id="wds_hr"><hr></a>
	                    <a href="wps/wps.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','5');" id="wps" style="text-decoration: none">WPS</a>
		                    <a id="wps_hr"><hr></a>
	                    <a href="wireless/apcli.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','6');" id="ap client" style="text-decoration: none">AP Client</a>
							<a id="ap client_hr"><hr></a>
	                    <a href="wireless/stainfo.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','7');" id="station list" style="text-decoration: none">Station List</a>
							<a id="station list_hr"><hr></a>
	                    <a href="wireless/ant_diversity.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','8');" id="antenna diversity" style="text-decoration: none">Antenna Diversity</a>
							<a id="antenna diversity_hr"><hr></a>
	                    <a href="wireless/mesh.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','9');" id="mesh settings" style="text-decoration: none">mesh settings</a>
							<a id="mesh settings_hr"><hr></a>
	                    <a href="station/profile.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','10');" id="profile" style="text-decoration: none">profile</a>
							<a id="profile_hr"><hr></a>
	                    <a href="station/link_status.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','11');" id="link status" style="text-decoration: none">Link Status</a>
							<a id="link status_hr"><hr></a>
	                    <a href="station/site_survey.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','12');" id="site survey" style="text-decoration: none">Survey</a>
							<a id="site survey_hr"><hr></a>
	                    <a href="station/statistics.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','13');" id="site statistics" style="text-decoration: none">Statistics</a>
							<a id="site statistics_hr"><hr></a>
	                    <a href="station/advance.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','14');" id="site advance" style="text-decoration: none">Advance</a>
							<a id="site advance_hr"><hr></a>
	                    <a href="station/qos.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','15');" id="site qos" style="text-decoration: none">Qos</a>
							<a id="site qos_hr"><hr></a>
	                    <a href="station/11n_cfg.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','16');" id="11n configurations" style="text-decoration: none">Configurations</a>
							<a id="11n configurations_hr"><hr></a>
	                    <a href="station/about.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','17');" id="about" style="text-decoration: none">About</a>
							<a id="about_hr"><hr></a>
	                    <a href="wps/wps_sta.asp" target="mainFrame" onClick="subMenu('4'); clicked_subMenu('4','18');" id="site wps" style="text-decoration: none">WPS</a>
							<a id="site wps_hr"><hr></a>
					</font>
                    </DIV>
		
                   	<DIV class="substyle" id="sub5">
					<font face="Verdana">
						<a href="firewall/port_filtering.asp" target="mainFrame" onClick="subMenu('5'); clicked_subMenu('5','1');" id="port filtering" style="text-decoration: none">Port filtering</a>
						<a id="port filtering_hr"><hr></a>
	                    <a href="firewall/port_forward.asp" target="mainFrame" onClick="subMenu('5'); clicked_subMenu('5','2');" id="port forwarding" style="text-decoration: none">Port forwarding</a>
	                    <a id="port forwarding_hr"><hr></a>
	                    <a href="firewall/DMZ.asp" target="mainFrame" onClick="subMenu('5'); clicked_subMenu('5','3');" id="dmz" style="text-decoration: none">DMZ</a>
	                    <a id="dmz_hr"><hr></a>
						<a href="firewall/system_firewall.asp" target="mainFrame" onClick="subMenu('5'); clicked_subMenu('5','4');" id="system firewall" style="text-decoration: none">System Firewall</a>
						<a id="system firewall_hr"><hr></a>
	                    <a href="firewall/content_filtering.asp" target="mainFrame" onClick="subMenu('5'); clicked_subMenu('5','5');" id="content filtering" style="text-decoration: none">Content Filtering</a>
	                    <a id="content filtering_hr"><hr></a>
                    </font> 
                  	</DIV>

                    <DIV class="substyle" id="sub6">
					<font face="Verdana">
						<a href="adm/management.asp" target="mainFrame" onClick="subMenu('6'); clicked_subMenu('6','1');" id="management" style="text-decoration: none">Management</a>
						<a id="management_hr"><hr></a>
	                    <a href="adm/upload_firmware.asp" target="mainFrame" onClick="subMenu('6'); clicked_subMenu('6','2');" id="upload firmware" style="text-decoration: none">Upload Firmware</a>
	                    <a id="upload firmware_hr"><hr></a>
	                    <a href="adm/settings.asp" target="mainFrame" onClick="subMenu('6'); clicked_subMenu('6','3');" id="settings management" style="text-decoration: none">Settings Management</a>
	                    <a id="settings management_hr"><hr></a>
						<a href="adm/status.asp" target="mainFrame" onClick="subMenu('6'); clicked_subMenu('6','4');" id="status" style="text-decoration: none">Status</a>
						<a id="status_hr"><hr></a>
	                    <a href="adm/statistic.asp" target="mainFrame" onClick="subMenu('6'); clicked_subMenu('6','5');" id="statistics" style="text-decoration: none">Statistics</a>
	                    <a id="statistics_hr"><hr></a>
	                    <a href="adm/syslog.asp" target="mainFrame" onClick="subMenu('6'); clicked_subMenu('6','6');" id="system log" style="text-decoration: none">System Log</a>
	                    <a id="system log_hr"><hr></a>
                        <script language="JavaScript" type="text/javascript">	
						if (opmode != '0'){
	            			document.write("<a href=\"adm/status3g.asp\" target=\"mainFrame\" onClick=\"subMenu('6'); clicked_subMenu('6','7');\" id=\"3g budget status\" >3G Budget Status</a>");
							document.write("<a id=\"3g budget status_hr\"></a>");
						}
						</script>                       
                    </font> 
                  	</DIV>

        		</td>
            	<td height="545" class="sublist" width="714" align="left" valign="top"  bgcolor="#DBE2EC">

            			<font face="Verdana">

            			<iframe id="inlineFrame" name="mainFrame" src="awb_index_home.asp" align="top" border="0" frameborder="0" width="680" height="533" bgcolor="#DBE2EC" target="mainFrame">
                        </iframe>
        		        </font>
        		</td>
           	</tr>
        	<tr>
            	<td height="16" class="sublist" width="313" align="center">&nbsp;
				</td>
            	<td height="16" class="sublist" width="714" align="center"  bgcolor="#DBE2EC">
<p align="center">
<font color="#808080" face="Arial">&nbsp;</font><font face="Arial">
</font>
</td>
           	</tr>
      	</table>
    	        	
          </center>
        </div>
    	        	
  </tr>
</table>

  </center>
</div>

<script language="JavaScript" type="text/javascript">
function subMenu(obj)
{
	clickItem = obj;
	
	fnDisPatch(clickItem);
}
        </script>
</body>
</html>
