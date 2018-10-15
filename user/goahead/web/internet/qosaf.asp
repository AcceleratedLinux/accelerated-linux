<!-- Copyright (c), Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<script type="text/javascript" src="/common.js"></script>

<title> </title>
<script language="JavaScript" type="text/javascript">

restartPage_init();

Butterlate.setTextDomain("internet");

function initTranslation()
{
	var e;

	e = document.getElementById("QoSAFNameStr");
	e.innerHTML = _("qos AF Name");
	e = document.getElementById("QoSAFRateStr");
	e.innerHTML = _("qos Rate");
	e = document.getElementById("QoSAFCeilStr");
	e.innerHTML = _("qos Ceil");
/*	e = document.getElementById("QoSAFUploadBandwidth");
	e.innerHTML = _("qos AF Upload Bandwidth");*/
	e = document.getElementById("QoSAFModifyStr");
	e.value = _("qos group modify");
}
	
function is_valid_text(text, is_extra)
{
	var num = "0123456789";
	var ascii = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	var extra = "~!@#$%^&*)_+{};<>[]:,.";
	var extra;
    var i;
    var valid_string = num + ascii + extra;
	
	if (!is_extra) extra = "";
    if (text.length > 32 || text.length < 0)
        return false;
    for (i = 0; i < text.length; i++)
	{
        if (!is_contained_char(text.substr(i, 1), valid_string))
            return false;
	}
    return true;
}

function initValue()
{
	initTranslation();

    document.afattr.af_index.value = opener.document.forms[0].ModifyAFIndex.value;
    document.afattr.af_name.value = opener.document.forms[0].ModifyAFName.value;
    document.afattr.af_rate.value = opener.document.forms[0].ModifyAFRate.value;
    document.afattr.af_ceil.value = opener.document.forms[0].ModifyAFCeil.value;
}


function checkForm()
{
	if(	document.afattr.af_name.value == "" ||
		document.afattr.af_rate.value == "" || 
		document.afattr.af_ceil.value == "" ){
		alert("Please fill the every field.");
		return false;
	}
	if (!is_valid_text(document.afattr.af_name.value, false)) {
		alert("Illegal characters are not allowed.");
		document.afattr.af_name.focus();
		return false;
	}
	if (isNaN(document.afattr.af_rate.value) || 
		document.afattr.af_rate.value.indexOf('.') != -1){
		alert("Integer number only.");
		document.afattr.af_rate.focus();
		return false;
	}
	if (isNaN(document.afattr.af_ceil.value) || 
		document.afattr.af_ceil.value.indexOf('.') != -1){
		alert("Integer number only.");
		document.afattr.af_ceil.focus();
		return false;
	}

	var rate = parseInt(document.afattr.af_rate.value);
	var ceil = parseInt(document.afattr.af_ceil.value);

	if( ceil > 100){
		alert("The ceil number format is bigger than 100 .");
		return false;
	}

	if( ceil < 0 || rate < 0  ){
		alert("The value can't be negative.");
		return false;
	}

	if( rate > ceil){
		alert("The rate can't be bigger than ceil.");
		return false;
	}
	
	restartPage_block();

	return true;
}



function submit_form()
{
	if (checkForm() == true){
		document.afattr.submit();
		opener.location.reload();
		window.close();
	}
}
</script>
</head>

<body onLoad="initValue()">
<table class="body"><tr><td>


<form method=post name="afattr" action="/goform/QoSAFAttribute">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">

  <input type=hidden name="af_index" id="af_index" value="">
  <tr>
	<td colspan="2" class="title" id="AFModifyTitle">
		<script>
			if(opener.document.forms[0].ModifyAFName.value == "")
				document.write("NoName");
			else
				document.write(opener.document.forms[0].ModifyAFName.value);
		</script>
	&nbsp;</td>
  </tr>
  <tr>
    <td class="head" id="QoSAFNameStr"> Name</td>
    <td><input type=text name="af_name" maxlength=32 size="20"></td>
  </tr>
  <tr>
    <td class="head" id="QoSAFRateStr">Rate</td>
    <td id="QoSAFUploadBandwidth">
    <input type=text name="af_rate" maxlength=2 size="20"> % of upload bandwidth</td>
  </tr>
  <tr>
    <td class="head" id="QoSAFCeilStr">Ceil</td>
    <td><input type=text name="af_ceil" maxlength=3 size="20"> % of upload bandwidth</td>
  </tr>
</table>
<input type="submit" id="QoSAFModifyStr" name="modify" value="modify" onClick="return checkForm()">

</form>

</td></tr></table>

</body>
</html>
