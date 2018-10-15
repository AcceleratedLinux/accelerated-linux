<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Ralink Wireless Station 11n Configuration</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

function initTranslation()
{
	var e = document.getElementById("11nTitle");
	e.innerHTML = _("11n title");
	e = document.getElementById("11nIntroduction");
	e.innerHTML = _("11n introduction");

	e = document.getElementById("11nConfig");
	e.innerHTML = _("11n config");
	e = document.getElementById("11nAMPDU");
	e.innerHTML = _("11n ampdu");
	e = document.getElementById("11nAMPDUEnable");
	e.innerHTML = _("station enable");
	e = document.getElementById("11nAMPDUManual");
	e.innerHTML = _("11n ampdu manual");
	e = document.getElementById("11nAMPDUAuto");
	e.innerHTML = _("wireless auto");
	e = document.getElementById("11nMPDUDensity");
	e.innerHTML = _("11n mpdu density");
	e = document.getElementById("11nAMSDU");
	e.innerHTML = _("basic ht amsdu");
	e = document.getElementById("11nAMSDUEnable");
	e.innerHTML = _("station enable");
	e = document.getElementById("11nApply");
	e.value = _("wireless apply");
}

function initValue()
{
	var baenable = <% getCfgZero(1, "staPolicy"); %>;
	var autoba = <% getCfgZero(1, "HT_AutoBA"); %>;
	var density = <% getCfgZero(1, "HT_MpduDensity"); %>;
	var amsdu = <% getCfgZero(1, "HT_AMSDU"); %>;

	initTranslation();
	if (baenable)
		document.sta_11n_configuration.a_mpdu_enable.checked = true;
	else
		document.sta_11n_configuration.a_mpdu_enable.checked = false;

	if (autoba) {
		document.sta_11n_configuration.autoBA[1].checked = true;
		Auto_BA_Click(1);
	}
	else
		document.sta_11n_configuration.autoBA[0].checked = true;

	if (document.sta_11n_configuration.autoBA[1].checked)
	{
	}
	else
	{
		document.sta_11n_configuration.mpdu_density.options.selectedIndex = density;
		Mpdu_Aggregtion_Click();
	}

	if (amsdu)
		document.sta_11n_configuration.a_msdu_enable.checked = true;
}

function open_org_add_page()
{
	window.open("ampdu_org_add.asp","ampdu_org_add","toolbar=no, location=yes, scrollbars=yes, resizable=no, width=660, height=600");
}

function open_org_del_page()
{
	cwin = window.open("station/ampdu_org_del.asp","ampdu_org_del","toolbar=no, location=yes, scrollbars=yes, resizable=no, width=660, height=600");
}

function getBssid()
{
	cwin.document.forms["sta_org_del"].selectedbssid.value = document.sta_11n_configuration.selectedbssid.value;
}

function open_rev_del_page()
{
	window.open("ampdu_rev_del.asp","ampdu_rev_del","toolbar=no, location=yes, scrollbars=yes, resizable=no, width=660, height=600");
}

function Mpdu_Aggregtion_Click()
{
	document.sta_11n_configuration.mpdu_density.disabled = true;
	document.sta_11n_configuration.autoBA[0].disabled = true;
	document.sta_11n_configuration.autoBA[1].disabled = true;

	if (document.sta_11n_configuration.a_mpdu_enable.checked == true) {
		document.sta_11n_configuration.mpdu_density.disabled = false;
		document.sta_11n_configuration.autoBA[0].disabled = false;
		document.sta_11n_configuration.autoBA[1].disabled = false;
	}
}

function Auto_BA_Click(autoba)
{
	if (autoba == 1)
		document.sta_11n_configuration.mpdu_density.disabled = true;
	else
		document.sta_11n_configuration.mpdu_density.disabled = false;
}

function submit_apply()
{
	document.sta_11n_configuration.submit();
}

function selectedBSSID(tmp)
{
	document.sta_11n_configuration.selectedbssid.value = tmp;
}
</script>
</head>


<body onLoad="initValue()">
<table class="body"><tr><td>

<h1 id="11nTitle">Station 11n Configurations</h1>
<p id="11nIntroduction">The Status page shows the settings and current operation status of the Station.</p>
<hr />

<form method=post name="sta_11n_configuration" action="/goform/setSta11nCfg">
<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td class="title" colspan="2" id="11nConfig">11n Configuration</td>
  </tr>
  <tr>
    <td class="head" rowspan="2" id="11nAMPDU">MPDU Aggregation</td>
    <td><input type="checkbox" name="a_mpdu_enable" onClick="Mpdu_Aggregtion_Click()"><font id="11nAMPDUEnable">enable</font></td>
  </tr>
  <tr>
    <td>
      <input type="radio" name="autoBA" value=0 checked onClick="Auto_BA_Click(0)"><font id="11nAMPDUManual">Manual</font>&nbsp;&nbsp;
      <input type="radio" name="autoBA" value=1 onClick="Auto_BA_Click(1)"><font id="11nAMPDUAuto">Auto</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="11nMPDUDensity">MPDU density</td>
    <td>
      <select name="mpdu_density" size="1">
        <option value=0 selected>0</option>
        <option value=1>1</option>
        <option value=2>2</option>
        <option value=3>3</option>
        <option value=4>4</option>
        <option value=5>5</option>
        <option value=6>6</option>
        <option value=7>7</option>
      </select>
    </td>
  </tr>
  <tr>
    <td class="head" id="11nAMSDU">Aggregation MSDU(A-MSDU)</td>
     <td><input type="checkbox" name="a_msdu_enable"><font id="11nAMSDUEnable">enable</font></td>
  </tr>
</table>
<br />

<table width="540" border="0" cellpadding="2" cellspacing="1">
  <tr align="center">
    <td>
      <input type="button" style="{width:120px;}" value="Apply" id="11nApply" onClick="submit_apply()"> &nbsp; &nbsp;
    </td>
  </tr>
</table>
</form>


</td></tr></table>
</body>
</html>

