<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<script type="text/javascript" src="/common.js"></script>

<title>Wizard - Lang Set</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("admin");

restartPage_init();

var sbuttonMax=1;

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

function disableTextField (field)
{
  if(document.all || document.getElementById){
    field.disabled = true;
  }else {
    field.oldOnFocus = field.onfocus;
    field.onfocus = skip;
  }
}

function enableTextField (field)
{
  if(document.all || document.getElementById)
    field.disabled = false;
  else {
    field.onfocus = field.oldOnFocus;
  }
}

function initTranslation()
{
	var e = document.getElementById("wizardLangTitle");
	e.innerHTML = _("wizardLang Title");
	e = document.getElementById("wizardLangIntroduction");
	e.innerHTML = _("wizardLang Introduction");
	e = document.getElementById("manLangApply");
	e.value = _("admin next");
	e = document.getElementById("manLangCancel");
	e.value = _("admin cancel");

	e = document.getElementById("manLangSet");
	e.innerHTML = _("man language setting");
	e = document.getElementById("manSelectLang");
	e.innerHTML = _("man select language");
}

function initValue()
{
	var lang_element = document.getElementById("langSelection");
	var lang_en = "<% getLangBuilt("en"); %>";
	var lang_zhtw = "<% getLangBuilt("zhtw"); %>";
	var lang_zhcn = "<% getLangBuilt("zhcn"); %>";
	var lang_kr = "<% getLangBuilt("kr"); %>";

	lang_en = "1";
	lang_zhtw = "0";
	lang_zhcn = "0";
	lang_kr = "0";

	initTranslation();
	lang_element.options.length = 0;
	if (lang_en == "1")
		lang_element.options[lang_element.length] = new Option('English', 'en');
	if (lang_zhtw == "1")
		lang_element.options[lang_element.length] = new Option('繁體中文', 'zhtw');
	if (lang_zhcn == "1")
		lang_element.options[lang_element.length] = new Option('简体中文', 'zhcn');
	if (lang_kr == "1")
		lang_element.options[lang_element.length] = new Option('한국어', 'kr');

	if (document.cookie.length > 0) {
		var s = document.cookie.indexOf("language=");
		var e = document.cookie.indexOf(";", s);
		var lang = "en";
		var i;

		if (s != -1) {
			if (e == -1)
				lang = document.cookie.substring(s+9);
			else
				lang = document.cookie.substring(s+9, e);
		}
		for (i=0; i<lang_element.options.length; i++) {
			if (lang == lang_element.options[i].value) {
				lang_element.options.selectedIndex = i;
				break;
			}
		}
	}
}

function setLanguage()
{
	document.cookie="language="+document.Lang.langSelection.value+"; path=/";
	/*parent.menu.location.reload();*/

	return true;
}

function updateLang()
{
	document.cookie="language="+document.Lang.langSelection.value+"; path=/";
	//Butterlate.setTextDomain("admin");
	parent.updateLang(); // call  awb_index.php to reload page.
}

</script>
</head>
<body onload="initValue(); parent.setMenu(); parent.fnDispWizard(1); " bgcolor="#FFFFFF">

<div align="center">
 <center>

<table class="body"><tr><td>

<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="wizardLangTitle">Wizard - Lang Set</td>
</tr>
<tr>

<tr>
<td colspan="2">
<p class="head" id="wizardLangIntroduction">Lang Set.</p>
</td>
<tr>
</table>
<br>

<!-- ================= Langauge Settings ================= -->
<form method="post" name="Lang" action="/goform/setSysLang_wizard">
<table width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" id="manLangSet">Language Settings</td>
  </tr>
  <tr>
    <td class="head" id="manSelectLang">Select Language</td>
    <td>
      <select name="langSelection" id="langSelection" onChange="updateLang();">
        <!-- added by initValue -->
      </select>
    </td>
  </tr>
</table>
<table width="540" border="0" cellpadding="2" cellspacing="1">
  <tr id="sbutton0" align="center">
    <td>
	  <input type="hidden" name="goform_next" value="/wizard/wizard_timeset.asp">
      <input type=submit style="{width:120px;}" value="Apply" id="manLangApply" onClick="sbutton_disable(sbuttonMax); restartPage_block(); return setLanguage();"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="manLangCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
 </center>
</div>
</body></html>
