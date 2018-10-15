<html><head><title>System command</title>

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("admin");

function initTranslation()
{
	var e = document.getElementById("syscommandTitle");
	e.innerHTML = _("syscommand title");
	e = document.getElementById("syscommandIntroduction");
	e.innerHTML = _("syscommand introduction");

	e = document.getElementById("syscommandSysCommand");
	e.innerHTML = _("syscommand system command");
	e = document.getElementById("syscommandCommand");
	e.innerHTML = _("syscommand command");
	e = document.getElementById("syscommandApply");
	e.value = _("admin apply");
	e = document.getElementById("syscommandCancel");
	e.value = _("admin cancel");
}

function formCheck()
{
	if( document.SystemCommand.command.value == ""){
		alert("Please specify a command.");
		return false;
	}

	return true;
}

function setFocus()
{
	initTranslation();
	document.SystemCommand.command.focus();
}

</script>

</head>
<body onload="setFocus()">
<table class="body"><tr><td>
<h1 id="syscommandTitle">System Command</h1>
<p id="syscommandIntroduction"> Run a system command as root: </p>

<!-- ================= System command ================= -->
<form method="post" name="SystemCommand" action="/goform/SystemCommand">
<table border="1" cellpadding="2" cellspacing="1" width="95%">
<tbody><tr>
  <td class="title" colspan="2" id="syscommandSysCommand">System command: </td>
</tr>
<tr>
  <td class="head" id="syscommandCommand">Command:</td>
	<td> <input type="text" name="command" size="30" maxlength="256" > </td>
</tr>
<tr><td colspan=2>
		<textarea cols="63" rows="20" wrap="off" readonly="1">
<% showSystemCommandASP(); %>
        </textarea></td>
</tr>
</table>
<input value="Apply" id="syscommandApply" name="SystemCommandSubmit" onclick="return formCheck()" type="submit"> &nbsp;&nbsp;
<input value="Reset" id="syscommandCancel" name="reset" type="reset">
</form> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;

<!-- ================ repeat last system command ================= -->
<form method="post" name="repeatLastSystemCommand" action="/goform/repeatLastSystemCommand">
<input value="Repeat Last Command" id="repeatLastCommand" name="repeatLastCommand" type="submit"> &nbsp;&nbsp;
</form>


<br>
</td></tr></table>
</body></html>
