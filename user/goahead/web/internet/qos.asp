<html><head><title>QoS/Bandwidth Management Settings</title>

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script type="text/javascript" src="/common.js"></script>

<script language="JavaScript" type="text/javascript">

restartPage_init();

Butterlate.setTextDomain("internet");

var QOS_MAX_HARD_LIMITED = "60M";						// Our max bandwidth that we can deal with.
var QOS_MIN_HARD_LIMITED = "32k";						// Our min bandwidth that we hope.

var QoS = "<% getCfgGeneral(1, "QoSEnable"); %>";
var uploadBandwidth = "<% getCfgGeneral(1, "QoSUploadBandwidth"); %>";
var downloadBandwidth = "<% getCfgGeneral(1, "QoSDownloadBandwidth"); %>";
var uploadBandwidth_custom = "<% getCfgGeneral(1, "QoSUploadBandwidth_custom"); %>";
var downloadBandwidth_custom = "<% getCfgGeneral(1, "QoSDownloadBandwidth_custom"); %>";


QoS = (QoS == "") ? "0" : QoS;

var AF1Name = "<% getCfgGeneral(1, "QoSAF1Name"); %>";
var AF2Name = "<% getCfgGeneral(1, "QoSAF2Name"); %>";
var AF3Name = "<% getCfgGeneral(1, "QoSAF3Name"); %>";
var AF4Name = "<% getCfgGeneral(1, "QoSAF4Name"); %>";
var AF5Name = "<% getCfgGeneral(1, "QoSAF5Name"); %>";
var AF6Name = "<% getCfgGeneral(1, "QoSAF6Name"); %>";
AF1Name = (AF1Name == "") ? "NoName1" : AF1Name;
AF2Name = (AF2Name == "") ? "NoName2" : AF2Name;
AF3Name = (AF3Name == "") ? "NoName3" : AF3Name;
AF4Name = (AF4Name == "") ? "NoName4" : AF4Name;
AF5Name = (AF5Name == "") ? "NoName5" : AF5Name;
AF6Name = (AF6Name == "") ? "Default" : AF6Name;

var AF1Rate = "<% getCfgGeneral(1, "QoSAF1Rate"); %>";
var AF2Rate = "<% getCfgGeneral(1, "QoSAF2Rate"); %>";
var AF3Rate = "<% getCfgGeneral(1, "QoSAF3Rate"); %>";
var AF4Rate = "<% getCfgGeneral(1, "QoSAF4Rate"); %>";
var AF5Rate = "<% getCfgGeneral(1, "QoSAF5Rate"); %>";
var AF6Rate = "<% getCfgGeneral(1, "QoSAF6Rate"); %>";

AF1Rate = (AF1Rate == "") ? "10" : AF1Rate;		// default 10% 
AF2Rate = (AF2Rate == "") ? "10" : AF2Rate;
AF3Rate = (AF3Rate == "") ? "10" : AF3Rate;
AF4Rate = (AF4Rate == "") ? "10" : AF4Rate;
AF5Rate = (AF5Rate == "") ? "10" : AF5Rate;
AF6Rate = (AF6Rate == "") ? "10" : AF6Rate;

var AF1Ceil = "<% getCfgGeneral(1, "QoSAF1Ceil"); %>";
var AF2Ceil = "<% getCfgGeneral(1, "QoSAF2Ceil"); %>";
var AF3Ceil = "<% getCfgGeneral(1, "QoSAF3Ceil"); %>";
var AF4Ceil = "<% getCfgGeneral(1, "QoSAF4Ceil"); %>";
var AF5Ceil = "<% getCfgGeneral(1, "QoSAF5Ceil"); %>";
var AF6Ceil = "<% getCfgGeneral(1, "QoSAF6Ceil"); %>";
AF1Ceil = (AF1Ceil == "") ? 100 : AF1Ceil;		// default 100%
AF2Ceil = (AF2Ceil == "") ? 100 : AF2Ceil;
AF3Ceil = (AF3Ceil == "") ? 100 : AF3Ceil;
AF4Ceil = (AF4Ceil == "") ? 100 : AF4Ceil;
AF5Ceil = (AF5Ceil == "") ? 100 : AF5Ceil;
AF6Ceil = (AF6Ceil == "") ? 100 : AF6Ceil;

