<!-- Copyright (c), Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Edit Directory</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("usb");
var index = opener.document.forms[0].selectIndex.value;
var dir_name = opener.document.forms[0].selectDir.value;
var allowuser = opener.document.forms[0].selectPermit.value;
var path = opener.document.forms[0].selectPath.value;

function initTranslation()
{
	var e = document.getElementById("editdirName");
	e.innerHTML = _("smb adddir name");
	var e = document.getElementById("editdirPath");
	e.innerHTML = _("storage part path");
	e = document.getElementById("editdirAccessUser");
	e.innerHTML = _("smb adddir accessusers");

	e = document.getElementById("editdirApply");
	e.value = _("usb apply");
	e = document.getElementById("editdirCancel");
	e.value = _("usb cancel");
}
	
function initValue()
{
	initTranslation();
}

function EditDirClose()
{
	opener.location.reload();
}

function submit_apply()
{
	document.smb_editdir.hidden_index.value = opener.document.forms[0].selectIndex.value;
	document.smb_editdir.submit();
	opener.location.reload();
	window.close();
}
</script>
</head>

<body onLoad="initValue()" onUnload="EditDirClose()">
<table class="body"><tr><td>

<form method=post name="smb_editdir" action="/goform/SmbDirEdit">
<input type=hidden name=hidden_index value="">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr> 
    <td class="title" id="editdirName">Directory Name</td>
    <td>
      <script language="JavaScript" type="text/javascript">
        document.write(dir_name);  
      </script>
    </td>
  </tr>
  <tr> 
    <td class="title" id="editdirPath">Path</td>
    <td>
      <script language="JavaScript" type="text/javascript">
        document.write(path);  
      </script>
    </td>
  </tr>
</table>

<hr />
<br />

<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" id="editdirAccessUser">Access User</td>
  </tr>
  <script language="JavaScript" type="text/javascript">
  var TRDHeader = "<tr class=head align=center><td>";
  var TDConneter = "</td><td>";
  var TDFooter = "</td><tr>";
  for (i=1;i<=8;i++)
  {
    var user = eval('opener.document.forms[0].hidden_user'+i+'.value');
    if (user != "")
    {
	    var item = TRDHeader;

	    item += user;
	    item += TDConneter;
	    if (allowuser.indexOf(user) >= 0)
	    	item += "<input type=\"checkbox\" name=\"allow_user\" value=\""+user+"\" checked>";
	    else
	    	item += "<input type=\"checkbox\" name=\"allow_user\" value=\""+user+"\">";
	    item += TDFooter;
	    document.write(item);
    }
  }
</script>
</table>

<hr />
<br />

<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=button style="{width:120px;}" value="Apply" id="editdirApply" onClick="submit_apply()"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="editdirCancel" onClick="window.close()">
    </td>
  </tr>
</table>
</form>

</td></tr></table>
</body>
</html>

