<!-- Copyright (c), Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Create New User Account</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("usb");
var ftpb = '<% getFtpBuilt(); %>';
var smbb = '<% getSmbBuilt(); %>';

function initTranslation()
{
	var e = document.getElementById("adduserbasic");
	e.innerHTML = _("adduser basic");
	e = document.getElementById("adduserName");
	e.innerHTML = _("adduser name");
	e = document.getElementById("adduserPW");
	e.innerHTML = _("adduser password");

	e = document.getElementById("adduserFtp");
	e.innerHTML = _("adduser ftp");
	e = document.getElementById("adduserFtpEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("adduserFtpDisable");
	e.innerHTML = _("usb disable");

	e = document.getElementById("adduserSmb");
	e.innerHTML = _("adduser smb");
	e = document.getElementById("adduserSmbEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("adduserSmbDisable");
	e.innerHTML = _("usb disable");

	e = document.getElementById("adduserApply");
	e.value = _("usb apply");
	e = document.getElementById("adduserCancel");
	e.value = _("usb cancel");
}
	
function initValue()
{
	initTranslation();

	document.storage_adduser.adduser_ftp[0].disabled = true;
	document.storage_adduser.adduser_ftp[1].disabled = true;
	document.storage_adduser.adduser_smb[0].disabled = true;
	document.storage_adduser.adduser_smb[1].disabled = true;
	if (ftpb == "1")
	{
		document.storage_adduser.adduser_ftp[0].disabled = false;
		document.storage_adduser.adduser_ftp[1].disabled = false;
	}
	if (smbb == "1")
	{
		document.storage_adduser.adduser_smb[0].disabled = false;
		document.storage_adduser.adduser_smb[1].disabled = false;
	}
}

function checkData()
{
	if (document.storage_adduser.adduser_name.value == "")
	{
		alert('Please specify User Name');
		document.storage_adduser.adduser_name.focus();
		document.storage_adduser.adduser_name.select();
		return false;
	}
	else if (document.storage_adduser.adduser_name.value.match(/[ `~!@#$%^&*\()+\|{}\[\]:;\"\'<,>.\/\\?]/))
	{
		alert('Don\'t enter /[ `~!@#$%^&*\()+\|{}\[\]:;\"\'<,>.\/\\?]/ in this feild');
		document.storage_adduser.adduser_name.focus();
		document.storage_adduser.adduser_name.select();
		return false;
	}
	else
	{
		var i = 0;
		while(i < 9)
		{
			if (document.storage_adduser.adduser_name.value == eval('opener.document.forms[0].hiddenUser'+i+'.value') || document.storage_adduser.adduser_name.value == "anonymous")
			{
				alert('This user has exited!');
				return false;
			}
			i++;
		}
	}

	return true;
}

function adduserClose()
{
	opener.location.reload();
}

function submit_apply()
{
	if (checkData() == true)
	{
		document.storage_adduser.submit();
		window.close();
	}
}
</script>
</head>

<body onLoad="initValue()" onUnload="adduserClose()">
<table class="body"><tr><td>

<form method=post name="storage_adduser" action="/goform/StorageAddUser">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="adduserbasic">Basic Setup</td>
  </tr>
  <tr>
    <td class="head" id="adduserName">User Name</td>
    <td>
      <input type=text name=adduser_name size=8 maxlength=8 value="">
    </td>
  </tr>
  <tr>
    <td class="head" id="adduserPW">Password</td>
    <td><input type="password" name="adduser_pw" size="16" maxlength="16" value=""></td>
  </tr>
</table>

<br />

<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" id="adduserFtp">Ftp Setup</td>
    <td>
      <input type=radio name=adduser_ftp value="1"><font id="adduserFtpEnable">Enable</font>&nbsp;
      <input type=radio name=adduser_ftp value="0" checked><font id="adduserFtpDisable">Disable</font>
    </td>
  </tr>
  <tr> 
    <td class="title" id="adduserSmb">Samba Setup</td>
    <td>
      <input type=radio name=adduser_smb value="1"><font id="adduserSmbEnable">Enable</font>&nbsp;
      <input type=radio name=adduser_smb value="0" checked><font id="adduserSmbDisable">Disable</font>
    </td>
  </tr>
</table>

<hr />
<br />

<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=button style="{width:120px;}" value="Apply" id="adduserApply" onClick="submit_apply()"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="adduserCancel" onClick="window.close()">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
</body>
</html>

