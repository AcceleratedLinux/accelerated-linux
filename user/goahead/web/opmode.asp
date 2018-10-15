<html>
<head>
<title>Operation Mode</title>
<link rel="stylesheet" href="style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script type="text/javascript" src="/common.js"></script>

<script language="JavaScript" type="text/javascript">
restartPage_init();

Butterlate.setTextDomain("main");

var sbuttonMax=1;

var opmode;
var old_mode;

function ModeOnChange()
{

	if (document.opmode.opMode.options.selectedIndex == 0) {
		document.getElementById("open mode readme").innerHTML =_("opmode mode b intro");
	}
	else if (document.opmode.opMode.options.selectedIndex == 1) {
		document.getElementById("open mode readme").innerHTML =_("opmode mode g intro");
	}
	else if (document.opmode.opMode.options.selectedIndex == 2) /*Ethernet Converter*/
	{
		document.getElementById("open mode readme").innerHTML =_("opmode mode e intro");
	}
	else if (document.opmode.opMode.options.selectedIndex == 3) /*AP Client*/
	{	// change ap client --> WISP, minglin, 2010/01/06 *start*
		document.getElementById("open mode readme").innerHTML = _("opmode mode wisp");//_("opmode mode a intro");
		// change ap client --> WISP, minglin, 2010/01/06 *end*
	}	
}

function changeMode()
{

	var nat_en = "<% getCfgZero(1, "natEnabled"); %>";
	var dpbsta = "<% getDpbSta(); %>";
	var ec_en = "<% getCfgZero(1, "ethConvert"); %>";
	var mii_built = "<% getMiiInicBuilt(); %>"
	var mii_en = "<% getCfg2Zero(1, "InicMiiEnable"); %>";

	// Accelecon hacks to limit options:

		nat_en = "0";
		dpbsta = "0";
		ec_en = "0";
		mii_built = "0";
		mii_en = "0";

	document.getElementById("eth_conv").style.visibility = "hidden";
	document.getElementById("eth_conv").style.display = "none";
	document.getElementById("eth_conv").style.disable = true;
	document.getElementById("nat").style.visibility = "hidden";
	document.getElementById("nat").style.display = "none";
	document.getElementById("nat").style.disable = true;
	document.getElementById("miiInic").style.visibility = "hidden";
	document.getElementById("miiInic").style.display = "none";
	document.getElementById("miiInic").style.disable = true;	
	
	if (document.opmode.opMode.options.selectedIndex == 0) { /*Bridge*/
		opmode = 0;
		if (dpbsta == "1")
		{
			document.getElementById("eth_conv").style.visibility = "visible";
			document.getElementById("eth_conv").style.display = "block";
			document.getElementById("eth_conv").style.disable = false;
			
			if (ec_en == "1") {
				document.opmode.ethConv.options.selectedIndex = 1;
			}
		}
		if (mii_built == "1") 
		{
			document.getElementById("miiInic").style.visibility = "visible";
			document.getElementById("miiInic").style.display = "block";
			document.getElementById("miiInic").style.disable = false;
			
			if (mii_en == "1") {
				document.opmode.miiMode.options.selectedIndex = 1;
			}
		}
		document.getElementById("open mode readme").innerHTML =_("opmode mode b intro");
	}
	else if (document.opmode.opMode.options.selectedIndex == 1) { /*Gateway*/
		opmode = 1;
		document.getElementById("open mode readme").innerHTML =_("opmode mode g intro");
		if (nat_en == "1") {
			document.opmode.natEnbl.options.selectedIndex = 1;
		}
	}
	else if (document.opmode.opMode.options.selectedIndex == 2) { /*Ethernet Converter*/
		opmode = 2;
		document.getElementById("nat").style.disable = false;
		document.opmode.natEnbl.options.selectedIndex = 1;
		
		document.getElementById("open mode readme").innerHTML =_("opmode mode e intro");
	}		
	else if (document.opmode.opMode.options.selectedIndex == 3) { /*AP Client*/
		opmode = 3;
		document.getElementById("open mode readme").innerHTML =_("opmode mode a intro");
		if (nat_en == "1") {
			document.opmode.natEnbl.options.selectedIndex = 1;
		}
	}
}

