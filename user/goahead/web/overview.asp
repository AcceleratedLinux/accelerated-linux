<HTML>
<HEAD>
<TITLE>Ralink APSoC Overview</TITLE>
<LINK REL="stylesheet" HREF="style/normal_ws.css" TYPE="text/css">
<meta http-equiv="content-type" content="text/html; charset=UTF-8">

<script type="text/javascript" src="/lang/b28n.js"></script>
<script type="text/javascript">
Butterlate.setTextDomain("main");

function initTranslation()
{
	// var e = document.getElementById("ovIntroduction");
	// e.innerHTML = _("overview introduction");
	var e = document.getElementById("ovSelectLang");
	e.innerHTML = _("overview select language");
	e = document.getElementById("ovLangApply");
	e.value = _("main apply");

	e = document.getElementById("ovStatus");
	e.innerHTML = _("overview status link");
	e = document.getElementById("ovStatistic");
	e.innerHTML = _("overview statistic link");
	e = document.getElementById("ovManagement");
	e.innerHTML = _("overview management link");
}

function initValue() {
	var lang_element = document.getElementById("langSelection");
	var lang_en = "<% getLangBuilt("en"); %>";
	var lang_zhtw = "<% getLangBuilt("zhtw"); %>";
	var lang_zhcn = "<% getLangBuilt("zhcn"); %>";

	initTranslation();
	//lang_element.options.length = 0;
	if (lang_en == "1")
		lang_element.options[lang_element.length] = new Option('English', 'en');
	if (lang_zhtw == "1")
		lang_element.options[lang_element.length] = new Option('Traditional Chinese', 'zhtw');
	if (lang_zhcn == "1")
		lang_element.options[lang_element.length] = new Option('Simple Chinese', 'zhcn');

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
	parent.menu.location.reload();
	return true;
}
</script>
</HEAD>

<BODY onLoad="initValue()">
<table class="body"><tr><td>

<H1>Ralink APSoC</H1>
<p id="ovIntroduction" />

<!-- ----------------- Langauge Settings ----------------- -->
<form method="post" name="Lang" action="/goform/setSysLang">
<blockquote><fieldset>
<legend id="ovSelectLang">Select Language</legend>
<select name="langSelection" id="langSelection">
<!-- added by initValue -->
</select>&nbsp;&nbsp;
<input type=submit style="{width:50px;}" value="Apply" id="ovLangApply" onClick="return setLanguage()">
</fieldset></blockquote>
</form>

<blockquote><fieldset><p>
<a href="/adm/status.asp" id="ovStatus">Status</a><br />
<a href="/adm/statistic.asp" id="ovStatistic">Statistic</a><br />
<a href="/adm/management.asp" id="ovManagement">Management</a><br />
<br />
</p></fieldset></blockquote>

<center>
<img src="graphics/webserver_logo1.gif" border="0">
</center>

</td></tr></table>
</BODY>
</HTML>
