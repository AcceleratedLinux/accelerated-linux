<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>Wireless WMM Settings</title>

<script language="JavaScript" type="text/javascript">
function CheckValue()
{
   var APAifsn_s;
   var APCwmin_s;
   var APCwmax_s;
   var APTxop_s;
   var APACM_s;
   var BSSAifsn_s;
   var BSSCwmin_s;
   var BSSCwmax_s;
   var BSSTxop_s;
   var BSSACM_s;
   var AckPolicy_s;

   APAifsn_s = '';
   APAifsn_s = APAifsn_s + document.wmm.ap_aifsn_acbe.value;
   APAifsn_s = APAifsn_s + ';';
   APAifsn_s = APAifsn_s + document.wmm.ap_aifsn_acbk.value;
   APAifsn_s = APAifsn_s + ';';
   APAifsn_s = APAifsn_s + document.wmm.ap_aifsn_acvi.value;
   APAifsn_s = APAifsn_s + ';';
   APAifsn_s = APAifsn_s + document.wmm.ap_aifsn_acvo.value;
   document.wmm.ap_aifsn_all.value = APAifsn_s;
   document.wmm.ap_aifsn_acbe.disabled = true;
   document.wmm.ap_aifsn_acbk.disabled = true;
   document.wmm.ap_aifsn_acvi.disabled = true;
   document.wmm.ap_aifsn_acvo.disabled = true;

   APCwmin_s = '';
   APCwmin_s = APCwmin_s + document.wmm.ap_cwmin_acbe.value;
   APCwmin_s = APCwmin_s + ';';
   APCwmin_s = APCwmin_s + document.wmm.ap_cwmin_acbk.value;
   APCwmin_s = APCwmin_s + ';';
   APCwmin_s = APCwmin_s + document.wmm.ap_cwmin_acvi.value;
   APCwmin_s = APCwmin_s + ';';
   APCwmin_s = APCwmin_s + document.wmm.ap_cwmin_acvo.value;
   document.wmm.ap_cwmin_all.value = APCwmin_s;
   document.wmm.ap_cwmin_acbe.disabled = true;
   document.wmm.ap_cwmin_acbk.disabled = true;
   document.wmm.ap_cwmin_acvi.disabled = true;
   document.wmm.ap_cwmin_acvo.disabled = true;

   APCwmax_s = '';
   APCwmax_s = APCwmax_s + document.wmm.ap_cwmax_acbe.value;
   APCwmax_s = APCwmax_s + ';';
   APCwmax_s = APCwmax_s + document.wmm.ap_cwmax_acbk.value;
   APCwmax_s = APCwmax_s + ';';
   APCwmax_s = APCwmax_s + document.wmm.ap_cwmax_acvi.value;
   APCwmax_s = APCwmax_s + ';';
   APCwmax_s = APCwmax_s + document.wmm.ap_cwmax_acvo.value;
   document.wmm.ap_cwmax_all.value = APCwmax_s;
   document.wmm.ap_cwmax_acbe.disabled = true;
   document.wmm.ap_cwmax_acbk.disabled = true;
   document.wmm.ap_cwmax_acvi.disabled = true;
   document.wmm.ap_cwmax_acvo.disabled = true;

   APTxop_s = '';
   APTxop_s = APTxop_s + document.wmm.ap_txop_acbe.value;
   APTxop_s = APTxop_s + ';';
   APTxop_s = APTxop_s + document.wmm.ap_txop_acbk.value;
   APTxop_s = APTxop_s + ';';
   APTxop_s = APTxop_s + document.wmm.ap_txop_acvi.value;
   APTxop_s = APTxop_s + ';';
   APTxop_s = APTxop_s + document.wmm.ap_txop_acvo.value;
   document.wmm.ap_txop_all.value = APTxop_s;
   document.wmm.ap_txop_acbe.disabled = true;
   document.wmm.ap_txop_acbk.disabled = true;
   document.wmm.ap_txop_acvi.disabled = true;
   document.wmm.ap_txop_acvo.disabled = true;

   APACM_s = '';
   if (document.wmm.ap_acm_acbe.checked)
      APACM_s = APACM_s + '1';
   else
      APACM_s = APACM_s + '0';
   APACM_s = APACM_s + ';';
   if (document.wmm.ap_acm_acbk.checked)
      APACM_s = APACM_s + '1';
   else
      APACM_s = APACM_s + '0';
   APACM_s = APACM_s + ';';
   if (document.wmm.ap_acm_acvi.checked)
      APACM_s = APACM_s + '1';
   else
      APACM_s = APACM_s + '0';
   APACM_s = APACM_s + ';';
   if (document.wmm.ap_acm_acvo.checked)
      APACM_s = APACM_s + '1';
   else
      APACM_s = APACM_s + '0';
   document.wmm.ap_acm_acbe.disabled = true;
   document.wmm.ap_acm_acbk.disabled = true;
   document.wmm.ap_acm_acvi.disabled = true;
   document.wmm.ap_acm_acvo.disabled = true;
   document.wmm.ap_acm_all.value = APACM_s;

   AckPolicy_s = '';
   if (document.wmm.ap_ackpolicy_acbe.checked)
      AckPolicy_s = AckPolicy_s + '1';
   else
      AckPolicy_s = AckPolicy_s + '0';
   AckPolicy_s = AckPolicy_s + ';';
   if (document.wmm.ap_ackpolicy_acbk.checked)
      AckPolicy_s = AckPolicy_s + '1';
   else
      AckPolicy_s = AckPolicy_s + '0';
   AckPolicy_s = AckPolicy_s + ';';
   if (document.wmm.ap_ackpolicy_acvi.checked)
      AckPolicy_s = AckPolicy_s + '1';
   else
      AckPolicy_s = AckPolicy_s + '0';
   AckPolicy_s = AckPolicy_s + ';';
   if (document.wmm.ap_ackpolicy_acvo.checked)
      AckPolicy_s = AckPolicy_s + '1';
   else
      AckPolicy_s = AckPolicy_s + '0';
   document.wmm.ap_ackpolicy_acbe.disabled = true;
   document.wmm.ap_ackpolicy_acbk.disabled = true;
   document.wmm.ap_ackpolicy_acvi.disabled = true;
   document.wmm.ap_ackpolicy_acvo.disabled = true;
   document.wmm.ap_ackpolicy_all.value = AckPolicy_s;

   BSSAifsn_s = '';
   BSSAifsn_s = BSSAifsn_s + document.wmm.sta_aifsn_acbe.value;
   BSSAifsn_s = BSSAifsn_s + ';';
   BSSAifsn_s = BSSAifsn_s + document.wmm.sta_aifsn_acbk.value;
   BSSAifsn_s = BSSAifsn_s + ';';
   BSSAifsn_s = BSSAifsn_s + document.wmm.sta_aifsn_acvi.value;
   BSSAifsn_s = BSSAifsn_s + ';';
   BSSAifsn_s = BSSAifsn_s + document.wmm.sta_aifsn_acvo.value;
   document.wmm.sta_aifsn_all.value = BSSAifsn_s;
   document.wmm.sta_aifsn_acbe.disabled = true;
   document.wmm.sta_aifsn_acbk.disabled = true;
   document.wmm.sta_aifsn_acvi.disabled = true;
   document.wmm.sta_aifsn_acvo.disabled = true;

   BSSCwmin_s = '';
   BSSCwmin_s = BSSCwmin_s + document.wmm.sta_cwmin_acbe.value;
   BSSCwmin_s = BSSCwmin_s + ';';
   BSSCwmin_s = BSSCwmin_s + document.wmm.sta_cwmin_acbk.value;
   BSSCwmin_s = BSSCwmin_s + ';';
   BSSCwmin_s = BSSCwmin_s + document.wmm.sta_cwmin_acvi.value;
   BSSCwmin_s = BSSCwmin_s + ';';
   BSSCwmin_s = BSSCwmin_s + document.wmm.sta_cwmin_acvo.value;
   document.wmm.sta_cwmin_all.value = BSSCwmin_s;
   document.wmm.sta_cwmin_acbe.disabled = true;
   document.wmm.sta_cwmin_acbk.disabled = true;
   document.wmm.sta_cwmin_acvi.disabled = true;
   document.wmm.sta_cwmin_acvo.disabled = true;

   BSSCwmax_s = '';
   BSSCwmax_s = BSSCwmax_s + document.wmm.sta_cwmax_acbe.value;
   BSSCwmax_s = BSSCwmax_s + ';';
   BSSCwmax_s = BSSCwmax_s + document.wmm.sta_cwmax_acbk.value;
   BSSCwmax_s = BSSCwmax_s + ';';
   BSSCwmax_s = BSSCwmax_s + document.wmm.sta_cwmax_acvi.value;
   BSSCwmax_s = BSSCwmax_s + ';';
   BSSCwmax_s = BSSCwmax_s + document.wmm.sta_cwmax_acvo.value;
   document.wmm.sta_cwmax_all.value = BSSCwmax_s;
   document.wmm.sta_cwmax_acbe.disabled = true;
   document.wmm.sta_cwmax_acbk.disabled = true;
   document.wmm.sta_cwmax_acvi.disabled = true;
   document.wmm.sta_cwmax_acvo.disabled = true;

   BSSTxop_s = '';
   BSSTxop_s = BSSTxop_s + document.wmm.sta_txop_acbe.value;
   BSSTxop_s = BSSTxop_s + ';';
   BSSTxop_s = BSSTxop_s + document.wmm.sta_txop_acbk.value;
   BSSTxop_s = BSSTxop_s + ';';
   BSSTxop_s = BSSTxop_s + document.wmm.sta_txop_acvi.value;
   BSSTxop_s = BSSTxop_s + ';';
   BSSTxop_s = BSSTxop_s + document.wmm.sta_txop_acvo.value;
   document.wmm.sta_txop_all.value = BSSTxop_s;
   document.wmm.sta_txop_acbe.disabled = true;
   document.wmm.sta_txop_acbk.disabled = true;
   document.wmm.sta_txop_acvi.disabled = true;
   document.wmm.sta_txop_acvo.disabled = true;

   BSSACM_s = '';
   if (document.wmm.sta_acm_acbe.checked)
      BSSACM_s = BSSACM_s + '1';
   else
      BSSACM_s = BSSACM_s + '0';
   BSSACM_s = BSSACM_s + ';';
   if (document.wmm.sta_acm_acbk.checked)
      BSSACM_s = BSSACM_s + '1';
   else
      BSSACM_s = BSSACM_s + '0';
   BSSACM_s = BSSACM_s + ';';
   if (document.wmm.sta_acm_acvi.checked)
      BSSACM_s = BSSACM_s + '1';
   else
      BSSACM_s = BSSACM_s + '0';
   BSSACM_s = BSSACM_s + ';';
   if (document.wmm.sta_acm_acvo.checked)
      BSSACM_s = BSSACM_s + '1';
   else
      BSSACM_s = BSSACM_s + '0';
   document.wmm.sta_acm_acbe.disabled = true;
   document.wmm.sta_acm_acbk.disabled = true;
   document.wmm.sta_acm_acvi.disabled = true;
   document.wmm.sta_acm_acvo.disabled = true;
   document.wmm.sta_acm_all.value = BSSACM_s;

   return true;
}

