<html>
<head>
<title><% getCfgGeneral(1, "HostName"); %></title>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta http-equiv="PRAGMA" content="NO-CACHE">
<meta http-equiv="CACHE-CONTROL" content="NO-CACHE">
<script language="JavaScript" type="text/javascript">

function initLanguage()
{
	var lang = "<% getCfgGeneral(1, "Language"); %>";
	var cook = "en";
	var lang_en = "<% getLangBuilt("en"); %>";
	var lang_zhtw = "<% getLangBuilt("zhtw"); %>";
	var lang_zhcn = "<% getLangBuilt("zhcn"); %>";
	var lang_kr = "<% getLangBuilt("kr"); %>";

	if (document.cookie.length > 0) {
		var s = document.cookie.indexOf("language=");
		var e = document.cookie.indexOf(";", s);
		if (s != -1) {
			if (e == -1)
				cook = document.cookie.substring(s+9);
			else
				cook = document.cookie.substring(s+9, e);
		}
	}

	lang = "en";
	cook = "en";
	lang_en = "1";

	if (lang == "en") {
		document.cookie="language=en; path=/";
		if (cook != lang)
			window.location.reload();
		if (lang_en != "1") {
			if (lang_zhtw == "1") {
				document.cookie="language=zhtw; path=/";
				window.location.reload();
			}
			else if (lang_zhcn == "1") {
				document.cookie="language=zhcn; path=/";
				window.location.reload();
			}
			else if (lang_kr == "1") {
				document.cookie="language=kr; path=/";
				window.location.reload();
			}
		}
	}
	else if (lang == "zhtw") {
		document.cookie="language=zhtw; path=/";
		if (cook != lang)
			window.location.reload();
		if (lang_zhtw != "1") {
			if (lang_en == "1") {
				document.cookie="language=en; path=/";
				window.location.reload();
			}
			else if (lang_zhcn == "1") {
				document.cookie="language=zhcn; path=/";
				window.location.reload();
			}
			else if (lang_kr == "1") {
				document.cookie="language=kr; path=/";
				window.location.reload();
		}
	}
	}
	else if (lang == "zhcn") {
		document.cookie="language=zhcn; path=/";
		if (cook != lang)
			window.location.reload();
		if (lang_zhcn != "1") {
			if (lang_en == "1") {
				document.cookie="language=en; path=/";
				window.location.reload();
			}
			else if (lang_zhtw == "1") {
				document.cookie="language=zhtw; path=/";
				window.location.reload();
			}
			else if (lang_kr == "1") {
				document.cookie="language=kr; path=/";
				window.location.reload();
			}
		}
	}
	else if (lang == "kr") {
		document.cookie="language=kr; path=/";
		if (cook != lang)
			window.location.reload();
		if (lang_kr != "1") {
			if (lang_en == "1") {
				document.cookie="language=en; path=/";
				window.location.reload();
			}else if (lang_zhtw == "1") {
				document.cookie="language=zhtw; path=/";
				window.location.reload();
			}
			else if (lang_zhcn == "1") {
				document.cookie="language=zhcn; path=/";
				window.location.reload();
			}
		}
	}
}

function onInit()
{
	initLanguage();
	/*<% FirmwareUpgradePostASP(); %>*/
}

</script>

</head>

<frameset rows="193" framespacing="0" frameborder="no" border="0" onLoad="onInit()">
  <frameset cols="*" framespacing="0" frameborder="no" border="0" onLoad="onInit()">
  <frame src="awb_index.asp" name="topFrame" scrolling="auto" id="topFrame" title="topFrame" target="mainFrame"  />
  </frameset>
</frameset>

<noframes>
<body></body>
</noframes>

</html>
