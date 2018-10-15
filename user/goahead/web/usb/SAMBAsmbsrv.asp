<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>SAMBA Settings</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("usb");

var smbenabled = '<% getCfgZero(1, "SmbEnabled"); %>';
var smbwg = '<% getCfgGeneral(1, "HostName"); %>';
var smbnetbios = '<% getCfgGeneral(1, "SmbNetBIOS"); %>';
var dir_count = 0;

function initTranslation()
{
	var e = document.getElementById("smbTitle");
	e.innerHTML = _("smb title");

	e = document.getElementById("smbSrvSet");
	e.innerHTML = _("smb server setup");
	e = document.getElementById("smbSrv");
	e.innerHTML = _("smb server enable");
	e = document.getElementById("smbSrvEnable");
	e.innerHTML = _("usb enable");
	e = document.getElementById("smbSrvDisable");
	e.innerHTML = _("usb disable");
	e = document.getElementById("smbSrvWG");
	e.innerHTML = _("smb server workgroup");
	e = document.getElementById("smbSrvNetBIOS");
	e.innerHTML = _("smb server netbios");
	e = document.getElementById("smbShareDirList");
	e.innerHTML = _("smb server dirlist");
	e = document.getElementById("smbDirName");
	e.innerHTML = _("smb server dirname");
	e = document.getElementById("smbDirPath");
	e.innerHTML = _("smb server dirpath");
	e = document.getElementById("smbDirAllowUsers");
	e.innerHTML = _("smb server allowuser");

	e = document.getElementById("smbAdd");
	e.value = _("usb add");
	e = document.getElementById("smbEdit");
	e.value = _("usb edit");
	e = document.getElementById("smbDel");
	e.value = _("usb del");
	e = document.getElementById("smbApply");
	e.value = _("usb apply");
	e = document.getElementById("smbCancel");
	e.value = _("usb cancel");
}

function initValue()
{
	initTranslation();

	// alert(smbenabled);

	document.storage_smb.smb_workgroup.disabled = true;
	document.storage_smb.smb_netbios.disabled = true;

	if (smbenabled == "1")
	{
		// alert("SAMBA E");
		document.storage_smb.smb_enabled[0].checked = true;
		document.storage_smb.smb_workgroup.disabled = false;
		document.storage_smb.smb_workgroup.value = smbwg;
		document.storage_smb.smb_netbios.disabled = false;
		document.storage_smb.smb_netbios.value = smbnetbios;
	}
	else
	{
		// alert("SAMBA D");
		document.storage_smb.smb_enabled[1].checked = true;
	}
}

function CheckSelect()
{
	// alert("dir_count: "+dir_count);
	if (dir_count <= 0)
	{
		alert("No any option can be choosed!");
		return false;
	}
	else if (dir_count == 1)
	{
		if (document.storage_smb.smb_dir.checked == false)	
		{
			alert("please select one option!");
			return false;
		}
		document.storage_smb.selectIndex.value = 0;
		document.storage_smb.selectDir.value = document.storage_smb.smb_dir.value;
		document.storage_smb.selectPermit.value = document.storage_smb.smb_dir_permit.value;
		document.storage_smb.selectPath.value = document.storage_smb.smb_dir_path.value;
	}
	else
	{
		for(i=0;i<dir_count;i++)
		{
			if (document.storage_smb.smb_dir[i].checked == true)
			{
				document.storage_smb.selectIndex.value = i;
				document.storage_smb.selectDir.value = document.storage_smb.smb_dir[i].value;
				document.storage_smb.selectPermit.value = document.storage_smb.smb_dir_permit[i].value;
				document.storage_smb.selectPath.value = document.storage_smb.smb_dir_path[i].value;
				break;
			}
		}
		if (i == dir_count)
		{
			alert("please select one option!");
			return false;
		}
	}

	return true;
}

function CheckValue()
{
	if (document.storage_smb.smb_enabled[0].checked == true)
	{
		if (document.storage_smb.smb_workgroup.value == "")
		{
			alert('Please specify SAMBA Workgroup');
			document.storage_smb.smb_workgroup.focus();
			document.storage_smb.smb_workgroup.select();
			return false;
		}
		else if (document.storage_smb.smb_workgroup.value.indexOf(" ") >= 0)
		{
			alert('Don\'t enter Blank Space in this feild');
			document.storage_smb.smb_workgroup.focus();
			document.storage_smb.smb_workgroup.select();
			return false;
		}

		if (document.storage_smb.smb_netbios.value == "")
		{
			alert('Please specify SAMBA NetBIOS Name');
			document.storage_smb.smb_netbios.focus();
			document.storage_smb.smb_netbios.select();
			return false;
		}
		else if (document.storage_smb.smb_netbios.value.indexOf(" ") >= 0)
		{
			alert('Don\'t enter Blank Space in this feild');
			document.storage_smb.smb_netbios.focus();
			document.storage_smb.smb_netbios.select();
			return false;
		}
	}

	return true;
}

function smb_enable_switch()
{
	if (document.storage_smb.smb_enabled[1].checked == true)
	{
		document.storage_smb.smb_workgroup.disabled = true;
		document.storage_smb.smb_netbios.disabled = true;
	}
	else
	{
		document.storage_smb.smb_workgroup.disabled = false;
		document.storage_smb.smb_netbios.disabled = false;
	}
}

