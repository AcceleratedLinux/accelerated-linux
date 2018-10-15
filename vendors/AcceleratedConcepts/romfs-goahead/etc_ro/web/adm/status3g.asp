<html>
<head>
<title>Access Point Status</title>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("admin");
//20090505 Gord# add for testing saving bar
var counter = 20;

var secs;
var timerID = null;
var timerRunning = false;

function StartTheTimer(){
	if (secs==0){
		TimeoutReload(5);
		//window.location.reload();
		window.location.href=window.location.href;	//reload page
    }else{
        self.status = secs
        secs = secs - 1
        timerRunning = true
        timerID = self.setTimeout("StartTheTimer()", 1000)
    }
}

function TimeoutReload(timeout)
{
	secs = timeout;
	if(timerRunning)
		clearTimeout(timerID)
	timerRunning = false
	StartTheTimer();	
}

function startProgress(){
	document.getElementById("progressouter").style.display="block";
	savingbar();
}

function savingbar(){
	if (counter < 101){
   	document.getElementById("progressinner").style.width = counter+"%";
     	counter++;

		if(counter == 101)
			counter = 20;
     	setTimeout("savingbar()",200);
   }
}

function style_display_on()
{
	if (window.ActiveXObject) { // IE
		return "block";
	}
	else if (window.XMLHttpRequest) { // Mozilla, Safari,...
		return "table-row";
	}
}

function id_invisible(id)
{
var E=document.getElementById(id);
if( E !=null) {
	document.getElementById(id).style.visibility = "hidden";
	document.getElementById(id).style.display = "none";
}
else {
  alert("Error:id_invisible:id="+id);//Use for Debug
  }      
}

function id_visible(id)
{
var E=document.getElementById(id);
if( E !=null) {
	document.getElementById(id).style.visibility = "visible";
	document.getElementById(id).style.display = style_display_on();
}
else {
  alert("Error:id_visible:id="+id);//Use for Debug
  }  
}