var Rules = "<% getCfgGeneral(1, "QoSRules"); %>";



function modifyAF(index)
{
	var tmp = eval('AF'+index+'Name');
	document.trans.ModifyAFIndex.value = "" + index;
	if(tmp.length){
		document.trans.ModifyAFName.value = eval('AF'+index+'Name');
		document.trans.ModifyAFRate.value = eval('AF'+index+'Rate');
		document.trans.ModifyAFCeil.value = eval('AF'+index+'Ceil');
	}else{
		document.trans.ModifyAFName.value = "";
		document.trans.ModifyAFRate.value = "";
		document.trans.ModifyAFCeil.value = "";
	}
	openwindow = window.open("qosaf.asp", "AF"+index, "toolbar=no, direction=no, location=no, scrollbars=no, resizable=no, width=600, height=200");
	openwindow.focus();
}


function AddRule()
{
	openwindow = window.open("qosclassifier.asp", "Add", "toolbar=no, direction=no, location=no, scrollbars=no, resizable=yes, width=600, height=700");
	openwindow.focus();
}


function initTranslation()
{
	var e;

	e = document.getElementById("QoSTitleStr");
	e.innerHTML = _("qos title");
	e = document.getElementById("QoSIntroStr");
	e.innerHTML = _("qos intro");
	e = document.getElementById("QoSSetupStr");
	e.innerHTML = _("qos setup");
	e = document.getElementById("QoSStr");
	e.innerHTML = _("qos str");
	e = document.getElementById("QoSDisableStr");
	e.innerHTML = _("qos disable");
	e = document.getElementById("QoSEnableStr");
	e.innerHTML = _("qos enable");
/*	e = document.getElementById("QoSDSCPMakerOnlyStr");
	e.innerHTML = _("qos dscpmarkeronly");
*/	e = document.getElementById("QoSUpBWStr");
	e.innerHTML = _("qos upload bandwidth");
	e = document.getElementById("QoSBWCustomStr");
	e.innerHTML = _("qos bandwidth custom");
	e = document.getElementById("QoSBWCustomStr2");
	e.innerHTML = _("qos bandwidth custom");
	e = document.getElementById("QoSDownBWStr");
	e.innerHTML = _("qos download bandwidth");
	e = document.getElementById("QoSSetupSubmitStr");
	e.value = _("qos setup submit");
/*	e = document.getElementById("QoSSetupResetStr");
	e.value = _("qos setup reset");
*/	e = document.getElementById("QoSGroupStr");
	e.innerHTML = _("qos group");
	e = document.getElementById("QoSGroupAttrStr");
	e.innerHTML = _("qos group attr");
	e = document.getElementById("QoSGroupModifyStr5");
	e.value = _("qos group modify");
	e = document.getElementById("QoSGroupModifyStr2");
	e.value = _("qos group modify");
	e = document.getElementById("QoSGroupModifyStr6");
	e.value = _("qos group modify");
	e = document.getElementById("QoSGroupModifyStr1");
	e.value = _("qos group modify");
	e = document.getElementById("QoSRuleNoStr");
	e.innerHTML = _("qos rule no");
	e = document.getElementById("QoSRuleNameStr");
	e.innerHTML = _("qos rule name");
	e = document.getElementById("QoSRulePrioStr");
	e.innerHTML = _("qos rule prio");
	e = document.getElementById("QoSRuleInfoStr");
	e.innerHTML = _("qos rule info");
	e = document.getElementById("QoSRuleAddStr");
	e.value = _("qos rule add");
	e = document.getElementById("QoSRuleDelStr");
	e.value = _("qos rule del");
	e = document.getElementById("QoSLoadProfileStr");
	e.value = _("qos loaddefault");
	/*
		_("qos Rate")
		_("qos Ceil")
	*/
}

