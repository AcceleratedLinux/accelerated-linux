<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Ralink Wireless Station Advanced Configurations</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

function style_display_on()
{
	if (window.ActiveXObject) { // IE
		return "block";
	}
	else if (window.XMLHttpRequest) { // Mozilla, Safari,...
		return "table-row";
	}
}

function wirelessModeChange()
{
	//For Country Region
	var nmode = 1*document.sta_advance.wireless_mode.value;
	document.sta_advance.country_region_a.disabled = false;
	document.sta_advance.country_region_bg.disabled = false;
	if (nmode == 0 || nmode == 1 || nmode == 6 || nmode == 9)
		document.sta_advance.country_region_a.disabled = true;
	else if (nmode == 2 || nmode == 8)
		document.sta_advance.country_region_bg.disabled = true;
	
	/*
	document.getElementById("div_tx_rate").style.visibility = "hidden";
	document.getElementById("div_tx_rate").style.display = "none";
	*/

	//For 11n
	document.getElementById("div_ht_phy_mode").style.visibility = "hidden";
	document.getElementById("div_ht_phy_mode").style.display = "none";

	if (nmode >= 5) {
		document.getElementById("div_ht_phy_mode").style.visibility = "visible";
		if (window.ActiveXObject) // IE
			document.getElementById("div_ht_phy_mode").style.display = "block";
		else if (window.XMLHttpRequest)  // Mozilla, Safari,...
			document.getElementById("div_ht_phy_mode").style.display = "table";
	}
	/*
	else {
		document.getElementById("div_tx_rate").style.visibility = "visible";
		document.getElementById("div_tx_rate").style.display = style_display_on();
	}
	*/
}

function RadioStatusChange(rs)
{
	if (rs == 1) {
		document.sta_advance.radioButton.value = "RADIO OFF";
		document.sta_advance.radiohiddenButton.value = 0;
	}
	else {
		document.sta_advance.radioButton.value = "RADIO ON";
		document.sta_advance.radiohiddenButton.value = 1;
	}
}

function initTranslation()
{
	var e = document.getElementById("staadvTitle");
	e.innerHTML = _("staadv title");
	e = document.getElementById("staadvIntroduction");
	e.innerHTML = _("staadv introduction");

	e = document.getElementById("staadvConfig");
	e.innerHTML = _("staadv config");
	e = document.getElementById("staadvInfra");
	e.innerHTML = _("staadv infra");
	e = document.getElementById("staadvCountry");
	e.innerHTML = _("staadv country");
	e = document.getElementById("staadvBGProtect");
	e.innerHTML = _("staadv bgprotect");
	e = document.getElementById("staadvBGProAuto");
	e.innerHTML = _("wireless auto");
	e = document.getElementById("staadvBGProOn");
	e.innerHTML = _("wireless on");
	e = document.getElementById("staadvBGProOff");
	e.innerHTML = _("wireless off");
	/*
	e = document.getElementById("staadvTxRate");
	e.innerHTML = _("staadv tx rate");
	e = document.getElementById("staadvTxRateAuto");
	e.innerHTML = _("wireless auto");
	*/
	e = document.getElementById("staadvTxBurst");
	e.innerHTML = _("adv tx burst");
	e = document.getElementById("staadvHTPhyMode");
	e.innerHTML = _("basic ht phy mode");
	e = document.getElementById("staadvHT");
	e.innerHTML = _("link ht");
	e = document.getElementById("staadvBW");
	e.innerHTML = _("staadv bw");
	e = document.getElementById("staadvBWAuto");
	e.innerHTML = _("wireless auto");
	e = document.getElementById("staadvGI");
	e.innerHTML = _("staadv gi");
	e = document.getElementById("staadvGILong");
	e.innerHTML = _("wireless long");
	e = document.getElementById("staadvGIAuto");
	e.innerHTML = _("wireless auto");
	e = document.getElementById("staadvMCSAuto");
	e.innerHTML = _("wireless auto");
	e = document.getElementById("staadvApply");
	e.value = _("wireless apply");
}

function initValue()
{
	var is3t3r = '<% is3t3r(); %>';
	var ht_mcs = '<% getCfgZero(1, "HT_MCS"); %>';

	if (1*is3t3r == 1) {
		for (i = 16; i < 24; i++) {
			document.sta_advance.n_mcs.options[i] = new Option(i, i);
		}
	}
	var mcs_length = document.sta_advance.n_mcs.options.length;
	document.sta_advance.n_mcs.options[mcs_length] = new Option("32", "32");
	mcs_length++;
	document.sta_advance.n_mcs.options[mcs_length] = new Option("Auto", "33");
	document.sta_advance.n_mcs.options[mcs_length].id = "staadvMCSAuto";

	if (1*ht_mcs <= mcs_length-1)
		document.sta_advance.n_mcs.options.selectedIndex = ht_mcs;
	else if (1*ht_mcs == 32)
		document.sta_advance.n_mcs.options.selectedIndex = mcs_length-1;
	else if (1*ht_mcs == 33)
		document.sta_advance.n_mcs.options.selectedIndex = mcs_length;

}

