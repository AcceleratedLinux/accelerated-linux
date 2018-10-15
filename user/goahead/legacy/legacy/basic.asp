<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>Basic Wireless Settings</title>

<script language="JavaScript" type="text/javascript">
var PhyMode  = '<% getCfg3Zero(1, "WirelessMode"); %>';
var broadcastssidEnable  = '<% getCfg3Zero(1, "HideSSID"); %>';
var channel_index  = '<% getLegacyChannel(); %>';
var wdsMode  = '<% getCfg3Zero(1, "WdsEnable"); %>';
var wdsList  = '<% getCfg3General(1, "WdsList"); %>';
// var wdsPhyMode  = '<% getCfg2Zero(1, "WdsPhyMode"); %>';
var wdsEncrypType  = '<% getLegacyWdsEncType(); %>';
var wdsEncrypKey  = '<% getCfg3General(1, "WdsKey"); %>';
var countrycode = '<% getCfg3General(1, "CountryCode"); %>';
// var apcli_include = '<!--#include ssi=getWlanApCliInclude()-->';
var apcli_include = '0';

ChannelList_24G = new Array(14);
ChannelList_24G[0] = "2412MHz (Channel 1)";
ChannelList_24G[1] = "2417MHz (Channel 2)";
ChannelList_24G[2] = "2422MHz (Channel 3)";
ChannelList_24G[3] = "2427MHz (Channel 4)";
ChannelList_24G[4] = "2432MHz (Channel 5)";
ChannelList_24G[5] = "2437MHz (Channel 6)";
ChannelList_24G[6] = "2442MHz (Channel 7)";
ChannelList_24G[7] = "2447MHz (Channel 8)";
ChannelList_24G[8] = "2452MHz (Channel 9)";
ChannelList_24G[9] = "2457MHz (Channel 10)";
ChannelList_24G[10] = "2462MHz (Channel 11)";
ChannelList_24G[11] = "2467MHz (Channel 12)";
ChannelList_24G[12] = "2472MHz (Channel 13)";
ChannelList_24G[13] = "2484MHz (Channel 14)";

ChannelList_5G = new Array(33);
ChannelList_5G[0] = "5180MHz (Channel 36)";
ChannelList_5G[1] = "5200MHz (Channel 40)";
ChannelList_5G[2] = "5220MHz (Channel 44)";
ChannelList_5G[3] = "5240MHz (Channel 48)";
ChannelList_5G[4] = "5260MHz (Channel 52)";
ChannelList_5G[5] = "5280MHz (Channel 56)";
ChannelList_5G[6] = "5300MHz (Channel 60)";
ChannelList_5G[7] = "5320MHz (Channel 64)";
ChannelList_5G[16] = "5500MHz (Channel 100)";
ChannelList_5G[17] = "5520MHz (Channel 104)";
ChannelList_5G[18] = "5540MHz (Channel 108)";
ChannelList_5G[19] = "5560MHz (Channel 112)";
ChannelList_5G[20] = "5580MHz (Channel 116)";
ChannelList_5G[21] = "5600MHz (Channel 120)";
ChannelList_5G[22] = "5620MHz (Channel 124)";
ChannelList_5G[23] = "5640MHz (Channel 128)";
ChannelList_5G[24] = "5660MHz (Channel 132)";
ChannelList_5G[25] = "5680MHz (Channel 136)";
ChannelList_5G[26] = "5700MHz (Channel 140)";
ChannelList_5G[28] = "5745MHz (Channel 149)";
ChannelList_5G[29] = "5765MHz (Channel 153)";
ChannelList_5G[30] = "5785MHz (Channel 157)";
ChannelList_5G[31] = "5805MHz (Channel 161)";
ChannelList_5G[32] = "5825MHz (Channel 165)";

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

function insertChannelOption(vChannel, band)
{
	var y = document.createElement('option');

	if (1*band == 24)
	{
		y.text = ChannelList_24G[1*vChannel - 1];
		y.value = 1*vChannel;
	}
	else if (1*band == 5)
	{
		y.value = 1*vChannel;
		if (1*vChannel <= 140)
			y.text = ChannelList_5G[((1*vChannel) - 36) / 4];
		else
			y.text = ChannelList_5G[((1*vChannel) - 36 - 1) / 4];
	}

	if (1*band == 24)
		var x=document.getElementById("sz11gChannel");
	else if (1*band == 5)
		var x=document.getElementById("sz11aChannel");

	try
	{
		x.add(y,null); // standards compliant
	}
	catch(ex)
	{
		x.add(y); // IE only
	}
}