function onInit()
{
	initTranslation();

	document.QoSSetup.UploadBandwidth_Custom.disabled = true;
	document.QoSSetup.DownloadBandwidth_Custom.disabled = true;
	document.QoSSetup.UploadBandwidth_Custom.style.display = 'none';
	document.QoSSetup.DownloadBandwidth_Custom.style.display = 'none';

	if(QoS == "0"){
		document.QoSSetup.UploadBandwidth.disabled = true;
		document.QoSSetup.DownloadBandwidth.disabled = true;
		document.getElementById("div_qos_group").style.visibility = "hidden";
		document.getElementById("div_qos_group").style.display = "none";
		document.getElementById("div_qos_rules").style.visibility = "hidden";
		document.getElementById("div_qos_rules").style.display = "none";
		document.getElementById("div_qos_loaddefault").style.visibility = "hidden";
		document.getElementById("div_qos_loaddefault").style.display = "none";

		document.QoSDeleteRules.add_rule.style.display = 'none';
		document.QoSDeleteRules.del_rule.style.display = 'none';
	}else{
		document.QoSSetup.UploadBandwidth.disabled = false;
		document.QoSSetup.DownloadBandwidth.disabled = false;
		document.getElementById("div_qos_group").style.visibility = "visible";
		document.getElementById("div_qos_rules").style.visibility = "visible";
		document.getElementById("div_qos_loaddefault").style.visibility = "visible";
		if (window.ActiveXObject) { // IE
			document.getElementById("div_qos_group").style.display = "block";
			document.getElementById("div_qos_rules").style.display = "block";
			document.getElementById("div_qos_loaddefault").style.display = "block";
		}else if (window.XMLHttpRequest) { // Mozilla, Safari...
			document.getElementById("div_qos_group").style.display = "table";
			document.getElementById("div_qos_rules").style.display = "table";
			document.getElementById("div_qos_loaddefault").style.display = "table";
		}
		document.QoSDeleteRules.add_rule.style.display = '';
		document.QoSDeleteRules.del_rule.style.display = '';

		if(uploadBandwidth == "custom"){
			document.QoSSetup.UploadBandwidth_Custom.disabled = false;
			document.QoSSetup.UploadBandwidth_Custom.style.display = '';
			document.QoSSetup.UploadBandwidth_Custom.value = uploadBandwidth_custom;
		}else{
			for(var i=0; i< document.QoSSetup.UploadBandwidth.length; i++){
				if(document.QoSSetup.UploadBandwidth.options[i].value == uploadBandwidth){
					document.QoSSetup.UploadBandwidth.options.selectedIndex = i;
					break;
				}
			}
		}
		if(downloadBandwidth == "custom"){
			document.QoSSetup.DownloadBandwidth_Custom.disabled = false;
			document.QoSSetup.DownloadBandwidth_Custom.style.display = '';
			document.QoSSetup.DownloadBandwidth_Custom.value = downloadBandwidth_custom;
		}else{
			for(var i=0; i< document.QoSSetup.DownloadBandwidth.length; i++){
				if(document.QoSSetup.DownloadBandwidth.options[i].value == downloadBandwidth){
					document.QoSSetup.DownloadBandwidth.options.selectedIndex = i;
					break;
				}
			}
		}
	}

	if(QoS == "0")
		document.QoSSetup.QoSSelect.options.selectedIndex = 0;
	else if(QoS == "1")
		document.QoSSetup.QoSSelect.options.selectedIndex = 1;
	else if(QoS == "2")
		document.QoSSetup.QoSSelect.options.selectedIndex = 2;
}

