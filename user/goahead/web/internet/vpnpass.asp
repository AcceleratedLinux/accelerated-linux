<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>VPN Passthrough</title>

<script type="text/javascript" src="/lang/b28n.js"></script>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("internet");

function initValue()
{
	var l2tp = <% getCfgZero(1, "l2tpPassThru"); %>;
	var ipsec = <% getCfgZero(1, "ipsecPassThru"); %>;
	var pptp = <% getCfgZero(1, "pptpPassThru"); %>;

	var e = document.getElementById("vTitle");
	e.innerHTML = _("vpn title");
	e = document.getElementById("vIntroduction");
	e.innerHTML = _("vpn introduction");
	e = document.getElementById("vPassThru");
	e.innerHTML = _("vpn pass thru");

	e = document.getElementById("vL2tpPassThru");
	e.innerHTML = _("vpn l2tp pass thru");
	e = document.getElementById("vL2tpD");
	e.innerHTML = _("inet disable");
	e = document.getElementById("vL2tpE");
	e.innerHTML = _("inet enable");

	e = document.getElementById("vIpsecPassThru");
	e.innerHTML = _("vpn ipsec pass thru");
	e = document.getElementById("vIpsecD");
	e.innerHTML = _("inet disable");
	e = document.getElementById("vIpsecE");
	e.innerHTML = _("inet enable");

	e = document.getElementById("vPptpPassThru");
	e.innerHTML = _("vpn pptp pass thru");
	e = document.getElementById("vPptpD");
	e.innerHTML = _("inet disable");
	e = document.getElementById("vPptpE");
	e.innerHTML = _("inet enable");

	e = document.getElementById("vApply");
	e.value = _("inet apply");
	e = document.getElementById("vCancel");
	e.value = _("inet cancel");

	document.vpnpass.l2tpPT.options.selectedIndex = 1*l2tp;
	document.vpnpass.ipsecPT.options.selectedIndex = 1*ipsec;
	document.vpnpass.pptpPT.options.selectedIndex = 1*pptp;
}
</script>
</head>


<body onLoad="initValue()">
<div align="center">
 <center>
<table class="body"><tr><td>

<h1 id="vTitle"></h1>
<p id="vIntroduction"></p>
<hr />

<form method=post name="vpnpass" action="/goform/setVpnPaThru">
<table width="95%" border="1" cellpadding="2" cellspacing="1">
  <tr> 
    <td class="title" colspan="3" id="vPassThru">VPN Pass Through</td>
  </tr>
  <tr>
    <td class="head" id="vL2tpPassThru">L2TP passthrough</td>
    <td>
      <select name="l2tpPT" size="1">
        <option value="0" id="vL2tpD">Disable</option>
        <option value="1" id="vL2tpE">Enable</option>
      </select>
    </td>
  </tr>
  <tr>
    <td class="head" id="vIpsecPassThru">IPSec passthrough</td>
    <td>
      <select name="ipsecPT" size="1">
        <option value="0" id="vIpsecD">Disable</option>
        <option value="1" id="vIpsecE">Enable</option>
      </select>
    </td>
  </tr>
  <tr>
    <td class="head" id="vPptpPassThru">PPTP passthrough</td>
    <td>
      <select name="pptpPT" size="1">
        <option value="0" id="vPptpD">Disable</option>
        <option value="1" id="vPptpE">Enable</option>
      </select>
    </td>
  </tr>
</table>

<table width="95%" cellpadding="2" cellspacing="1">
<tr align="center">
  <td>
    <input type=submit style="{width:120px;}" value="Apply" id="vApply">&nbsp;&nbsp;
    <input type=reset  style="{width:120px;}" value="Cancel" id="vCancel" onClick="window.location.reload()">
  </td>
</tr>
</table>
</form>

</td></tr></table>
 </center>
</div>
</body>
</html>


