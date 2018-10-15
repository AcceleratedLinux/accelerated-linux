<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>WPA/WPA2-Enterprise Certificate Upload</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

function initTranslation()
{
	var e = document.getElementById("WapiCertTitle");
	e.innerHTML = _("wapi certtitle");
	e = document.getElementById("CertKeyTitle");
	e.innerHTML = _("wapi cert_as");
	
	e = document.getElementById("WapiCertApply");
	e.value = _("wireless apply");
	e = document.getElementById("WapiCertReset");
	e.value = _("wireless cancel");
}

function submit_apply()
{
	document.wapi_cert_as_upload.submit();
	window.opener.location.reload();
	window.close();
}
</script>
</head>

<body>
<form method="post" name="wapi_cert_as_upload" action="/cgi-bin/upload_wapi_as_cert.cgi" enctype="multipart/form-data">
<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td class="title" colspan="2" id="WapiCertTitle">Upload WAPI Certificate</td>
  </tr>
  <tr>
    <td class="head" id="WapiAsCertTitle">WAPI AS Certificate</td>
    <td>
      <input type="file" name="wapi_as_cert_file" maxlength="256">
    </td>
  </tr>
</table>
<br />
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td >
      <input type="submit" style="{width:120px;}" value="Apply" id="WapiCertApply">
      <input type="reset" style="{width:120px;}" value="Reset" id="WapiCertReset">
    </td>
  </tr>
</table>
</form>
</body>
</html>