function QoSSetupCheck()
{
	if(document.QoSSetup.QoSSelect.options.selectedIndex == 0){
		;  // do nothing
	}else if(	document.QoSSetup.QoSSelect.options.selectedIndex == 1 ||
				document.QoSSetup.QoSSelect.options.selectedIndex == 2){
		if(document.QoSSetup.UploadBandwidth.value == "custom"){
			if(document.QoSSetup.UploadBandwidth_Custom.value == ""){
				alert("Please fill the upload bandwidth.");
				return false;
			}
			if(!checkNum(document.QoSSetup.UploadBandwidth_Custom.value)){
				alert("The upload bandwidth format is wrong. (ex. \"10k\" \"20M\")");
				return false;
			}
				
			var value = getTrueValue(document.QoSSetup.UploadBandwidth_Custom.value);
			if( value  > getTrueValue(QOS_MAX_HARD_LIMITED) ){
				alert("The value of upload bandwidth is too large.(" + QOS_MAX_HARD_LIMITED + ")");
				return false;
			}
			if( value < QOS_MIN_HARD_LIMITED ){
				var ret = confirm("The value of upload bandwidth is too small, are you sure?");
				if(ret == false)
					return false;
			}
		}
		if(document.QoSSetup.DownloadBandwidth.value == "custom"){
			if(document.QoSSetup.DownloadBandwidth_Custom.value == ""){
				alert("Please fill the down bandwidth.");
				return false;
			}
			if(!checkNum(document.QoSSetup.DownloadBandwidth_Custom.value)){
				alert("The download bandwidth format is wrong. (ex. \"10k\" \"20M\")");
				return false;
			}

			var value = getTrueValue(document.QoSSetup.DownloadBandwidth_Custom.value);
			if( value <= 0 ){
				alert("The value of download bandwidth is too small.");
				return false;
			}

		}
	}

	restartPage_block();

	return true;
}

function QoSSelectChange()
{
	if(document.QoSSetup.QoSSelect.options.selectedIndex == 0){
		document.QoSSetup.UploadBandwidth.disabled = true;
		document.QoSSetup.DownloadBandwidth.disabled = true;
		document.QoSSetup.UploadBandwidth_Custom.disabled = true;
		document.QoSSetup.DownloadBandwidth_Custom.disabled = true;
	}else if(	document.QoSSetup.QoSSelect.options.selectedIndex == 1 ||
				document.QoSSetup.QoSSelect.options.selectedIndex == 2){
		document.QoSSetup.UploadBandwidth.disabled = false;
		LoadUploadBW();
		document.QoSSetup.DownloadBandwidth.disabled = false;
		LoadDownloadBW();
	}
}

function LoadUploadBW()
{
	if(document.QoSSetup.UploadBandwidth.options.selectedIndex == 0){
		document.QoSSetup.UploadBandwidth_Custom.value = uploadBandwidth_custom;
		document.QoSSetup.UploadBandwidth_Custom.disabled = false;
		document.QoSSetup.UploadBandwidth_Custom.style.display = '';
	}else{
		document.QoSSetup.UploadBandwidth_Custom.disabled = true;
		document.QoSSetup.UploadBandwidth_Custom.style.display = 'none';
	    if(uploadBandwidth != "custom"){
	  		for(var i=0; i< document.QoSSetup.UploadBandwidth.length; i++){
	  			if(document.QoSSetup.UploadBandwidth.options[i].value == uploadBandwidth){
	  				document.QoSSetup.UploadBandwidth.options.selectedIndex = i;
	  				break;
	  			}
	  		}
	    }
	}
}

function UploadBWChange()
{
	if(document.QoSSetup.UploadBandwidth.options.selectedIndex == 0){
		document.QoSSetup.UploadBandwidth_Custom.value = uploadBandwidth_custom;
		document.QoSSetup.UploadBandwidth_Custom.disabled = false;
		document.QoSSetup.UploadBandwidth_Custom.style.display = '';
	}else{
		document.QoSSetup.UploadBandwidth_Custom.disabled = true;
		document.QoSSetup.UploadBandwidth_Custom.style.display = 'none';
	}
}

function LoadDownloadBW()
{
	if(document.QoSSetup.DownloadBandwidth.options.selectedIndex == 0){
		document.QoSSetup.DownloadBandwidth_Custom.value = downloadBandwidth_custom;
		document.QoSSetup.DownloadBandwidth_Custom.disabled = false;
		document.QoSSetup.DownloadBandwidth_Custom.style.display = '';
	}else{
		document.QoSSetup.DownloadBandwidth_Custom.disabled = true;
		document.QoSSetup.DownloadBandwidth_Custom.style.display = 'none';
		if(downloadBandwidth != "custom"){
			for(var i=0; i< document.QoSSetup.DownloadBandwidth.length; i++){
				if(document.QoSSetup.DownloadBandwidth.options[i].value == downloadBandwidth){
					document.QoSSetup.DownloadBandwidth.options.selectedIndex = i;
					break;
				}
			}
		}
	}
}