function initTranslation()
{
<!--	var BC_data_start = '<% getWan3gData("Start"); %>'; -->
	var e = document.getElementById("statusTitle"); 
	e.innerHTML = _("status 3g title");
	e = document.getElementById("statusIntroduction");
	e.innerHTML = _("status 3g introduction");
	e = document.getElementById("statusInternetConfig");
	e.innerHTML = _("status 3g internet config");
/*
	e = document.getElementById("g3gconnectButton");
	e.value = _("3G Manually Start");
	e = document.getElementById("g3gdisconnectButton");
	e.value = _("3G Manually Stop");
	e = document.getElementById("manual_mode_only");
	e.value = _("manual mode only");

	e = document.getElementById("wan3gCelluarMfr");
	e.innerHTML = _("status 3g modem identification");
	e = document.getElementById("wan3gSigStrength");
	e.innerHTML = _("status 3g Signal Strength");
*/
	e = document.getElementById("id_ConnectionTime");
	e.innerHTML = _("status 3g Connection Time");
	e = document.getElementById("id_ElapsedTime");
	e.innerHTML = _("status 3g Elapsed Time");
	e = document.getElementById("id_Transfer");
	e.innerHTML = _("status 3g Total Transfer");

	e = document.getElementById("Access3GStatistics");
	e.innerHTML = _("status 3g Access3GStatistics");

	e = document.getElementById("updateBudgetButton");
	e.value = _("3G Update");
	e = document.getElementById("ForcedResetBudgetButton");
	e.value = _("Forced Reset Budget");

	var BC_enable_value   = '<% getCfgGeneral(1, "wan_3g_budget_control_enable"); %>';
	var wan2   = '<% getCfgGeneral(1, "wan2"); %>';

//	if ( (1*var_3g_start_mode) == 0  )  // Automatically
//		id_invisible("manuallySet");

	e = document.getElementById("BC_enable");
	e.innerHTML = _("status 3g BC Enable");

	e = document.getElementById("TBControl");
	e.innerHTML = _("status 3g TBC");

	e = document.getElementById("TBControl");
	e.innerHTML = _("status 3g TBC");
  
	e = document.getElementById("MaxTimeBudget");
	e.innerHTML = _("status 3g MaxTimeBudget");

	e = document.getElementById("DataBudgetControl");
	e.innerHTML = _("status 3g DataBudgetControl");
	e = document.getElementById("MaxDataBudget");
	e.innerHTML = _("status 3g MaxDataBudget");
	e = document.getElementById("DataBudgetTrafficflow");
	e.innerHTML = _("status 3g DataBudgetTrafficflow");

	e = document.getElementById("DropConnectionOverBT");
	e.innerHTML = _("status 3g DropConnectionOverBT");
	e = document.getElementById("DisallowOverBT");
	e.innerHTML = _("status 3g DisallowOverBT");

	e = document.getElementById("BillingStartingDate");
	e.innerHTML = _("status 3g BillingStartingDate");

	e = document.getElementById("DataBudgetStatus");
	e.innerHTML = _("status 3g DataBudgetStatus");	

	e = document.getElementById("TimeBudgetStatus");
	e.innerHTML = _("status 3g TimeBudgetStatus");	

	e = document.getElementById("EmailAlert");
	e.innerHTML = _("status 3g EmailAlert");	

	e = document.getElementById("wan3gRemainBudgetBytes");
	e.innerHTML = _("status 3g Remaining Budget Bytes");
	e = document.getElementById("wan3gRemainBudgetTime");
	e.innerHTML = _("status 3g Remaining Budget Time");

	if(BC_enable_value== "1" && wan2=="G3G"){
		e = document.getElementById("progressinner");
		e.innerHTML = _("admin uploading");
		document.getElementById("BC_enable_value").value = _("admin enable");
		var TBC_enable_value		= '<% getCfgGeneral(1, "wan_3g_time_budget_enable"); %>';
		var DBC_enable_value		= '<% getCfgGeneral(1, "wan_3g_data_budget_enable"); %>';
		//var get_msg;

		var DropCon_enable_value	= '<% getCfgGeneral(1, "wan_3g_policy_drop"); %>';
		var DisallowCon_enable_value	= '<% getCfgGeneral(1, "wan_3g_policy_disallow"); %>';
		var cycledate			= '<% getWan_3g_cycle_date0(); %>' + 1;
		var wan_3g_policy_alert	= '<% getCfgGeneral(1, "wan_3g_policy_alert"); %>';
	
		if(TBC_enable_value== "1"){
			document.getElementById("TBC_enable_value").value = _("admin enable");
			var Max_TBC_value		= "<% getWan_3g_max_online_monthly();%> "+ _("status 3g hours") + "/" + "(" + "<% getWan_3g_time_limit();%>" +"%" +")";
			document.getElementById("Max_TBC_value").value = "<% getWan3gData("BCTMax"); %>";;
			document.getElementById("wan3gRemainBudgetTime_value").value = '<% getWan3gData("BCTRemain"); %>';
			document.getElementById("Wan3gTimeBudgetStatus_value").value = '<% getWan3gData("BCTStatus"); %>';
			//get_msg = BCTStatus;
			//e = document.getElementById("Wan3gTimeBudgetStatus");
			//if ( get_msg == "REACH Pre-Limit" )
			//	e.innerHTML =  _("status 3g Reach Pre Limit");
			//else if ( get_msg == "Pre-Limit" )
			//	e.innerHTML =  _("status 3g Reach Pre Limit OK");
			//else if( get_msg == "OVER Budget" )
			//	e.innerHTML =  _("status 3g Over Budget");
			//else if( get_msg == "Budget OK" )
			//	e.innerHTML =  _("status 3g Budget OK");
			//else if( get_msg == "Over Budget" )
			//	e.innerHTML =  _("status 3g Over Budget");
			//e.disabled = true;
		}else{
			document.getElementById("TBC_enable_value").value = _("admin disable");
			id_invisible("MTBudget_col");
			id_invisible("TimeBudgetStatus_col");
			id_invisible("wan3gRemainBudgetTime_col");
		}
		if(DBC_enable_value== "1"){
			document.getElementById("DBC_enable_value").value = _("admin enable");		
			e = document.getElementById("MaxDataBudget");
			e.innerHTML = _("status 3g MaxDataBudget");
			e = document.getElementById("DataBudgetTrafficflow");
			e.innerHTML = _("status 3g DataBudgetTrafficflow");
			e = document.getElementById("DataBudgetStatus");
			e.innerHTML = _("status 3g DataBudgetStatus");
			var updownst			= <% getWan_3g_up_down_stream_limit0(); %>;	
			var Max_DBC_value		= "<%getWan_3g_max_xfer_monthly();%> "+ _("status 3g MB") + "/" + "(" + "<% getWan_3g_data_limit();%>" +"%" +")";
			document.getElementById("Max_DBC_value").value = "<% getWan3gData("BCDMax"); %>";
			document.getElementById("wan3gRemainBudgetBytes_value").value = '<% getWan3gData("BCDRemain"); %>';
			document.getElementById("Wan3gDataBudgetStatus_value").value = '<% getWan3gData("BCDStatus"); %>';
			//get_msg = BCDStatus;
			//e = document.getElementById("Wan3gDataBudgetStatus");
			//if ( get_msg == "REACH Pre-Limit" )
			//	e.innerHTML =  _("status 3g Reach Pre Limit");
			//else if ( get_msg == "Pre-Limit" )
			//	e.innerHTML =  _("status 3g Reach Pre Limit OK");
			//else if( get_msg == "OVER Budget" )
			//	e.innerHTML =  _("status 3g Over Budget");
			//else if( get_msg == "Budget OK" )
			//	e.innerHTML =  _("status 3g Budget OK");
			//else if( get_msg == "Over Budget" )
			//	e.innerHTML =  _("status 3g Over Budget");
			//e.disabled = true;
		}else{
			document.getElementById("DBC_enable_value").value = _("admin disable");
			//id_invisible("DBControl_col");
			id_invisible("MDBudget_col");
			id_invisible("DBudgetT_col");
			id_invisible("DataBudgetStatus_col");
			id_invisible("wan3gRemainBudgetBytes_col");
		}

		if(updownst== "0")
			document.getElementById("DBTF_value").value = _("status 3g download");
		else if (updownst== "1")
			document.getElementById("DBTF_value").value = _("status 3g upload");
		else	
			document.getElementById("DBTF_value").value = _("status 3g up_down");


		if(DropCon_enable_value== "1")
			document.getElementById("DropCon_enable_value").value = _("admin enable");
		else
			document.getElementById("DropCon_enable_value").value = _("admin disable");

		if(DisallowCon_enable_value== "1")
			document.getElementById("DisAllowCon_enable_value").value = _("admin enable");
		else
			document.getElementById("DisAllowCon_enable_value").value = _("admin disable");	

		if(cycledate == 1)
			document.getElementById("Bill_Starting_value").value = <% getWan_3g_cycle_date0(); %> + 1  + "-st" + " Day Per Month" ;
		else if (cycledate == 2)	
			document.getElementById("Bill_Starting_value").value = <% getWan_3g_cycle_date0(); %> + 1  + "-nd" + " Day Per Month" ;		
		else if (cycledate == 3)			
			document.getElementById("Bill_Starting_value").value = <% getWan_3g_cycle_date0(); %> + 1  + "-rd" + " Day Per Month" ;	
        	else
			document.getElementById("Bill_Starting_value").value =  <% getWan_3g_cycle_date0(); %> + 1 + "-th " + " Day Per Month" ;
/**/
/* --JCYU, Does the following code work with multi-lang ?
		var cycledateTxt = "dayid "+ cycledate;
		document.getElementById("Bill_Starting_value").value =  _(cycledateTxt);

*/
/**/
		if(wan_3g_policy_alert== "1")
			document.getElementById("Email_alert_value").value = _("admin enable");
		else
			document.getElementById("Email_alert_value").value = _("admin disable");
/**/
	}
	else{
		document.getElementById("BC_enable_value").value = _("admin disable");
		id_invisible("TBControl_col");
		id_invisible("MTBudget_col");
		id_invisible("DBControl_col");
		id_invisible("DBudgetT_col");
		id_invisible("MDBudget_col");
		id_invisible("DropConnectionOverBT_col");
		id_invisible("DisallowOverBT_col");
		id_invisible("BillingStartingDate_col");
		id_invisible("DataBudgetStatus_col");
		id_invisible("TimeBudgetStatus_col");
		id_invisible("EmailAlert_col");
		id_invisible("wan3gRemainBudgetBytes_col");
		id_invisible("wan3gRemainBudgetTime_col");
		id_invisible("Access3GStatistics_col");
		id_invisible("3GStatisticsTables");
		//id_invisible("manuallyBudgetSet");
		id_invisible("updateBudgetButton");
		id_invisible("ForcedResetBudgetButton");
	}
}

