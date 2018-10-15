<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>Administration</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("usb");
var count = 0;
var user0 = '<% getCfgGeneral(1, "Login"); %>';
var guest = '<% getCfgZero(1, "FtpAnonymous"); %>';
var user1 = '<% getCfgGeneral(1, "User1"); %>';
var upw1 = '<% getCfgGeneral(1, "User1Passwd"); %>';
var ftpuser1 = '<% getCfgZero(1, "FtpUser1"); %>';
var smbuser1 = '<% getCfgZero(1, "SmbUser1"); %>';
var user2 = '<% getCfgGeneral(1, "User2"); %>';
var upw2 = '<% getCfgGeneral(1, "User2Passwd"); %>';
var ftpuser2 = '<% getCfgZero(1, "FtpUser2"); %>';
var smbuser2 = '<% getCfgZero(1, "SmbUser2"); %>';
var user3 = '<% getCfgGeneral(1, "User3"); %>';
var upw3 = '<% getCfgGeneral(1, "User3Passwd"); %>';
var ftpuser3 = '<% getCfgZero(1, "FtpUser3"); %>';
var smbuser3 = '<% getCfgZero(1, "SmbUser3"); %>';
var user4 = '<% getCfgGeneral(1, "User4"); %>';
var upw4 = '<% getCfgGeneral(1, "User4Passwd"); %>';
var ftpuser4 = '<% getCfgZero(1, "FtpUser4"); %>';
var smbuser4 = '<% getCfgZero(1, "SmbUser4"); %>';
var user5 = '<% getCfgGeneral(1, "User5"); %>';
var upw5 = '<% getCfgGeneral(1, "User5Passwd"); %>';
var ftpuser5 = '<% getCfgZero(1, "FtpUser5"); %>';
var smbuser5 = '<% getCfgZero(1, "SmbUser5"); %>';
var user6 = '<% getCfgGeneral(1, "User6"); %>';
var upw6 = '<% getCfgGeneral(1, "User6Passwd"); %>';
var ftpuser6 = '<% getCfgZero(1, "FtpUser6"); %>';
var smbuser6 = '<% getCfgZero(1, "SmbUser6"); %>';
var user7 = '<% getCfgGeneral(1, "User7"); %>';
var upw7 = '<% getCfgGeneral(1, "User7Passwd"); %>';
var ftpuser7 = '<% getCfgZero(1, "FtpUser7"); %>';
var smbuser7 = '<% getCfgZero(1, "SmbUser7"); %>';
var user8 = '<% getCfgGeneral(1, "User8"); %>';
var upw8 = '<% getCfgGeneral(1, "User8Passwd"); %>';
var ftpuser8 = '<% getCfgZero(1, "FtpUser8"); %>';
var smbuser8 = '<% getCfgZero(1, "SmbUser8"); %>';
var user9 = '<% getCfgGeneral(1, "User8"); %>';
var upw9 = '<% getCfgGeneral(1, "User8Passwd"); %>';
var ftpuser9 = '<% getCfgZero(1, "FtpUser8"); %>';
var smbuser9 = '<% getCfgZero(1, "SmbUser8"); %>';
var user10 = '<% getCfgGeneral(1, "User8"); %>';
var upw10 = '<% getCfgGeneral(1, "User8Passwd"); %>';
var ftpuser10 = '<% getCfgZero(1, "FtpUser8"); %>';
var smbuser10 = '<% getCfgZero(1, "SmbUser8"); %>';

function initTranslation()
{
	var e = document.getElementById("storageAdmTitle");
	e.innerHTML = _("storage adm title");
	e = document.getElementById("storageAdmIntroduction");
	e.innerHTML = _("storage adm introduction");

	e = document.getElementById("storageAdmUser");
	e.innerHTML = _("storage adm user");
	e = document.getElementById("storageAdmUserName");
	e.innerHTML = _("storage user name");
	e = document.getElementById("storageAdmUserFtp");
	e.innerHTML = _("storage user ftp");
	e = document.getElementById("storageAdmUserSmb");
	e.innerHTML = _("storage user smb");

	e = document.getElementById("storageAdmUserAdd");
	e.value = _("usb add");
	e = document.getElementById("storageAdmUserEdit");
	e.value = _("usb edit");
	e = document.getElementById("storageAdmUserDel");
	e.value = _("usb del");
	e = document.getElementById("storageAdmApply");
	e.value = _("usb apply");
	e = document.getElementById("storageCancel");
	e.value = _("usb cancel");
}

function initValue()
{
	var tmp;

	initTranslation();
	for (i=1;i<11;i++)
	{
		if (eval('user'+i) != "")
			count++;
	}
}

function submit_apply(parm)
{
	document.storage_user_adm.hiddenButton.value = parm;
	document.storage_user_adm.submit();
}

function open_useradd_window()
{
	document.storage_user_adm.hiddenUser0.value = user0;
	document.storage_user_adm.hiddenUser1.value = user1;
	document.storage_user_adm.hiddenUser2.value = user2;
	document.storage_user_adm.hiddenUser3.value = user3;
	document.storage_user_adm.hiddenUser4.value = user4;
	document.storage_user_adm.hiddenUser5.value = user5;
	document.storage_user_adm.hiddenUser6.value = user6;
	document.storage_user_adm.hiddenUser7.value = user7;
	document.storage_user_adm.hiddenUser8.value = user8;
	document.storage_user_adm.hiddenUser9.value = user9;
	document.storage_user_adm.hiddenUser10.value = user10;
	if(count > 10)
		alert("User Accounts have exceeded Maximun!");
	else
		window.open("STORAGEadduser.asp","Storage_User_Add","toolbar=no, location=no, scrollbars=yes, resizable=no, width=640, height=640")
}