function DownloadBWChange()
{
	if(document.QoSSetup.DownloadBandwidth.options.selectedIndex == 0){
		document.QoSSetup.DownloadBandwidth_Custom.value = downloadBandwidth_custom;
		document.QoSSetup.DownloadBandwidth_Custom.disabled = false;
		document.QoSSetup.DownloadBandwidth_Custom.style.display = '';
	}else{
		document.QoSSetup.DownloadBandwidth_Custom.disabled = true;
		document.QoSSetup.DownloadBandwidth_Custom.style.display = 'none';
	}
}

function PrintRules()
{
	if(Rules == "")
		return;
	var a_rule = new Array();

	a_rule = Rules.split(";");
	for(i = 0; i < a_rule.length; i++){
		var j=0;
		var desc = "";
		var entry = new Array();
		entry = a_rule[i].split(",");

		var name = entry[j++];
		var af_index = entry[j++];
		var dp_index = entry[j++];
		var mac_address = entry[j++];
		if(mac_address.length)
			desc += "Mac address: " + mac_address + "<br>";

		var protocol = entry[j++];
		if(protocol.length)
			desc += "Protocol: " + protocol + "<br>";

		var dip = entry[j++];
		if(dip.length)
			desc += "Dst IP: " + dip + "<br>";

		var sip = entry[j++];
		if(sip.length)
			desc += "Src IP: " + sip + "<br>";

		var pkt_len_from = entry[j++];
		var pkt_len_to = entry[j++];
		if(pkt_len_from.length && pkt_len_to.length)
			desc += "Packet Length: " + pkt_len_from + " - " + pkt_len_to + "<br>";

		var dst_port_from = entry[j++];
		var dst_port_to = entry[j++];
		if(dst_port_from.length && dst_port_to.length){
			 desc += "Dst Port Range: " + dst_port_from + " - " + dst_port_to + "<br>";
		}else if (dst_port_from.length){
			 desc += "Dst Port : " + dst_port_from + "<br>";
		}

		var src_port_from  = entry[j++];
		var src_port_to  = entry[j++];
		if(src_port_from.length && src_port_to.length){
			 desc += "Src Port Range: " + src_port_from + " - " + src_port_to + "<br>";
		}else if (src_port_from.length){
			 desc += "Src Port : " + src_port_from + "<br>";
		}

		var layer7 = entry[j++];
		if(layer7.length)
			desc += "Application: " + layer7 + "<br>";

		var ingress_if  = entry[j++];
		if(ingress_if.length)
			desc += "Ingress Interface: " + ingress_if + "<br>";

		var dscp = entry[j++];
		if(dscp.length)
			desc += "DSCP: " + dscp + "<br>";

		var remarker  = entry[j++];
		if(remarker.length)
			desc += "Remark DSCP :" + remarker + "<br>";

		document.write("<tr><td>"+ (i+1) +"<input type=checkbox name=del_qos_" + i + "> </td> <td>" + name + "</td> <td>" + eval('AF'+af_index+'Name') + "</td>  <td>" + desc + "</td>  </tr>");
	}
}


function getTrueValue(str)
{
	var rc;
	rc = parseInt(str);
	if(str.charAt(str.length-1) == 'k' || str.charAt(str.length-1) == 'K')
		rc = rc * 1024;
	else if(str.charAt(str.length-1) == 'm' || str.charAt(str.length-1) == 'M')
		rc = rc * 1024 * 1024;
	return rc;
}

function checkNum(str)
{
	if(str.length < 2)
		return false;
	for(i=0; i<str.length-1; i++){
		if( str.charAt(i) >= '0' && str.charAt(i) <= '9')
			continue;
		return false;
	}
	if( str.charAt(str.length-1) == 'k' || str.charAt(str.length-1) == 'K' ||
		str.charAt(str.length-1) == 'm' || str.charAt(str.length-1) == 'M' || 
		(str.charAt(str.length-1) >= '0' && str.charAt(str.length-1) <= '9') )
		return true;
	return false;
}