function Check5GBandChannelException()
{
	var w_mode = document.wireless_basic.wirelessmode.options.selectedIndex;

	if (1*w_mode == 3)
	{
		var x = document.getElementById("sz11aChannel")
		var current_length = document.wireless_basic.sz11aChannel.options.length;
		var current_index = document.wireless_basic.sz11aChannel.options.selectedIndex;

		for (ch_idx = current_length - 1; ch_idx > 0; ch_idx--)
		{
			x.remove(ch_idx);
		}

		if ((countrycode == 'NONE') || (countrycode == 'FR') || (countrycode == 'US') ||
			(countrycode == 'IE') || (countrycode == 'JP') || (countrycode == 'HK'))
		{
			for(ch = 36; ch <= 48; ch+=4)
				insertChannelOption(ch, 5);
		}

		if ((countrycode == 'NONE') || (countrycode == 'FR') || (countrycode == 'US') ||
			(countrycode == 'IE') || (countrycode == 'TW') || (countrycode == 'HK'))
		{
			for(ch = 52; ch <= 64; ch+=4)
				insertChannelOption(ch, 5);
		}

		if (countrycode == 'NONE')
		{
			for(ch = 100; ch <= 140; ch+=4)
				insertChannelOption(ch, 5);
		}

		if ((countrycode == 'NONE') || (countrycode == 'US') || (countrycode == 'TW') ||
			(countrycode == 'CN') || (countrycode == 'HK'))
		{
			for(ch = 149; ch <= 161; ch+=4)
				insertChannelOption(ch, 5);
		}

		if ((countrycode == 'NONE') || (countrycode == 'US') ||
			(countrycode == 'CN') || (countrycode == 'HK'))
		{
				insertChannelOption(165, 5);
		}

		document.wireless_basic.sz11aChannel.options.selectedIndex = (1*current_index);
	}
}

