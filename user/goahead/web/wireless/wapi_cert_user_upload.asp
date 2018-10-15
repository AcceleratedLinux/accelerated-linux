<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>WAPI User Certificate Install</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

function initTranslation()
{
	var e = document.getElementById("WapiCertTitle");
	e.innerHTML = _("wapi cert_title");
	e = document.getElementById("WapiCertUserTitle");
	e.innerHTML = _("wapi cert_user");
	
	e = document.getElementById("WapiCertApply");
	e.value = _("wireless apply");
	e = document.getElementById("WapicertReset");
	e.value = _("wireless cancel");
}

function submit_apply()
{
	document.wapi_cert_user_upload.submit();
	window.opener.location.reload();
	window.close();
}
</script>
</head>

<body>
<form method="post" name="wapi_cert_user_upload" action="/cgi-bin/upload_wapi_user_cert.cgi" enctype="multipart/form-data">
<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td class="title" colspan="2" id="WapiCertTitle">Upload Certificate</td>
  </tr>
  <tr>
    <td class="head" id="WapiCertUserTitle">WAPI User Certificate</td>
    <td>
      <input type="file" name="wapi_user_cert_file" maxlength="256">
    </td>
  </tr>
</table>
<br />
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td >
      <input type="submit" style="{width:120px;}" value="Apply" id="WapiCertApply">
      <input type="reset" style="{width:120px;}" value="Reset" id="WapicertReset">
    </td>
  </tr>
</table>
</form>
</body>
</html>
