<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>RVT Settings</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

/*
var itxbf = '<% getCfgZero(1, "ITxBfEn"); %>';
var itxbfto = '<% getCfgGeneral(1, "ITxBfTimeout"); %>';
*/
var etxbf = '<% getCfgZero(1, "ETxBfEnCond"); %>';
//var etxbfto = '<% getCfgGeneral(1, "ETxBfTimeout"); %>';
var macenhance = '<% getCfgGeneral(1, "VideoTurbine"); %>';
var classifier = '<% getModIns(1, "cls"); %>';

function initTranslation()
{
	var e = document.getElementById("etxbfDis");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("etxbfEn");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("macenhDis");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("macenhEn");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("classifierDis");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("classifierEn");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("rvtApply");
	e.innerHTML = _("wireless apply");
	e = document.getElementById("rvtCancel");
	e.innerHTML = _("wireless cancel");
}

function initValue()
{
	initTranslation();

	/*
	itxbf = 1*itxbf;
	if (itxbf == 0) {
		document.wireless_rvt.itxbf[0].checked = true;
		document.wireless_rvt.itxbf[1].checked = false;
	}
	else {
		document.wireless_rvt.itxbf[0].checked = false;
		document.wireless_rvt.itxbf[1].checked = true;
	}

	itxbfto = 1*itxbfto;
	document.wireless_rvt.itxbfto.value = itxbfto;
	*/

	etxbf = 1*etxbf;
	if (etxbf == 0) {
		document.wireless_rvt.etxbf[0].checked = true;
		document.wireless_rvt.etxbf[1].checked = false;
	}
	else {
		document.wireless_rvt.etxbf[0].checked = false;
		document.wireless_rvt.etxbf[1].checked = true;
	}

	/*
	etxbfto = 1*etxbfto;
	document.wireless_rvt.etxbfto.value = etxbfto;
	*/

	macenhance = 1*macenhance;
	if (macenhance == 0) {
		document.wireless_rvt.macenhance[0].checked = true;
		document.wireless_rvt.macenhance[1].checked = false;
	}
	else {
		document.wireless_rvt.macenhance[0].checked = false;
		document.wireless_rvt.macenhance[1].checked = true;
	}

	classifier = 1*classifier;
	if (classifier == 0) {
		document.wireless_rvt.classifier[0].checked = true;
		document.wireless_rvt.classifier[1].checked = false;
	}
	else {
		document.wireless_rvt.classifier[0].checked = false;
		document.wireless_rvt.classifier[1].checked = true;
	}
}
</script>
</head>

<body onLoad="initValue()">

<h1 id="rvtTitle">RVT Settings</h1>
<p id="rvtIntro"></p>
<hr />

<form method="post" name="wireless_rvt" action="/goform/wirelessRvt">
<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td class="title" colspan="2" id="txbfTitle">TxBF Settings</td>
  </tr>
  <!--
  <tr>
    <td class="head" id="txbfIm">Implicit TxBF</td>
    <td>
      <input type=radio name=itxbf value="0"><font id="itxbfDis">Disable&nbsp;</font>
      <input type=radio name=itxbf value="1"><font id="itxbfEn">Enable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="txbfImTo">Implicit TxBF Timeout</td>
    <td><input type=text name=itxbfto size=5 maxlength=5> * 25uS</td>
  </tr>
  -->
  <tr>
    <td class="head" id="txbfEx">Explicit TxBF</td>
    <td>
      <input type=radio name=etxbf value="0"><font id="etxbfDis">Disable&nbsp;</font>
      <input type=radio name=etxbf value="1"><font id="etxbfEn">Enable</font>
    </td>
  </tr>
  <!--
  <tr>
    <td class="head" id="txbfExTo">Explicit TxBF Timeout</td>
    <td><input type=text name=etxbfto size=5 maxlength=5> * 25uS</td>
  </tr>
  -->
</table>

<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td class="title" colspan="2" id="macTitle">MAC Enhancement Settings</td>
  </tr>
  <tr>
    <td class="head" id="macEnh">MAC Enhancement</td>
    <td>
      <input type=radio name=macenhance value="0"><font id="macenhDis">Disable&nbsp;</font>
      <input type=radio name=macenhance value="1"><font id="macenhEn">Enable</font>
    </td>
  </tr>
</table>

<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td class="title" colspan="2" id="classTitle">Classifier Settings</td>
  </tr>
  <tr>
    <td class="head" id="classifier">Classifier</td>
    <td>
      <input type=radio name=classifier value="0"><font id="classifierDis">Disable&nbsp;</font>
      <input type=radio name=classifier value="1"><font id="classifierEn">Enable</font>
    </td>
  </tr>
</table>

<table width="540" border="0" cellpadding="2" cellspacing="1">
  <tr align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply" id="rvtApply"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="rvtCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>

</body>
</html>
