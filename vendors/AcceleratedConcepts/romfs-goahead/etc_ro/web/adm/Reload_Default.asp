<html><head><title>Upload Firmware</title>

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("admin");

function initTranslation()
{
	var e = document.getElementById("uploadSucess1");
	e.innerHTML = _("uploadSucess1");
	e = document.getElementById("uploadSucess2");
	e.innerHTML = _("uploadSucess2");
}

function pageInit()
{
	initTranslation();
}

function f_close(){
	alert("Please close your browser and reconnect to home page! ");
}

</script></head><body bgcolor="#DBE2EC" onLoad="pageInit()">
<div align="center"> <center>
<table class="body"> <tbody><tr><td>

<p><font id="uploadSucess1">Upgrade successfully. </font><br><br>
<font id="uploadSucess2" color="#FF0000">Please reboot your system first.</font> </p>

<form method="post" name="reboot_fm" action="/goform/setReboot" enctype="multipart/form-data">
<input value="Apply" id="reboot" name="reboot_in" type="submit" onClick="f_close();">
</form>

</td></tr></tbody></table>
 </center>
</div>
</body></html>