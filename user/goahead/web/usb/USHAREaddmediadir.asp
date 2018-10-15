<!-- Copyright (c), Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Create Media Directory</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("usb");
var path_count = 0;

function initTranslation()
{
	var e = document.getElementById("adddirName");
	e.innerHTML = _("smb adddir name");
	e = document.getElementById("adddirAccessUser");
	e.innerHTML = _("smb adddir accessusers");

	e = document.getElementById("adddirApply");
	e.value = _("usb apply");
	e = document.getElementById("adddirCancel");
	e.value = _("usb cancel");
}
	
function initValue()
{
	// initTranslation();
}

function checkData()
{
	// alert("path count: "+path_count);

	if (path_count <= 0)
	{
		alert("No Directory");
		return false;
	}
	else if (path_count == 1)
	{
		if (document.media_adddir.dir_path.checked == false)
		{
			alert("Please choose one option");
			return false;
		}
	}
	else if (path_count > 1)
	{
		for(i=0;i<path_count;i++)
		{
			if (document.media_adddir.dir_path[i].checked == true)
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

function submit_apply()
{
	if (checkData() == true)
	{
		document.media_adddir.submit();
		opener.location.reload();
		window.close();
	}
}
</script>
</head>

<body onLoad="initValue()" onUnload="opener.location.reload()">
<table class="body"><tr><td>

<form method=post name="media_adddir" action="/goform/MediaDirAdd">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="3" id="adddirPath">Access Path</td>
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