</script>
</head>
<!--     body      -->
<body onload="onInit()">

<div align="center">
 <center>
 
<table class="body"><tbody><tr><td>

<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="QoSTitleStr">Quality of Service Settings </td>
</tr>
<tr>

<tr>
<td colspan="2">
<p class="head" id="QoSIntroStr"> You may setup rules to provide Quality of Service guarantees for specific applications.</p>
</td>
<tr>
</table>
<br>

<form method=post name="trans" action="">
<input name=ModifyAFIndex type=hidden value="">
<input name=ModifyDPIndex type=hidden value="">
<input name=ModifyAFName type=hidden value="">
<input name=ModifyAFRate type=hidden value="">
<input name=ModifyAFCeil type=hidden value="">
</form>


<form method="post" name="QoSSetup" action="/goform/QoSSetup">
<table border=1 bordercolor=#9BABBD width="540">
<tr>
	<td class="title" colspan="2" id="QoSSetupStr">QoS Setup </td>
</tr>
<tr>
	<td class="head" id="QoSStr">
		Quality of Service
	</td>

	<td>
	<select name="QoSSelect" size="1" onChange="QoSSelectChange()">
	<option value=0 id="QoSDisableStr">Disable</option>
	<option value=1 id="QoSEnableStr">Enable</option>
<!--
	<option value=2 id="QoSDSCPMakerOnlyStr">DSCP mark only</option>
-->
	</select>
	</td>
</tr>
<tr>
	<td class="head" id="QoSUpBWStr">
		Upload Bandwidth:
	</td>
	<td>
		<select name="UploadBandwidth" id="UploadBandwidth" size="1" onChange="UploadBWChange()">
		<option value="custom" id="QoSBWCustomStr">User defined</option>
		<option value="64k">64k</option>
		<option value="96k">96k</option>
		<option value="128k">128k</option>
		<option value="192k">192k</option>
		<option value="256k">256k</option>
		<option value="384k" >384k</option>
		<option value="512k">512k</option>
		<option value="768k">768k</option>
		<option value="1M">1M</option>
		<option value="2M">2M</option>
		<option value="4M">4M</option>
		<option value="8M">8M</option>
		<option value="10M">10M</option>
		<option value="12M">12M</option>
		<option value="16M">16M</option>
		<option value="20M">20M</option>
		<option value="24M">24M</option>
		<option value="32M">32M</option>
		<option value="60M">60M</option>
		</select>

		<input type=text name=UploadBandwidth_Custom size=6 maxlength=8 style="display:none"> Bits/sec
	</td>
</tr>
<tr>
	<td class="head" id="QoSDownBWStr">
		Download Bandwidth:
	</td>
	<td>
		<select name="DownloadBandwidth" id="DownloadBandwidth" size="1" onChange="DownloadBWChange()">
		<option value="custom" id="QoSBWCustomStr2">User defined</option>
		<option value="64k">64k</option>
		<option value="96k">96k</option>
		<option value="128k">128k</option>
		<option value="192k">192k</option>
		<option value="256k">256k</option>
		<option value="384k" >384k</option>
		<option value="512k">512k</option>
		<option value="768k">768k</option>
		<option value="1M">1M</option>
		<option value="2M">2M</option>
		<option value="4M">4M</option>
		<option value="8M">8M</option>
		<option value="10M">10M</option>
		<option value="12M">12M</option>
		<option value="16M">16M</option>
		<option value="20M">20M</option>
		<option value="24M">24M</option>
		<option value="32M">32M</option>
		<option value="60M">60M</option>
		</select>

		<input type=text name=DownloadBandwidth_Custom size=6 maxlength=8 style="display:none"> Bits/sec
	</td>
</tr>

</table>
<input value="Apply" id="QoSSetupSubmitStr" name="QoSSetupSubmitStr" onclick="return QoSSetupCheck()" type="submit"> &nbsp;&nbsp;
<!--
<input value="Reset" id="QoSSetupResetStr" name="QoSSetupResetStr" type="reset">
-->
</form>

