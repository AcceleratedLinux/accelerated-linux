<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<script type="text/javascript" src="/common.js"></script>

<title>Wizard - Lang Set</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("admin");

restartPage_init();

function style_display_on()
{
	if (window.ActiveXObject)
	{ // IE
		return "block";
	}
	else if (window.XMLHttpRequest)
	{ // Mozilla, Safari,...
		return "table-row";
	}
}

function showSuccessMsg()
{
	var e = _("wizard Success Message");
	alert(e);
}

</script>
</head>
<body onload="showSuccessMsg(); parent.fnDispWizard(5);" bgcolor="#FFFFFF">
</body></html>