<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
<meta http-equiv="PRAGMA" content="NO-CACHE">
<meta http-equiv="CACHE-CONTROL" content="NO-CACHE">

<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>

<title></title>

<script language="JavaScript" type="text/javascript">

Butterlate.setTextDomain("main");

function initTranslation()
{
	var e;

	e = document.getElementById("setup wizard button");
	e.innerHTML =_("treeapp enter setup wizard");

	e = document.getElementById("hwelcome messageA");
	e.innerHTML =_("treeapp welcome messageA");
	e = document.getElementById("hwelcome messageB");
	e.innerHTML =_("treeapp welcome messageB");

	e = document.getElementById("husage message1");
	e.innerHTML =_("treeapp usage message1");
	e = document.getElementById("husage message2");
	e.innerHTML =_("treeapp usage message2");
	e = document.getElementById("husage message3");
	e.innerHTML =_("treeapp usage message3");
}

function initValue()
{
	initTranslation();
}

</script>
<base target="mainFrame">
</head>

<!--<body topmargin="0px" align="center" onLoad="initValue()" link="#FFFFFF" vlink="#FFFFFF" alink="#FFFFFF" bgcolor="#DBE2EC">-->
<body topmargin="0px" align="center" onLoad="initValue();" link="#FFFFFF" vlink="#FFFFFF" alink="#FFFFFF" bgcolor="#FFFFFF">
<table class="content" align="center">
	<tr><td height="20">
		<table width="100%" border="0" cellspacing="0" cellpadding="10" height="100%" link="#4F7599" vlink="#4F7599" alink="#4F7599">
			<tr><td align="center" width="100%" height="100%"> 
				&nbsp;</td></tr>
			<tr><td align="center" width="100%" height="100%"> 
				&nbsp;</td></tr>
			<tr><td align="center" width="100%" height="100%"> 
				&nbsp;</td></tr>
			
			<tr>
			<td align="center" width="100%" height="100%"> 
			
				<div align="center"> 
               <!-- <font color="#4F7599" align="center" face="Arial" id="welcome message">Welcome to Configuration page.<br>Please use menu bar 
                or Setup Wizard button <br>
                to configure or get information from device.</font></b></p>-->
                <b>
                <font color="#4F7599" align="center" face="Arial" id="hwelcome messageA">Welcome to the </font>
				<font color="#4F7599" align="center" face="Arial"><% getCfgGeneral(1, "HostName"); %></font>
			    <font color="#4F7599" align="center" face="Arial" id="hwelcome messageB"> Home Page</font><br><br>
			   
                 <font color="#4F7599" align="center" face="Arial" id="husage message1">If you wish to directly configure or view the status of this device,</font><br>
                 <font color="#4F7599" align="center" face="Arial" id="husage message2">please use the menu bar located above.</font><br><br>
                 <font color="#4F7599" align="center" face="Arial" id="husage message3">For basic configuration to get started, enter Setup Wizard.</font>
               </b>
                                 
	      	</div>
        	    </td></tr>
			<tr><td align="center" width="100%" height="100%"> 
				&nbsp;</td></tr>
			<tr><td align="center" width="100%" height="100%"> 
<table border="0" cellpadding="0" cellspacing="0" style="border-collapse: collapse" bordercolor="#4F7599" width="36%" id="AutoNumber1" height="31" bgcolor="#4F7599">
  <tr>
    <td width="100%" height="31">
    <p align="center"><b><font color="#FFFFFF" face="Arial">
    <a href="wizard/wizard_langset.asp" id="setup wizard button"  style="text-decoration: none">
   Enter Setup Wizard</a></font></b></td>
  </tr>
</table>
              </td></tr>
		</table>
	</td></tr> 
</table>

<p align="center">&nbsp;</p>
</body>
</html>
