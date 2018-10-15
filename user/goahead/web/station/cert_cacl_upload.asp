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
	var e = document.getElementById("CertTitle");
	e.innerHTML = _("cert title");
	e = document.getElementById("CertCACLTitle");
	e.innerHTML = _("cert ca_cl");
	
	e = document.getElementById("CertApply");
	e.value = _("wireless apply");
	e = document.getElementById("CertReset");
	e.value = _("wireless cancel");
}

</script>
</head>

<body>
<form method="post" name="cert_ca_client_upload" action="/cgi-bin/upload_caclcert.cgi" enctype="multipart/form-data">
<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td class="title" colspan="2" id="CertTitle">Upload Certificate</td>
  </tr>
  <tr>
    <td class="head" id="CertCACLTitle">CA & Client Certificate</td>
    <td>
      <input type="file" name="ca_client_cert_file" maxlength="256">
    </td>
  </tr>
</table>
<br />
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td >
      <input type="submit" style="{width:120px;}" value="Apply" id="CertApply">
      <input type="reset" style="{width:120px;}" value="Reset" id="CertReset">
    </td>
  </tr>
</table>
</form>
</body>
</html>