function PageInit()
{
	initTranslation();
}

function reflash()
{
	startProgress(); //20090505 Gord# add to test saving bar
}

function updateBudgetSubmit()
{
			document.getElementById("updateBudget").value = "UpdateBC";
			document.getElementById("ForcedResetBudget").value = "";
			document.getElementById("ID_3GManuallyForm").submit();
}

function resetBudgetSubmit()
{
			document.getElementById("updateBudget").value = "";
			document.getElementById("ForcedResetBudget").value = "ForcedResetBC";
			document.getElementById("ID_3GManuallyForm").submit();
}
</script>
</head>

<body onLoad="PageInit()">
<table  class="body"><tbody><tr><td>
<H1 id="statusTitle">3G Budget Status</H1>
<P id="statusIntroduction">This section displays various status information of the device.</P><hr />

<table width="95%" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="statusInternetConfig">Budget Control Parameters</td>
</tr>

<tr>
  <td class="head" id="BC_enable">Budget Control enable</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="BC_enable_value"></td>
</tr>
<tr id="TBControl_col">
  <td class="head" id="TBControl">Time Budget Control</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="TBC_enable_value"></td>
</tr>
<tr id="MTBudget_col">
  <td class="head" id="MaxTimeBudget">Max Time Budget</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="Max_TBC_value"></td>