function initValue()
{
	var ssidArray;
	var wdslistArray;
	var broadcastssidArray;
	var channel_11a_index;
	var current_channel_length;
     	var all_ssid = '<% getCfg3General(1, "SSID"); %>';
	var ssid = new Array;
	
	ssid = all_ssid.split(";");
	document.wireless_basic.ssid.value = ssid[0];
	/*
	for(i = 1; i < 4; i++)
	{
		if ((i == 3) && (1*apcli_include) == 1)
			document.wireless_basic.mssid_3.disabled = true;
		else if (ssid[i] != "")
			eval("document.wireless_basic.mssid_"+i).value = ssid[i];
		else
			eval("document.wireless_basic.mssid_"+i).value = "";
	}
	*/

	if (countrycode == '')
		countrycode = 'NONE';

	document.getElementById("div_11a_channel").style.visibility = "hidden";
	document.getElementById("div_11a_channel").style.display = "none";
	document.wireless_basic.sz11aChannel.disabled = true;
	document.getElementById("div_11b_channel").style.visibility = "hidden";
	document.getElementById("div_11b_channel").style.display = "none";
	document.wireless_basic.sz11bChannel.disabled = true;
	document.getElementById("div_11g_channel").style.visibility = "hidden";
	document.getElementById("div_11g_channel").style.display = "none";
	document.wireless_basic.sz11gChannel.disabled = true;

	PhyMode = 1*PhyMode;

	if ((PhyMode == 0) || (PhyMode == 2))
	{
		if (PhyMode == 0)
			document.wireless_basic.wirelessmode.options.selectedIndex = 0;
		else if (PhyMode == 2)
			document.wireless_basic.wirelessmode.options.selectedIndex = 2;

		document.getElementById("div_11g_channel").style.visibility = "visible";
		document.getElementById("div_11g_channel").style.display = style_display_on();
		document.wireless_basic.sz11gChannel.disabled = false;
	}
	else if (PhyMode == 1)
	{
		document.wireless_basic.wirelessmode.options.selectedIndex = 1;
		document.getElementById("div_11b_channel").style.visibility = "visible";
		document.getElementById("div_11b_channel").style.display = style_display_on();
		document.wireless_basic.sz11bChannel.disabled = false;
	}
	else if (PhyMode == 3)
	{
		document.wireless_basic.wirelessmode.options.selectedIndex = 3;
		document.getElementById("div_11a_channel").style.visibility = "visible";
		document.getElementById("div_11a_channel").style.display = style_display_on();
		document.wireless_basic.sz11aChannel.disabled = false;
	}

	broadcastssidArray = broadcastssidEnable.split(";");

	if (1*broadcastssidArray[0] == 0)
		document.wireless_basic.broadcastssid[0].checked = true;
	else
		document.wireless_basic.broadcastssid[1].checked = true;

	channel_index = 1*channel_index;

	if ((PhyMode == 0) || (PhyMode == 2))
	{
		document.wireless_basic.sz11gChannel.options.selectedIndex = channel_index;

		current_channel_length = document.wireless_basic.sz11gChannel.options.length;

		if ((channel_index + 1) > current_channel_length)
			document.wireless_basic.sz11gChannel.options.selectedIndex = 0;
	}
	else if (PhyMode == 1)
	{
		document.wireless_basic.sz11bChannel.options.selectedIndex = channel_index;

		current_channel_length = document.wireless_basic.sz11bChannel.options.length;

		if ((channel_index + 1) > current_channel_length)
			document.wireless_basic.sz11bChannel.options.selectedIndex = 0;
	}
	else if (PhyMode == 3)
	{
		if (countrycode == 'NONE')
		{
			if (channel_index <= 64)
			{
				channel_11a_index = channel_index;
				channel_11a_index = channel_11a_index / 4;
				if (channel_11a_index != 0)
					channel_11a_index = channel_11a_index - 8;
			}
			else if ((channel_index >= 100) && (channel_index <= 140))
			{
				channel_11a_index = channel_index;
				channel_11a_index = channel_11a_index / 4;
				channel_11a_index = channel_11a_index - 16;
			}
			else if (channel_index >= 149)
			{
				channel_11a_index = channel_index - 1;
				channel_11a_index = channel_11a_index / 4;
				channel_11a_index = channel_11a_index - 17;

				if (document.wireless_basic.n_bandwidth[1].checked == true)
				{
					channel_11a_index = channel_11a_index - 1;
				}
			}
			else
			{
				channel_11a_index = 0;
			}
		}
		else if ((countrycode == 'US') || (countrycode == 'HK') || (countrycode == 'FR') || (countrycode == 'IE'))
		{
			if (channel_index <= 64)
			{
				channel_11a_index = channel_index;
				channel_11a_index = channel_11a_index / 4;
				if (channel_11a_index != 0)
					channel_11a_index = channel_11a_index - 8;
			}
			else if (channel_index >= 149)
			{
				channel_11a_index = channel_index - 1;
				channel_11a_index = channel_11a_index / 4;
				channel_11a_index = channel_11a_index - 28;
			}
			else
			{
				channel_11a_index = 0;
			}
		}
		else if (countrycode == 'JP')
		{
			if (channel_index <= 48)
			{
				channel_11a_index = channel_index;
				channel_11a_index = channel_11a_index / 4;
				if (channel_11a_index != 0)
					channel_11a_index = channel_11a_index - 8;
			}
			else
			{
				channel_11a_index = 0;
			}
		}
		else if (countrycode == 'TW')
		{
			if (channel_index <= 64)
			{
				channel_11a_index = channel_index;
				channel_11a_index = channel_11a_index / 4;
				if (channel_11a_index != 0)
					channel_11a_index = channel_11a_index - 12;
			}
			else if (channel_index >= 149)
			{
				channel_11a_index = channel_index - 1;
				channel_11a_index = channel_11a_index / 4;
				channel_11a_index = channel_11a_index - 32;
			}
			else
			{
				channel_11a_index = 0;
			}
		}
		else if (countrycode == 'CN')
		{
			if (channel_index >= 149)
			{
				channel_11a_index = channel_index - 1;
				channel_11a_index = channel_11a_index / 4;
				channel_11a_index = channel_11a_index - 36;
			}
			else
			{
				channel_11a_index = 0;
			}
		}
		else
		{
			channel_11a_index = 0;
		}

		Check5GBandChannelException();

		if (channel_index > 0)
			document.wireless_basic.sz11aChannel.options.selectedIndex = channel_11a_index;
		else
			document.wireless_basic.sz11aChannel.options.selectedIndex = channel_index;
	}

	if (wdsList != "")
	{
		wdslistArray = wdsList.split(";");
		for(i = 1; i <= wdslistArray.length; i++)
			eval("document.wireless_basic.wds_"+i).value = wdslistArray[i - 1];
/*
		for (i = wdslistArray.length + 1; i <= 4; i++)
			eval("document.wireless_basic.wds_"+i).value = "00:00:00:00:00:00";
	}
	else
	{
		for(i = 1; i <= 4; i++)
			eval("document.wireless_basic.wds_"+i).value = "00:00:00:00:00:00";
*/
	}

	wdsMode = 1*wdsMode;

	if (wdsMode == 0)
		document.wireless_basic.wds_mode.options.selectedIndex = 0;
	else if (wdsMode == 4)
		document.wireless_basic.wds_mode.options.selectedIndex = 1;
	else if (wdsMode == 2)
		document.wireless_basic.wds_mode.options.selectedIndex = 2;
	else if (wdsMode == 3)
		document.wireless_basic.wds_mode.options.selectedIndex = 3;

	// document.wireless_basic.wds_phy_mode.options.selectedIndex = 1*wdsPhyMode;
	document.wireless_basic.wds_encryp_type.options.selectedIndex = 1*wdsEncrypType;
	document.wireless_basic.wds_encryp_key.value = wdsEncrypKey;

	WdsModeOnChange();

	if (1*apcli_include == 1)
	{
		document.wireless_basic.mssid_3.disabled = true;
	}
}