function initValue()
{
   var APAifsn    = '<% getCfg2General(1, "APAifsn"); %>';
   var APCwmin    = '<% getCfg2General(1, "APCwmin"); %>';
   var APCwmax    = '<% getCfg2General(1, "APCwmax"); %>';
   var APTxop     = '<% getCfg2General(1, "APTxop"); %>';
   var APACM      = '<% getCfg2General(1, "APACM"); %>';
   var BSSAifsn   = '<% getCfg2General(1, "BSSAifsn"); %>';
   var BSSCwmin   = '<% getCfg2General(1, "BSSCwmin"); %>';
   var BSSCwmax   = '<% getCfg2General(1, "BSSCwmax"); %>';
   var BSSTxop    = '<% getCfg2General(1, "BSSTxop"); %>';
   var BSSACM     = '<% getCfg2General(1, "BSSACM"); %>';
   var AckPolicy  = '<% getCfg2General(1, "AckPolicy"); %>';

   if (APAifsn   == "") APAifsn = "7;3;1;1";
   if (APCwmin   == "") APCwmin = "15;15;7;3";
   if (APCwmax   == "") APCwmax = "1023;63;15;7";
   if (APTxop    == "") APTxop = "0;0;94;47";
   if (APACM     == "") APACM = "0;0;0;0";
   if (BSSAifsn  == "") BSSAifsn = "7;3;2;2";
   if (BSSCwmin  == "") BSSCwmin = "15;15;7;3";
   if (BSSCwmax  == "") BSSCwmax = "1023;1023;15;7";
   if (BSSTxop   == "") BSSTxop = "0;0;94;47";
   if (BSSACM    == "") BSSACM = "0;0;0;0";
   if (AckPolicy == "") AckPolicy = "0;0;0;0";

   var APAifsnArray   = APAifsn.split(";");
   var APCwminArray   = APCwmin.split(";");
   var APCwmaxArray   = APCwmax.split(";");
   var APTxopArray    = APTxop.split(";");
   var APACMArray     = APACM.split(";");
   var BSSAifsnArray  = BSSAifsn.split(";");
   var BSSCwminArray  = BSSCwmin.split(";");
   var BSSCwmaxArray  = BSSCwmax.split(";");
   var BSSTxopArray   = BSSTxop.split(";");
   var BSSACMArray    = BSSACM.split(";");
   var AckPolicyArray = AckPolicy.split(";");

   document.wmm.ap_aifsn_acbe.value = APAifsnArray[0];
   document.wmm.ap_aifsn_acbk.value = APAifsnArray[1];
   document.wmm.ap_aifsn_acvi.value = APAifsnArray[2];
   document.wmm.ap_aifsn_acvo.value = APAifsnArray[3];

   document.wmm.ap_cwmin_acbe.value = APCwminArray[0];
   document.wmm.ap_cwmin_acbk.value = APCwminArray[1];
   document.wmm.ap_cwmin_acvi.value = APCwminArray[2];
   document.wmm.ap_cwmin_acvo.value = APCwminArray[3];

   document.wmm.ap_cwmax_acbe.value = APCwmaxArray[0];
   document.wmm.ap_cwmax_acbk.value = APCwmaxArray[1];
   document.wmm.ap_cwmax_acvi.value = APCwmaxArray[2];
   document.wmm.ap_cwmax_acvo.value = APCwmaxArray[3];

   document.wmm.ap_txop_acbe.value = APTxopArray[0];
   document.wmm.ap_txop_acbk.value = APTxopArray[1];
   document.wmm.ap_txop_acvi.value = APTxopArray[2];
   document.wmm.ap_txop_acvo.value = APTxopArray[3];

   if (1*APACMArray[0] == 1)
      document.wmm.ap_acm_acbe.checked = true;
   else
      document.wmm.ap_acm_acbe.checked = false;

   if (1*APACMArray[1] == 1)
      document.wmm.ap_acm_acbk.checked = true;
   else
      document.wmm.ap_acm_acbk.checked = false;

   if (1*APACMArray[2] == 1)
      document.wmm.ap_acm_acvi.checked = true;
   else
      document.wmm.ap_acm_acvi.checked = false;

   if (1*APACMArray[3] == 1)
      document.wmm.ap_acm_acvo.checked = true;
   else
      document.wmm.ap_acm_acvo.checked = false;

   if (1*AckPolicyArray[0] == 1)
      document.wmm.ap_ackpolicy_acbe.checked = true;
   else
      document.wmm.ap_ackpolicy_acbe.checked = false;

   if (1*AckPolicyArray[1] == 1)
      document.wmm.ap_ackpolicy_acbk.checked = true;
   else
      document.wmm.ap_ackpolicy_acbk.checked = false;

   if (1*AckPolicyArray[2] == 1)
      document.wmm.ap_ackpolicy_acvi.checked = true;
   else
      document.wmm.ap_ackpolicy_acvi.checked = false;

   if (1*AckPolicyArray[3] == 1)
      document.wmm.ap_ackpolicy_acvo.checked = true;
   else
      document.wmm.ap_ackpolicy_acvo.checked = false;

   document.wmm.sta_aifsn_acbe.value = BSSAifsnArray[0];
   document.wmm.sta_aifsn_acbk.value = BSSAifsnArray[1];
   document.wmm.sta_aifsn_acvi.value = BSSAifsnArray[2];
   document.wmm.sta_aifsn_acvo.value = BSSAifsnArray[3];

   document.wmm.sta_cwmin_acbe.value = BSSCwminArray[0];
   document.wmm.sta_cwmin_acbk.value = BSSCwminArray[1];
   document.wmm.sta_cwmin_acvi.value = BSSCwminArray[2];
   document.wmm.sta_cwmin_acvo.value = BSSCwminArray[3];

   document.wmm.sta_cwmax_acbe.value = BSSCwmaxArray[0];
   document.wmm.sta_cwmax_acbk.value = BSSCwmaxArray[1];
   document.wmm.sta_cwmax_acvi.value = BSSCwmaxArray[2];
   document.wmm.sta_cwmax_acvo.value = BSSCwmaxArray[3];

   document.wmm.sta_txop_acbe.value = BSSTxopArray[0];
   document.wmm.sta_txop_acbk.value = BSSTxopArray[1];
   document.wmm.sta_txop_acvi.value = BSSTxopArray[2];
   document.wmm.sta_txop_acvo.value = BSSTxopArray[3];

  if (1*BSSACMArray[0] == 1)
      document.wmm.sta_acm_acbe.checked = true;
   else
      document.wmm.sta_acm_acbe.checked = false;

   if (1*BSSACMArray[1] == 1)
      document.wmm.sta_acm_acbk.checked = true;
   else
      document.wmm.sta_acm_acbk.checked = false;

   if (1*BSSACMArray[2] == 1)
      document.wmm.sta_acm_acvi.checked = true;
   else
      document.wmm.sta_acm_acvi.checked = false;

   if (1*BSSACMArray[3] == 1)
      document.wmm.sta_acm_acvo.checked = true;
   else
      document.wmm.sta_acm_acvo.checked = false;
}