function open_diradd_window()
{
	document.storage_smb.hidden_user1.value = '<% getCfgGeneral(1, "User1"); %>';
	document.storage_smb.hidden_user2.value = '<% getCfgGeneral(1, "User2"); %>';
	document.storage_smb.hidden_user3.value = '<% getCfgGeneral(1, "User3"); %>';
	document.storage_smb.hidden_user4.value = '<% getCfgGeneral(1, "User4"); %>';
	document.storage_smb.hidden_user5.value = '<% getCfgGeneral(1, "User5"); %>';
	document.storage_smb.hidden_user6.value = '<% getCfgGeneral(1, "User6"); %>';
	document.storage_smb.hidden_user7.value = '<% getCfgGeneral(1, "User7"); %>';
	document.storage_smb.hidden_user8.value = '<% getCfgGeneral(1, "User8"); %>';

	window.open("SAMBAadddir.asp","Samba_Dir_Add","toolbar=no, location=no, scrollbars=yes, resizable=no, width=640, height=640")
}

function open_diredit_window()
{
	if (CheckSelect())
	{
		document.storage_smb.hidden_user1.value = '<% getCfgGeneral(1, "User1"); %>';
		document.storage_smb.hidden_user2.value = '<% getCfgGeneral(1, "User2"); %>';
		document.storage_smb.hidden_user3.value = '<% getCfgGeneral(1, "User3"); %>';
		document.storage_smb.hidden_user4.value = '<% getCfgGeneral(1, "User4"); %>';
		document.storage_smb.hidden_user5.value = '<% getCfgGeneral(1, "User5"); %>';
		document.storage_smb.hidden_user6.value = '<% getCfgGeneral(1, "User6"); %>';
		document.storage_smb.hidden_user7.value = '<% getCfgGeneral(1, "User7"); %>';
		document.storage_smb.hidden_user8.value = '<% getCfgGeneral(1, "User8"); %>';

		window.open("SAMBAeditdir.asp","Samba_Dir_Edit","toolbar=no, location=no, scrollbars=yes, resizable=no, width=640, height=640")
	}
}

function submit_apply(parm)
{
	if (parm == "delete")
	{
		if (CheckSelect())
		{
			document.storage_smb.hiddenButton.value = parm;
			document.storage_smb.submit();
		}
	}
	else if (parm == "apply")
	{
		if (CheckValue())
		{
			document.storage_smb.hiddenButton.value = parm;
			document.storage_smb.submit();
		}
	}
}

</script>
</head>

<body onLoad="initValue()">
<table class="body"><tr><td>
<h1 id="smbTitle">SAMBA Settings </h1>
<p id="smbIntroduction"></p>
<hr />

<form method=post name=storage_smb action="/goform/storageSmbSrv">
<input type=hidden name=selectIndex value="">
<input type=hidden name=selectDir value="">
<input type=hidden name=selectPermit value="">
<input type=hidden name=selectPath value="">
<input type=hidden name=hiddenButton value="">
<input type=hidden name=hidden_user1 value="">
<input type=hidden name=hidden_user2 value="">
<input type=hidden name=hidden_user3 value="">
<input type=hidden name=hidden_user4 value="">
<input type=hidden name=hidden_user5 value="">
<input type=hidden name=hidden_user6 value="">
<input type=hidden name=hidden_user7 value="">
<input type=hidden name=hidden_user8 value="">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="smbSrvSet">SAMBA Server Setup</td>
  </tr>
  <tr> 
    <td class="head" id="smbSrv">SAMBA Server</td>
    <td>
      <input type=radio name=smb_enabled value="1" onClick="smb_enable_switch()"><font id="smbSrvEnable">Enable</font>&nbsp;
      <input type=radio name=smb_enabled value="0" onClick="smb_enable_switch()" checked><font id="smbSrvDisable">Disable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="smbSrvWG">Workgroup</td>
    <td>
      <input type=text name=smb_workgroup size=16 maxlength=16 value="ralink">
    </td>
  </tr>
  <tr>
    <td class="head" id="smbSrvNetBIOS">NetBIOS Name</td>
    <td>
      <input type=text name=smb_netbios size=16 maxlength=16 value="RalinkSoC">
    </td>
  </tr>
</table>

<hr />
<br />

<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="4" id="smbShareDirList">Sharing Directory List</td>
  </tr>
  <tr>
    <td bgcolor="#E8F8FF" width=15px>&nbsp;</td>
    <td align="center" bgcolor="#E8F8FF" width=150px id="smbDirName">Directory Name</td>
    <td align="center" bgcolor="#E8F8FF" width=150px id="smbDirPath">Directory Path</td>
    <td align="center" bgcolor="#E8F8FF" id="smbDirAllowUsers">Allowes Users</td>
  </tr>
  <% ShowSmbDir(); %>
<script language="JavaScript" type="text/javascript">
dir_count = parseInt('<% getCount(1, "AllSmbDir"); %>');
</script>
</table>
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=button style="{width:120px;}" value="Add" id="smbAdd" onClick="open_diradd_window()"> &nbsp; &nbsp;
      <input type=button style="{width:120px;}" value="Edit" id="smbEdit" onClick="open_diredit_window()"> &nbsp; &nbsp;
      <input type=button style="{width:120px;}" value="Del" id="smbDel" onClick="submit_apply('delete')"> &nbsp; &nbsp;
    </td>
  </tr>
</table>
<hr />
<br />
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=button style="{width:120px;}" value="Apply" id="smbApply" onClick="submit_apply('apply')">
      <input type=button style="{width:120px;}" value="Cancle" id="smbCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
</body>
</html>
