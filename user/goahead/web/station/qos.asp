<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Ralink Wireless Station QoS</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");
/*
function initValue()
{
	var wmmenable = <!--#include ssi=getStaWMMEnable()-->;
	var apsd =  <!--#include ssi=getStaAPSDEnable()-->;
	var dls =  <!--#include ssi=getStaDLSEnable()-->;
	var acbk =  <!--#include ssi=getStaPSACBK()-->;
	var acbe =  <!--#include ssi=getStaPSACBE()-->;
	var acvo =  <!--#include ssi=getStaPSACVO()-->;
	var acvi =  <!--#include ssi=getStaPSACVI()-->;
	
	if ( wmmenable )
	{
		document.sta_qos.wmm_enable.checked = true;
		if ( apsd )
		{
			document.sta_qos.wmm_ps_enable.checked = true;
			if ( acbk )
				document.sta_qos.wmm_ps_mode_acbk.checked = true;

			if ( acbe )
				document.sta_qos.wmm_ps_mode_acbe.checked = true;

			if ( acvo )
				document.sta_qos.wmm_ps_mode_acvo.checked = true;

			if ( acvi )
				document.sta_qos.wmm_ps_mode_acvi.checked = true;
		}

		if ( dls )
			document.sta_qos.wmm_dls_enable.checked = true;
	}

	WMM_Click();
}
*/

function WMM_Click()
{
	document.sta_qos.wmm_ps_enable.disabled = false;
	document.sta_qos.wmm_dls_enable.disabled = false;

	if (document.sta_qos.wmm_enable.checked == false)
	{
		document.sta_qos.wmm_ps_enable.disabled = true;
		document.sta_qos.wmm_dls_enable.disabled = true;
		document.sta_qos.wmm_ps_enable.checked = false;
		document.sta_qos.wmm_dls_enable.checked = false;
	}
	WMM_PS_Click();
	WMM_DLS_Click();
}

function WMM_PS_Click()
{
	document.sta_qos.wmm_ps_mode_acbe.disabled = true;
	document.sta_qos.wmm_ps_mode_acbk.disabled = true;
	document.sta_qos.wmm_ps_mode_acvi.disabled = true;
	document.sta_qos.wmm_ps_mode_acvo.disabled = true;
	
	if (document.sta_qos.wmm_ps_enable.checked == true)
	{
		document.sta_qos.wmm_ps_mode_acbe.disabled = false;
		document.sta_qos.wmm_ps_mode_acbk.disabled = false;
		document.sta_qos.wmm_ps_mode_acvi.disabled = false;
		document.sta_qos.wmm_ps_mode_acvo.disabled = false;
	}

}

function WMM_DLS_Click()
{
	document.sta_qos.mac0.disabled = false;
	document.sta_qos.mac1.disabled = false;
	document.sta_qos.mac2.disabled = false;
	document.sta_qos.mac3.disabled = false;
	document.sta_qos.mac4.disabled = false;
	document.sta_qos.mac5.disabled = false;
	document.sta_qos.timeout.disabled = false;

	if (document.sta_qos.wmm_dls_enable.checked == false)
	{
		document.sta_qos.mac0.disabled = true;
		document.sta_qos.mac1.disabled = true;
		document.sta_qos.mac2.disabled = true;
		document.sta_qos.mac3.disabled = true;
		document.sta_qos.mac4.disabled = true;
		document.sta_qos.mac5.disabled = true;
		document.sta_qos.timeout.disabled = true;
	}
}

function submit_apply(btntype)
{
	document.sta_qos.button_type.value = btntype;  // 1: wmm , 2: dls setup, 3: tear down
	if (btntype == 2 && document.sta_qos.wmm_dls_enable.checked == true)
	{
		if (document.sta_qos.timeout.value < 1 || document.sta_qos.timeout.value > 65535)
		{
			alert("Range of DLS Timeout is 1 ~ 65535");
			return;
		}
	}
	document.sta_qos.submit();
}

function Move_To_Dls()
{

}
function initTranslation()
{
	var e = document.getElementById("qosTitle");
	e.innerHTML = _("qos title");
	e = document.getElementById("qosIntroduction");
	e.innerHTML = _("qos introduction");

	e = document.getElementById("qosConfig");
	e.innerHTML = _("qos config");
	e = document.getElementById("qosWMM");
	e.innerHTML = _("qos wmm");
	e = document.getElementById("qosWMMEnable");
	e.innerHTML = _("station enable");
	e = document.getElementById("qosWMMPWSave");
	e.innerHTML = _("qos wmm power save");
	e = document.getElementById("qosPSMode");
	e.innerHTML = _("qos wmm ps mode");
	e = document.getElementById("qosWMMPWSaveEnable");
	e.innerHTML = _("station enable");
	e = document.getElementById("qosWMMDLS");
	e.innerHTML = _("qos dls");
	e = document.getElementById("qosWMMDLSEnable");
	e.innerHTML = _("station enable");
	e = document.getElementById("qosWMMApply");
	e.value = _("wireless apply");

	e = document.getElementById("qosDLS");
	e.innerHTML = _("qos dls");
	e = document.getElementById("qosDLSMac");
	e.innerHTML = _("stalist macaddr");
	e = document.getElementById("qosDLSTimeoutValue");
	e.innerHTML = _("qos dls timeoutvalue");
	e = document.getElementById("qosSecond");
	e.innerHTML = _("qos second");
	e = document.getElementById("qosDLSAppy");
	e.value = _("wireless apply");

	e = document.getElementById("qosDLSStaus");
	e.innerHTML = _("qos dls status");
	e = document.getElementById("qosDLSStatusMAC");
	e.innerHTML = _("stalist macaddr");
	e = document.getElementById("qosDLSStatusTimeout");
	e.innerHTML = _("qos dls timeout");
	e = document.getElementById("qosTearDown");
	e.value = _("qos teardown");
}