function PageInit()
{
	initValue();
	initTranslation();
	wirelessModeChange();
}
</script>
</head>


<body onload="PageInit()">
<table class="body"><tr><td>

<h1 id="staadvTitle">Station Advanced Configurations</h1>
<p id="staadvIntroduction">The Status page shows the settings and current operation status of the Station.</p>
<hr />

<form method=post name="sta_advance" action="/goform/setStaAdvance">
<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td class="title" colspan="6" id="staadvConfig">Advance Configuration</td>
  </tr>
  <tr>
    <td class="head" id="staadvInfra">Wireless Mode(Infra)</td>
    <td >
      <select id="wireless_mode" name="wireless_mode" size="1" onChange="wirelessModeChange()">
        <% getStaWirelessMode(); %>
      </select>
    </td>
  </tr>
  <tr>
    <td class="head" id="staadvCountry" <% amode = getStaSuppAMode();
        if (amode == "1") write("rowspan=2"); %>>Country Region Code</td>
    <td>11 B/G &nbsp;&nbsp;&nbsp;
      <select id="country_region_bg" name="country_region_bg" size="1">
        <option value=0 <% var cr_bg = getCfgZero(0, "CountryRegion");
                           if (cr_bg == "0") write("selected"); %>>0:CH1-11</option>
        <option value=1 <% if (cr_bg == "1") write("selected"); %>>1:CH1-13</option>
        <option value=2 <% if (cr_bg == "2") write("selected"); %>>2:CH10-11</option>
        <option value=3 <% if (cr_bg == "3") write("selected"); %>>3:CH10-13</option>
        <option value=4 <% if (cr_bg == "4") write("selected"); %>>4:CH14</option>
        <option value=5 <% if (cr_bg == "5") write("selected"); %>>5:CH1-14</option>
        <option value=6 <% if (cr_bg == "6") write("selected"); %>>6:CH3-9</option>
        <option value=7 <% if (cr_bg == "7") write("selected"); %>>7:CH5-13</option>
      </select>
    </td>
  </tr>
  <tr <% if (amode != "1") write("style=\"visibility:hidden;display:none\""); %>>
    <td>11 A&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
      <select id="country_region_a" name="country_region_a" size="1">
        <option value=0 <% var cr_a = getCfgZero(0, "CountryRegionABand");
                           if (cr_a == "0") write("selected"); %>>0:CH36,40,44,48,52,56,60,64,149,153,157,161,165</option>
        <option value=1 <% if (cr_a == "1") write("selected"); %>>1:CH36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140</option>
        <option value=2 <% if (cr_a == "2") write("selected"); %>>2:CH36,40,44,48,52,56,60,64</option>
        <option value=3 <% if (cr_a == "3") write("selected"); %>>3:CH52,56,60,64,149,153,157,161</option>
        <option value=4 <% if (cr_a == "4") write("selected"); %>>4:CH149,153,157,161,165</option>
        <option value=5 <% if (cr_a == "5") write("selected"); %>>5:CH149,153,157,161,</option>
        <option value=6 <% if (cr_a == "6") write("selected"); %>>6:CH36,40,44,48</option>
        <option value=7 <% if (cr_a == "7") write("selected"); %>>7:CH36,40,44,48,52,56,60,64,100,104,108,112,116,120,124,128,132,136,140,149,153,157,161,165</option>
        <option value=8 <% if (cr_a == "8") write("selected"); %>>8:CH52,56,60,64</option>
        <option value=9 <% if (cr_a == "9") write("selected"); %>>9:CH34,38,42,46</option>
        <option value=10 <% if (cr_a == "10") write("selected"); %>>10:CH34,36,38,40,42,44,46,48,52,56,60,64</option>
      </select>
    </td>
  </tr>
  <tr>
    <td class="head" id="staadvBGProtect">B/G Protection</td>
    <td>
      <select name="bg_protection" size="1">
        <option value=0 id="staadvBGProAuto" <% var bg_prot = getCfgZero(0, "BGProtection");
                           if (bg_prot == "0") write("selected"); %>>Auto</option>
        <option value=1 id="staadvBGProOn" <% if (bg_prot == "1") write("selected"); %>>On</option>
        <option value=2 id="staadvBGProOff" <% if (bg_prot == "2") write("selected"); %>>Off</option>
      </select>
    </td>
  </tr>
  <!-- if wifi chip supports 11n, TxRate Setting replaces with MCS -->
  <!--
  <tr id="div_tx_rate" name="div_tx_rate" <% var wm = 1*getCfgZero(0, "WirelessMode");
      if (wm >= 5)
        write("style=\"visibility:hidden;display:none\""); %>>
    <td class="head" id="staadvTxRate">Tx Rate</td>
    <td>
      <select name="tx_rate" size="1">
        <option value=0 id="staadvTxRateAuto" <% var rate = getCfgZero(0, "TxRate");
                           if (rate == "0") write("selected"); %>>Auto</option>
        <option value=1 <% if (rate == "1") write("selected"); %>>1 Mbps</option>
        <option value=2 <% if (rate == "2") write("selected"); %>>2 Mbps</option>
        <option value=3 <% if (rate == "3") write("selected"); %>>5.5 Mbps</option>
        <option value=4 <% if (rate == "4") write("selected"); %>>11 Mbps</option>
        <option value=5 <% if (rate == "5") write("selected"); %>>6 Mbps</option>
        <option value=6 <% if (rate == "6") write("selected"); %>>9 Mbps</option>
        <option value=7 <% if (rate == "7") write("selected"); %>>12 Mbps</option>
        <option value=8 <% if (rate == "8") write("selected"); %>>18 Mbps</option>
        <option value=9 <% if (rate == "9") write("selected"); %>>24 Mbps</option>
        <option value=10 <% if (rate == "10") write("selected"); %>>36 Mbps</option>
        <option value=11 <% if (rate == "11") write("selected"); %>>48 Mbps</option>
        <option value=12 <% if (rate == "12") write("selected"); %>>54 Mbps</option>
      </select>
    </td>
  </tr>
  -->
  <tr>
    <td colspan=2>
      <input type="checkbox" name="tx_burst" <% var burst = getCfgZero(0, "TxBurst");
             if (burst == "1") write("checked"); %>><font id="staadvTxBurst">Tx BURST</font>
    </td>
  </tr>