function wirelessModeChange()
{
	var wmode;
   
	document.getElementById("div_11a_channel").style.visibility = "hidden";
	document.getElementById("div_11a_channel").style.display = "none";
	document.wireless_basic.sz11aChannel.disabled = true;
	document.getElementById("div_11b_channel").style.visibility = "hidden";
	document.getElementById("div_11b_channel").style.display = "none";
	document.wireless_basic.sz11bChannel.disabled = true;
	document.getElementById("div_11g_channel").style.visibility = "hidden";
	document.getElementById("div_11g_channel").style.display = "none";
	document.wireless_basic.sz11gChannel.disabled = true;

	wmode = document.wireless_basic.wirelessmode.options.selectedIndex;

	wmode = 1*wmode;
	if (wmode == 0)
	{
		document.wireless_basic.wirelessmode.options.selectedIndex = 0;
		document.getElementById("div_11g_channel").style.visibility = "visible";
		document.getElementById("div_11g_channel").style.display = style_display_on();
		document.wireless_basic.sz11gChannel.disabled = false;
	}
	else if (wmode == 1)
	{
		document.wireless_basic.wirelessmode.options.selectedIndex = 1;
		document.getElementById("div_11b_channel").style.visibility = "visible";
		document.getElementById("div_11b_channel").style.display = style_display_on();
		document.wireless_basic.sz11bChannel.disabled = false;
	}
	else if (wmode == 2)
	{
		document.wireless_basic.wirelessmode.options.selectedIndex = 2;
		document.getElementById("div_11g_channel").style.visibility = "visible";
		document.getElementById("div_11g_channel").style.display = style_display_on();
		document.wireless_basic.sz11gChannel.disabled = false;
	}
	else if (wmode == 3)
	{
		document.wireless_basic.wirelessmode.options.selectedIndex = 3;
		document.getElementById("div_11a_channel").style.visibility = "visible";
		document.getElementById("div_11a_channel").style.display = style_display_on();
		document.wireless_basic.sz11aChannel.disabled = false;

		Check5GBandChannelException();
	}
}