function PageInit()
{
	initTranslation();
}
</script>
</head>


<body onload="PageInit()">
<table class="body"><tr><td>

<h1 id="qosTitle">Station Advanced Configurations</h1>
<p id="qosIntroduction">The Status page shows the settings and current operation status of the Station.</p>
<hr />

<form method="post" name="sta_qos" action="/goform/setStaQoS">
<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td class="title" colspan="2" id="qosConfig">Qos Configuration</td>
  </tr>
  <tr>
    <td class="head" id="qosWMM">WMM</td>
    <td>
      <input type=checkbox name=wmm_enable
      <% var wmm = getCfgZero(0, "WmmCapable"); if (wmm == "1") write("checked"); %>
      OnClick="WMM_Click()"><font id="qosWMMEnable">enable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="qosWMMPWSave">WMM Power Saving</td>
    <td>
      <input type=checkbox name=wmm_ps_enable
      <% var ps = getCfgZero(0, "APSDCapable");
             if (wmm == "1" && ps == "1") write("checked");
             if (wmm != "1") write("disabled"); %>
      onClick="WMM_PS_Click()"><font id="qosWMMPWSaveEnable">enable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="qosPSMode">PS Mode</td>
    <td>
      <input type=checkbox name=wmm_ps_mode_acbe <% var acbe = getCfgNthZero(0, "APSDAC", 0);
        if (wmm == "1" && ps == "1" && acbe == "1") write("checked");
        if (wmm != "1" || ps != "1") write("disabled"); %>>AC_BE &nbsp;&nbsp;
      <input type=checkbox name=wmm_ps_mode_acbk <% var acbk = getCfgNthZero(0, "APSDAC", 1);
        if (wmm == "1" && ps == "1" && acbk == "1") write("checked");
        if (wmm != "1" || ps != "1") write("disabled"); %>>AC_BK &nbsp;&nbsp;
      <input type=checkbox name=wmm_ps_mode_acvi <% var acvi = getCfgNthZero(0, "APSDAC", 2);
        if (wmm == "1" && ps == "1" && acvi == "1") write("checked");
        if (wmm != "1" || ps != "1") write("disabled"); %>>AC_VI &nbsp;&nbsp;
      <input type=checkbox name=wmm_ps_mode_acvo <% var acvo = getCfgNthZero(0, "APSDAC", 3);
        if (wmm == "1" && ps == "1" && acvo == "1") write("checked");
        if (wmm != "1" || ps != "1") write("disabled"); %>>AC_VO &nbsp;&nbsp;
    </td>
  </tr>
</table>

<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td >
      <input type="button" name="WMMButton" style="{width:120px;}" value="WMM Apply" id="qosWMMApply" onClick="submit_apply(1)"> &nbsp; &nbsp;
    </td>
  </tr>
</table>
<br />

<table div="wmm_dls_setup" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr>
    <td class="title" id="qosDLS" colspan="2">Direct Link Setup</td>
  </tr>
  <tr>
    <td class="head" id="qosWMMDLS">Direct Link Setup</td>
    <td>
      <input type=checkbox name=wmm_dls_enable <% var dls = getCfgZero(0, "DLSCapable");
        if (wmm == "1" && dls == "1") write("checked");
	if (wmm != "1") write("disabled"); %>
      onClick="WMM_DLS_Click()"><font id="qosWMMDLSEnable">enable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="qosDLSMac">MAC Address</td>
    <td>
      <input type=text size=2 maxlength=2 name=mac0
        <% if (dls != "1") write("disabled"); %>>&nbsp;-&nbsp;
      <input type=text size=2 maxlength=2 name=mac1
        <% if (dls != "1") write("disabled"); %>>&nbsp;-&nbsp;
      <input type=text size=2 maxlength=2 name=mac2
        <% if (dls != "1") write("disabled"); %>>&nbsp;-&nbsp;
      <input type=text size=2 maxlength=2 name=mac3
        <% if (dls != "1") write("disabled"); %>>&nbsp;-&nbsp;
      <input type=text size=2 maxlength=2 name=mac4
        <% if (dls != "1") write("disabled"); %>>&nbsp;-&nbsp;
      <input type=text size=2 maxlength=2 name=mac5
        <% if (dls != "1") write("disabled"); %>>
    </td>
  </tr>
  <tr>
    <td class="head" id="qosDLSTimeoutValue">Timeout Value</td>
    <td>
      <input type=text name=timeout align="right" id="qosSecond"> sec
    </td>
  </tr>
</table>

<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td >
      <input type="button" name="DlsSetupButton" style="{width:120px;}" value="DLS Apply" id="qosDLSAppy" onClick="submit_apply(2)"> &nbsp; &nbsp;
    </td>
  </tr>
</table>
<br />

<table div="wmm_dls_status" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" id="qosDLSStaus">DLS Status</td>
  </tr>
  <tr>
    <td width=65% bgcolor="#E8F8FF" id="qosDLSStatusMAC">MAC Address</td>
    <td bgcolor="#E8F8FF" id="qosDLSStatusTimeout">Timeout</td>
  </tr>
  <% getStaDLSList(); %>
</table>
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td >
<!--      <input type="button" name="MoveToDlsButton" style="{width:120px;}" value="Move To Dls Setup" onClick="Move_To_Dls()"> &nbsp; &nbsp;-->
      <input type="button" name="DlsStatusButton" style="{width:120px;}" value="Tear Down" id="qosTearDown" onClick="submit_apply(3)"> &nbsp; &nbsp;
    </td>
  </tr>
</table>

<input type=hidden name=button_type value="">
</form>

</td></tr></table>
</body>
</html>