function open_useredit_window()
{
	var i = 0;
	var index = 0;

	if (count == 0)
	{
		alert("No user can be choosed!");
	}
	else
	{
		while (i <= count)
		{
			if (count == 1)
			{
				if (document.storage_user_adm.storage_user_select.checked == true)
				{
					index = document.storage_user_adm.storage_user_select.value;
					break;
				}
			}
			else if (count > 1)
			{
				if (document.storage_user_adm.storage_user_select[i].checked == true)
				{
					index = document.storage_user_adm.storage_user_select[i].value;
					break;
				}
			}
			i++;
		}
		// alert("user: "+index);
		if (index > 0)
		{
			document.storage_user_adm.hiddenUser0.value = user0;
			document.storage_user_adm.hiddenUser1.value = user1;
			document.storage_user_adm.hiddenUser2.value = user2;
			document.storage_user_adm.hiddenUser3.value = user3;
			document.storage_user_adm.hiddenUser4.value = user4;
			document.storage_user_adm.hiddenUser5.value = user5;
			document.storage_user_adm.hiddenUser6.value = user6;
			document.storage_user_adm.hiddenUser7.value = user7;
			document.storage_user_adm.hiddenUser8.value = user8;
			document.storage_user_adm.selectIndex.value = index;
			document.storage_user_adm.selectUser.value = eval('user'+index);
			document.storage_user_adm.selectPassword.value = eval('upw'+index);
			document.storage_user_adm.selectFtp.value = eval('ftpuser'+index);
			document.storage_user_adm.selectSmb.value = eval('smbuser'+index);
			window.open("STORAGEedituser.asp","Storage_User_Edit","toolbar=no, location=no, scrollbars=yes, resizable=no, width=640, height=640")
		}
		else
		{
			alert("please select one user account!");
		}
	}
}

</script>
</head>

<body onLoad="initValue()">
<table class="body"><tr><td>
<h1 id="storageAdmTitle">Administration</h1>
<p id="storageAdmIntroduction"></p>
<hr />
<form method=post name=storage_user_adm action="/goform/storageAdm">
<input type=hidden name=hiddenButton value="">
<input type=hidden name=hiddenUser0 value="">
<input type=hidden name=hiddenUser1 value="">
<input type=hidden name=hiddenUser2 value="">
<input type=hidden name=hiddenUser3 value="">
<input type=hidden name=hiddenUser4 value="">
<input type=hidden name=hiddenUser5 value="">
<input type=hidden name=hiddenUser6 value="">
<input type=hidden name=hiddenUser7 value="">
<input type=hidden name=hiddenUser8 value="">
<input type=hidden name=hiddenUser9 value="">
<input type=hidden name=hiddenUser10 value="">
<input type=hidden name=selectIndex value="">
<input type=hidden name=selectUser value="">
<input type=hidden name=selectPassword value="">
<input type=hidden name=selectFtp value="">
<input type=hidden name=selectSmb value="">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="4"><font id="storageAdmUser">User Management</font>
      </td>
  </tr>
  <tr align=center> 
    <td bgcolor="#E8F8FF" width=15px id="storageAdmUserSelect">&nbsp;</td>
    <td bgcolor="#E8F8FF" id="storageAdmUserName">User Name</td>
    <td bgcolor="#E8F8FF" id="storageAdmUserFtp">Allow to use FTP</td>
    <td bgcolor="#E8F8FF" id="storageAdmUserSmb">Allow to use Samba</td>
  </tr>
<script language="JavaScript" type="text/javascript">
var TRDHeader = "<tr align=center><td>";
var TDConneter = "</td><td>";
var TDFooter = "</td><tr>";

document.write(TRDHeader+"--"+TDConneter+user0+TDConneter);
document.write(_("usb enable")+TDConneter);
document.write(_("usb enable")+TDFooter);

if (guest == "1")
	document.write(TRDHeader+"--"+TDConneter+"anonymous"+TDConneter+_("usb enable")+TDConneter+_("usb disable")+TDFooter);
else
	document.write(TRDHeader+"--"+TDConneter+"anonymous"+TDConneter+_("usb disable")+TDConneter+_("usb disable")+TDFooter);
for (var i=1;i<11;i++)
{
	if (eval("user"+i) != "")
	{
		var item = TRDHeader;
		item += "<input type=radio name=storage_user_select value="+i+">";
		item += TDConneter;
		item += eval("user"+i);
		item += TDConneter;
		if (eval("ftpuser"+i) == "1")
		{
			item += _("usb enable");
			item += TDConneter;
		}
		else
		{
			item += _("usb disable");
			item += TDConneter;
		}
		if (eval("smbuser"+i) == "1")
			item += _("usb enable");	
		else
			item += _("usb disable");
		item += TDFooter;		
		document.write(item);	
	}
}
</script>
</table>
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type="button" style="{width:80px;}" name=storage_user_add value="Add" id="storageAdmUserAdd" onClick="open_useradd_window()">&nbsp;&nbsp;
      <input type="button" style="{width:80px;}" name=storage_user_edit value="edit" id="storageAdmUserEdit" onClick="open_useredit_window()">&nbsp;&nbsp;
      <input type="button" style="{width:80px;}" value="Delete" id="storageAdmUserDel" onClick="submit_apply('delete')"> &nbsp; &nbsp;
</table>
<hr />
<br>
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type="button" style="{width:80px;}" value="Apply" id="storageAdmApply" onClick="submit_apply('apply')"> 
      <input type="button" style="{width:80px;}" value="Cancle" id="storageCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
</body>
</html>