function WdsModeOnChange()
{
	/*
	document.getElementById("div_wds_phy_mode").style.visibility = "hidden";
	document.getElementById("div_wds_phy_mode").style.display = "none";
	document.wireless_basic.wds_phy_mode.disabled = true;
	*/
	document.getElementById("div_wds_encryp_type").style.visibility = "hidden";
	document.getElementById("div_wds_encryp_type").style.display = "none";
	document.wireless_basic.wds_encryp_type.disabled = true;
	document.getElementById("div_wds_encryp_key").style.visibility = "hidden";
	document.getElementById("div_wds_encryp_key").style.display = "none";
	document.wireless_basic.wds_encryp_key.disabled = true;
	document.getElementById("wds_mac_list_1").style.visibility = "hidden";
	document.getElementById("wds_mac_list_1").style.display = "none";
	document.wireless_basic.wds_1.disabled = true;
	document.getElementById("wds_mac_list_2").style.visibility = "hidden";
	document.getElementById("wds_mac_list_2").style.display = "none";
	document.wireless_basic.wds_2.disabled = true;
	document.getElementById("wds_mac_list_3").style.visibility = "hidden";
	document.getElementById("wds_mac_list_3").style.display = "none";
	document.wireless_basic.wds_3.disabled = true;
	document.getElementById("wds_mac_list_4").style.visibility = "hidden";
	document.getElementById("wds_mac_list_4").style.display = "none";
	document.wireless_basic.wds_4.disabled = true;

	if (document.wireless_basic.wds_mode.options.selectedIndex >= 1)
	{
		/*
		document.getElementById("div_wds_phy_mode").style.visibility = "visible";
		document.getElementById("div_wds_phy_mode").style.display = style_display_on();
		document.wireless_basic.wds_phy_mode.disabled = false;
		*/
		document.getElementById("div_wds_encryp_type").style.visibility = "visible";
		document.getElementById("div_wds_encryp_type").style.display = style_display_on();
		document.wireless_basic.wds_encryp_type.disabled = false;

		if (document.wireless_basic.wds_encryp_type.options.selectedIndex >= 2)
		{
			document.getElementById("div_wds_encryp_key").style.visibility = "visible";
			document.getElementById("div_wds_encryp_key").style.display = style_display_on();
			document.wireless_basic.wds_encryp_key.disabled = false;
		}
	}

	if (document.wireless_basic.wds_mode.options.selectedIndex >= 2)
	{
		document.getElementById("wds_mac_list_1").style.visibility = "visible";
		document.getElementById("wds_mac_list_1").style.display = style_display_on();
		document.wireless_basic.wds_1.disabled = false;
		document.getElementById("wds_mac_list_2").style.visibility = "visible";
		document.getElementById("wds_mac_list_2").style.display = style_display_on();
		document.wireless_basic.wds_2.disabled = false;
		document.getElementById("wds_mac_list_3").style.visibility = "visible";
		document.getElementById("wds_mac_list_3").style.display = style_display_on();
		document.wireless_basic.wds_3.disabled = false;
		document.getElementById("wds_mac_list_4").style.visibility = "visible";
		document.getElementById("wds_mac_list_4").style.display = style_display_on();
		document.wireless_basic.wds_4.disabled = false;
	}
}

function WdsSecurityOnChange()
{
	document.getElementById("div_wds_encryp_key").style.visibility = "hidden";
	document.getElementById("div_wds_encryp_key").style.display = "none";
	document.wireless_basic.wds_encryp_key.disabled = true;

	if (document.wireless_basic.wds_encryp_type.options.selectedIndex >= 2)
	{
		document.getElementById("div_wds_encryp_key").style.visibility = "visible";
		document.getElementById("div_wds_encryp_key").style.display = style_display_on();
		document.wireless_basic.wds_encryp_key.disabled = false;
	}
	else if (document.wireless_basic.wds_encryp_type.options.selectedIndex == 1)
	{
	}
}