function initTranslation()
{
	var e = document.getElementById("oTitle");
	e.innerHTML = _("treeapp operation mode");
	e = document.getElementById("oIntroduction");
	e.innerHTML = _("opmode introduction");

	e = document.getElementById("oTitle_t");
	e.innerHTML = _("opmode title");

	e = document.getElementById("oModeB");
	e.innerHTML = _("opmode mode b");
	e = document.getElementById("oModeG");
	e.innerHTML = _("opmode mode g");
	/*e = document.getElementById("oModeE");
	e.innerHTML = _("opmode mode e");
	e = document.getElementById("oModeA");
	e.innerHTML = _("opmode mode a");*/
	
	e = document.getElementById("oSelect");
	e.innerHTML = _("treeapp operation mode");

	e = document.getElementById("oNat");
	e.innerHTML = _("opmode nat");
	e = document.getElementById("oNatD");
	e.innerHTML = _("main disable");
	e = document.getElementById("oNatE");
	e.innerHTML = _("main enable");

	e = document.getElementById("oEthConv");
	e.innerHTML = _("opmode eth conv");
	e = document.getElementById("oEthConvD");
	e.innerHTML = _("main disable");
	e = document.getElementById("oEthConvE");
	e.innerHTML = _("main enable");
	
	e = document.getElementById("oApply");
	e.value = _("main apply");
	e = document.getElementById("oCancel");
	e.value = _("main cancel");
}

function initValue()
{
	opmode = "<% getCfgZero(1, "OperationMode"); %>";
	old_mode = opmode;

	initTranslation();

	if (opmode == "1")
	{
		document.opmode.opMode.options.selectedIndex = 1;
		document.getElementById("open mode readme").innerHTML =_("opmode mode g intro");
	}		
	else if (opmode == "0")
	{
		document.opmode.opMode.options.selectedIndex = 0;
		document.getElementById("open mode readme").innerHTML =_("opmode mode b intro");
	}
	else if (opmode == "3")
	{
		document.opmode.opMode.options.selectedIndex = 3; /*Ethernet Converter*/
		document.getElementById("open mode readme").innerHTML =_("opmode mode e intro");
	}
	else if (opmode == "2")
	{
		document.opmode.opMode.options.selectedIndex = 2; /*AP Client*/
		document.getElementById("open mode readme").innerHTML =_("opmode mode a intro");
	}
	
	changeMode();
	ModeOnChange();
}

</script>
</head>

<body onLoad="initValue()" bgcolor="#FFFFFF">
<div align="center">
 <center>
<table class="body"><tr><td align="center">

<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="oTitle">Operation Mode</td>
</tr>
<tr>

<tr>
<td colspan="2">
<p class="head" id="oIntroduction">Operation Mode info...</p>
</td>
<tr>
</table>
<br>

<form method="post" name="opmode" action="/goform/setOpMode">

<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="3" id="oTitle_t" width="520">Operation Mode</td>
</tr>

<tr>
  <td width="104" bgcolor="#DBE2EC"> <font id="oSelect">Mode</font> </td>
  <!--<td width="83">-->
  <td width="421">
    <select id="opMode" name="opMode" size="1" onChange="ModeOnChange()">
      <option value="0" id="oModeB">Bridge</option>
     <!-- <option value="1" id="oModeG">Gateway</option> -->
      <option value="2" id="oModeA">WISP</option>
     <!-- <option value="3" id="oModeE">Ethernet Converter</option> -->
    </select>
    <table border="1" cellspacing="1" width="421">
      <tr>
        <td width="417" bgcolor="#FFFFFF" bordercolor="#DBE2EC" ><font color="#000000" id="open mode readme"></font></td>
      </tr>
    </table>
    
  </td>
</tr>

</table>

<table width="540" border="1" cellpadding="2" cellspacing="1">

<!-- Accelecon hack to limit options

<tr id="nat">
  <td width="104" bgcolor="#DBE2EC"> <font id="oNat">NAT Enabled: </font> </td>
  <td width="421">
    <select id="natEnbl" name="natEnbl" size="1">
      <option value="0" id="oNatD">Disable</option>
      <option value="1" id="oNatE">Enable</option>
    </select>
  </td>
</tr>

<tr id="eth_conv">
  <td width="104" bgcolor="#DBE2EC"> <font id="oEthConv">Ethernet Converter Enabled: </font> </td>
  <td width="421">
    <select id="ethConv" name="ethConv" size="1">
      <option value="0" id="oEthConvD">Disable</option>
      <option value="1" id="oEthConvE">Enable</option>
    </select>
  </td>
</tr>

<tr id="miiInic">
  <td width="104" bgcolor="#DBE2EC"> <font id="oMiiMode">INIC Mii Mode: </font> </td>
  <td width="421">
    <select id="miiMode" name="miiMode" size="1">
      <option value="0" id="oMiiModeD">Disable</option>
      <option value="1" id="oMiiModeE">Enable</option>
    </select>
  </td>
</tr>

-->

</table>

<br>
<table width="540" cellpadding="2" cellspacing="1">
<tr id="sbutton0">
  <td>
<p align="center">
<input type="submit" style="{width:120px;}" value="Apply" id="oApply" onClick="sbutton_disable(sbuttonMax); restartPage_block();">&nbsp;&nbsp;
<input type="reset" style="{width:120px;}" value="Reset" id="oCancel" onClick="window.location.reload()">
  </td>
</tr>
</table>

</form>

</td></tr></table>
 </center>
</div>
</body>
</html>