function submit_apply()
{
  if (CheckValue() == true) {
    document.wmm.submit();
    window.close();
  }
}
</script>
<title>Ralink Wireless WMM Configuration</title>

<body onLoad="initValue()">
<form method=post name=wmm action="/goform/inicWmm">
  <table align="center" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
    <tr> 
      <td align="center" bgcolor="#2C5EA4"colspan="7"><font color="#FFFFFF"><b>WMM Parameters of Access Point</b></font></td>
    </tr>
    <tr>
      <td align="center" bgcolor="#E8F8FF" nowrap>&nbsp;</td>
      <td align="center" bgcolor="#E8F8FF" nowrap>Aifsn</td>
      <td align="center" bgcolor="#E8F8FF" nowrap>CWMin</td>
      <td align="center" bgcolor="#E8F8FF" nowrap>CWMax</td>
      <td align="center" bgcolor="#E8F8FF" nowrap>Txop</td>
      <td align="center" bgcolor="#E8F8FF" nowrap>ACM</td>
      <td align="center" bgcolor="#E8F8FF" nowrap>AckPolicy</td>
    </tr>
    <tr>
      <td align="center" bgcolor="#E8F8FF" nowrap><b>AC_BE</b></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_aifsn_acbe size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_cwmin_acbe size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_cwmax_acbe size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_txop_acbe size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=checkbox name=ap_acm_acbe value="1" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=checkbox name=ap_ackpolicy_acbe value="1" style="font-size: 9pt;"></td>
    </tr>
    <input type=hidden name=ap_aifsn_all>
    <input type=hidden name=ap_cwmin_all>
    <input type=hidden name=ap_cwmax_all>
    <input type=hidden name=ap_txop_all>
    <input type=hidden name=ap_acm_all>
    <input type=hidden name=ap_ackpolicy_all>
    <tr>
      <td align="center" bgcolor="#E8F8FF" nowrap><b>AC_BK</b></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_aifsn_acbk size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_cwmin_acbk size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_cwmax_acbk size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_txop_acbk size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=checkbox name=ap_acm_acbk value="1" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=checkbox name=ap_ackpolicy_acbk value="1" style="font-size: 9pt;"></td>
    </tr>
    <tr>
      <td align="center" bgcolor="#E8F8FF" nowrap><b>AC_VI</b></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_aifsn_acvi size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_cwmin_acvi size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_cwmax_acvi size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_txop_acvi size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=checkbox name=ap_acm_acvi value="1" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=checkbox name=ap_ackpolicy_acvi value="1" style="font-size: 9pt;"></td>
    </tr>
    <tr>
      <td align="center" bgcolor="#E8F8FF" nowrap><b>AC_VO</b></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_aifsn_acvo size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_cwmin_acvo size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_cwmax_acvo size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=ap_txop_acvo size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=checkbox name=ap_acm_acvo value="1" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=checkbox name=ap_ackpolicy_acvo value="1" style="font-size: 9pt;"></td>
    </tr>
  </table>
  <br>
  <table align="center" width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
    <tr> 
      <td align="center" bgcolor="#2C5EA4"colspan="7"><font color="#FFFFFF"><b>WMM Parameters of Station</b></font></td>
    </tr>
    <tr>
      <td align="center" bgcolor="#E8F8FF" nowrap>&nbsp;</td>
      <td align="center" bgcolor="#E8F8FF" nowrap>Aifsn</td>
      <td align="center" bgcolor="#E8F8FF" nowrap>CWMin</td>
      <td align="center" bgcolor="#E8F8FF" nowrap>CWMax</td>
      <td align="center" bgcolor="#E8F8FF" nowrap>Txop</td>
      <td align="center" bgcolor="#E8F8FF" nowrap>ACM</td>
    </tr>
    <tr>
      <td align="center" bgcolor="#E8F8FF" nowrap><b>AC_BE</b></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_aifsn_acbe size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_cwmin_acbe size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_cwmax_acbe size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_txop_acbe size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=checkbox name=sta_acm_acbe value="1" style="font-size: 9pt;"></td>
    </tr>
    <input type=hidden name=sta_aifsn_all>
    <input type=hidden name=sta_cwmin_all>
    <input type=hidden name=sta_cwmax_all>
    <input type=hidden name=sta_txop_all>
    <input type=hidden name=sta_acm_all>
    <tr>
      <td align="center" bgcolor="#E8F8FF" nowrap><b>AC_BK</b></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_aifsn_acbk size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_cwmin_acbk size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_cwmax_acbk size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_txop_acbk size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=checkbox name=sta_acm_acbk value="1" style="font-size: 9pt;"></td>
    </tr>
    <tr>
      <td align="center" bgcolor="#E8F8FF" nowrap><b>AC_VI</b></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_aifsn_acvi size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_cwmin_acvi size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_cwmax_acvi size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_txop_acvi size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=checkbox name=sta_acm_acvi value="1" style="font-size: 9pt;"></td>
    </tr>
    <tr>
      <td align="center" bgcolor="#E8F8FF" nowrap><b>AC_VO</b></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_aifsn_acvo size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_cwmin_acvo size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_cwmax_acvo size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=text name=sta_txop_acvo size=4 maxlength=4 value="" style="font-size: 9pt;"></td>
      <td align="center" bgcolor="#FFFFFF" nowrap><input type=checkbox name=sta_acm_acvo value="1" style="font-size: 9pt;"></td>
    </tr>
  </table>

  <table align="center" width = "540" border = "0" cellpadding = "2" cellspacing = "1">
    <tr align="center">
      <td>
        <input type=button style="{width:120px;}" value="Apply" onClick="submit_apply()">&nbsp;&nbsp;
        <input type=reset  style="{width:120px;}" value="Cancel" onClick="window.location.reload()">
        <input type=button name="Close" value="Close" style="{width:120px;}" onClick="window.close()">
      </td>
    </tr>
  </table>
</form>
</body>
</html>
