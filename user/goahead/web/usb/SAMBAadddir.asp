<!-- Copyright (c), Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Create Directory</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("usb");
var path_count = 0;

function initTranslation()
{
	var e = document.getElementById("adddirName");
	e.innerHTML = _("smb adddir name");
	e = document.getElementById("adddirAccessUser");
	e.innerHTML = _("smb adddir accessusers");

	e = document.getElementById("addAccessPath");
	e.innerHTML = _("smb adddir accesspath");
	e = document.getElementById("adddirPath");
	e.innerHTML = _("storage part path");
	e = document.getElementById("adddirPartition");
	e.innerHTML = _("storage disk dirpart");

	e = document.getElementById("adddirApply");
	e.value = _("usb apply");
	e = document.getElementById("adddirCancel");
	e.value = _("usb cancel");
}
	
function initValue()
{
	initTranslation();
}

function checkData()
{
	if (document.smb_adddir.dir_name.value == "")
	{
		alert('Please specify Directory Name');
		document.smb_adddir.dir_name.focus();
		document.smb_adddir.dir_name.select();
		return false;
	}
	else if (document.smb_adddir.dir_name.value.match(/[*\()+\|{}\[\]:\"\'<>\/\\?]/)) 
	{
		alert("Don't input /[*\()+\|{}\[\]:\"\'<>\/\\?]/");
		document.smb_adddir.dir_name.focus();
		document.smb_adddir.dir_name.select();
		return false;
	}

	// alert("path count: "+path_count);

	if (path_count <= 0)
	{
		alert("No Directory");
		return false;
	}
	else if (path_count == 1)
	{
		if (document.smb_adddir.dir_path.checked == false)
		{
			alert("Please choose one option");
			return false;
		}
	}
	else if (path_count > 1)
	{
		for(i=0;i<path_count;i++)
		{
			if (document.smb_adddir.dir_path[i].checked == true)
				break;
		}
		if (i == path_count)
		{
			alert("Please choose one option");
			return false;
		}
	}

	return true;
}

function addDirClose()
{
	opener.location.reload();
}

function submit_apply()
{
	if (checkData() == true)
	{
		document.smb_adddir.submit();
		opener.location.reload();
		window.close();
	}
}
</script>
</head>

<body onLoad="initValue()" onUnload="addDirClose()">
<table class="body"><tr><td>

<form method=post name="smb_adddir" action="/goform/SmbDirAdd">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" id="adddirName">Directory Name</td>
    <td>
      <input type=text name=dir_name size=16 maxlength=16 value="">
    </td>
  </tr>
  <tr>
    <td class="title" id="adddirAccessUser">Access User</td>
    <td>
    <script language="JavaScript" type="text/javascript">
    for (i=1;i<=8;i++)
    {
      var user = eval('opener.document.forms[0].hidden_user'+i+'.value');

      document.write("&nbsp;");
      if (user != "")
      {
	document.write("<input type=\"checkbox\" name=\"allow_user\" value=\""+user+"\">");
	document.write(user);
	document.write("<br />");
      }
    }
    </script>
    </td>
  </tr>
</table>

<hr />
<br />

<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="3" id="addAccessPath">Access Path</td>
  </tr>
  <tr> 
    <td bgcolor="#E8F8FF" width=15px>&nbsp;</td>
    <td align="center" bgcolor="#E8F8FF" id="adddirPath">Path</td>
    <td align="center" bgcolor="#E8F8FF" id="adddirPartition">Partition</td>
  </tr>
  <% ShowAllDir(); %>
<script language="JavaScript" type="text/javascript">
path_count = parseInt('<% getCount(1, "AllDir"); %>');
</script>
</table>

<hr />
<br />

<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=button style="{width:120px;}" value="Apply" id="adddirApply" onClick="submit_apply()"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="adddirCancel" onClick="window.close()">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
</body>
</html>


