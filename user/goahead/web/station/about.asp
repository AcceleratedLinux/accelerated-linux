<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

function initTranslation()
{
	var e = document.getElementById("aboutTitle");
	e.innerHTML = _("about title");
	e = document.getElementById("aboutIntroduction");
	e.innerHTML = _("about introduction");

	e = document.getElementById("aboutAbout");
	e.innerHTML = _("about about");
	e = document.getElementById("aboutDriverVersion");
	e.innerHTML = _("about driver version");
	e = document.getElementById("aboutMacAddr");
	e.innerHTML = _("stalist macaddr");
}

function PageInit()
{
	initTranslation();
}
</script>

<title>Ralink Wireless Station About</title>
</head>


<body onload="PageInit()">
<table class="body"><tr><td>

<h1 id="aboutTitle">Station About</h1>
<p id="aboutIntroduction">The Status page shows the settings and current operation status of the Station.</p>
<hr />

<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td class="title" colspan="2" id="aboutAbout">About</td>
  </tr>
  <tr>
    <td class="head" id="aboutDriverVersion">Driver Version</td>
    <td><% getStaDriverVer(); %></td>
  </tr>
  <tr>
    <td class="head" id="aboutMacAddr">Mac Address</td>
    <td><% getStaMacAddr(); %></td>
  </tr>
  </table>


</td></tr></table>
</body>
</html>