</table>
<br />

<table id="div_ht_phy_mode" name="div_ht_phy_mode" width="540" border="1" cellpadding="2" cellspacing="1" <%
      if (wm == "0" || wm == "1" || wm == "2" || wm == "3")
        write("style=\"visibility:hidden;display:none\""); %>>
  <tr>
    <td class="title" colspan="2" id="staadvHTPhyMode">HT Physical Mode</td>
  </tr>
  <tr>
    <td class="head" id="staadvHT">HT</td>
    <td>
      <input type=radio name="n_mode" value="0" <% var n_op = getCfgZero(0, "HT_OpMode");
             if (n_op == "0") write("checked"); %>>MM&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
      <input type=radio name="n_mode" value="1" <%
             if (n_op == "1") write("checked"); %>>GF
    </td>
  </tr>
  <tr>
    <td class="head" id="staadvBW">BW</td>
    <td >
      <input type=radio name="n_bandwidth" value="0" <% var n_bw = getCfgZero(0, "HT_BW");
             if (n_bw == "0") write("checked"); %>>20&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
      <input type=radio name="n_bandwidth" value="1" <%
             if (n_bw == "1") write("checked"); %>><font id="staadvBWAuto">Auto</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="staadvGI">GI</td>
    <td >
      <input type=radio name="n_gi" value="0" <% var ngi = getCfgZero(0, "HT_GI");
             if (ngi == "0") write("checked"); %>><font id="staadvGILong">Long</font>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
      <input type=radio name="n_gi" value="1" <%
             if (ngi == "1") write("checked"); %>><font id="staadvGIAuto">Auto</font>
    </td>
  </tr>
  <!--
  <tr>
    <td class="head">STBC</td>
    <td>
      <input type=radio name=n_stbc value="0" >None&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
      <input type=radio name=n_stbc value="1" checked >Auto
    </td>
  </tr>
  -->
  <tr>
    <td class="head">MCS</td>
    <td>
      <select name="n_mcs" size="1">
        <option value=0>0</option>
        <option value=1>1</option>
        <option value=2>2</option>
        <option value=3>3</option>
        <option value=4>4</option>
        <option value=5>5</option>
        <option value=6>6</option>
        <option value=7>7</option>
        <option value=8>8</option>
        <option value=9>9</option>
        <option value=10>10</option>
        <option value=11>11</option>
        <option value=12>12</option>
        <option value=13>13</option>
        <option value=14>14</option>
        <option value=15>15</option>
	<!--
        <option value=32>32</option>
        <option value=33 id="staadvMCSAuto">AUTO</option>
	-->
      </select>
    </td>
  </tr>
</table>
<br />

<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td >
      <input type="button" name="radioButton" style="{width:120px;}" value=<%
        var radio = getStaRadioStatus();
            if (radio == "1") write("\"RADIO OFF\"");
            else write("\"RADIO ON\""); %>
      onClick="if (this.value.indexOf('OFF') >= 0) RadioStatusChange(1); else RadioStatusChange(0); document.sta_advance.submit();"> &nbsp; &nbsp;
      <input type="submit" style="{width:120px;}" value="Apply" id="staadvApply">
    </td>
  </tr>
</table>
<input type=hidden name=radiohiddenButton value="2">
</form>
</td></tr></table>
</body>
</html>
