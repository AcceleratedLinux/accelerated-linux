<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>DHCP Client List</title>

<script type="text/javascript" src="/lang/b28n.js"></script>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("internet");

function initValue()
{
	var e = document.getElementById("dTitle");
	e.innerHTML = _("dhcp title");
	e = document.getElementById("dIntroduction");
	e.innerHTML = _("dhcp introduction");
	e = document.getElementById("dClients");
	e.innerHTML = _("dhcp clients");
	e = document.getElementById("dHostname");
	e.innerHTML = _("inet hostname");
	e = document.getElementById("dMac");
	e.innerHTML = _("inet mac");
	e = document.getElementById("dIp");
	e.innerHTML = _("inet ip");
	e = document.getElementById("dExpr");
	e.innerHTML = _("dhcp expire");
}
</script>
</head>


<body onLoad="initValue()" bgcolor="#FFFFFF">
<div align="center">
 <center>
<table class="body"><tr><td>

<table width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">

<tr>
  <td class="title" colspan="2" id="dTitle">Local Area Network (LAN) Settings</td>
</tr>
<tr>
<td colspan="2">
<p class="head" id="dIntroduction"></p>
</td>
</tr>

</table>

<br>

<table width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="4" id="dClients">DHCP Clients</td>
  </tr>
  <tr>
    <td class="head" bgcolor=#DBE2EC id="dHostname">Hostname</td>
    <td class="head" bgcolor=#DBE2EC id="dMac">MAC Address</td>
    <td class="head" bgcolor=#DBE2EC id="dIp">IP Address</td>
    <td class="head" bgcolor=#DBE2EC id="dExpr">Expires in</td>
  </tr>
  <% getDhcpCliList(); %>
</table>

</td></tr></table>
 </center>
</div>
</body>
</html>