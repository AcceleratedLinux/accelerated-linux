<html>
<head>
<title></title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
</head>
<script type="text/javascript" src="/lang/b28n.js"></script>
<script language="JavaScript">
Butterlate.setTextDomain("admin");

function GotoPrevious()
{
    window.location.replace(document.referrer);
}

function initTranslation()
{
//	var e = document.getElementById("id_saved");
//	e.innerHTML = _("config saved ok");
}

function initValue()
{
	initTranslation();
}
</script>


<!--<body onload="initValue()"> Carl modified to remove user press ok!-->
<body onload="GotoPrevious()">
<!--
<table width="100%" border="0" cellspacing="0" cellpadding="10" height="100%">
  <tr> 
    <td> 
      <div align="center"> 
        <p><b><font color="#FF0000" id="id_saved"><br>
          Configuration has been saved!</font></b></p>
        <form name="form1" method="post" action="">
          <p>
          <input type="button" name="save" onClick="GotoPrevious()" value="OK">
          </p>
        </form>
        <p>&nbsp;</p>
      </div>
    </td>
  </tr>
  <tr>
    <td height="60" valign="bottom"> 
      <table width="100%" border="0" cellspacing="0" cellpadding="0">
        <tr>
          <td>
            <div align="right">&nbsp;</div>
          </td>
        </tr>
      </table>
    </td>
  </tr>
</table>
-->
</body>
</html>
