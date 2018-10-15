<html><head><title>Statistic</title>

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("admin");

function initTranslation()
{
	var e = document.getElementById("statisticTitle");
	e.innerHTML = _("statistic title");
	e = document.getElementById("statisticIntroduction");
	e.innerHTML = _("statistic introduction");

	e = document.getElementById("statisticMM");
	e.innerHTML = _("statistic memory");
	e = document.getElementById("statisticMMTotal");
	e.innerHTML = _("statistic memory total");
	e = document.getElementById("statisticMMLeft");
	e.innerHTML = _("statistic memory left");

	e = document.getElementById("statisticWANLAN");
	e.innerHTML = _("statistic wanlan");
	e = document.getElementById("statisticWANRxPkt");
	e.innerHTML = _("statistic wan rx pkt");
	e = document.getElementById("statisticWANRxBytes");
	e.innerHTML = _("statistic wan rx bytes");
	e = document.getElementById("statisticWANTxPkt");
	e.innerHTML = _("statistic wan tx pkt");
	e = document.getElementById("statisticWANTxBytes");
	e.innerHTML = _("statistic wan tx bytes");
	e = document.getElementById("statisticLANRxPkt");
	e.innerHTML = _("statistic lan rx pkt");
	e = document.getElementById("statisticLANRxBytes");
	e.innerHTML = _("statistic lan rx bytes");
	e = document.getElementById("statisticLANTxPkt");
	e.innerHTML = _("statistic lan tx pkt");
	e = document.getElementById("statisticLANTxBytes");
	e.innerHTML = _("statistic lan tx bytes");

	e = document.getElementById("statisticAllIF");
	e.innerHTML = _("statistic all interface");
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

function style_display_on()
{
	if (window.ActiveXObject) { // IE
		return "block";
	}
	else if (window.XMLHttpRequest) { // Mozilla, Safari,...
		return "table-row";
	}
}

function PageInit()
{
	initTranslation();
	// hidden wan connection statistic
	var opmode = "<% getCfgZero(1, "OperationMode"); %>";
	if(opmode == 0){
		document.getElementById("statisticWANLAN_t").value = "LAN";
		id_invisible("statisticWANRxPkt_t");
		id_invisible("statisticWANRxBytes_t");
		id_invisible("statisticWANTxPkt_t");
		id_invisible("statisticWANTxBytes_t");
	}
}

function formCheck()
{
	if( document.SystemCommand.command.value == ""){
		alert("Please specify a command.");
		return false;
	}

	return true;
}

</script>

</head>
<body onLoad="PageInit()" bgcolor="#FFFFFF">
<div align="center">
 <center>

<table class="body"><tr><td>


<table width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="statisticTitle">Statistic</td>
</tr>
<tr>
<td colspan="2">
<p class="head" id="statisticIntroduction">SoC statistics </p>
</td>
</tr>

</table>

<br>

<table border="1" cellpadding="2" cellspacing="1" width="540">
<tbody>

<!-- =================  MEMORY  ================= -->
<tr>
  <td class="title" colspan="2" id="statisticMM">Memory</td>
</tr>
<tr>
  <td class="head" id="statisticMMTotal">Memory total: </td>
  <td> <% getMemTotalASP(); %>&nbsp;</td>
</tr>
<tr>
  <td class="head" id="statisticMMLeft">Memory left: </td>
  <td> <% getMemLeftASP(); %>&nbsp;</td>
</tr>


<!-- =================  WAN/LAN  ================== -->
<tr id="statisticWANLAN_t">
  <td class="title" colspan="2" id="statisticWANLAN">WAN/LAN</td>
</tr>
<tr id="statisticWANRxPkt_t">
  <td class="head" id="statisticWANRxPkt">WAN Rx packets: </td>
  <td> <% getWANRxPacketASP(); %>&nbsp;</td>
</tr>
<tr id="statisticWANRxBytes_t">
  <td class="head" id="statisticWANRxBytes">WAN Rx bytes: </td>
  <td> <% getWANRxByteASP(); %>&nbsp;</td>
</tr>
<tr id="statisticWANTxPkt_t">
  <td class="head" id="statisticWANTxPkt">WAN Tx packets: </td>
  <td> <% getWANTxPacketASP(); %>&nbsp;</td>
</tr>
<tr id="statisticWANTxBytes_t">
  <td class="head" id="statisticWANTxBytes">WAN Tx bytes: </td>
  <td> <% getWANTxByteASP(); %>&nbsp;</td>
</tr>
<tr>
  <td class="head" id="statisticLANRxPkt">LAN Rx packets: &nbsp; &nbsp; &nbsp; &nbsp;</td>
  <td> <% getLANRxPacketASP(); %>&nbsp;</td>
</tr>
<tr>
  <td class="head" id="statisticLANRxBytes">LAN Rx bytes: </td>
  <td> <% getLANRxByteASP(); %>&nbsp;</td>
</tr>
<tr>
  <td class="head" id="statisticLANTxPkt">LAN Tx packets: </td>
  <td> <% getLANTxPacketASP(); %>&nbsp;</td>
</tr>
<tr>
  <td class="head" id="statisticLANTxBytes">LAN Tx bytes: </td>
  <td> <% getLANTxByteASP(); %>&nbsp;</td>
</tr>

<!-- =================  ALL  ================= -->
<tr>
  <td class="title" colspan="2" id="statisticAllIF">All interfaces</td>
<tr>

<script type="text/javascript">
var i;
var a = new Array();
a = [<% getAllNICStatisticASP(); %>];
for(i=0; i<a.length; i+=5){
	// name
	document.write("<tr> <td class=head> " + _('statistic interface name') + "</td><td class=head>");
	document.write(a[i]);
	document.write("</td></tr>");

	// Order is important! rxpacket->rxbyte->txpacket->txbyte
	// rxpacket
	document.write("<tr> <td class=head> " + _('statistic interface rx packet') + " </td><td>");
	document.write(a[i+1]);
	document.write("</td></tr>");

	// rxbyte
	document.write("<tr> <td class=head> " + _('statistic interface rx byte') + " </td><td>");
	document.write(a[i+2]);
	document.write("</td></tr>");

	// txpacket
	document.write("<tr> <td class=head> " + _('statistic interface tx packet') + " </td><td>");
	document.write(a[i+3]);
	document.write("</td></tr>");

	// txbyte
	document.write("<tr> <td class=head> " + _('statistic interface tx byte') + " </td><td>");
	document.write(a[i+4]);
	document.write("</td></tr>");
}
</script>

</tbody>
</table>

</td></tr></table>

 </center>
</div>

</body></html>