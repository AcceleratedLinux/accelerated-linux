<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<script type="text/javascript" src="/common.js"></script>
<title>Basic Wireless Settings</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("wireless");

restartPage_init();

var sbuttonMax=1;

var PhyMode  = '<% getCfgZero(1, "WirelessMode"); %>';
var HiddenSSID  = '<% getCfgZero(1, "HideSSID"); %>';
var APIsolated = '<% getCfgZero(1, "NoForwarding"); %>';
var mbssidapisolated = '<% getCfgZero(1, "NoForwardingBTNBSSID"); %>';
var channel_index  = '<% getWlanChannel(); %>';
var fxtxmode = '<% getCfgGeneral(1, "FixedTxMode"); %>';
var countrycode = '<% getCfgGeneral(1, "CountryCode"); %>';
var ht_mode = '<% getCfgZero(1, "HT_OpMode"); %>';
var ht_bw = '<% getCfgZero(1, "HT_BW"); %>';
var ht_gi = '<% getCfgZero(1, "HT_GI"); %>';
var ht_stbc = '<% getCfgZero(1, "HT_STBC"); %>';
var ht_mcs = '<% getCfgZero(1, "HT_MCS"); %>';
var ht_htc = '<% getCfgZero(1, "HT_HTC"); %>';
//var ht_rdg = '<% getCfgZero(1, "HT_RDG"); %>';
//var ht_linkadapt = '<% getCfgZero(1, "HT_LinkAdapt"); %>';
var ht_extcha = '<% getCfgZero(1, "HT_EXTCHA"); %>';
var ht_amsdu = '<% getCfgZero(1, "HT_AMSDU"); %>';
//var ht_autoba = '<% getCfgZero(1, "HT_AutoBA"); %>';
//var ht_badecline = '<% getCfgZero(1, "HT_BADecline"); %>';
//var ht_disallow_tkip = '<% getCfgZero(1, "HT_DisallowTKIP"); %>';
//var ht_f_40mhz = '<% getCfgZero(1, "HT_40MHZ_INTOLERANT"); %>';
//var wifi_optimum = '<!--#include ssi=getWlanWiFiTest()-->';
var apcli_include = '<% getWlanApcliBuilt(); %>';
//var tx_stream_idx = '<% getCfgZero(1, "HT_TxStream"); %>';
//var rx_stream_idx = '<% getCfgZero(1, "HT_RxStream"); %>';
var is3t3r = '<% is3t3r(); %>';

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

HT5GExtCh = new Array(22);
HT5GExtCh[0] = new Array(1, "5200MHz (Channel 40)"); // channel 36's extension channel
HT5GExtCh[1] = new Array(0, "5180MHz (Channel 36)"); // channel 40's extension channel
HT5GExtCh[2] = new Array(1, "5240MHz (Channel 48)"); // channel 44's extension channel
HT5GExtCh[3] = new Array(0, "5220MHz (Channel 44)"); // channel 48's extension channel
HT5GExtCh[4] = new Array(1, "5280MHz (Channel 56)"); // channel 52's extension channel
HT5GExtCh[5] = new Array(0, "5260MHz (Channel 52)"); // channel 56's extension channel
HT5GExtCh[6] = new Array(1, "5320MHz (Channel 64)"); // channel 60's extension channel
HT5GExtCh[7] = new Array(0, "5300MHz (Channel 60)"); // channel 64's extension channel
HT5GExtCh[8] = new Array(1, "5520MHz (Channel 104)"); // channel 100's extension channel
HT5GExtCh[9] = new Array(0, "5500MHz (Channel 100)"); // channel 104's extension channel
HT5GExtCh[10] = new Array(1, "5560MHz (Channel 112)"); // channel 108's extension channel
HT5GExtCh[11] = new Array(0, "5540MHz (Channel 108)"); // channel 112's extension channel
HT5GExtCh[12] = new Array(1, "5600MHz (Channel 120)"); // channel 116's extension channel
HT5GExtCh[13] = new Array(0, "5580MHz (Channel 116)"); // channel 120's extension channel
HT5GExtCh[14] = new Array(1, "5640MHz (Channel 128)"); // channel 124's extension channel
HT5GExtCh[15] = new Array(0, "5620MHz (Channel 124)"); // channel 128's extension channel
HT5GExtCh[16] = new Array(1, "5680MHz (Channel 136)"); // channel 132's extension channel
HT5GExtCh[17] = new Array(0, "5660MHz (Channel 132)"); // channel 136's extension channel
HT5GExtCh[18] = new Array(1, "5765MHz (Channel 153)"); // channel 149's extension channel
HT5GExtCh[19] = new Array(0, "5745MHz (Channel 149)"); // channel 153's extension channel
HT5GExtCh[20] = new Array(1, "5805MHz (Channel 161)"); // channel 157's extension channel
HT5GExtCh[21] = new Array(0, "5785MHz (Channel 157)"); // channel 161's extension channel

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

function CreateExtChannelOption(vChannel)
{
/*
	var y = document.createElement('option');

	y.text = ChannelList_24G[1*vChannel - 1];
//	y.value = 1*vChannel;
	y.value = 1;

	var x = document.getElementById("n_extcha");

	try
	{
		x.add(y,null); // standards compliant
	}
	catch(ex)
	{
		x.add(y); // IE only
	}
 */
}

function insertExtChannelOption()
{
/*
	var wmode = document.wireless_basic.wirelessmode.options.selectedIndex;
	var option_length; 
	var CurrentCh;

	if ((1*wmode >=3))
	{
		var x = document.getElementById("n_extcha");
		var length = document.wireless_basic.n_extcha.options.length;

		if (length > 1)
		{
			x.selectedIndex = 1;
			x.remove(x.selectedIndex);
		}
		{
			CurrentCh = document.wireless_basic.sz11gChannel.value;
			option_length = document.wireless_basic.sz11gChannel.options.length;

			if ((CurrentCh >=1) && (CurrentCh <= 4))
			{
				x.options[0].text = ChannelList_24G[1*CurrentCh + 4 - 1];
				x.options[0].value = 1*CurrentCh + 4;
			}
			else if ((CurrentCh >= 5) && (CurrentCh <= 7))
			{
				x.options[0].text = ChannelList_24G[1*CurrentCh - 4 - 1];
				x.options[0].value = 0; //1*CurrentCh - 4;
				CurrentCh = 1*CurrentCh;
				CurrentCh += 4;
				CreateExtChannelOption(CurrentCh);
			}
			else if ((CurrentCh >= 8) && (CurrentCh <= 9))
			{
				x.options[0].text = ChannelList_24G[1*CurrentCh - 4 - 1];
				x.options[0].value = 0; //1*CurrentCh - 4;

				if (option_length >=14)
				{
					CurrentCh = 1*CurrentCh;
					CurrentCh += 4;
					CreateExtChannelOption(CurrentCh);
				}
			}
			else if (CurrentCh == 10)
			{
				x.options[0].text = ChannelList_24G[1*CurrentCh - 4 - 1];
				x.options[0].value = 0; //1*CurrentCh - 4;

				if (option_length > 14)
				{
					CurrentCh = 1*CurrentCh;
					CurrentCh += 4;
					CreateExtChannelOption(CurrentCh);
				}
			}
			else if (CurrentCh >= 11)
			{
				x.options[0].text = ChannelList_24G[1*CurrentCh - 4 - 1];
				x.options[0].value = 0; //1*CurrentCh - 4;
			}
			else
			{
				x.options[0].text = "Auto Select";
				x.options[0].value = 0;
			}
		}
	}
 */
}

function ChannelOnChange()
{
/*
	if (document.wireless_basic.n_bandwidth[1].checked == true)
	{
		var w_mode = document.wireless_basic.wirelessmode.options.selectedIndex;

		if (1*w_mode >= 3)
		{
			if (document.wireless_basic.n_bandwidth[1].checked == true)
			{
				//document.getElementById("extension_channel").style.visibility = "visible";
				//document.getElementById("extension_channel").style.display = style_display_on();
				//document.wireless_basic.n_extcha.disabled = false;
			}

			if (document.wireless_basic.sz11gChannel.options.selectedIndex == 0)
			{
				//document.getElementById("extension_channel").style.visibility = "hidden";
				//document.getElementById("extension_channel").style.display = "none";
				//document.wireless_basic.n_extcha.disabled = true;
			}
		}
	}

	insertExtChannelOption();
 */
}