function CheckValue()
{
	var wireless_mode;
	var submit_ssid_num;
	var channel_11a_index;
	var check_wds_mode;
	var all_wds_list;

	if (document.wireless_basic.ssid.value == "")
	{
		alert("Please enter SSID!");
		document.wireless_basic.ssid.focus();
		document.wireless_basic.ssid.select();
		return false;
	} 
	else if (document.wireless_basic.ssid.value.indexOf(";") >= 0)
	{
		alert("SSID can not include ';' character!");
		document.wireless_basic.ssid.focus();
		return false;
	}

	submit_ssid_num = 1;

	/*
	for(i = 1; i < 4; i++)
	{
		if (eval("document.wireless_basic.mssid_"+i).value != "")
		{
			if (i == 3)
			{
				if (1*apcli_include == 0)
					submit_ssid_num++;
			}
			else
				submit_ssid_num++;
			if (eval("document.wireless_basic.mssid_"+i).value.indexOf(";") >= 0)
			{
				alert("SSID"+i+" can not include ';' character!");
				eval("document.wireless_basic.mssid_"+i).focus();
				return false;
			}
		}
	}
	*/

	document.wireless_basic.bssid_num.value = submit_ssid_num;

	all_wds_list = '';
	if (document.wireless_basic.wds_mode.options.selectedIndex >= 2)
	{
		var re = /[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}/;
		for (i = 1; i <= 4; i++)
		{
			if (eval("document.wireless_basic.wds_"+i).value == "")
				continue;
			if (!re.test(eval("document.wireless_basic.wds_"+i).value)) {
				alert("Please fill WDS remote AP MAC Address in correct format! (XX:XX:XX:XX:XX:XX)");
				return false;
			}
			else {
				all_wds_list += eval("document.wireless_basic.wds_"+i).value;
				all_wds_list += ';';
			}
		}
		if (all_wds_list == "")
		{
			alert("WDS remote AP MAC Address are empty !!!");
			document.wireless_basic.wds_1.focus();
			document.wireless_basic.wds_1.select(); 
			return false;
		}
		else
		{
			document.wireless_basic.wds_list.value = all_wds_list;
			document.wireless_basic.wds_1.disabled = true;
			document.wireless_basic.wds_2.disabled = true;
			document.wireless_basic.wds_3.disabled = true;
			document.wireless_basic.wds_4.disabled = true;
		}
		//document.wireless_basic.rebootAP.value = 1;
	}

	return true;
}

function RadioStatusChange(rs)
{
	if (rs == 1) {
		document.wireless_basic.radioButton.value = "RADIO OFF";
		document.wireless_basic.radiohiddenButton.value = 0;
	}
	else {
		document.wireless_basic.radioButton.value = "RADIO ON";
		document.wireless_basic.radiohiddenButton.value = 1;
	}
}
</script>
</head>


<body onLoad="initValue()">
<table class="body"><tr><td>

<h1>Basic Wireless Settings </h1>
<p> You could configure the minimum number of Wireless settings for communication, such as Network Name (SSID) and Channel. The Access Point can be set simply with only the minimum setting items. </p>
<hr />