</tr>
<tr id="DBControl_col">
  <td class="head" id="DataBudgetControl">Data Budget Control</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="DBC_enable_value"></td>
</tr>
<tr id="DBudgetT_col">
  <td class="head" id="DataBudgetTrafficflow">Data Budget Traffic flow</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="DBTF_value"></td>
</tr>
<tr id="MDBudget_col">
  <td class="head" id="MaxDataBudget">Max Data Budget</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="Max_DBC_value"></td>
</tr>
<tr id="DropConnectionOverBT_col">
  <td class="head" id="DropConnectionOverBT">Drop Current Connection When Over Budget</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="DropCon_enable_value"></td>
</tr>
<tr id="DisallowOverBT_col">
  <td class="head" id="DisallowOverBT">Disallow New Connection When Over Budget</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="DisAllowCon_enable_value"></td>
</tr>
<tr id="BillingStartingDate_col">
  <td class="head" id="BillingStartingDate">Billing Starting Date</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="Bill_Starting_value"></td>
</tr>
<!-- -->
<tr id="DataBudgetStatus_col">
  <td class="head" id="DataBudgetStatus">Data Budget Status</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="Wan3gDataBudgetStatus_value"></td>
</tr>
<tr id="TimeBudgetStatus_col">
  <td class="head" id="TimeBudgetStatus">Time Budget Status</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="Wan3gTimeBudgetStatus_value"></td>
</tr>

<!-- <tr id="EmailAlert"_col>  -->
<tr id="EmailAlert_col">
  <td class="head" id="EmailAlert">Email Alert</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="Email_alert_value"></td>
</tr>
<!--
<tr id="wan3gCelluarMfr_col">
  <td class="head" id="wan3gCelluarMfr">3G modem identification</td>
  <td><input size="32" type="text" readOnly="true" disable="false"></td>
