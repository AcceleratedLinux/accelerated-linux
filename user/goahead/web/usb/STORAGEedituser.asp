<!-- Copyright (c), Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Edit User Account</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("usb");
var index = opener.document.forms[0].selectIndex.value;
var user = opener.document.forms[0].selectUser.value;
var pw = opener.document.forms[0].selectPassword.value;
var ftp = opener.document.forms[0].selectFtp.value;
var smb = opener.document.forms[0].selectSmb.value;
var ftpb = '<% getFtpBuilt(); %>';
var smbb = '<% getSmbBuilt(); %>';

function initTranslation()
{
	var e = document.getElementById("edituserbasic");
	e.innerHTML = _("adduser basic");
	e = document.getElementById("edituserName");
	e.innerHTML = _("adduser name");
	e = document.getElementById("edituserPW");
	e.innerHTML = _("adduser password");

	e = document.getElementById("edituserFtp");
	e.innerHTML = _("adduser ftp");
	e = document.getElementById("edituserFtpEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("edituserFtpDisable");
	e.innerHTML = _("usb disable");

	e = document.getElementById("edituserSmb");
	e.innerHTML = _("adduser smb");
	e = document.getElementById("edituserSmbEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("edituserSmbDisable");
	e.innerHTML = _("usb disable");

	e = document.getElementById("edituserApply");
	e.value = _("usb apply");
	e = document.getElementById("edituserCancel");
	e.value = _("usb cancel");
}
	
function initValue()
{
	initTranslation();

	document.storage_edituser.edituser_ftp[0].disabled = true;
	document.storage_edituser.edituser_ftp[1].disabled = true;
	document.storage_edituser.edituser_smb[0].disabled = true;
	document.storage_edituser.edituser_smb[1].disabled = true;
	document.storage_edituser.edituser_pw.value = pw;
	if (ftpb == "1") {
		document.storage_edituser.edituser_ftp[0].disabled = false;
		document.storage_edituser.edituser_ftp[1].disabled = false;
		if (ftp == "1")
			document.storage_edituser.edituser_ftp[0].checked = true;
		else
			document.storage_edituser.edituser_ftp[1].checked = true;
	}
	if (smbb == "1") {
		document.storage_edituser.edituser_smb[0].disabled = false;
		document.storage_edituser.edituser_smb[1].disabled = false;
		if (smb == "1")
			document.storage_edituser.edituser_smb[0].checked = true;
		else
			document.storage_edituser.edituser_smb[1].checked = true;
	}
}

function checkData()
{
	return true;
}

function edituserClose()
{
	opener.location.reload();
}

function submit_apply()
{
	if (checkData() == true)
	{
		document.storage_edituser.hiddenIndex.value = index;
		document.storage_edituser.submit();
		window.close();
	}
}
</script>
</head>

<body onLoad="initValue()" onUnload="edituserClose()">
<table class="body"><tr><td>

<form method=post name="storage_edituser" action="/goform/StorageEditUser">
<input type=hidden name=hiddenIndex value="">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="edituserbasic">Basic Setup</td>
  </tr>
  <tr>
    <td class="head" id="edituserName">User Name</td>
    <td>
    <script language="JavaScript" type="text/javascript">
      document.write(user);
    </script>
    </td>
  </tr>
  <tr>
    <td class="head" id="edituserPW">Password</td>
    <td><input type="password" name="edituser_pw" size="16" maxlength="16" value=""></td>
  </tr>
</table>

<br />

<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" id="edituserFtp">Ftp Setup</td>
    <td>
      <input type=radio name=edituser_ftp value="1"><font id="edituserFtpEnable">Enable</font>&nbsp;
      <input type=radio name=edituser_ftp value="0"><font id="edituserFtpDisable">Disable</font>
    </td>
  </tr>
  <tr> 
    <td class="title" id="edituserSmb">Samba Setup</td>
    <td>
      <input type=radio name=edituser_smb value="1"><font id="edituserSmbEnable">Enable</font>&nbsp;
      <input type=radio name=edituser_smb value="0"><font id="edituserSmbDisable">Disable</font>
    </td>
  </tr>
</table>

<hr />
<br />

<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=button style="{width:120px;}" value="Apply" id="edituserApply" onClick="submit_apply()"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="edituserCancel" onClick="window.close()">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
</body>
</html>

