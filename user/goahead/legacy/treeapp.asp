<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html>

<head>
<title>DTree</title>
<meta http-equiv="content-type" content="text/html;charset=utf-8" />
<link rel="stylesheet" href="/dtree/dtree.css" type="text/css" />
<link rel="StyleSheet" href="dtree.css" type="text/css" />
<script type="text/javascript" src="/dtree/dtree.js"></script>
<script type="text/javascript" src="/lang/b28n.js"></script>
</head>

<body bgcolor=#FFFFFF onLoad="initValue()">
<script language="JavaScript">    
var isFimwareUpload = 0;
Butterlate.setTextDomain("main");

function initValue()
{
	var e = document.getElementById("openall");
	e.innerHTML = _("treeapp openall");
	e = document.getElementById("closeall");
	e.innerHTML = _("treeapp closeall");
}

function setUnderFirmwareUpload(flag){
	isFimwareUpload = flag;
}
function go(zz) {
	if(!isFimwareUpload)
		top.view.location=zz;
}
function refresh(){
	window.location.reload(false);
}
</script>

<p><a href="javascript: a.openAll();" id="openall">open</a> | <a href="javascript: a.closeAll();" id="closeall">close</a></p>

<script type="text/javascript">
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

a = new dTree('a');
a.config.useStatusText=true;
a.config.useCookies=false;

//  nodeID, parent nodeID,  Name,  URL
a.add(000,  -1, _("treeapp ralink"),                "javascript:go('overview.asp');");
a.add(200,   0, _("treeapp operation mode"),        "javascript:go('opmode.asp');");
a.add(300,   0, _("treeapp internet settings"),     "javascript:a.oo(300);");
if (opmode != '0') {
	a.add(301, 300, _("treeapp wan"),                   "javascript:go('internet/wan.asp');");
}
a.add(302, 300, _("treeapp lan"),                   "javascript:go('internet/lan.asp');");
if (dhcpen == "1") {
	a.add(303, 300, _("treeapp dhcp clients"),          "javascript:go('internet/dhcpcliinfo.asp');");
}
if (vpnen == "1") {
	a.add(304, 300, _("treeapp vpn passthrough"),       "javascript:go('internet/vpnpass.asp');");
}
if (opmode != '0') {
	a.add(305, 300, _("treeapp routing"),       "javascript:go('internet/routing.asp');");
}

if (swqos == '1') {
	a.add(306, 300, _("treeapp qos"),		"javascript:go('internet/qos.asp');");
}

if ((opmode == '0' && dpbsta == '1' && ethconv == '1') || opmode == '2')
{
	a.add(400,   0, _("treeapp wireless settings"),     "javascript:a.oo(400);");
	a.add(401, 400, _("treeapp profile"),               "javascript:go('station/profile.asp');");
	a.add(402, 400, _("treeapp link status"),           "javascript:go('station/link_status.asp');");
	a.add(403, 400, _("treeapp site survey"),           "javascript:go('station/site_survey.asp');");
	a.add(404, 400, _("treeapp statistics"),            "javascript:go('station/statistics.asp');");
	a.add(405, 400, _("treeapp advance"),               "javascript:go('station/advance.asp');");
	a.add(406, 400, _("treeapp qos"),                   "javascript:go('station/qos.asp');");
	a.add(407, 400, _("treeapp 11n configurations"),    "javascript:go('station/11n_cfg.asp');");
	a.add(408, 400, _("treeapp about"),                 "javascript:go('station/about.asp');");
	if (wscb == "1")
		a.add(409, 400, _("treeapp wps"),                   "javascript:go('wps/wps_sta.asp');");
}
else
{
	a.add(400,   0, _("treeapp wireless settings"),     "javascript:a.oo(400);");
	a.add(401, 400, _("treeapp basic"),                 "javascript:go('wireless/basic.asp');");
	a.add(402, 400, _("treeapp advanced"),              "javascript:go('wireless/advanced.asp');");
	a.add(403, 400, _("treeapp security"),              "javascript:go('wireless/security.asp');");
	if (wdsb == "1")
	{
		a.add(404, 400, _("treeapp wds"),                   "javascript:go('wireless/wds.asp');");
	}
	if (opmode == '3')
		a.add(405, 400, _("treeapp ap client"),     "javascript:go('wireless/apcli.asp');");
	a.add(406, 400, _("treeapp station list"),          "javascript:go('wireless/stainfo.asp');");

	if (ad == '1')
		a.add(407, 400, "Antenna Diversity",			"javascript:go('wireless/ant_diversity.asp');");
	if (meshb == "1")
	{
		a.add(410, 400, _("treeapp mesh settings"),     "javascript:go('wireless/mesh.asp');");
	}
}
var rai = "<% getIfLiveWeb("rai0"); %>";
if (rai == "1") {
	a.add(500,   0, _("treeapp inic settings"),         "javascript:a.oo(500);");
	a.add(501, 500, _("treeapp basic"),                 "javascript:go('inic/basic.asp');");
	a.add(502, 500, _("treeapp advanced"),              "javascript:go('inic/advanced.asp');");
	a.add(503, 500, _("treeapp security"),              "javascript:go('inic/security.asp');");
	a.add(504, 500, _("treeapp wps"),                   "javascript:go('wps/wps_inic.asp');");
}
var raL = "<% getIfLiveWeb("raL0"); %>";
if (raL == "1") {
	a.add(600,   0, _("treeapp legacy settings"),       "javascript:a.oo(600);");
	a.add(601, 600, _("treeapp basic"),                 "javascript:go('legacy/basic.asp');");
	a.add(602, 600, _("treeapp advanced"),              "javascript:go('legacy/advanced.asp');");
	a.add(603, 600, _("treeapp security"),              "javascript:go('legacy/security.asp');");
	// a.add(604, 600, 'WPS',			"javascript:go('wps/wps_inic.asp');");
	a.add(605, 600, _("treeapp station list"),          "javascript:go('legacy/stainfo.asp');");
}