</tr>
<tr id="wan3gSigStrength_col">
  <td class="head" id="wan3gSigStrength">Signal Strength</td>
  <td><input size="32" type="text" readOnly="true" disable="false"></td>
</tr>
-->
<!-- -->
<tr id="wan3gRemainBudgetBytes_col">
  <td class="head" id="wan3gRemainBudgetBytes">Remaining Budget Bytes</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="wan3gRemainBudgetBytes_value"></td>
</tr>
<tr id="wan3gRemainBudgetTime_col">
  <td class="head" id="wan3gRemainBudgetTime">Remaining Budget Time</td>
  <td><input size="32" type="text" readOnly="true" disable="false" id="wan3gRemainBudgetTime_value"></td>
</tr>
</table>
<table width="95%" border="1" cellpadding="2" cellspacing="1", id="3GStatisticsTables">
<tr id="Access3GStatistics_col">
  <td class="title" colspan="5" id="Access3GStatistics">3G Access Statistics</td>
</tr>
<tr>
<!--td width="30%" class="opt_field">Login(Date Time)</td -->
<!-- |connection | summated elapsed time | xfer data | Signal Strength| -->
<td class="title" id="id_ConnectionTime">Connection time(minutes)</td>
<td class="title" id="id_ElapsedTime">Summated Elapsed time(minutes)</td>
<td class="title" id="id_Transfer">Total Transfer(MB)</td>
<td class="title" id="id_RCV">RCV(MB)</td>
<td class="title" id="id_TX">TX(MB)</td>
<!--<td class="opt_field">RSSI</td>-->
</tr>
<tr>
<!--td width="30%" class="opt_field">Login(Date Time)</td -->
<!-- |connection | summated elapsed time | xfer data | Signal Strength| -->
<td ><% getWan3g_this_elapse(); %></td>
<td ><% getWan3g_total_sum_elapsed_time(); %></td>
<td ><% getWan3g_sum_3g_data_xfer(); %></td>
<td ><% getWan3g_RX_payload(); %></td>
<td ><% getWan3g_TX_payload(); %></td>
<!--<td class="opt_field">RSSI</td>-->
</tr>
</table>
</td></tr>
<tr></tr>
<tr id="manuallyBudgetSet">
<td align="center" colspan="2" id="row_3g_buttons" class="data_invisible">
<form method="post" name="Manually3GForm" id="ID_3GManuallyForm" action="/goform/Wan3UpdateForcedReset">
  <input type=hidden value="" name="Update" id="updateBudget" >
  <input type="button" value="Update" name="UpdateButton" id="updateBudgetButton" onClick="updateBudgetSubmit();">
  <input type=hidden value="" name="ForcedResetBudget" id="ForcedResetBudget">
  <input type="button" value="Forced Reset Budget" name="ForcedResetBudgetButton" id="ForcedResetBudgetButton" onClick="resetBudgetSubmit();">
</form>
</td></tr>
<!-- 
  <tr id="manuallySet">
<form method="post" name="3GManuallyForm" action="/goform/Wan3gmstop_start">
<td align="center" colspan="2" id="row_3g_buttons" class="data_invisible">
  <input type="submit" value="Manually Start 3G" name="3gconnect" id="g3gconnectButton">
  <input type="submit" value="Manually Stop 3G" name="3gdisconnect" id="g3gdisconnectButton">&nbsp;
	<font id="manual_mode_only">manual mode only</font></td>
</form>
</tr>
-->
<!-- 20090505 Gord Add Saving Bar-->
<!-- -->
<div id="progressouter" style="width: 500px; height: 20px; border: 6px; display:none;">
   <div id="progressinner" style="position: relative; height: 20px; background-color: #2C5EA4; 
   width: 20%; font-size:15px; font-weight:bolder; color:#FFFFF0; ">Uploading Now...</div>
</div>
<!-- -->
<!-- Gord End -->
</tbody></table>
</body>
</html>