<form method=post name=wireless_basic action="/goform/legacyBasic" onSubmit="return CheckValue()">
<table width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2">Wireless Network</td>
  </tr>
  <!--
  <tr> 
    <td class="head">Radio On/Off</td>
    <td>
      <input type="button" name="radioButton" style="{width:120px;}" value="RADIO ON"
      onClick="if (this.value.indexOf('OFF') >= 0) RadioStatusChange(1); else RadioStatusChange(0); document.wireless_basic.submit();"> &nbsp; &nbsp;
      <input type=hidden name=radiohiddenButton value="2">
    </td>
  </tr>
  -->
  <tr> 
    <td class="head">Network Mode</td>
    <td>
      <select name="wirelessmode" id="wirelessmode" size="1" onChange="wirelessModeChange()">
        <option value=0>11b/g mixed mode</option>
        <option value=1>11b only</option>
        <option value=2>11g only</option>
        <option value=3>11a only</option>
      </select>
    </td>
  </tr>
  <input type="hidden" name="bssid_num" value="1">
  <tr> 
    <td class="head">Network Name(SSID)</td>
    <td><input type=text name=ssid size=20 maxlength=32 value=""></td>
  </tr>
  <!--
  <tr> 
    <td class="head">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Multiple SSID1</td>
    <td><input type=text name=mssid_1 size=20 maxlength=32 value=""></td>
  </tr>
  <tr> 
    <td class="head">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Multiple SSID2</td>
    <td><input type=text name=mssid_2 size=20 maxlength=32 value=""></td>
  </tr>
  <tr> 
    <td class="head">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Multiple SSID3</td>
    <td><input type=text name=mssid_3 size=20 maxlength=32 value=""></td>
  </tr>
  -->
  <tr> 
    <td class="head">Broadcast Network Name (SSID)</td>
    <td>
      <input type=radio name=broadcastssid value="1" checked>Enable&nbsp;
      <input type=radio name=broadcastssid value="0">Disable
    </td>
  </tr>
  <tr> 
    <td class="head">BSSID</td>
    <td>&nbsp;&nbsp;<% getLegacyCurrentMac(); %></td>
  </tr>
  <tr id="div_11a_channel" name="div_11a_channel" style="visibility:visible;">
    <td class="head">Frequency (Channel)</td>
    <td>
      <select id="sz11aChannel" name="sz11aChannel" size="1">
	<option value=0>AutoSelect</option>
	<% getLegacy11aChannels(); %>
      </select>
    </td>
  </tr>
  <tr id="div_11b_channel" name="div_11b_channel" style="visibility:visible;">
    <td class="head">Frequency (Channel)</td>
    <td>
      <select id="sz11bChannel" name="sz11bChannel" size="1">
	<option value=0>AutoSelect</option>
	<% getLegacy11bChannels(); %>
      </select>
    </td>
  </tr>
  <tr id="div_11g_channel" name="div_11g_channel" style="visibility:visible;">
    <td class="head">Frequency (Channel)</td>
    <td>
      <select id="sz11gChannel" name="sz11gChannel" size="1">
	<option value=0>AutoSelect</option>
	<% getLegacy11gChannels(); %>
      </select>
    </td>
  </tr>
</table>

<table width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2">Wireless Distribution System(WDS)</td>
  </tr>
  <tr> 
    <td class="head">WDS Mode</td>
    <td>
      <select name="wds_mode" id="wds_mode" size="1" onchange="WdsModeOnChange()">
	<option value=0 SELECTED>Disable</option>
	<option value=4>Lazy Mode</option>
	<option value=2>Bridge Mode</option>
	<option value=3>Repeater Mode</option>
      </select>
    </td>
  </tr>
  <tr id="div_wds_encryp_type" name="div_wds_encryp_type" style="visibility:visible;"> 
    <td class="head">EncrypType</td>
    <td>
      <select name="wds_encryp_type" id="wds_encryp_type" size="1" onchange="WdsSecurityOnChange()">
	<option value="NONE;NONE;NONE;NONE" selected>NONE</option>
	<option value="WEP;WEP;WEP;WEP">WEP</option>
	<option value="TKIP;TKIP;TKIP;TKIP">TKIP</option>
	<option value="AES;AES;AES;AES">AES</option>
      </select>
    </td>
  </tr>
  <tr id="div_wds_encryp_key" name="div_wds_encryp_key" style="visibility:visible;">
    <td class="head">Encryp Key</td>
    <td><input type=text name=wds_encryp_key size=28 maxlength=64 value=""></td>
  </tr>
  <tr id="wds_mac_list_1" name="wds_mac_list_1" style="visibility:visible;">
    <td class="head">AP MAC Address</td>
    <td><input type=text name=wds_1 size=20 maxlength=17 value=""></td>
  </tr>
  <tr id="wds_mac_list_2" name="wds_mac_list_2" style="visibility:visible;">
    <td class="head">AP MAC Address</td>
    <td><input type=text name=wds_2 size=20 maxlength=17 value=""></td>
  </tr>
  <tr id="wds_mac_list_3" name="wds_mac_list_3" style="visibility:visible;">
    <td class="head">AP MAC Address</td>
    <td><input type=text name=wds_3 size=20 maxlength=17 value=""></td>
  </tr>
  <tr id="wds_mac_list_4" name="wds_mac_list_4" style="visibility:visible;">
    <td class="head">AP MAC Address</td>
    <td><input type=text name=wds_4 size=20 maxlength=17 value=""></td>
  </tr>
  <input type="hidden" name="wds_list" value="1">
</table>
<br />

<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>  

</td></tr></table>
</body>
</html>

