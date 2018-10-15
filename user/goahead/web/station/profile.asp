<!-- Copyright (c), Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<title>Station Profile</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

function open_profile_page() {
	window.open("add_profile_page.asp","profile_page","toolbar=no, location=yes, scrollbars=yes, resizable=no, width=660, height=600");
}

function edit_profile_page(){
	document.sta_profile.hiddenButton.value = 'edit';
	document.sta_profile.submit();
	window.open("edit_profile_page.asp","profile_page","toolbar=no, location=yes, scrollbars=yes, resizable=no, width=660, height=600");
}

function selectedProfileChange()
{	
	document.sta_profile.deleteProfileButton.disabled=false;
	document.sta_profile.editProfileButton.disabled=false;
	document.sta_profile.activateProfileButton.disabled=false;
}

function submit_apply(parm)
{
	document.sta_profile.hiddenButton.value = parm;
	document.sta_profile.submit();
}

function initTranslation()
{
	var e = document.getElementById("profTitle");
	e.innerHTML = _("prof title");
	e = document.getElementById("profIntroduction");
	e.innerHTML = _("prof introduction");

	e = document.getElementById("profList");
	e.innerHTML = _("prof list");
	e = document.getElementById("profSelect");
	e.innerHTML = _("station select");
	e = document.getElementById("profProfile");
	e.innerHTML = _("prof profile");
	e = document.getElementById("profSSID");
	e.innerHTML = _("station ssid");
	e = document.getElementById("profChannel");
	e.innerHTML = _("station channel");
	e = document.getElementById("profAuth");
	e.innerHTML = _("station auth");
	e = document.getElementById("staproEncryp");
	e.innerHTML = _("station encryp");
	e = document.getElementById("staproNetType");
	e.innerHTML = _("station network type");
	e = document.getElementById("profAdd");
	e.value = _("station add");
	e = document.getElementById("profDel");
	e.value = _("station del");
	e = document.getElementById("profEdit");
	e.value = _("station edit");
	e = document.getElementById("profActive");
	e.value = _("station active");
}

function PageInit()
{
	initTranslation();
}
</script>
</head>


<body onload="PageInit()">
<table class="body"><tr><td>

<h1 id="profTitle">Station Profile</h1>
<p id="profIntroduction">The Status page shows the settings and current operation status of the Station.</p>
<hr />

<form method="post" name="sta_profile" action="/goform/setStaProfile">
<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr> 
    <td class="title" colspan="7" id="profList">Pofile List</td>
  </tr>
  <tr>
    <td bgcolor="#E8F8FF" width=15px id="profSelect">&nbsp;</td>
    <td bgcolor="#E8F8FF" id="profProfile">Profile</td>
    <td bgcolor="#E8F8FF" id="profSSID">SSID</td>
    <td bgcolor="#E8F8FF" id="profChannel">Channel</td>
    <td bgcolor="#E8F8FF" id="profAuth">Authentication</td>
    <td bgcolor="#E8F8FF" id="staproEncryp">Encryption</td>
    <td bgcolor="#E8F8FF" id="staproNetType">Network Type</td>
  </tr>
  <% getStaProfile(); %>
</table>
<br />
<center>
  <h4>Note: At present, STA only guarantees to store Two profiles!</h4>
</center>

<table width="540" cellpadding="2" cellspacing="1">
<tr align="center">
  <td>
    <input type="button" name="addProfileButton" id="profAdd" style="{width:120px;}" value="Add" onClick="open_profile_page()"> &nbsp; &nbsp;
    <input type="button" name="deleteProfileButton" id="profDel" style="{width:120px;}" value="Delete" disabled onClick="submit_apply('delete')"> &nbsp; &nbsp;
    <input type="button" name="editProfileButton" id="profEdit" style="{width:120px;}" value="Edit" disabled onClick="edit_profile_page()"> &nbsp; &nbsp;
    <input type="button" name="activateProfileButton" id="profActive" style="{width:120px;}" value="Activate" disabled onClick="submit_apply('activate')"> &nbsp; &nbsp;
  </td>
</tr>
</table>
<input type=hidden name=hiddenButton value="">
</form>


</td></tr></table>
</body>
</html>