if (opmode != '0') {
	a.add(700,   0, _("treeapp firewall"),              "javascript:a.oo(700);");
	a.add(701, 700, _("treeapp ip/port filtering"),     "javascript:go('firewall/port_filtering.asp');");
	a.add(703, 700, _("treeapp port forwarding"),       "javascript:go('firewall/port_forward.asp');");
	a.add(704, 700, _("treeapp dmz"),                   "javascript:go('firewall/DMZ.asp');");
	a.add(705, 700, _("treeapp system firewall"),       "javascript:go('firewall/system_firewall.asp');");
	a.add(706, 700, _("treeapp content filtering"),     "javascript:go('firewall/content_filtering.asp');");
}

if (usbb == "1")
{
	if ((webcamb == "1") || (printersrvb == "1") || (usbiNICb == "1"))
		a.add(800,   0, _("treeapp usb"),		"javascript:a.oo(800);");
	if (webcamb == "1")
		a.add(801, 800, _("treeapp webcam"),		"javascript:go('usb/UVCwebcam.asp');");
	if (printersrvb == "1")
		a.add(802, 800, _("treeapp printersrv"),	"javascript:go('usb/P910NDprintersrv.asp');");
	if (usbiNICb == "1")
		a.add(803, 800, _("treeapp usbinic"),		"javascript:go('usb/INICusb_inic.asp');");
	if (storageb == "1")
	{
		a.add(850,   0, _("treeapp storage"),		"javascript:a.oo(850);");
		a.add(851, 850, _("treeapp useradmin"),	"javascript:go('usb/STORAGEuser_admin.asp');");
		a.add(852, 850, _("treeapp disk"),		"javascript:go('usb/STORAGEdisk_admin.asp');");
		if (ftpb == "1")
			a.add(853, 850, _("treeapp ftpsrv"),		"javascript:go('usb/STORAGEftpsrv.asp');");
		if (smbb == "1")
			a.add(854, 850, _("treeapp sambasrv"),		"javascript:go('usb/STORAGEsmbsrv.asp');");
		if (mediab == "1")
			a.add(855, 850, _("treeapp mediasrv"),		"javascript:go('usb/USHAREmediasrv.asp');");
	}
}

a.add(900,   0, _("treeapp administration"),        "javascript:a.oo(900);");
a.add(901, 900, _("treeapp management"),            "javascript:go('adm/management.asp');");
a.add(902, 900, _("treeapp upload firmware"),       "javascript:go('adm/upload_firmware.asp');");
a.add(903, 900, _("treeapp settings management"),   "javascript:go('adm/settings.asp');");
a.add(904, 900, _("treeapp status"),                "javascript:go('adm/status.asp');");
a.add(905, 900, _("treeapp statistics"),            "javascript:go('adm/statistic.asp');");
a.add(906, 900, _("treeapp system command"),        "javascript:go('adm/system_command.asp');");
a.add(908, 900, _("treeapp system log"),            "javascript:go('adm/syslog.asp');");
a.add(907, 900, _("treeapp sdk history"),           "javascript:go('cgi-bin/history.sh');");
document.write(a);
</script>

</body>

</html>