<br>
<table id="div_qos_group" border=1 bordercolor=#9BABBD width="540" style="visibility: hidden;">
<tr>
	<td class="title" id="QoSGroupStr"> Group </td>
	<td class="title" id="QoSGroupAttrStr"> Attribute</td>
</tr>

<!-- EF / AF5 -->
<tr>
	<td class="head">
		<script  language="JavaScript" type="text/javascript">
			document.write(AF5Name);
		</script>
	&nbsp;</td>
	<td>
		<script  language="JavaScript" type="text/javascript">
			document.write(_("qos Rate") + AF5Rate + "% ");
		</script>
		<input id="QoSGroupModifyStr5" name="QoSGroupModifyStr5" value="modify" onclick="modifyAF(5)" type="button"><br>
		<script language="JavaScript" type="text/javascript">
			document.write(_("qos Ceil") + AF5Ceil + "% ");
		</script>
	</td>
</tr>

<!-- AF2 -->
<tr>
	<td class="head">
		<script language="JavaScript" type="text/javascript">
			document.write(AF2Name);
		</script>
	&nbsp;</td>
	<td>
		<script language="JavaScript" type="text/javascript">
			document.write(_("qos Rate") + AF2Rate + "% ");
		</script>
		<input id="QoSGroupModifyStr2" name="QoSGroupModifyStr2" value="modify" onclick="modifyAF(2)" type="button"><br>
		<script language="JavaScript" type="text/javascript">
			document.write(_("qos Ceil") + AF2Ceil + "% ");
		</script>
	</td>
</tr>

<!-- Default/AF6 -->
<tr>
	<td class="head">
		<script language="JavaScript" type="text/javascript">
			document.write(AF6Name);
		</script>
	&nbsp;</td>
	<td>
		<script language="JavaScript" type="text/javascript">
			document.write(_("qos Rate") + AF6Rate + "% ");
		</script>
		<input id="QoSGroupModifyStr6" name="QoSGroupModifyStr6" value="modify" onclick="modifyAF(6)" type="button"><br>
		<script language="JavaScript" type="text/javascript">
			document.write(_("qos Ceil") + AF6Ceil + "% ");
		</script>
	</td>
</tr>

<!-- Default/AF1 -->
<tr>
	<td class="head">
		<script language="JavaScript" type="text/javascript">
			document.write(AF1Name);
		</script>
	&nbsp;</td>
	<td>
		<script language="JavaScript" type="text/javascript">
			document.write(_("qos Rate") + AF1Rate + "% ");
		</script>
		<input id="QoSGroupModifyStr1" name="QoSGroupModifyStr1" value="modify" onclick="modifyAF(1)" type="button"><br>
		<script language="JavaScript" type="text/javascript">
			document.write(_("qos Ceil") + AF1Ceil + "% ");
		</script>
	</td>
</tr>
</table>

<br>

<form method="post" name="QoSDeleteRules" action="/goform/QoSDeleteRules">
<table id="div_qos_rules" name="div_qos_rules" border=1 bordercolor=#9BABBD width="600" style="visibility: hidden;">
<tr>
	<td class="title" id="QoSRuleNoStr">  No. </td>
	<td class="title" id="QoSRuleNameStr">  Name. </td>
	<td class="title" id="QoSRulePrioStr">  Prio. </td>
	<td class="title" id="QoSRuleInfoStr">  Info. </td>
</tr>

<script language="JavaScript" type="text/javascript">
	PrintRules();
</script>
</table>
<input type="button" id="QoSRuleAddStr" name="add_rule" value=add onClick="AddRule();">
<input type="submit" id="QoSRuleDelStr" name="del_rule" value=delete>
</form>

<table id="div_qos_loaddefault">
<form method="post" name="QoSLoadDefault" action="/goform/QoSLoadDefaultProfile">
<tr><td>
	<input type="submit" name="QoSLoadProfileStr" id="QoSLoadProfileStr" value="Load Default" onclick="restartPage_block();">
</td></tr>
</form>
</table>

</td></tr></tbody></table>

 </center>
</div>

</body></html>