function Channel_BandWidth_onClick()
{
	//var w_mode = document.wireless_basic.wirelessmode.options.selectedIndex;

	//if (document.wireless_basic.n_bandwidth[0].checked == true)
	//{
		//document.getElementById("extension_channel").style.visibility = "hidden";
		//document.getElementById("extension_channel").style.display = "none";
		//document.wireless_basic.n_extcha.disabled = true;
		/*if (1*w_mode >= 3)
			Check5GBandChannelException();*/
	//}
	//else
	//{
		//document.getElementById("extension_channel").style.visibility = "visible";
		//document.getElementById("extension_channel").style.display = style_display_on();
		//document.wireless_basic.n_extcha.disabled = false;

		/*if (1*w_mode >= 3)
		{
			Check5GBandChannelException();

			if (document.wireless_basic.sz11aChannel.options.selectedIndex == 0)
			{
				document.getElementById("extension_channel").style.visibility = "hidden";
				document.getElementById("extension_channel").style.display = "none";
				document.wireless_basic.n_extcha.disabled = true;
			}
		}*/
	//}
}

function Check5GBandChannelException()
{
	var w_mode = document.wireless_basic.wirelessmode.options.selectedIndex;

	if (1*w_mode >= 3)
	{
		var x = document.getElementById("sz11aChannel")
		var current_length = document.wireless_basic.sz11aChannel.options.length;
		var current_index = document.wireless_basic.sz11aChannel.options.selectedIndex;
		var current_channel = document.wireless_basic.sz11aChannel.value;
		 
		if (1*current_index == 0)
		{
			if (1*channel_index != 0)
				current_index = 1;
		}

		for (ch_idx = current_length - 1; ch_idx > 0; ch_idx--)
		{
			x.remove(ch_idx);
		}

		if (document.wireless_basic.n_bandwidth[1].checked == true)
		{
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
				for(ch = 100; ch <= 136; ch+=4)
					insertChannelOption(ch, 5);
			}

			if ((countrycode == 'NONE') || (countrycode == 'US') || (countrycode == 'TW') ||
				(countrycode == 'CN') || (countrycode == 'HK'))
			{
				for(ch = 149; ch <= 161; ch+=4)
					insertChannelOption(ch, 5);
			}

			if ((1*current_channel == 140) || (1*current_channel == 165))
			{
				document.wireless_basic.sz11aChannel.options.selectedIndex = (1*current_index) -1;
			}
			else
			{
				document.wireless_basic.sz11aChannel.options.selectedIndex = (1*current_index);
			}
		}
		else
		{
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

}

function initTranslation()
{
	var e = document.getElementById("basicTitle");
	e.innerHTML = _("basic title");
	e = document.getElementById("basicIntroduction");
	e.innerHTML = _("basic introduction");

	e = document.getElementById("basicWirelessNet");
	e.innerHTML = _("basic wireless network");
	e = document.getElementById("basicRadioButton");
	e.innerHTML = _("basic radio button");
	e = document.getElementById("basicNetMode");
	e.innerHTML = _("basic network mode");
	e = document.getElementById("basicSSID");
	e.innerHTML = _("basic ssid");
	e = document.getElementById("basicHSSID0");
	e.innerHTML = _("basic hssid");
	e = document.getElementById("basicHSSID1");
	e.innerHTML = _("basic hssid");
	e = document.getElementById("basicHSSID2");
	e.innerHTML = _("basic hssid");
	e = document.getElementById("basicHSSID3");
	e.innerHTML = _("basic hssid");
	e = document.getElementById("basicHSSID4");
	e.innerHTML = _("basic hssid");
	e = document.getElementById("basicHSSID5");
	e.innerHTML = _("basic hssid");
	e = document.getElementById("basicHSSID6");
	e.innerHTML = _("basic hssid");
	e = document.getElementById("basicHSSID7");
	e.innerHTML = _("basic hssid");
	e = document.getElementById("basicMSSID1");
	e.innerHTML = _("basic multiple ssid");
	e = document.getElementById("basicMSSID2");
	e.innerHTML = _("basic multiple ssid");
	e = document.getElementById("basicMSSID3");
	e.innerHTML = _("basic multiple ssid");
	e = document.getElementById("basicMSSID4");
	e.innerHTML = _("basic multiple ssid");
	e = document.getElementById("basicMSSID5");
	e.innerHTML = _("basic multiple ssid");
	e = document.getElementById("basicMSSID6");
	e.innerHTML = _("basic multiple ssid");
	e = document.getElementById("basicMSSID7");
	e.innerHTML = _("basic multiple ssid");
	e = document.getElementById("basicBroadcastSSIDEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("basicBroadcastSSIDDisable");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("basicBroadcastSSID");
	e.innerHTML = _("basic broadcast ssid");
	e = document.getElementById("basicApIsolatedEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("basicApIsolatedDisable");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("basicApIsolated");
	e.innerHTML = _("basic apisolated");
	e = document.getElementById("basicIsolatedSSID0");
	e.innerHTML = _("basic isolated");
	e = document.getElementById("basicIsolatedSSID1");
	e.innerHTML = _("basic isolated");
	e = document.getElementById("basicIsolatedSSID2");
	e.innerHTML = _("basic isolated");
	e = document.getElementById("basicIsolatedSSID3");
	e.innerHTML = _("basic isolated");
	e = document.getElementById("basicIsolatedSSID4");
	e.innerHTML = _("basic isolated");
	e = document.getElementById("basicIsolatedSSID5");
	e.innerHTML = _("basic isolated");
	e = document.getElementById("basicIsolatedSSID6");
	e.innerHTML = _("basic isolated");
	e = document.getElementById("basicIsolatedSSID7");
	e.innerHTML = _("basic isolated");
	e = document.getElementById("basicMBSSIDApIsolatedEnable");
	e.innerHTML = _("wireless enable");
	e = document.getElementById("basicMBSSIDApIsolatedDisable");
	e.innerHTML = _("wireless disable");
	e = document.getElementById("basicMBSSIDApIsolated");
	e.innerHTML = _("basic mbssidapisolated");
	e = document.getElementById("basicBSSID");
	e.innerHTML = _("basic bssid");
	e = document.getElementById("basicFreqA");
	e.innerHTML = _("basic frequency");
	e = document.getElementById("basicFreqAAuto");
	e.innerHTML = _("basic frequency auto");
	e = document.getElementById("basicFreqB");
	e.innerHTML = _("basic frequency");
	e = document.getElementById("basicFreqBAuto");
	e.innerHTML = _("basic frequency auto");
	e = document.getElementById("basicFreqG");
	e.innerHTML = _("basic frequency");
	e = document.getElementById("basicFreqGAuto");
	e.innerHTML = _("basic frequency auto");
	e = document.getElementById("basicRate");
	e.innerHTML = _("basic rate");

	e = document.getElementById("basicHTPhyMode");
	e.innerHTML = _("basic ht phy mode");
	e = document.getElementById("basicHTOPMode");
	e.innerHTML = _("basic ht op mode");
    	/*e = document.getElementById("basicHTMixed");
	e.innerHTML = _("basic ht op mixed");*/
/*
    	e = document.getElementById("basicHTChannelBW");
	e.innerHTML = _("basic ht channel bandwidth");
    	e = document.getElementById("basicHTGI");
	e.innerHTML = _("basic ht guard interval");
      	e = document.getElementById("basicHTLongGI");
	e.innerHTML = _("wireless long");
      	e = document.getElementById("basicHTAutoGI");
	e.innerHTML = _("wireless auto");
    	e = document.getElementById("basicHTAutoMCS");
	e.innerHTML = _("wireless auto");
    	e = document.getElementById("basicHTRDG");
	e.innerHTML = _("basic ht rdg");
    	e = document.getElementById("basicHTRDGDisable");
	e.innerHTML = _("wireless disable");
    	e = document.getElementById("basicHTRDGEnable");
	e.innerHTML = _("wireless enable");
    	e = document.getElementById("basicHTExtChannel");
	e.innerHTML = _("basic ht extension channel");
    	e = document.getElementById("basicHTSTBC");
 */
	e.innerHTML = _("basic ht stbc");
    	e = document.getElementById("basicHTSTBCDisable");
	e.innerHTML = _("wireless disable");
    	e = document.getElementById("basicHTSTBCEnable");
	e.innerHTML = _("wireless enable");
    	e = document.getElementById("basicHTAMSDU");
	e.innerHTML = _("basic ht amsdu");
    	e = document.getElementById("basicHTAMSDUDisable");
	e.innerHTML = _("wireless disable");
    	e = document.getElementById("basicHTAMSDUEnable");
	e.innerHTML = _("wireless enable");
/*
    	e = document.getElementById("basicHTAddBA");
	e.innerHTML = _("basic ht addba");
    	e = document.getElementById("basicHTAddBADisable");
	e.innerHTML = _("wireless disable");
    	e = document.getElementById("basicHTAddBAEnable");
	e.innerHTML = _("wireless enable");
    	e = document.getElementById("basicHTDelBA");
	e.innerHTML = _("basic ht delba");
    	e = document.getElementById("basicHTDelBADisable");
	e.innerHTML = _("wireless disable");
    	e = document.getElementById("basicHTDelBAEnable");
	e.innerHTML = _("wireless enable");
	//add multi lang. , minglin, 2009/01/27 *start*
		e = document.getElementById("basicHTAllowTKIP");
	e.innerHTML = _("basic ht allowtkip");
    	e = document.getElementById("basicHTAllowTKIPDisable");
	e.innerHTML = _("wireless disable");
    	e = document.getElementById("basicHTAllowTKIPEnable");
	e.innerHTML = _("wireless enable");
	//add multi lang. , minglin, 2009/01/27 *end*
    	e = document.getElementById("basicOther");
	e.innerHTML = _("basic other");
    	e = document.getElementById("basicHTTxStream");
	e.innerHTML = _("basic ht txstream");
    	e = document.getElementById("basicHTRxStream");
	e.innerHTML = _("basic ht rxstream");
 */
	
	e = document.getElementById("basicApply");
	e.value = _("wireless apply");
	e = document.getElementById("basicCancel");
	e.value = _("wireless cancel");
}

function initValue()
{
	var ssidArray;
	var HiddenSSIDArray;
	var channel_11a_index;
	var current_channel_length;
	var radio_off = '<% getCfgZero(1, "RadioOff"); %>';
	var mssidb = "<% getMBSSIDBuilt(); %>";

	initTranslation();
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
	document.getElementById("div_11n").style.display = "none";
	document.wireless_basic.n_mode.disabled = true;
	//document.wireless_basic.n_bandwidth.disabled = true;
	//document.wireless_basic.n_rdg.disabled = true;
	//document.wireless_basic.n_gi.disabled = true;
	//document.wireless_basic.n_mcs.disabled = true;
	//document.getElementById("div_11n_plugfest").style.display = "none";
	//document.wireless_basic.f_40mhz.disabled = true;

	PhyMode = 1*PhyMode;

	/*if ((PhyMode >= 5))
	{
		if (window.ActiveXObject) // IE
			document.getElementById("div_11n").style.display = "block";
		else if (window.XMLHttpRequest)  // Mozilla, Safari,...
			document.getElementById("div_11n").style.display = "table";
		document.wireless_basic.n_mode.disabled = false;
		document.wireless_basic.n_bandwidth.disabled = false;
		document.wireless_basic.n_rdg.disabled = false;
		document.wireless_basic.n_gi.disabled = false;
		document.wireless_basic.n_mcs.disabled = false;
		//document.getElementById("div_11n_plugfest").style.display = "block";
		//document.wireless_basic.f_40mhz.disabled = false;
	}*/

	if ((PhyMode == 0) || (PhyMode == 4))
	{
		if (PhyMode == 0)
			document.wireless_basic.wirelessmode.options.selectedIndex = 0;
		else if (PhyMode == 4)
			document.wireless_basic.wirelessmode.options.selectedIndex = 2;

		document.getElementById("div_11g_channel").style.visibility = "visible";
		document.getElementById("div_11g_channel").style.display = style_display_on();
		document.wireless_basic.sz11gChannel.disabled = false;
	}
	if ((PhyMode == 9)|| (PhyMode == 6)|| (PhyMode == 7))
	{
		if (PhyMode == 9)
			document.wireless_basic.wirelessmode.options.selectedIndex = 3;
		else if (PhyMode == 6)
			document.wireless_basic.wirelessmode.options.selectedIndex = 4;
		else if (PhyMode == 7)
			document.wireless_basic.wirelessmode.options.selectedIndex = 5;

		if (window.ActiveXObject) // IE
			document.getElementById("div_11n").style.display = "block";
		else if (window.XMLHttpRequest)  // Mozilla, Safari,...
			document.getElementById("div_11n").style.display = "table";
		document.wireless_basic.n_mode.disabled = false;
		//document.wireless_basic.n_bandwidth.disabled = false;
		//document.wireless_basic.n_rdg.disabled = false;
		//document.wireless_basic.n_gi.disabled = false;
		//document.wireless_basic.n_mcs.disabled = false;

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
	
	for (i=1;i<8;i++)
	{
		var ssid = eval("document.wireless_basic.mssid_"+i+".disabled");
		var div = eval("document.getElementById(\"div_hssid"+i+"\").style");

		div.visibility = "hidden";
		div.display = "none";
		ssid = true;
		document.wireless_basic.hssid[i].disabled = true;
	}
	if (mssidb == "1")
	{
		/*for (i=1;i<8;i++)*/
		for (i=1;i<2;i++)
		{
			var ssid = eval("document.wireless_basic.mssid_"+i+".disabled");
			var div = eval("document.getElementById(\"div_hssid"+i+"\").style");

			div.visibility = "visible";
			div.display = style_display_on();
			ssid = false;
			document.wireless_basic.hssid[i].disabled = false;
		}
	}

	if (HiddenSSID.indexOf("0") >= 0)
	{
		document.wireless_basic.broadcastssid[0].checked = true;
	}
	else
	{
		document.wireless_basic.broadcastssid[1].checked = true;
	}
	var HiddenSSIDArray = HiddenSSID.split(";");
	for (i=0;i<8;i++)
	{
		if (HiddenSSIDArray[i] == "1")
			document.wireless_basic.hssid[i].checked = true;
		else
			document.wireless_basic.hssid[i].checked = false;
	}

	if (APIsolated.indexOf("1") >= 0)
	{
		document.wireless_basic.apisolated[0].checked = true;
	}
	else
	{
		document.wireless_basic.apisolated[1].checked = true;
	}
	var APIsolatedArray = APIsolated.split(";");
	for (i=0;i<8;i++)
	{
		if (APIsolatedArray[i] == "1")
			document.wireless_basic.isolated_ssid[i].checked = true;
		else
			document.wireless_basic.isolated_ssid[i].checked = false;
	}

	if (1*ht_bw == 0)
	{
		//document.wireless_basic.n_bandwidth[0].checked = true;
		//document.getElementById("extension_channel").style.visibility = "hidden";
		//document.getElementById("extension_channel").style.display = "none";
		//document.wireless_basic.n_extcha.disabled = true;
	}
	else
	{
		//document.wireless_basic.n_bandwidth[1].checked = true;
		//document.getElementById("extension_channel").style.visibility = "visible";
		//document.getElementById("extension_channel").style.display = style_display_on();
		//document.wireless_basic.n_extcha.disabled = false;
	}

	channel_index = 1*channel_index;

	if ((PhyMode == 0) || (PhyMode == 4) || (PhyMode == 9) || (PhyMode == 6)|| (PhyMode == 7))
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


	//ABG Rate
	if (PhyMode == 0)
	{
		ht_mcs = 1*ht_mcs;
		document.wireless_basic.abg_rate.options.length = 0;
		document.wireless_basic.abg_rate.options[0] = new Option("Auto", "0");
		document.wireless_basic.abg_rate.options[1] = new Option("1 Mbps", "1");
		document.wireless_basic.abg_rate.options[2] = new Option("2 Mbps", "2");
		document.wireless_basic.abg_rate.options[3] = new Option("5.5 Mbps", "5");
		document.wireless_basic.abg_rate.options[4] = new Option("6 Mbps", "6");
		document.wireless_basic.abg_rate.options[5] = new Option("9 Mbps", "9");
		document.wireless_basic.abg_rate.options[6] = new Option("11 Mbps", "11");
		document.wireless_basic.abg_rate.options[7] = new Option("12 Mbps", "12");
		document.wireless_basic.abg_rate.options[8] = new Option("18 Mbps", "18");
		document.wireless_basic.abg_rate.options[9] = new Option("24 Mbps", "24");
		document.wireless_basic.abg_rate.options[10] = new Option("36 Mbps", "36");
		document.wireless_basic.abg_rate.options[11] = new Option("48 Mbps", "48");
		document.wireless_basic.abg_rate.options[12] = new Option("54 Mbps", "54");
		if (fxtxmode == "CCK" || fxtxmode == "cck") {
			if (ht_mcs == 0)
				document.wireless_basic.abg_rate.options.selectedIndex = 1;
			else if (ht_mcs == 1)
				document.wireless_basic.abg_rate.options.selectedIndex = 2;
			else if (ht_mcs == 2)
				document.wireless_basic.abg_rate.options.selectedIndex = 3;
			else if (ht_mcs == 3)
				document.wireless_basic.abg_rate.options.selectedIndex = 6;
			else
				document.wireless_basic.abg_rate.options.selectedIndex = 0;
		}
		else {
			if (ht_mcs == 0)
				document.wireless_basic.abg_rate.options.selectedIndex = 4;
			else if (ht_mcs == 1)
				document.wireless_basic.abg_rate.options.selectedIndex = 5;
			else if (ht_mcs == 2)
				document.wireless_basic.abg_rate.options.selectedIndex = 7;
			else if (ht_mcs == 3)
				document.wireless_basic.abg_rate.options.selectedIndex = 8;
			else if (ht_mcs == 4)
				document.wireless_basic.abg_rate.options.selectedIndex = 9;
			else if (ht_mcs == 5)
				document.wireless_basic.abg_rate.options.selectedIndex = 10;
			else if (ht_mcs == 6)
				document.wireless_basic.abg_rate.options.selectedIndex = 11;
			else if (ht_mcs == 7)
				document.wireless_basic.abg_rate.options.selectedIndex = 12;
			else
				document.wireless_basic.abg_rate.options.selectedIndex = 0;
		}
	}
	else if (PhyMode == 1)
	{
		ht_mcs = 1*ht_mcs;
		document.wireless_basic.abg_rate.options.length = 0;
		document.wireless_basic.abg_rate.options[0] = new Option("Auto", "0");
		document.wireless_basic.abg_rate.options[1] = new Option("1 Mbps", "1");
		document.wireless_basic.abg_rate.options[2] = new Option("2 Mbps", "2");
		document.wireless_basic.abg_rate.options[3] = new Option("5.5 Mbps", "5");
		document.wireless_basic.abg_rate.options[4] = new Option("11 Mbps", "11");
		if (ht_mcs == 0)
			document.wireless_basic.abg_rate.options.selectedIndex = 1;
		else if (ht_mcs == 1)
			document.wireless_basic.abg_rate.options.selectedIndex = 2;
		else if (ht_mcs == 2)
			document.wireless_basic.abg_rate.options.selectedIndex = 3;
		else if (ht_mcs == 3)
			document.wireless_basic.abg_rate.options.selectedIndex = 4;
		else
			document.wireless_basic.abg_rate.options.selectedIndex = 0;
	}
	else if (PhyMode == 4)
	{
		ht_mcs = 1*ht_mcs;
		document.wireless_basic.abg_rate.options.length = 0;
		document.wireless_basic.abg_rate.options[0] = new Option("Auto", "0");
		document.wireless_basic.abg_rate.options[1] = new Option("6 Mbps", "6");
		document.wireless_basic.abg_rate.options[2] = new Option("9 Mbps", "9");
		document.wireless_basic.abg_rate.options[3] = new Option("12 Mbps", "12");
		document.wireless_basic.abg_rate.options[4] = new Option("18 Mbps", "18");
		document.wireless_basic.abg_rate.options[5] = new Option("24 Mbps", "24");
		document.wireless_basic.abg_rate.options[6] = new Option("36 Mbps", "36");
		document.wireless_basic.abg_rate.options[7] = new Option("48 Mbps", "48");
		document.wireless_basic.abg_rate.options[8] = new Option("54 Mbps", "54");
			if (ht_mcs == 0)
				document.wireless_basic.abg_rate.options.selectedIndex = 1;
			else if (ht_mcs == 1)
				document.wireless_basic.abg_rate.options.selectedIndex = 2;
			else if (ht_mcs == 2)
				document.wireless_basic.abg_rate.options.selectedIndex = 3;
			else if (ht_mcs == 3)
				document.wireless_basic.abg_rate.options.selectedIndex = 4;
			else if (ht_mcs == 4)
				document.wireless_basic.abg_rate.options.selectedIndex = 5;
			else if (ht_mcs == 5)
				document.wireless_basic.abg_rate.options.selectedIndex = 6;
			else if (ht_mcs == 6)
				document.wireless_basic.abg_rate.options.selectedIndex = 7;
			else if (ht_mcs == 7)
				document.wireless_basic.abg_rate.options.selectedIndex = 8;
			else
				document.wireless_basic.abg_rate.options.selectedIndex = 0;
	}
	else
	{
		document.getElementById("div_abg_rate").style.visibility = "hidden";
		document.getElementById("div_abg_rate").style.display = "none";
		document.wireless_basic.abg_rate.disabled = true;
	}

	document.getElementById("div_mbssidapisolated").style.visibility = "hidden";
	document.getElementById("div_mbssidapisolated").style.display = "none";
	document.wireless_basic.mbssidapisolated.disabled = true;
	if (mssidb == "1")
	{
		document.getElementById("div_mbssidapisolated").style.visibility = "visible";
		document.getElementById("div_mbssidapisolated").style.display = style_display_on();
		document.wireless_basic.mbssidapisolated.disabled = false;
		if (mbssidapisolated == "1")
			document.wireless_basic.mbssidapisolated[0].checked = true;
		else
			document.wireless_basic.mbssidapisolated[1].checked = true;
	}

	insertExtChannelOption();

	if (1*ht_mode == 0)
	{
		document.wireless_basic.n_mode[0].checked = true;
	}
	else if (1*ht_mode == 1)
	{
		document.wireless_basic.n_mode[1].checked = true;
	}

	else if (1*ht_mode == 2)
	{
		document.wireless_basic.n_mode[2].checked = true;
	}

/*
	if (1*ht_gi == 0)
	{
		document.wireless_basic.n_gi[0].checked = true;
	}
	else if (1*ht_gi == 1)
	{
		document.wireless_basic.n_gi[1].checked = true;
	}
	else if (1*ht_gi == 2)
	{
		document.wireless_basic.n_gi[2].checked = true;
	}

	if (1*is3t3r == 1) {
		for (i = 16; i < 24; i++) {
			document.wireless_basic.n_mcs.options[i] = new Option(i, i);
		}
	}
	var mcs_length = document.wireless_basic.n_mcs.options.length;
	if (1*is3t3r == 1) {
		document.wireless_basic.n_mcs.options[mcs_length] = new Option("32", "32");
		mcs_length++;
		document.wireless_basic.n_mcs.options[mcs_length] = new Option("Auto", "33");
	}

	if (1*ht_mcs <= mcs_length-1)
		document.wireless_basic.n_mcs.options.selectedIndex = ht_mcs;
	else if (1*ht_mcs == 32)
		document.wireless_basic.n_mcs.options.selectedIndex = mcs_length-2;
	else if (1*ht_mcs == 33)
		document.wireless_basic.n_mcs.options.selectedIndex = mcs_length-1;
 */

/*
	if (1*ht_rdg == 0)
		document.wireless_basic.n_rdg[0].checked = true;
	else
		document.wireless_basic.n_rdg[1].checked = true;

	var option_length = document.wireless_basic.n_extcha.options.length;

	if (1*ht_extcha == 0)
	{
		if (option_length > 1)
			document.wireless_basic.n_extcha.options.selectedIndex = 0;
	}
	else if (1*ht_extcha == 1)
	{
		if (option_length > 1)
			document.wireless_basic.n_extcha.options.selectedIndex = 1;
	}
	else
	{
		document.wireless_basic.n_extcha.options.selectedIndex = 0;
	}

	if ((1*PhyMode == 9))
	{
		if (document.wireless_basic.sz11gChannel.options.selectedIndex == 0)
		{
			document.getElementById("extension_channel").style.visibility = "hidden";
			document.getElementById("extension_channel").style.display = "none";
			document.wireless_basic.n_extcha.disabled = true;
		}
	}
 */

	if (1*ht_stbc == 0)
		document.wireless_basic.n_stbc[0].checked = true;
	else
		document.wireless_basic.n_stbc[1].checked = true;

	if (1*ht_amsdu == 0)
		document.wireless_basic.n_amsdu[0].checked = true;
	else
		document.wireless_basic.n_amsdu[1].checked = true;

/*
	if (1*ht_autoba == 0)
		document.wireless_basic.n_autoba[0].checked = true;
	else
		document.wireless_basic.n_autoba[1].checked = true;

	if (1*ht_badecline == 0)
		document.wireless_basic.n_badecline[0].checked = true;
	else
		document.wireless_basic.n_badecline[1].checked = true;

	if (1*ht_disallow_tkip == 1)
		document.wireless_basic.n_disallow_tkip[1].checked = true;
	else
		document.wireless_basic.n_disallow_tkip[0].checked = true;
 */

	//if (1*ht_f_40mhz == 0)
		//document.wireless_basic.f_40mhz[0].checked = true;
	//else
		//document.wireless_basic.f_40mhz[1].checked = true;

	/*
	if (1*wifi_optimum == 0)
		document.wireless_basic.wifi_opt[0].checked = true;
	else
		document.wireless_basic.wifi_opt[1].checked = true;
	*/

	if (1*apcli_include == 1)
	{
		document.wireless_basic.mssid_7.disabled = true;
	}

/*
	if (1*is3t3r == 1) {
		document.wireless_basic.rx_stream.options[2] = new Option("3", "3");
		document.wireless_basic.tx_stream.options[2] = new Option("3", "3");
	}
	var txpath = '<% getCfgGeneral(1, "TXPath"); %>';
	var rxpath = '<% getCfgGeneral(1, "RXPath"); %>';
	if (txpath == "1")
	{
		document.getElementById("div_HtTx2Stream").style.visibility = "hidden";
		document.getElementById("div_HtTx2Stream").style.display = "none";
		tx_stream_idx = 1;
	}
	else
	{
		document.getElementById("div_HtTx2Stream").style.visibility = "visible";
		document.getElementById("div_HtTx2Stream").style.display = style_display_on();
	}
	if (rxpath == "1")
	{
		document.getElementById("div_HtRx2Stream").style.visibility = "hidden";
		document.getElementById("div_HtRx2Stream").style.display = "none";
		rx_stream_idx = 1;
	}
	else
	{
		document.getElementById("div_HtRx2Stream").style.visibility = "visible";
		document.getElementById("div_HtRx2Stream").style.display = style_display_on();
	}
	document.wireless_basic.rx_stream.options.selectedIndex = rx_stream_idx - 1;
	document.wireless_basic.tx_stream.options.selectedIndex = tx_stream_idx - 1;
 */

	if (1*radio_off == 1)
		document.wireless_basic.radioButton.value = _("wireless turn on");
		/*document.wireless_basic.radioButton.value = "Turn On";*/
	else
		document.wireless_basic.radioButton.value = _("wireless turn off");
		/*document.wireless_basic.radioButton.value = "Turn Off";*/

	//document.getElementById("div_11n_plugfest").style.visibility = "hidden";
	//document.getElementById("div_11n_plugfest").style.display = "none";
	
	for (i=0;i<8;i++)
	{
		document.getElementById("basicIsolatedSSID"+i).style.visibility = "hidden";
		document.getElementById("basicIsolatedSSID"+i).style.display = "none";
		document.getElementById("isolated_ssid"+i).style.visibility = "hidden";
		document.getElementById("isolated_ssid"+i).style.display = "none";
	}
	Channel_BandWidth_onClick();
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
	document.getElementById("div_abg_rate").style.visibility = "hidden";
	document.getElementById("div_abg_rate").style.display = "none";
	document.wireless_basic.abg_rate.disabled = true;
	document.getElementById("div_11n").style.display = "none";
	document.wireless_basic.n_mode.disabled = true;
	//document.wireless_basic.n_bandwidth.disabled = true;
	//document.wireless_basic.n_rdg.disabled = true;
	//document.wireless_basic.n_gi.disabled = true;
	//document.wireless_basic.n_mcs.disabled = true;
	//document.getElementById("div_11n_plugfest").style.display = "none";
	//document.wireless_basic.f_40mhz.disabled = true;

	wmode = document.wireless_basic.wirelessmode.options.selectedIndex;

	wmode = 1*wmode;
	if (wmode == 0) /*bg*/
	{
		document.wireless_basic.wirelessmode.options.selectedIndex = 0;
		document.getElementById("div_11g_channel").style.visibility = "visible";
		document.getElementById("div_11g_channel").style.display = style_display_on();
		document.wireless_basic.sz11gChannel.disabled = false;
	}
	else if (wmode == 1) /*b*/
	{
		document.wireless_basic.wirelessmode.options.selectedIndex = 1;
		document.getElementById("div_11b_channel").style.visibility = "visible";
		document.getElementById("div_11b_channel").style.display = style_display_on();
		document.wireless_basic.sz11bChannel.disabled = false;
	}
	else if (wmode == 2) /*g*/
	{
		document.wireless_basic.wirelessmode.options.selectedIndex = 2;
		document.getElementById("div_11g_channel").style.visibility = "visible";
		document.getElementById("div_11g_channel").style.display = style_display_on();
		document.wireless_basic.sz11gChannel.disabled = false;
	}
	else if ((wmode == 3)||(wmode == 4)||(wmode == 5)) /*bgn,n,gn*/
	{
		if (wmode == 3)
			document.wireless_basic.wirelessmode.options.selectedIndex = 3;
		else if (wmode == 4)
			document.wireless_basic.wirelessmode.options.selectedIndex = 4;
		else if (wmode == 5)
			document.wireless_basic.wirelessmode.options.selectedIndex = 5;

		document.getElementById("div_11g_channel").style.visibility = "visible";
		document.getElementById("div_11g_channel").style.display = style_display_on();
		document.wireless_basic.sz11gChannel.disabled = false;
		if (window.ActiveXObject) // IE
			document.getElementById("div_11n").style.display = "block";
		else if (window.XMLHttpRequest)  // Mozilla, Safari,...
			document.getElementById("div_11n").style.display = "table";
		document.wireless_basic.n_mode.disabled = false;
		//document.wireless_basic.n_bandwidth.disabled = false;
		//document.wireless_basic.n_rdg.disabled = false;
		//document.wireless_basic.n_gi.disabled = false;
		//document.wireless_basic.n_mcs.disabled = false;
		//document.getElementById("div_11n_plugfest").style.display = "block";
		//document.wireless_basic.f_40mhz.disabled = false;

		if (document.wireless_basic.sz11gChannel.options.selectedIndex == 0)
		{
			//document.getElementById("extension_channel").style.visibility = "hidden";
			//document.getElementById("extension_channel").style.display = "none";
			//document.wireless_basic.n_extcha.disabled = true;
		}

		insertExtChannelOption();
	}

	//ABG Rate
	if (wmode == 0)
	{
		ht_mcs = 1*ht_mcs;
		document.wireless_basic.abg_rate.options.length = 0;
		document.wireless_basic.abg_rate.options[0] = new Option("Auto", "0");
		document.wireless_basic.abg_rate.options[1] = new Option("1 Mbps", "1");
		document.wireless_basic.abg_rate.options[2] = new Option("2 Mbps", "2");
		document.wireless_basic.abg_rate.options[3] = new Option("5.5 Mbps", "5");
		document.wireless_basic.abg_rate.options[4] = new Option("6 Mbps", "6");
		document.wireless_basic.abg_rate.options[5] = new Option("9 Mbps", "9");
		document.wireless_basic.abg_rate.options[6] = new Option("11 Mbps", "11");
		document.wireless_basic.abg_rate.options[7] = new Option("12 Mbps", "12");
		document.wireless_basic.abg_rate.options[8] = new Option("18 Mbps", "18");
		document.wireless_basic.abg_rate.options[9] = new Option("24 Mbps", "24");
		document.wireless_basic.abg_rate.options[10] = new Option("36 Mbps", "36");
		document.wireless_basic.abg_rate.options[11] = new Option("48 Mbps", "48");
		document.wireless_basic.abg_rate.options[12] = new Option("54 Mbps", "54");
		if (fxtxmode == "CCK" || fxtxmode == "cck") {
			if (ht_mcs == 0)
				document.wireless_basic.abg_rate.options.selectedIndex = 1;
			else if (ht_mcs == 1)
				document.wireless_basic.abg_rate.options.selectedIndex = 2;
			else if (ht_mcs == 2)
				document.wireless_basic.abg_rate.options.selectedIndex = 3;
			else if (ht_mcs == 3)
				document.wireless_basic.abg_rate.options.selectedIndex = 6;
			else
				document.wireless_basic.abg_rate.options.selectedIndex = 0;
		}
		else {
			if (ht_mcs == 0)
				document.wireless_basic.abg_rate.options.selectedIndex = 4;
			else if (ht_mcs == 1)
				document.wireless_basic.abg_rate.options.selectedIndex = 5;
			else if (ht_mcs == 2)
				document.wireless_basic.abg_rate.options.selectedIndex = 7;
			else if (ht_mcs == 3)
				document.wireless_basic.abg_rate.options.selectedIndex = 8;
			else if (ht_mcs == 4)
				document.wireless_basic.abg_rate.options.selectedIndex = 9;
			else if (ht_mcs == 5)
				document.wireless_basic.abg_rate.options.selectedIndex = 10;
			else if (ht_mcs == 6)
				document.wireless_basic.abg_rate.options.selectedIndex = 11;
			else if (ht_mcs == 7)
				document.wireless_basic.abg_rate.options.selectedIndex = 12;
			else
				document.wireless_basic.abg_rate.options.selectedIndex = 0;
		}

		document.getElementById("div_abg_rate").style.visibility = "visible";
		document.getElementById("div_abg_rate").style.display = style_display_on();
		document.wireless_basic.abg_rate.disabled = false;
	}
	else if (wmode == 1)
	{
		ht_mcs = 1*ht_mcs;
		document.wireless_basic.abg_rate.options.length = 0;
		document.wireless_basic.abg_rate.options[0] = new Option("Auto", "0");
		document.wireless_basic.abg_rate.options[1] = new Option("1 Mbps", "1");
		document.wireless_basic.abg_rate.options[2] = new Option("2 Mbps", "2");
		document.wireless_basic.abg_rate.options[3] = new Option("5.5 Mbps", "5");
		document.wireless_basic.abg_rate.options[4] = new Option("11 Mbps", "11");
		if (ht_mcs == 0)
			document.wireless_basic.abg_rate.options.selectedIndex = 1;
		else if (ht_mcs == 1)
			document.wireless_basic.abg_rate.options.selectedIndex = 2;
		else if (ht_mcs == 2)
			document.wireless_basic.abg_rate.options.selectedIndex = 3;
		else if (ht_mcs == 3)
			document.wireless_basic.abg_rate.options.selectedIndex = 4;
		else
			document.wireless_basic.abg_rate.options.selectedIndex = 0;

		document.getElementById("div_abg_rate").style.visibility = "visible";
		document.getElementById("div_abg_rate").style.display = style_display_on();
		document.wireless_basic.abg_rate.disabled = false;
	}
	else if (wmode == 2)
	{
		ht_mcs = 1*ht_mcs;
		document.wireless_basic.abg_rate.options.length = 0;
		document.wireless_basic.abg_rate.options[0] = new Option("Auto", "0");
		document.wireless_basic.abg_rate.options[1] = new Option("6 Mbps", "6");
		document.wireless_basic.abg_rate.options[2] = new Option("9 Mbps", "9");
		document.wireless_basic.abg_rate.options[3] = new Option("12 Mbps", "12");
		document.wireless_basic.abg_rate.options[4] = new Option("18 Mbps", "18");
		document.wireless_basic.abg_rate.options[5] = new Option("24 Mbps", "24");
		document.wireless_basic.abg_rate.options[6] = new Option("36 Mbps", "36");
		document.wireless_basic.abg_rate.options[7] = new Option("48 Mbps", "48");
		document.wireless_basic.abg_rate.options[8] = new Option("54 Mbps", "54");
			if (ht_mcs == 0)
				document.wireless_basic.abg_rate.options.selectedIndex = 1;
			else if (ht_mcs == 1)
				document.wireless_basic.abg_rate.options.selectedIndex = 2;
			else if (ht_mcs == 2)
				document.wireless_basic.abg_rate.options.selectedIndex = 3;
			else if (ht_mcs == 3)
				document.wireless_basic.abg_rate.options.selectedIndex = 4;
			else if (ht_mcs == 4)
				document.wireless_basic.abg_rate.options.selectedIndex = 5;
			else if (ht_mcs == 5)
				document.wireless_basic.abg_rate.options.selectedIndex = 6;
			else if (ht_mcs == 6)
				document.wireless_basic.abg_rate.options.selectedIndex = 7;
			else if (ht_mcs == 7)
				document.wireless_basic.abg_rate.options.selectedIndex = 8;
			else
				document.wireless_basic.abg_rate.options.selectedIndex = 0;

		document.getElementById("div_abg_rate").style.visibility = "visible";
		document.getElementById("div_abg_rate").style.display = style_display_on();
		document.wireless_basic.abg_rate.disabled = false;
	}
	
}

function switch_hidden_ssid()
{
	if (document.wireless_basic.broadcastssid[0].checked == true)
		for (i=0;i<8;i++)
			document.wireless_basic.hssid[i].checked = false;
	else
		for (i=0;i<8;i++)
			document.wireless_basic.hssid[i].checked = true;
}

function switch_isolated_ssid()
{
	if (document.wireless_basic.apisolated[0].checked == true)
		for (i=0;i<8;i++)
			document.wireless_basic.isolated_ssid[i].checked = true;
	else
		for (i=0;i<8;i++)
			document.wireless_basic.isolated_ssid[i].checked = false;
}

function CheckValue()
{
	var wireless_mode;
	var submit_ssid_num;
	var channel_11a_index;

	if (document.wireless_basic.mssid_0.value == "")
	{
		alert(_("basic alert null ssid"));
		document.wireless_basic.ssid.focus();
		document.wireless_basic.ssid.select();
		return false;
	}

	submit_ssid_num = 1;

	for (i = 1; i < 8; i++)
	{
		if (eval("document.wireless_basic.mssid_"+i).value != "")
		{
			if (i == 7)
			{
				if (1*apcli_include == 0)
					submit_ssid_num++;
			}
			else
				submit_ssid_num++;
		}
	}
	/* check ssid is equal?, minglin, 2010/01/05 *start*/
	for(r = 0; r < submit_ssid_num ; r++){
		for(k = 0; k < submit_ssid_num ; k++){
			if((eval("document.wireless_basic.mssid_"+k).value == eval("document.wireless_basic.mssid_"+r).value) && (r != k)){
				alert(_("basic alert error! ssid names are equal"));
				eval("document.wireless_basic.mssid_"+k).focus();
				eval("document.wireless_basic.mssid_"+k).select();
				return false;
			}
		}
	}
	/* check ssid is equal?, minglin, 2010/01/05 *end*/
	document.wireless_basic.bssid_num.value = submit_ssid_num;
	
	sbutton_disable(sbuttonMax); 
	restartPage_block();
	
	return true;
}

function RadioStatusChange(rs)
{
	if (rs == 1) {
		/*document.wireless_basic.radioButton.value = "Turn Off";*/
		document.wireless_basic.radioButton.value = _("wireless turn off");
		document.wireless_basic.radiohiddenButton.value = 0;
	}
	else {
		/*document.wireless_basic.radioButton.value = "Turn On";*/
		document.wireless_basic.radioButton.value = _("wireless turn on");
		document.wireless_basic.radiohiddenButton.value = 1;
	}
}

function RadioOnChange()
{
	if (document.wireless_basic.radioButton.value == _("wireless turn off")) 
		RadioStatusChange(1); 
	else 
		RadioStatusChange(0);  
}

</script>
</head>


<body onLoad="initValue()" bgcolor="#FFFFFF">
<div align="center">
 <center>
<table class="body"><tr><td>

<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="basicTitle">Basic Wireless Settings</td>
</tr>
<tr>

<tr>
<td colspan="2">
<p class="head" id="basicIntroduction"> You could configure the minimum number of Wireless settings for communication, such as Network Name (SSID) and Channel. The Access Point can be set simply with only the minimum setting items. </p>
</td>
<tr>
</table>

<br>

<form method=post name=wireless_basic action="/goform/wirelessBasic" onSubmit="return CheckValue()">
<table width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="basicWirelessNet">Wireless Network</td>
  </tr>
  <tr> 
    <td class="head" id="basicRadioButton">Radio On/Off</td>
    <td>
      <!--<input type="button" name="radioButton" style="{width:120px;}" value="Turn On" id="basicRadioONOFF"
      onClick="if (this.value.indexOf('Turn Off') >= 0) RadioStatusChange(1); else RadioStatusChange(0); document.wireless_basic.submit();"> &nbsp; &nbsp;
      <input type=hidden name=radiohiddenButton value="2">-->
      <input type="button" name="radioButton" style="{width:120px;}" value="Turn On" id="basicRadioONOFF"
      onClick="RadioOnChange(); document.wireless_basic.submit();"> &nbsp; &nbsp;
      <input type=hidden name=radiohiddenButton value="2">
    </td>
  </tr>
  <tr> 
    <td class="head" id="basicNetMode">Network Mode</td>
    <td>
      <select name="wirelessmode" id="wirelessmode" size="1" onChange="wirelessModeChange()">
        <option value=0>11b/g mixed mode</option>
        <option value=1>11b only</option>
        <option value=4>11g only</option>
        <option value=9>11b/g/n mixed mode</option>
		<option value=6>11n only</option>
		<option value=7>11g/n mixed mode</option>
      </select>
      <!--
      <select name="wirelessmode" id="wirelessmode" size="1" onChange="wirelessModeChange()">
	<option value=0>802.11 b/g mixed mode</option>
	<option value=1>802.11 b only</option>
	<option value=2>802.11 a only</option>
	<option value=3>802.11 a/b/g mixed mode</option>
	<option value=4>802.11 g</option>
	<option value=5>802.11 a/b/g/n mixed mode</option>
	<option value=6>802.11 n only</option>
	<option value=7>802.11 g/n mixed mode</option>
	<option value=8>802.11 a/n mixed mode</option>
	<option value=9>802.11 b/g/n mixed mode</option>
      </select>
      -->
    </td>
  </tr>
  <input type="hidden" name="bssid_num" value="1">
  <tr> 
    <td class="head" id="basicSSID">Network Name(SSID)</td>
    <td>
      <input type=text name=mssid_0 size=20 maxlength=32 value="<% getCfgGeneral(1, "SSID1"); %>">
      &nbsp;&nbsp;<font id="basicHSSID0">Hidden</font>
      <input type=checkbox name=hssid value="0">
      &nbsp;&nbsp;<font id="basicIsolatedSSID0">Isolated</font>
      <input type=checkbox name=isolated_ssid id="isolated_ssid0" value="0">
    </td>
  </tr>
  <tr id="div_hssid1"> 
    <td class="head"><font id="basicMSSID1">Multiple SSID</font>1</td>
    <td>
      <input type=text name=mssid_1 size=20 maxlength=32 value="<% getCfgGeneral(1, "SSID2"); %>">
      &nbsp;&nbsp;<font id="basicHSSID1">Hidden</font>
      <input type=checkbox name=hssid value="1">
      &nbsp;&nbsp;<font id="basicIsolatedSSID1">Isolated</font>
      <input type=checkbox name=isolated_ssid id="isolated_ssid1" value="1">
    </td>
  </tr>
  <tr id="div_hssid2"> 
    <td class="head"><font id="basicMSSID2">Multiple SSID</font>2</td>
    <td>
      <input type=text name=mssid_2 size=20 maxlength=32 value="<% getCfgGeneral(1, "SSID3"); %>">
      &nbsp;&nbsp;<font id="basicHSSID2">Hidden</font>
      <input type=checkbox name=hssid value="2">
      &nbsp;&nbsp;<font id="basicIsolatedSSID2">Isolated</font>
      <input type=checkbox name=isolated_ssid id="isolated_ssid2" value="2">
    </td>
  </tr>
  <tr id="div_hssid3"> 
    <td class="head"><font id="basicMSSID3">Multiple SSID</font>3</td>
    <td><input type=text name=mssid_3 size=20 maxlength=32 value="<% getCfgGeneral(1, "SSID4"); %>">
      &nbsp;&nbsp;<font id="basicHSSID3">Hidden</font>
      <input type=checkbox name=hssid value="3">
      &nbsp;&nbsp;<font id="basicIsolatedSSID3">Isolated</font>
      <input type=checkbox name=isolated_ssid id="isolated_ssid3" value="3">
    </td>
  </tr>
  <tr id="div_hssid4"> 
    <td class="head"><font id="basicMSSID4">Multiple SSID</font>4</td>
    <td><input type=text name=mssid_4 size=20 maxlength=32 value="<% getCfgGeneral(1, "SSID5"); %>">
      &nbsp;&nbsp;<font id="basicHSSID4">Hidden</font>
      <input type=checkbox name=hssid value="4">
      &nbsp;&nbsp;<font id="basicIsolatedSSID4">Isolated</font>
      <input type=checkbox name=isolated_ssid id="isolated_ssid4" value="4">
    </td>
  </tr>
  <tr id="div_hssid5"> 
    <td class="head"><font id="basicMSSID5">Multiple SSID</font>5</td>
    <td><input type=text name=mssid_5 size=20 maxlength=32 value="<% getCfgGeneral(1, "SSID6"); %>">
      &nbsp;&nbsp;<font id="basicHSSID5">Hidden</font>
      <input type=checkbox name=hssid value="5">
      &nbsp;&nbsp;<font id="basicIsolatedSSID5">Isolated</font>
      <input type=checkbox name=isolated_ssid id="isolated_ssid5" value="5">
    </td>
  </tr>
  <tr id="div_hssid6"> 
    <td class="head"><font id="basicMSSID6">Multiple SSID</font>6</td>
    <td><input type=text name=mssid_6 size=20 maxlength=32 value="<% getCfgGeneral(1, "SSID7"); %>">
      &nbsp;&nbsp;<font id="basicHSSID6">Hidden</font>
      <input type=checkbox name=hssid value="6">
      &nbsp;&nbsp;<font id="basicIsolatedSSID6">Isolated</font>
      <input type=checkbox name=isolated_ssid id="isolated_ssid6" value="6">
    </td>
  </tr>
  <tr id="div_hssid7"> 
    <td class="head"><font id="basicMSSID7">Multiple SSID</font>7</td>
    <td><input type=text name=mssid_7 size=20 maxlength=32 value="<% getCfgGeneral(1, "SSID8"); %>">
      &nbsp;&nbsp;<font id="basicHSSID7">Hidden</font>
      <input type=checkbox name=hssid value="7">
      &nbsp;&nbsp;<font id="basicIsolatedSSID7">Isolated</font>
      <input type=checkbox name=isolated_ssid id="isolated_ssid7" value="7">
    </td>
  </tr>
  <tr> 
    <td class="head" id="basicBroadcastSSID">Broadcast Network Name (SSID)</td>
    <td>
      <input type=radio name=broadcastssid value="1" checked onClick="switch_hidden_ssid()"><font id="basicBroadcastSSIDEnable">Enable&nbsp;</font>
      <input type=radio name=broadcastssid value="0" onClick="switch_hidden_ssid()"><font id="basicBroadcastSSIDDisable">Disable</font>
    </td>
  </tr>
  <tr> 
    <td class="head" id="basicApIsolated">AP Isolation</td>
    <td>
      <input type=radio name=apisolated value="1" onClick="switch_isolated_ssid()"><font id="basicApIsolatedEnable">Enable&nbsp;</font>
      <input type=radio name=apisolated value="0" checked onClick="switch_isolated_ssid()"><font id="basicApIsolatedDisable">Disable</font>
    </td>
  </tr>
  <tr id="div_mbssidapisolated"> 
    <td class="head" id="basicMBSSIDApIsolated">MBSSID AP Isolation</td>
    <td>
      <input type=radio name=mbssidapisolated value="1"><font id="basicMBSSIDApIsolatedEnable">Enable&nbsp;</font>
      <input type=radio name=mbssidapisolated value="0" checked><font id="basicMBSSIDApIsolatedDisable">Disable</font>
    </td>
  </tr>
  <tr> 
    <td class="head" id="basicBSSID">BSSID</td>
    <td>&nbsp;&nbsp;<% getWlanCurrentMac(); %></td>
  </tr>
  <tr id="div_11a_channel" name="div_11a_channel">
    <td class="head"><font id="basicFreqA">Frequency (Channel)</font></td>
    <td>
      <select id="sz11aChannel" name="sz11aChannel" size="1" onChange="ChannelOnChange()">
	<option value=0 id="basicFreqAAuto">AutoSelect</option>
	<% getWlan11aChannels(); %>
      </select>
    </td>
  </tr>
  <tr id="div_11b_channel" name="div_11b_channel">
    <td class="head"><font id="basicFreqB">Frequency (Channel)</font></td>
    <td>
      <select id="sz11bChannel" name="sz11bChannel" size="1" onChange="ChannelOnChange()">
	<option value=0 id="basicFreqBAuto">AutoSelect</option>
	<% getWlan11bChannels(); %>
      </select>
    </td>
  </tr>
  <tr id="div_11g_channel" name="div_11g_channel">
    <td class="head"><font id="basicFreqG">Frequency (Channel)</font></td>
    <td>
      <select id="sz11gChannel" name="sz11gChannel" size="1" onChange="ChannelOnChange()">
	<option value=0 id="basicFreqGAuto">AutoSelect</option>
	<% getWlan11gChannels(); %>
      </select>
    </td>
  </tr>
  <tr id="div_abg_rate">
    <td class="head"><font id="basicRate">Rate</font></td>
    <td>
      <select name="abg_rate" size="1">
      </select>
    </td>
  </tr>
</table>
<br>
<table id="div_11n" name="div_11n" width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD" style="display:none">
  <tr> 
    <td class="title" colspan="2" id="basicHTPhyMode">HT Physical Mode</td>
  </tr>
  <tr style="display:none">
    <td class="head" id="basicHTOPMode">Operating Mode</td>
    <td>
      <input type=radio name=n_mode value="0" checked><font id="basicHTMixed">Neighbor Friendly</font>
      <input type=radio name=n_mode value="1">Greenfield
    </td>
  </tr>
<!--
  <tr>
    <td class="head" id="basicHTChannelBW">Channel BandWidth</td>
    <td>
      <input type=radio name=n_bandwidth value="0" onClick="Channel_BandWidth_onClick()" checked>20&nbsp;
      <input type=radio name=n_bandwidth value="1" onClick="Channel_BandWidth_onClick()">20/40
    </td>
  </tr>
  <tr>
    <td class="head" id="basicHTGI">Guard Interval</td>
    <td>
      <input type=radio name=n_gi value="0" checked><font id="basicHTLongGI">long&nbsp;</font>
      <input type=radio name=n_gi value="1"><font id="basicHTAutoGI">Auto</font>
    </td>
  </tr>
  <tr>
    <td class="head">MCS</td>
    <td>
      <select name="n_mcs" size="1">
	<option value = 0>0</option>
	<option value = 1>1</option>
	<option value = 2>2</option>
	<option value = 3>3</option>
	<option value = 4>4</option>
	<option value = 5>5</option>
	<option value = 6>6</option>
	<option value = 7>7</option>
	<option value = 8>8</option>
	<option value = 9>9</option>
	<option value = 10>10</option>
	<option value = 11>11</option>
	<option value = 12>12</option>
	<option value = 13>13</option>
	<option value = 14>14</option>
	<option value = 15>15</option>
	<option value = 32>32</option>
	<option value = 33 selected id="basicHTAutoMCS">Auto</option>
      </select>
    </td>
  </tr>
  <tr>
    <td class="head" id="basicHTRDG">Reverse Direction Grant(RDG)</td>
    <td>
      <input type=radio name=n_rdg value="0" checked><font id="basicHTRDGDisable">Disable&nbsp;</font>
      <input type=radio name=n_rdg value="1"><font id="basicHTRDGEnable">Enable</font>
    </td>
  </tr>
  <tr name="extension_channel" id="extension_channel">
    <td class="head" id="basicHTExtChannel">Extension Channel</td>
    <td>
      <select id="n_extcha" name="n_extcha" size="1">
	<option value=1 selected>2412MHz (Channel 1)</option>
      </select>
    </td>
  </tr>
-->
  <tr>
    <td class="head" id="basicHTSTBC">STBC</td>
    <td>
      <input type=radio name=n_stbc value="0" checked><font id="basicHTSTBCDisable">Disable&nbsp;</font>
      <input type=radio name=n_stbc value="1"><font id="basicHTSTBCEnable">Enable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="basicHTAMSDU">Aggregation MSDU(A-MSDU)</td>
    <td>
      <input type=radio name=n_amsdu value="0" checked><font id="basicHTAMSDUDisable">Disable&nbsp;</font>
      <input type=radio name=n_amsdu value="1"><font id="basicHTAMSDUEnable">Enable</font>
    </td>
  </tr>
<!--
  <tr>
    <td class="head" id="basicHTAddBA">Auto Block ACK</td>
    <td>
      <input type=radio name=n_autoba value="0" checked><font id="basicHTAddBADisable">Disable&nbsp;</font>
      <input type=radio name=n_autoba value="1"><font id="basicHTAddBAEnable">Enable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="basicHTDelBA">Decline BA Request</td>
    <td>
      <input type=radio name=n_badecline value="0" checked><font id="basicHTDelBADisable">Disable&nbsp;</font>
      <input type=radio name=n_badecline value="1"><font id="basicHTDelBAEnable">Enable</font>
    </td>
  </tr>
  <tr>
    <td class="head" id="basicHTAllowTKIP">HT Disallow TKIP</td>
    <td>
      <input type=radio name=n_disallow_tkip value="0" checked><font id="basicHTAllowTKIPDisable">Disable&nbsp;</font>
      <input type=radio name=n_disallow_tkip value="1"><font id="basicHTAllowTKIPEnable">Enable</font>
    </td>
  </tr>
-->
</table>
<br>
<!-- <table id="div_11n_plugfest" name="div_11n_plugfest" width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD" style="display:none">-->
<!--
<table id="div_11n_plugfest" name="div_11n_plugfest" width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="2" id="basicOther">Other</td>
  </tr>
  <tr>
    <td width="45%" bgcolor="#E8F8FF" nowrap>40 Mhz Intolerant</td>
    <td bgcolor="#FFFFFF"><font color="#003366" face=arial><b>
      <input type=radio name=f_40mhz value="0" checked>Diable&nbsp;
      <input type=radio name=f_40mhz value="1">Enable
    </b></font></td>
  </tr>
  <tr>
    <td class="head">WiFi Optimum</td>
    <td>
      <input type=radio name=wifi_opt value="0" checked>Diable&nbsp;
      <input type=radio name=wifi_opt value="1">Enable
    </td>
  </tr>
  <tr>
    <td class="head" id="basicHTTxStream">HT TxStream</td>
    <td>
      <select name="tx_stream" size="1">
	<option value = 1>1</option>
	<option value = 2 id="div_HtTx2Stream">2</option>
      </select>
    </td>
  </tr>
  <tr>
    <td class="head" id="basicHTRxStream">HT RxStream</td>
    <td>
      <select name="rx_stream" size="1">
	<option value = 1>1</option>
	<option value = 2 id="div_HtRx2Stream">2</option>
      </select>
    </td>
  </tr>
</table>
-->
<br />

<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr id="sbutton0" align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply" id="basicApply" > &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="basicCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>  

</td></tr></table>
 </center>
</div>
</body>
</html>
