<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=UTF-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<script type="text/javascript" src="/common.js"></script>

<title>System Management</title>

<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("admin");

restartPage_init();

var sbuttonMax=5;

var greenapb = '<% getGAPBuilt(); %>';
var http_request = false;

// Accelecon hack to limit choices:

	greenapb = "0";

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

function makeRequest(url, content) {
    http_request = false;
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_request = new XMLHttpRequest();
        if (http_request.overrideMimeType) {
            http_request.overrideMimeType('text/xml');
        }
    } else if (window.ActiveXObject) { // IE
        try {
            http_request = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_request = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_request) {
        alert(_('admin giving up cannot create an XMLHTTP instance'));
        return false;
    }
    http_request.onreadystatechange = alertContents;
    http_request.open('POST', url, true);
    http_request.send(content);
	
	sbutton_disable(sbuttonMax); 
	restartPage_block(); 
}

function alertContents() {
    if (http_request.readyState == 4) {
        if (http_request.status == 200) {
			// refresh
			window.location.reload();
        } else {
            alert(_('admin there was a problem with the request'));
        }
    }
}



function atoi(str, num)
{
    i=1;
    if(num != 1 ){
        while (i != num && str.length != 0){
            if(str.charAt(0) == '.'){
                i++;
            }
            str = str.substring(1);
        }
        if(i != num )
            return -1;
    }

    for(i=0; i<str.length; i++){
        if(str.charAt(i) == '.'){
            str = str.substring(0, i);
            break;
        }
    }
    if(str.length == 0)
        return -1;
    return parseInt(str, 10);
}

function isAllNum(str)
{
	for (var i=0; i<str.length; i++){
	    if((str.charAt(i) >= '0' && str.charAt(i) <= '9') || (str.charAt(i) == '.' ))
			continue;
		return 0;
	}
	return 1;
}

function is_valid_account(str)
{
	var num = "0123456789";
	var ascii = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	var extra = "_-.@";
    var i;
    var valid_string = num + ascii + extra;
    if (
        str.indexOf("..") > -1 ||
        str.substr(0,1) == '.' ||
        str.substr(-1,1) == '.'
        ) {
        alert(_('admin invalid account format'));
        return false;
    }
    if (str.length > 256)
    {
        alert(_('admin account is too long'));
        return false;
    }
    for (i = 0; i < str.length; i++)
    {
        if (!is_contained_char(str[i], valid_string))
        {
            alert(_('admin account contains illegal character'));
            return false;
        }
    }
    return true;
}
function AdmFormCheck()
{
	if (document.Adm.admuser.value == "") {
		alert(_('admin please specify the administrator account'));
		return false;
	}
	
	if (document.Adm.admpass.value == "") {
		alert(_('admin please specify the administrator password'));
		return false;
	}
	else if (document.Adm.admpass.value == "****") {
		return false;
	}
	else
	{
		if ((document.Adm.admpass.value.indexOf('*') !=-1)
		)
		{
			alert(_('admin password contains illegal character'));
			document.Adm.admpass.focus();
			return false;
		}
	}
	
	sbutton_disable(sbuttonMax); 
	restartPage_block(); 
	
	return true;
}

function NTPFormCheck()
{
    if(	document.NTP.time_zone.options.selectedIndex == 98 )
	{
		alert(_('admin please select your time zone settings'));
		document.NTP.time_zone.focus();
		return false;
	}
	if(document.NTP.NTPServerIP.value != ""){
		if (!is_valid_domainname(document.NTP.NTPServerIP.value)){
			alert(_('admin not a valid domain name'));
			document.NTP.NTPServerIP.focus();
			document.NTP.NTPServerIP.select();
			return false;
		}
		
		if (document.NTP.NTPSync.value == "" || document.NTP.NTPSync.value.indexOf(" ") != -1 )
		{
			alert(_('admin not a valid ntpsync'));
			document.NTP.NTPSync.focus();
			document.NTP.NTPSync.select();
			return false;
		} /* [Yian,2009/07/10] */
		
		if( isNaN(document.NTP.NTPSync.value) ||
			document.NTP.NTPSync.value < 0  ||
			document.NTP.NTPSync.value.indexOf('.') > -1 ||
			parseInt(document.NTP.NTPSync.value) > 300){
			alert(_('admin the synchronization value is not valid integer'));
			document.NTP.NTPSync.focus();
			document.NTP.NTPSync.select();
			return false;
		}
	}
	sbutton_disable(sbuttonMax); 
	restartPage_block(); 
		
	return true;
}

//kentchang, 2010-02-03
function GreenAPFormCheck()
{
	var shour = new Array();
	var ehour = new Array(); 
	var sminute = new Array(); 
	var eminute = new Array();
	var action = new Array(); 

	shour[0] = ehour[0] = sminute[0] = eminute[0] = action[0] = "99";	

    	for(var i=1; i<=4; i++){
	
		shour[i] = eval("document.GreenAP.GAPSHour"+i+".options.selectedIndex");
		ehour[i] = eval("document.GreenAP.GAPEHour"+i+".options.selectedIndex");
		sminute[i] = eval("document.GreenAP.GAPSMinute"+i+".options.selectedIndex");
		eminute[i] = eval("document.GreenAP.GAPEMinute"+i+".options.selectedIndex");
		action[i] = eval("document.GreenAP.GAPAction"+i+".value");
	
        	if(action[i] != "Disable"){
        		//SH > EH
	    		if(shour[i] > ehour[i]){
				alert(_("greenap duration")+i+_("greenap incorrect"));
				return false;
		  	}//SH = EH and SM >EM
		 	else if( shour[i] == ehour[i] && sminute[i] > eminute[i]){
			 	alert(_("greenap duration")+i+_("greenap incorrect"));
				return false;
                 	}   
		}
	}

	for(var j=1; j<=4; j++){
		if(action[j] != "Disable"){
			for(var k=1; k<=4; k++){					
				if(j != k && action[k] != "Disable"){					
					//SHj = EHj = SHk = EHk 	
				  	if(shour[j] == ehour[j] && shour[k] == ehour[k] && shour[j] == shour[k]) {
						//SMj <= SMk <= EMj
						if(sminute[j] <= sminute[k] && sminute[k] <= eminute[j]){
							alert(_("greenap duration")+j+","+k+_("greenap conflict"));
							return false;
						}
						//SMj <= EMk <= EMj
						if(sminute[j] <= eminute[k] && eminute[k] <= eminute[j]){
							alert(_("greenap duration")+j+","+k+_("greenap conflict"));
							return false;
						}
						//SMj < SMk and EMk <EMj
						if(sminute[j] < sminute[k] && eminute[k] < eminute[j]){
							alert(_("greenap duration")+j+","+k+_("greenap conflict"));
							return false;
						}
					}
				
					//SHj = EHj = SHk 	
				  	if(shour[j] == shour[k] && shour[k] == ehour[j] && shour[k] != ehour[k]){
						//SMj <= SMk <= EMj
						if(sminute[j] <= sminute[k] && sminute[k] <= eminute[j]){
							alert(_("greenap duration")+j+","+k+_("greenap conflict"));
							return false;
						}
						//SMk < SMj
						if(sminute[k] < sminute[j]){
							alert(_("greenap duration")+j+","+k+_("greenap conflict"));
							return false;
						}
		
				  	}
				  	//SHj = EHj = EHk    	
				  	if(shour[j] == ehour[k] && ehour[k] == ehour[j] && ehour[k] != shour[k]){
						//SMj <= EMk <= EMj
						if(sminute[j] <= eminute[k] && eminute[k] <= eminute[j]){
							alert(_("greenap duration")+j+","+k+_("greenap conflict"));
							return false;
						}
						//EMj < EMk
						if(eminute[j] < eminute[k]){
							alert(_("greenap duration")+j+","+k+_("greenap conflict"));
							return false;
						}
				    	}
				  	//SHk = EHk = SHj	
				  	if(shour[k] == shour[j] && shour[j] == ehour[k] && shour[j] != ehour[j]){
						//SMk <= SMj <= EMk
						if(sminute[k] <= sminute[j] && sminute[j] <= eminute[k]){
							alert(_("greenap duration")+j+","+k+_("greenap conflict"));
							return false;
						}
						//SMj < SMk
						if(sminute[j] < sminute[k]){
							alert(_("greenap duration")+j+","+k+_("greenap conflict"));
							return false;
						}
				    	}
				  	//SHk = EHk = EHj	
				  	if(shour[k] == ehour[j] && ehour[j] == ehour[k] && ehour[j] != shour[j]){
						//SMk <= EMj <= EMk
						if(sminute[k] <= eminute[j] && eminute[j] <= eminute[k]){
							alert(_("greenap duration")+j+","+k+_("greenap conflict"));
							return false;
						}
						//EMk < EMj
						if(eminute[k] < eminute[j]){
							alert(_("greenap duration")+j+","+k+_("greenap conflict"));
							return false;
						}
						
				    	}

					//SHj = SHk	
				  	if(shour[j] == shour[k] && shour[j] != ehour[j] && shour[k] != ehour[k]){
						alert(_("greenap duration")+j+","+k+_("greenap conflict"));
						return false;
				  	}
					//EHj = EHk	
				  	if(ehour[j] == ehour[k] && ehour[j] != shour[j] && ehour[k] != shour[k]){ 
						alert(_("greenap duration")+j+","+k+_("greenap conflict"));
						return false;
				  	}
					//EHj = SHk 	
				  	if(ehour[j] == shour[k] && ehour[j] != shour[j] && shour[k] != ehour[k]){
						//SMk <= EMj
						if(sminute[k] <= eminute[j]){
							alert(_("greenap duration")+j+","+k+_("greenap conflict"));
							return false;
						}
				  	}
					//SHj = EHk 	
				  	if(shour[j] == ehour[k] && shour[j] != ehour[j] && ehour[k] != shour[k]){
						//SMj <= EMk
						if(sminute[j] <= eminute[k]){
							alert(_("greenap duration")+j+","+k+_("greenap conflict"));
							return false;
						}
				  	}

				  	//SHj < SHk < EHj
				  	if(shour[j] < shour[k] && shour[k] < ehour[j]){
						alert(_("greenap duration")+j+","+k+_("greenap conflict"));
						return false;
				    	}				  
				  	//SHj < EHk < EHj
				  	if(shour[j] < ehour[k] && ehour[k] < ehour[j]){
						alert(_("greenap duration")+j+","+k+_("greenap conflict"));
						return false;
				    	}
				  	//SHk < SHj and EHj <EHk
				  	if(shour[k] < ehour[j] && ehour[j] < ehour[k]){
						alert(_("greenap duration")+j+","+k+_("greenap conflict"));
						return false;
				    	}								
				}
			}
		}
	}

 	sbutton_disable(sbuttonMax);
        restartPage_block();
	
	return true;
}

function DDNSFormCheck()
{	// minglin, 2010/01/05 *start*
	if(document.DDNS.DDNSProvider.value != "none"){
		if(document.DDNS.Account.value == "" ){
			alert(_('admin account password and ddns name cannot be empty'));
			document.DDNS.Account.focus();
			return false;
		}else if(document.DDNS.Password.value == ""){
			alert(_('admin account password and ddns name cannot be empty'));
			document.DDNS.Password.focus();
			return false;
		}else if(document.DDNS.DDNS.value == ""){
			alert(_('admin account password and ddns name cannot be empty'));
			document.DDNS.DDNS.focus();
			return false;
		}
	}
	if(!is_valid_domainname(document.DDNS.DDNS.value) || document.DDNS.DDNS.value.indexOf('.') == -1){
		alert(_('admin not a valid domain name is required'));
		document.DDNS.DDNS.focus();
		return false;
	}
	/*if() {
		alert(_('admin not a valid domain name is required'));
		document.DDNS.DDNS.focus();
		return false;
	}*/// minglin, 2010/01/05 *end*
	sbutton_disable(sbuttonMax); 
	restartPage_block(); 
	return true;
}

function disableTextField (field)
{
  if(document.all || document.getElementById){
    field.disabled = true;
  }else {
    field.oldOnFocus = field.onfocus;
    field.onfocus = skip;
  }
}

function enableTextField (field)
{
  if(document.all || document.getElementById)
    field.disabled = false;
  else {
    field.onfocus = field.oldOnFocus;
  }
}

function DDNSupdateState()
{
	if(document.DDNS.DDNSProvider.options.selectedIndex != 0){
		enableTextField(document.DDNS.Account);
		enableTextField(document.DDNS.Password);
		enableTextField(document.DDNS.DDNS);
		/* minglin, 2010/01/04 *start*/
		document.DDNS.manDdnsApply.disabled = false;
		document.DDNS.manDdnsCancel.disabled = false;
		/* minglin, 2010/01/04 *end*/
	}else{
		disableTextField(document.DDNS.Account);
		disableTextField(document.DDNS.Password);
		disableTextField(document.DDNS.DDNS);
		/* minglin, 2010/01/04 *start*/
		document.DDNS.manDdnsApply.disabled = true;
		document.DDNS.manDdnsCancel.disabled = true;
		/* minglin, 2010/01/04 *end*/
	}
}

function initTranslation()
{
	var e = document.getElementById("manTitle");
	e.innerHTML = _("man title");
	e = document.getElementById("manIntroduction");
	e.innerHTML = _("man introduction");
	e = document.getElementById("manLangApply");
	e.value = _("admin apply");
	e = document.getElementById("manLangCancel");
	e.value = _("admin cancel");

	e = document.getElementById("manLangSet");
	e.innerHTML = _("man language setting");
	e = document.getElementById("manSelectLang");
	e.innerHTML = _("man select language");
	e = document.getElementById("manAdmSet");
	e.innerHTML = _("man admin setting");
	e = document.getElementById("manAdmAccount");
	e.innerHTML = _("man admin account");
	e = document.getElementById("manAdmPasswd");
	e.innerHTML = _("man admin passwd");
	e = document.getElementById("manAdmApply");
	e.value = _("admin apply");
	e = document.getElementById("manAdmCancel");
	e.value = _("admin cancel");

	e = document.getElementById("manNTPSet");
	e.innerHTML = _("man ntp setting");
	e = document.getElementById("manNTPTimeZone");
	e.innerHTML = _("man ntp timezone");
	
	e = document.getElementById("manntpinterdateline");
	e.innerHTML = _("man ntp International Date Line");
	e = document.getElementById("manntpmidisland");
	e.innerHTML = _("man ntp mid island");
	e = document.getElementById("manntpAdak");
	e.innerHTML = _("man ntp Adak");
	e = document.getElementById("manntphawaii");
	e.innerHTML = _("man ntp hawaii");
	e = document.getElementById("manntpalaska");
	e.innerHTML = _("man ntp alaska");
	e = document.getElementById("manntppacific");
	e.innerHTML = _("man ntp pacific");
	e = document.getElementById("manntparizona");
	e.innerHTML = _("man ntp arizona");
	e = document.getElementById("manntpChihuahua");
	e.innerHTML = _("man ntp Chihuahua");
	e = document.getElementById("manntpmountain");
	e.innerHTML = _("man ntp mountain");
	e = document.getElementById("manntpmidus");
	e.innerHTML = _("man ntp mid us");
	e = document.getElementById("manntpcentral");
	e.innerHTML = _("man ntp central");
	e = document.getElementById("manntpGuadalajara");
	e.innerHTML = _("man ntp Guadalajara");
	e = document.getElementById("manntpGuatemala");
	e.innerHTML = _("man ntp Guatemala");
	e = document.getElementById("manntpSaskatchewan");
	e.innerHTML = _("man ntp Saskatchewan");
	e = document.getElementById("manntpBogota");
	e.innerHTML = _("man ntp Bogota");
	e = document.getElementById("manntpCubaStandardTime");
	e.innerHTML = _("man ntp Cuba Standard Time");
	e = document.getElementById("manntpeastern");
	e.innerHTML = _("man ntp eastern");
	e = document.getElementById("manntpVenezuela");
	e.innerHTML = _("man ntp Venezuela");
	e = document.getElementById("manntpAtlanticTime");
	e.innerHTML = _("man ntp Atlantic Time");
	e = document.getElementById("manntpFalklandIslands");
	e.innerHTML = _("man ntp Falkland Islands");
	e = document.getElementById("manntpLaPaz");
	e.innerHTML = _("man ntp La Paz");
	e = document.getElementById("manntpParaguay");
	e.innerHTML = _("man ntp Paraguay");
	e = document.getElementById("manntpSanLuisArgentina");
	e.innerHTML = _("man ntp San Luis Argentina");
	e = document.getElementById("manntpSantiago");
	e.innerHTML = _("man ntp Santiago");
	e = document.getElementById("manntpNewfoundland");
	e.innerHTML = _("man ntp Newfoundland");
	e = document.getElementById("manntpArgentina");
	e.innerHTML = _("man ntp Argentina");
	e = document.getElementById("manntpBrasilia");
	e.innerHTML = _("man ntp Brasilia");
	e = document.getElementById("manntpGeorgetown");
	e.innerHTML = _("man ntp Georgetown");
	e = document.getElementById("manntpGreenland");
	e.innerHTML = _("man ntp Greenland");
	e = document.getElementById("manntpStPierre");
	e.innerHTML = _("man ntp St Pierre");
	e = document.getElementById("manntpUruguay");
	e.innerHTML = _("man ntp Uruguay");
	e = document.getElementById("manntpmidatlan");
	e.innerHTML = _("man ntp Mid Atlantic");
	e = document.getElementById("manntpazores");
	e.innerHTML = _("man ntp azores");
	e = document.getElementById("manntpCape");
	e.innerHTML = _("man ntp Cape");
	e = document.getElementById("manntpDublin");
	e.innerHTML = _("man ntp Dublin");
	e = document.getElementById("manntpGreenwich");
	e.innerHTML = _("man ntp Greenwich");
	e = document.getElementById("manntpMorocco");
	e.innerHTML = _("man ntp Morocco");
	e = document.getElementById("manntpAmsterdam");
	e.innerHTML = _("man ntp Amsterdam");
	e = document.getElementById("manntpBelgrade");
	e.innerHTML = _("man ntp Belgrade");
	e = document.getElementById("manntpBrussels");
	e.innerHTML = _("man ntp Brussels");
	e = document.getElementById("manntpNamibia");
	e.innerHTML = _("man ntp Namibia");
	e = document.getElementById("manntpSarajevo");
	e.innerHTML = _("man ntp Sarajevo");
	e = document.getElementById("manntpWestCentralAfrica");
	e.innerHTML = _("man ntp West Central Africa");
	e = document.getElementById("manntpAthens");
	e.innerHTML = _("man ntp Athens");
	e = document.getElementById("manntpBeirutAthens");
	e.innerHTML = _("man ntp BeirutAthens");
	e = document.getElementById("manntpBucharest");
	e.innerHTML = _("man ntp Bucharest");
	e = document.getElementById("manntpCairo");
	e.innerHTML = _("man ntp Cairo");
	e = document.getElementById("manntpHarare");
	e.innerHTML = _("man ntp Harare");
	e = document.getElementById("manntpHelsinki");
	e.innerHTML = _("man ntp Helsinki");
	e = document.getElementById("manntpJordan");
	e.innerHTML = _("man ntp Jordan");
	e = document.getElementById("manntpSyria");
	e.innerHTML = _("man ntp Syria");
	e = document.getElementById("manntpTelAviv");
	e.innerHTML = _("man ntp Tel Aviv");
	e = document.getElementById("manntpTurkey");
	e.innerHTML = _("man ntp Turkey");
	e = document.getElementById("manntpBaghdad");
	e.innerHTML = _("man ntp Baghdad");
	e = document.getElementById("manntpKuwait");
	e.innerHTML = _("man ntp Kuwait");
	e = document.getElementById("manntpMoscow");
	e.innerHTML = _("man ntp Moscow");
	e = document.getElementById("manntpNairobi");
	e.innerHTML = _("man ntp Nairobi");
	e = document.getElementById("manntpTehran");
	e.innerHTML = _("man ntp Tehran");
	e = document.getElementById("manntpAbuDhabi");
	e.innerHTML = _("man ntp Abu Dhabi");
	e = document.getElementById("manntpBaku");
	e.innerHTML = _("man ntp Baku");
	e = document.getElementById("manntpMauritius");
	e.innerHTML = _("man ntp Mauritius");
	e = document.getElementById("manntpTbilisi");
	e.innerHTML = _("man ntp Tbilisi");
	e = document.getElementById("manntpYerevan");
	e.innerHTML = _("man ntp Yerevan");
	e = document.getElementById("manntpKabul");
	e.innerHTML = _("man ntp Kabul");
	e = document.getElementById("manntpEkaterinburg");
	e.innerHTML = _("man ntp Ekaterinburg");
	e = document.getElementById("manntpPakistan");
	e.innerHTML = _("man ntp Pakistan");
	e = document.getElementById("manntpTashkent");
	e.innerHTML = _("man ntp Tashkent");
	e = document.getElementById("manntpChennai");
	e.innerHTML = _("man ntp Chennai");
	e = document.getElementById("manntpKathmandu");
	e.innerHTML = _("man ntp Kathmandu");
	e = document.getElementById("manntpAlmaty");
	e.innerHTML = _("man ntp Almaty");
	e = document.getElementById("manntpBangladesh");
	e.innerHTML = _("man ntp Bangladesh");
	e = document.getElementById("manntpCentralsia");
	e.innerHTML = _("man ntp Central Asia");
	e = document.getElementById("manntpRangoon");
	e.innerHTML = _("man ntp Rangoon");
	e = document.getElementById("manntpBangkok");
	e.innerHTML = _("man ntp Bangkok");
	e = document.getElementById("manntpKrasnoyarsk");
	e.innerHTML = _("man ntp Krasnoyarsk");
	e = document.getElementById("manntpBeijing");
	e.innerHTML = _("man ntp Beijing");
	e = document.getElementById("manntpIrkutsk");
	e.innerHTML = _("man ntp Irkutsk");
	e = document.getElementById("manntpKualaLumpur");
	e.innerHTML = _("man ntp Kuala Lumpur");
	e = document.getElementById("manntpPerth");
	e.innerHTML = _("man ntp Perth");
	e = document.getElementById("manntpTaipei");
	e.innerHTML = _("man ntp Taipei");
	e = document.getElementById("manntpUlaanbaatar");
	e.innerHTML = _("man ntp Ulaanbaatar");
	e = document.getElementById("manntpOsaka");
	e.innerHTML = _("man ntp Osaka");
	e = document.getElementById("manntpSeoul");
	e.innerHTML = _("man ntp Seoul");
	e = document.getElementById("manntpYakutsk");
	e.innerHTML = _("man ntp Yakutsk");
	e = document.getElementById("manntpAdelaide");
	e.innerHTML = _("man ntp Adelaide");
	e = document.getElementById("manntpDarwin");
	e.innerHTML = _("man ntp Darwin");
	e = document.getElementById("manntpBrisbane");
	e.innerHTML = _("man ntp Brisbane");
	e = document.getElementById("manntpCanberra");
	e.innerHTML = _("man ntp Canberra");
	e = document.getElementById("manntpGuam");
	e.innerHTML = _("man ntp Guam");
	e = document.getElementById("manntpHobart");
	e.innerHTML = _("man ntp Hobart");
	e = document.getElementById("manntpVladivostok");
	e.innerHTML = _("man ntp Vladivostok");
	e = document.getElementById("manntpMagadan");
	e.innerHTML = _("man ntp Magadan");
	e = document.getElementById("manntpsolomon");
	e.innerHTML = _("man ntp solomon");
	e = document.getElementById("manntpNorfolk");
	e.innerHTML = _("man ntp Norfolk");
	e = document.getElementById("manntpAuckland");
	e.innerHTML = _("man ntp Auckland");
	e = document.getElementById("manntpFiji");
	e.innerHTML = _("man ntp Fiji");
	e = document.getElementById("manntpKamchatka");
	e.innerHTML = _("man ntp Kamchatka");
	e = document.getElementById("manntpNuku");
	e.innerHTML = _("man ntp Nuku");
	/* minglin, 2010/01/19 */
	document.getElementById("manntpmsg").innerHTML = _("admin please select your time zone settings");
	/* minglin, 2010/01/19 */
	/* daylight saving time, minglin, 2010/02/02 *start*/
	document.getElementById("mandaylightsavingtime").innerHTML = _("man ntp daylight saving");
	document.getElementById("mandaylightsavingtimeoffset").innerHTML = _("man ntp daylight saving offset");
	document.getElementById("mandaylightsavingtimestarttime").innerHTML = _("man ntp daylight saving start");
	document.getElementById("mandaylightsavingtimeendtime").innerHTML = _("man ntp daylight saving end");
	/* daylight saving time, minglin, 2010/02/02 *end*/
	e = document.getElementById("manNTPServer");
	e.innerHTML = _("man ntp server");
	e = document.getElementById("manNTPSync");
	e.innerHTML = _("man ntp sync");
	e = document.getElementById("manNTPApply");
	e.value = _("admin apply");
	e = document.getElementById("manNTPCancel");
	e.value = _("admin cancel");

	e = document.getElementById("manNTPCurrentTime");
	e.innerHTML = _("man ntp current time");
	e = document.getElementById("manNTPSyncWithHost");
	e.value = _("man ntp sync with host");

	e = document.getElementById("manGAPTitle");
	e.innerHTML = _("man gap title");
	e = document.getElementById("manGAPTime");
	e.innerHTML = _("man gap time");
	e = document.getElementById("manGAPAction");
	e.innerHTML = _("man gap action");
	e = document.getElementById("manGAPActionDisable1");
	e.innerHTML = _("admin disable");
	e = document.getElementById("manGAPActionDisable2");
	e.innerHTML = _("admin disable");
	e = document.getElementById("manGAPActionDisable3");
	e.innerHTML = _("admin disable");
	e = document.getElementById("manGAPActionDisable4");
	e.innerHTML = _("admin disable");

	e = document.getElementById("manGreenAPApply");
	e.value = _("admin apply");
	e = document.getElementById("manGreenAPCancle");
	e.value = _("admin cancel");

	e = document.getElementById("manDdnsSet");
	e.innerHTML = _("man ddns setting");
	e = document.getElementById("DdnsProvider");
	e.innerHTML = _("man ddns provider");
	e = document.getElementById("manDdnsNone");
	e.innerHTML = _("man ddns none");
	e = document.getElementById("manDdnsAccount");
	e.innerHTML = _("man ddns account");
	e = document.getElementById("manDdnsPasswd");
	e.innerHTML = _("man ddns passwd");
	e = document.getElementById("manDdns");
	e.innerHTML = _("man ddns");
	e = document.getElementById("manDdnsApply");
	e.value = _("admin apply");
	e = document.getElementById("manDdnsCancel");
	e.value = _("admin cancel");
}

function initValue()
{
	var tz = "<% getCfgGeneral(1, "TZ"); %>";
	var ddns_provider = "<% getCfgGeneral(1, "DDNSProvider"); %>";

	var lang_element = document.getElementById("langSelection");
	var lang_en = "<% getLangBuilt("en"); %>";
	var lang_zhtw = "<% getLangBuilt("zhtw"); %>";
	var lang_zhcn = "<% getLangBuilt("zhcn"); %>";
	var lang_kr = "<% getLangBuilt("kr"); %>";

	var dateb = "<% getDATEBuilt(); %>";
	var ddnsb = "<% getDDNSBuilt(); %>";

	// Accelecon hacks to limit choices:

		lang_zhtw = "0";
		lang_zhcn = "0";
		lang_kr = "0";
		ddnsb = "0";

	initTranslation();
	lang_element.options.length = 0;
	if (lang_en == "1")
		lang_element.options[lang_element.length] = new Option('English', 'en');
	if (lang_zhtw == "1")
		lang_element.options[lang_element.length] = new Option('繁體中文', 'zhtw');//new Option('Traditional Chinese', 'zhtw');
	if (lang_zhcn == "1")
		lang_element.options[lang_element.length] = new Option('简体中文', 'zhcn');
	if (lang_kr == "1")
		lang_element.options[lang_element.length] = new Option('한국어', 'kr');

	if (document.cookie.length > 0) {
		var s = document.cookie.indexOf("language=");
		var e = document.cookie.indexOf(";", s);
		var lang = "en";
		var i;

		if (s != -1) {
			if (e == -1)
				lang = document.cookie.substring(s+9);
			else
				lang = document.cookie.substring(s+9, e);
		}
		for (i=0; i<lang_element.options.length; i++) {
			if (lang == lang_element.options[i].value) {
				lang_element.options.selectedIndex = i;
				break;
			}
		}
	}
	
	if (dateb == "1")
	{
		document.getElementById("div_date").style.visibility = "visible";
		document.getElementById("div_date").style.display = style_display_on();
		document.NTP.ntpcurrenttime.disabled = false;
	} 
	else
	{
		document.getElementById("div_date").style.visibility = "hidden";
		document.getElementById("div_date").style.display = "none";
		document.NTP.ntpcurrenttime.disabled = true;
	}

	if (tz == "GMT_-12")
		document.NTP.time_zone.options.selectedIndex = 0;
	else if(tz == "GMT_-11")
		document.NTP.time_zone.options.selectedIndex = 1;
	else if (tz == "GMT_-10")
		document.NTP.time_zone.options.selectedIndex = 2;
	else if (tz == "GMT1_-10")
		document.NTP.time_zone.options.selectedIndex = 3;
	else if (tz == "GMT_-09")
		document.NTP.time_zone.options.selectedIndex = 4;
	else if (tz == "GMT_-08")
		document.NTP.time_zone.options.selectedIndex = 5;
	else if (tz == "GMT_-07")
		document.NTP.time_zone.options.selectedIndex = 6;
	else if (tz == "GMT1_-07")
		document.NTP.time_zone.options.selectedIndex = 7;
	else if (tz == "GMT2_-07")
		document.NTP.time_zone.options.selectedIndex = 8;
	else if (tz == "GMT_-06")
		document.NTP.time_zone.options.selectedIndex = 9;
	else if (tz == "GMT1_-06")
		document.NTP.time_zone.options.selectedIndex = 10;
	else if (tz == "GMT2_-06")
		document.NTP.time_zone.options.selectedIndex = 11;
	else if (tz == "GMT3_-06")
		document.NTP.time_zone.options.selectedIndex = 12;
	else if (tz == "GMT4_-06")
		document.NTP.time_zone.options.selectedIndex = 13;
	else if (tz == "GMT_-05")
		document.NTP.time_zone.options.selectedIndex = 14;
	else if (tz == "GMT1_-05")
		document.NTP.time_zone.options.selectedIndex = 15;
	else if (tz == "GMT2_-05")
		document.NTP.time_zone.options.selectedIndex = 16;
	else if (tz == "GMT_-04:30")
		document.NTP.time_zone.options.selectedIndex = 17;
	else if (tz == "GMT_-04")
		document.NTP.time_zone.options.selectedIndex = 18;
	else if (tz == "GMT1_-04")
		document.NTP.time_zone.options.selectedIndex = 19;
	else if (tz == "GMT2_-04")
		document.NTP.time_zone.options.selectedIndex = 20;
	else if (tz == "GMT3_-04")
		document.NTP.time_zone.options.selectedIndex = 21;
	else if (tz == "GMT4_-04")
		document.NTP.time_zone.options.selectedIndex = 22;
	else if (tz == "GMT5_-04")
		document.NTP.time_zone.options.selectedIndex = 23;
	else if (tz == "GMT_-03:30")
		document.NTP.time_zone.options.selectedIndex = 24;
	else if (tz == "GMT_-03")
		document.NTP.time_zone.options.selectedIndex = 25;
	else if (tz == "GMT1_-03")
		document.NTP.time_zone.options.selectedIndex = 26;
	else if (tz == "GMT2_-03")
		document.NTP.time_zone.options.selectedIndex = 27;
	else if (tz == "GMT3_-03")
		document.NTP.time_zone.options.selectedIndex = 28;
	else if (tz == "GMT4_-03")
		document.NTP.time_zone.options.selectedIndex = 29;
	else if (tz == "GMT5_-03")
		document.NTP.time_zone.options.selectedIndex = 30;
	else if (tz == "GMT_-02")
		document.NTP.time_zone.options.selectedIndex = 31;
	else if (tz == "GMT_-01")
		document.NTP.time_zone.options.selectedIndex = 32;
	else if (tz == "GMT1_-01")
		document.NTP.time_zone.options.selectedIndex = 33;
	else if (tz == "GMT_000")
		document.NTP.time_zone.options.selectedIndex = 34;
	else if (tz == "GMT1_000")
		document.NTP.time_zone.options.selectedIndex = 35;
	else if (tz == "GMT2_000")
		document.NTP.time_zone.options.selectedIndex = 36;
	else if (tz == "GMT_001")
		document.NTP.time_zone.options.selectedIndex = 37;
	else if (tz == "GMT1_001")
		document.NTP.time_zone.options.selectedIndex = 38;
	else if (tz == "GMT2_001")
		document.NTP.time_zone.options.selectedIndex = 39;
	else if (tz == "GMT3_001")
		document.NTP.time_zone.options.selectedIndex = 40;
	else if (tz == "GMT4_001")
		document.NTP.time_zone.options.selectedIndex = 41;
	else if (tz == "GMT5_001")
		document.NTP.time_zone.options.selectedIndex = 42;
	else if (tz == "GMT_002")
		document.NTP.time_zone.options.selectedIndex = 43;
	else if (tz == "GMT1_002")
		document.NTP.time_zone.options.selectedIndex = 44;
	else if (tz == "GMT2_002")
		document.NTP.time_zone.options.selectedIndex = 45;
	else if (tz == "GMT3_002")
		document.NTP.time_zone.options.selectedIndex = 46;
	else if (tz == "GMT4_002")
		document.NTP.time_zone.options.selectedIndex = 47;
	else if (tz == "GMT5_002")
		document.NTP.time_zone.options.selectedIndex = 48;
	else if (tz == "GMT6_002")
		document.NTP.time_zone.options.selectedIndex = 49;
	else if (tz == "GMT7_002")
		document.NTP.time_zone.options.selectedIndex = 50;
	else if (tz == "GMT8_002")
		document.NTP.time_zone.options.selectedIndex = 51;
	else if (tz == "GMT9_002")
		document.NTP.time_zone.options.selectedIndex = 52;
	else if (tz == "GMT_003")
		document.NTP.time_zone.options.selectedIndex = 53;
	else if (tz == "GMT1_003")
		document.NTP.time_zone.options.selectedIndex = 54;
	else if (tz == "GMT2_003")
		document.NTP.time_zone.options.selectedIndex = 55;
	else if (tz == "GMT3_003")
		document.NTP.time_zone.options.selectedIndex = 56;
	else if (tz == "GMT_003:30")
		document.NTP.time_zone.options.selectedIndex = 57;
	else if (tz == "GMT_004")
		document.NTP.time_zone.options.selectedIndex = 58;
	else if (tz == "GMT1_004")
		document.NTP.time_zone.options.selectedIndex = 59;
	else if (tz == "GMT2_004")
		document.NTP.time_zone.options.selectedIndex = 60;
	else if (tz == "GMT3_004")
		document.NTP.time_zone.options.selectedIndex = 61;
	else if (tz == "GMT4_004")
		document.NTP.time_zone.options.selectedIndex = 62;
	else if (tz == "GMT_004:30")
		document.NTP.time_zone.options.selectedIndex = 63;
	else if (tz == "GMT_005")
		document.NTP.time_zone.options.selectedIndex = 64;
	else if (tz == "GMT1_005")
		document.NTP.time_zone.options.selectedIndex = 65;
	else if (tz == "GMT2_005")
		document.NTP.time_zone.options.selectedIndex = 66;
	else if (tz == "GMT_005:30")
		document.NTP.time_zone.options.selectedIndex = 67;
	else if (tz == "GMT_005:45")
		document.NTP.time_zone.options.selectedIndex = 68;
	else if (tz == "GMT_006")
		document.NTP.time_zone.options.selectedIndex = 69;
	else if (tz == "GMT1_006")
		document.NTP.time_zone.options.selectedIndex = 70;
	else if (tz == "GMT2_006")
		document.NTP.time_zone.options.selectedIndex = 71;
	else if (tz == "GMT_006:30")
		document.NTP.time_zone.options.selectedIndex = 72;
	else if (tz == "GMT_007")
		document.NTP.time_zone.options.selectedIndex = 73;
	else if (tz == "GMT1_007")
		document.NTP.time_zone.options.selectedIndex = 74;
	else if (tz == "GMT_008")
		document.NTP.time_zone.options.selectedIndex = 75;
	else if (tz == "GMT1_008")
		document.NTP.time_zone.options.selectedIndex = 76;
	else if (tz == "GMT2_008")
		document.NTP.time_zone.options.selectedIndex = 77;
	else if (tz == "GMT3_008")
		document.NTP.time_zone.options.selectedIndex = 78;
	else if (tz == "GMT4_008")
		document.NTP.time_zone.options.selectedIndex = 79;
	else if (tz == "GMT5_008")
		document.NTP.time_zone.options.selectedIndex = 80;
	else if (tz == "GMT_009")
		document.NTP.time_zone.options.selectedIndex = 81;
	else if (tz == "GMT1_009")
		document.NTP.time_zone.options.selectedIndex = 82;
	else if (tz == "GMT2_009")
		document.NTP.time_zone.options.selectedIndex = 83;
	else if (tz == "GMT_009:30")
		document.NTP.time_zone.options.selectedIndex = 84;
	else if (tz == "GMT1_009:30")
		document.NTP.time_zone.options.selectedIndex = 85;
	else if (tz == "GMT_010")
		document.NTP.time_zone.options.selectedIndex = 86;
	else if (tz == "GMT1_010")
		document.NTP.time_zone.options.selectedIndex = 87;
	else if (tz == "GMT2_010")
		document.NTP.time_zone.options.selectedIndex = 88;
	else if (tz == "GMT3_010")
		document.NTP.time_zone.options.selectedIndex = 89;
	else if (tz == "GMT4_010")
		document.NTP.time_zone.options.selectedIndex = 90;
	else if (tz == "GMT_011")
		document.NTP.time_zone.options.selectedIndex = 91;
	else if (tz == "GMT1_011")
		document.NTP.time_zone.options.selectedIndex = 92;
	else if (tz == "GMT_011:30")
		document.NTP.time_zone.options.selectedIndex = 93;
	else if (tz == "GMT_012")
		document.NTP.time_zone.options.selectedIndex = 94;
	else if (tz == "GMT1_012")
		document.NTP.time_zone.options.selectedIndex = 95;
	else if (tz == "GMT2_012")
		document.NTP.time_zone.options.selectedIndex = 96;
    else if (tz == "GMT_013")
		document.NTP.time_zone.options.selectedIndex = 97;
    else
		document.NTP.time_zone.options.selectedIndex = 98;
	/*auto detecte time zone ,minglin,2010/02/02 *start*/
	if(tz.substr(0, 4) == 'ato_' ){
		var tz_msg = "(GMT"
		if(tz.substr(4, 1) == '-') tz_msg += '-'; 
		else tz_msg += "+";
		
		if(tz.charAt(tz.length-3) == ':'  && tz.charAt(tz.length-5) == '1') tz_msg += tz.substr(tz.length-5) + ") ";
		else if(tz.charAt(tz.length-3) == ':'  && tz.charAt(tz.length-5) != '1') tz_msg += tz.substr(tz.length-4) + ") ";
		else if(tz.charAt(tz.length-2) == '1' ) tz_msg += tz.substr(tz.length-2) + ":00) ";
		else tz_msg += tz.substr(tz.length-1) + ":00) ";
		
		document.NTP.time_zone.options[98].innerHTML = tz_msg + _('man ntp auto detecte time zone');
		document.NTP.time_zone.options[98].value = tz;
	}
	/*auto detecte time zone ,minglin,2010/02/02 *end*/
	/* whan 3G enable then disable sync with host btn, minglin, 2010/02/02 *start*/
	//if( '<% getCfgGeneral(1, "wan2"); %>' == "G3G"){
	//	document.NTP.manNTPSyncWithHost.disable = true;
	//}
	/* whan 3G enable then disable sync with host btn, minglin, 2010/02/02 *end*/
	/* init daylight saving time, minglin, 2010/02/03 *start*/
	if('<% getCfgGeneral(1, "DaylightEnable"); %>' == "1" ){
		document.NTP.onoffdaylightsaving.checked = true;
	}
	// ============== offset =============
	for(var s = 0; s < document.NTP.daylightsavingtimeOffset.length; s++){
		if(document.NTP.daylightsavingtimeOffset.options[s].value == '<% getCfgGeneral(1, "DaylightOffset"); %>'){
			document.NTP.daylightsavingtimeOffset.options[s].selected = true;
			break;
		}
	}
	// ============== Start Month =============
	for(var s = 0; s < document.NTP.daylightsavingtimeStartMonth.length; s++){
		if(document.NTP.daylightsavingtimeStartMonth.options[s].value == '<% getCfgGeneral(1, "DaylightStartMonth"); %>'){
			document.NTP.daylightsavingtimeStartMonth.options[s].selected = true;
			break;
		}
	}
	// ============== Start Day =============
	for(var s = 0; s < document.NTP.daylightsavingtimeStartDay.length; s++){
		if(document.NTP.daylightsavingtimeStartDay.options[s].value == '<% getCfgGeneral(1, "DaylightStartDay"); %>'){
			document.NTP.daylightsavingtimeStartDay.options[s].selected = true;
			break;
		}
	}
	// ============== Start Time =============
	for(var s = 0; s < document.NTP.daylightsavingtimeStartTime.length; s++){
		if(document.NTP.daylightsavingtimeStartTime.options[s].value == '<% getCfgGeneral(1, "DaylightStartTime"); %>'){
			document.NTP.daylightsavingtimeStartTime.options[s].selected = true;
			break;
		}
	}
	// ============== End Month =============
	for(var s = 0; s < document.NTP.daylightsavingtimeEndMonth.length; s++){
		if(document.NTP.daylightsavingtimeEndMonth.options[s].value == '<% getCfgGeneral(1, "DaylightEndMonth"); %>'){
			document.NTP.daylightsavingtimeEndMonth.options[s].selected = true;
			break;
		}
	}
	// ============== End Day =============
	for(var s = 0; s < document.NTP.daylightsavingtimeEndDay.length; s++){
		if(document.NTP.daylightsavingtimeEndDay.options[s].value == '<% getCfgGeneral(1, "DaylightEndDay"); %>'){
			document.NTP.daylightsavingtimeEndDay.options[s].selected = true;
			break;
		}
	}
	// ============== End Time =============
	for(var s = 0; s < document.NTP.daylightsavingtimeEndTime.length; s++){
		if(document.NTP.daylightsavingtimeEndTime.options[s].value == '<% getCfgGeneral(1, "DaylightEndTime"); %>'){
			document.NTP.daylightsavingtimeEndTime.options[s].selected = true;
			break;
		}
	}
	OnDaylightSelecteChange();
	document.getElementById("ds_tr").style.visibility  = "hidden";
	document.getElementById("ds_tr").style.display = "none";
	/* init daylight saving time, minglin, 2010/02/03 *end*/
	if (greenapb == "1")
	{
		document.getElementById("div_greenap").style.visibility = "visible";
		document.getElementById("div_greenap").style.display = style_display_on();
		document.getElementById("div_greenap_submit").style.visibility = "visible";
		document.getElementById("div_greenap_submit").style.display = style_display_on();
	}
	else
	{
		document.getElementById("div_greenap").style.visibility = "hidden";
		document.getElementById("div_greenap").style.display = "none";
		document.getElementById("div_greenap_submit").style.visibility = "hidden";
		document.getElementById("div_greenap_submit").style.display = "none";
	}
	set_greenap();

	if (ddnsb == "1")
	{
		document.getElementById("div_ddns").style.visibility = "visible";
		document.getElementById("div_ddns").style.display = style_display_on();
		document.getElementById("div_ddns_submit").style.visibility = "visible";
		document.getElementById("div_ddns_submit").style.display = style_display_on();
		document.DDNS.Account.disabled = false;
		document.DDNS.Password.disabled = false;
		document.DDNS.DDNS.disabled = false;
		if (ddns_provider == "none")
			document.DDNS.DDNSProvider.options.selectedIndex = 0;
		else if (ddns_provider == "dyndns.org")
			document.DDNS.DDNSProvider.options.selectedIndex = 1;
		else if (ddns_provider == "freedns.afraid.org")
			document.DDNS.DDNSProvider.options.selectedIndex = 2;
		else if (ddns_provider == "zoneedit.com")
			document.DDNS.DDNSProvider.options.selectedIndex = 3;
		else if (ddns_provider == "no-ip.com")
			document.DDNS.DDNSProvider.options.selectedIndex = 4;

		DDNSupdateState();
	} 
	else
	{
		document.getElementById("div_ddns").style.visibility = "hidden";
		document.getElementById("div_ddns").style.display = "none";
		document.getElementById("div_ddns_submit").style.visibility = "hidden";
		document.getElementById("div_ddns_submit").style.display = "none";
		document.DDNS.Account.disabled = true;
		document.DDNS.Password.disabled = true;
		document.DDNS.DDNS.disabled = true;
	}
}

function set_greenap()
{
	var ntp_server = "<% getCfgGeneral(1, "NTPServerIP"); %>";

	for(var j=1;j<=4;j++)
	{
	    var shour_e = eval("document.GreenAP.GAPSHour"+j);
	    var sminute_e = eval("document.GreenAP.GAPSMinute"+j);
	    var ehour_e = eval("document.GreenAP.GAPEHour"+j);
	    var eminute_e = eval("document.GreenAP.GAPEMinute"+j);
	    var action_e = eval("document.GreenAP.GAPAction"+j);

	    shour_e.disabled = true;
	    sminute_e.disabled = true;
	    ehour_e.disabled = true;
	    eminute_e.disabled = true;
	    action_e.disabled = true;
	    if (ntp_server != "" && greenapb == "1")
	    {
		action_e.disabled = false;
		switch(j)
		{
		case 1:
		    var action = "<% getCfgGeneral(1, "GreenAPAction1"); %>";
		    var time = "<% getCfgGeneral(1, "GreenAPStart1"); %>";
		    var stimeArray = time.split(" ");
		    time = "<% getCfgGeneral(1, "GreenAPEnd1"); %>";
		    var etimeArray = time.split(" ");
		    break;
		case 2:
		    var action = "<% getCfgGeneral(1, "GreenAPAction2"); %>";
		    var time = "<% getCfgGeneral(1, "GreenAPStart2"); %>";
		    var stimeArray = time.split(" ");
		    time = "<% getCfgGeneral(1, "GreenAPEnd2"); %>";
		    var etimeArray = time.split(" ");
		    break;
		case 3:
		    var action = "<% getCfgGeneral(1, "GreenAPAction3"); %>";
		    var time = "<% getCfgGeneral(1, "GreenAPStart3"); %>";
		    var stimeArray = time.split(" ");
		    time = "<% getCfgGeneral(1, "GreenAPEnd3"); %>";
		    var etimeArray = time.split(" ");
		    break;
		case 4:
		    var action = "<% getCfgGeneral(1, "GreenAPAction4"); %>";
		    var time = "<% getCfgGeneral(1, "GreenAPStart4"); %>";
		    var stimeArray = time.split(" ");
		    time = "<% getCfgGeneral(1, "GreenAPEnd4"); %>";
		    var etimeArray = time.split(" ");
		    break;
		}
		if (action == "Disable")
		    action_e.options.selectedIndex = 0;
		else if (action == "WiFiOFF")
		    action_e.options.selectedIndex = 1;
		else if (action == "TX25")
		    action_e.options.selectedIndex = 2;
		else if (action == "TX50")
		    action_e.options.selectedIndex = 3;
		else if (action == "TX75")
		    action_e.options.selectedIndex = 4;
		greenap_action_switch(j);
		if (action != "" && action != "Disable")
		{
		    shour_e.options.selectedIndex = stimeArray[1];
		    sminute_e.options.selectedIndex = stimeArray[0];
		    ehour_e.options.selectedIndex = etimeArray[1];
		    eminute_e.options.selectedIndex = etimeArray[0];
		}
	    }
	}
}

function setLanguage()
{
	document.cookie="language="+document.Lang.langSelection.value+"; path=/";
	/*parent.menu.location.reload();*/
	return true;
}

function syncWithHost()
{
	var tz_flash = "<% getCfgGeneral(1, "TZ"); %>";
	var currentTime = new Date();
	/* get UTC time, minglin, 2010/01/18 *Start*/
	var seconds = currentTime.getUTCSeconds();
	var minutes = currentTime.getUTCMinutes();
	var hours = currentTime.getUTCHours();
	var month = currentTime.getUTCMonth() + 1;
	var day = currentTime.getUTCDate();
	var year = currentTime.getUTCFullYear();
	var timezone_offset = currentTime.getTimezoneOffset();
	/* get UTC time, minglin, 2010/01/18 *Start*/
	var seconds_str = " ";
	var minutes_str = " ";
	var hours_str = " ";
	var month_str = " ";
	var day_str = " ";
	var year_str = " ";

	if(seconds < 10)
		seconds_str = "0" + seconds;
	else
		seconds_str = ""+seconds;

	if(minutes < 10)
		minutes_str = "0" + minutes;
	else
		minutes_str = ""+minutes;

	if(hours < 10)
		hours_str = "0" + hours;
	else
		hours_str = ""+hours;

	if(month < 10)
		month_str = "0" + month;
	else
		month_str = ""+ month;

	if(day < 10)
		day_str = "0" + day;
	else
		day_str = day;
	
	// caculation Time Zone  
	var start_ds=document.NTP.daylightsavingtimeStartMonth.value+document.NTP.daylightsavingtimeStartDay.value+document.NTP.daylightsavingtimeStartTime.value;
	var End_ds=document.NTP.daylightsavingtimeEndMonth.value+document.NTP.daylightsavingtimeEndDay.value+document.NTP.daylightsavingtimeEndTime.value;
	var now_ds=month_str+day_str+hours_str;
	var tz_tmp;
	
	if(document.NTP.onoffdaylightsaving.checked && (start_ds.valueOf() < now_ds.valueOf() && now_ds.valueOf() < End_ds.valueOf() ) )
		tz_tmp= -(( timezone_offset / 60 ) + document.NTP.daylightsavingtimeOffset.value.valueOf() );// time zone
	else
		tz_tmp= -( timezone_offset / 60 );// time zone
 
	var half_hours = (parseInt(tz_tmp) == tz_tmp)? 0 : (10 * (tz_tmp - parseInt(tz_tmp))) ;
	var tmp_zone="";
	if(document.NTP.time_zone.options.selectedIndex == 98)//if(tz_flash.substr(0,4) == 'ato_' || tz_flash == "" || tz_flash == "0" || document.NTP.time_zone.options.selectedIndex == 98) // tz == auto detecte or null ?
	{
		alert(_('admin please select your time zone settings'));
		document.NTP.time_zone.focus();
		return false;
		if(tz_tmp >= 0 && tz_tmp < 10 ){
			if(half_hours != 0) tmp_zone = "ato_00" + parseInt(tz_tmp) + ":" + (half_hours * 6);
			else tmp_zone = "ato_00" + parseInt(tz_tmp);
		}else if(tz_tmp >= 0 && tz_tmp >= 10){
			if(half_hours != 0) tmp_zone = "ato_0" + parseInt(tz_tmp) + ":" + (half_hours * 6);
			else tmp_zone = "ato_0" + parseInt(tz_tmp);
		}else if(tz_tmp < 0 && tz_tmp > -10){
			if(half_hours != 0) tmp_zone = "ato_-0" + -parseInt(tz_tmp) + ":" + -(half_hours * 6);
			else tmp_zone = "ato_-0" + -parseInt(tz_tmp);
		}else if(tz_tmp < 0 && tz_tmp <= -10){
			if(half_hours != 0) tmp_zone = "ato_-" + -parseInt(tz_tmp) + ":" + -(half_hours * 6);
			else tmp_zone = "ato_-" + -parseInt(tz_tmp);
		}else{
			tmp_zone = tz_flash;
		}
	}else{tmp_zone = document.NTP.time_zone.value;}
	/*
	if(tz_tmp >= 0 && tz_tmp < 10 ){
		if(half_hours != 0) tmp_zone = "GMT_00" + parseInt(tz_tmp) + ":" + (half_hours * 6);
		else tmp_zone = "GMT_00" + parseInt(tz_tmp);
	}else if(tz_tmp >= 0 && tz_tmp >= 10){
		if(half_hours != 0) tmp_zone = "GMT_0" + parseInt(tz_tmp) + ":" + (half_hours * 6);
		else tmp_zone = "GMT_0" + parseInt(tz_tmp);
	}else if(tz_tmp < 0 && tz_tmp > -10){
		if(half_hours != 0) tmp_zone = "GMT_-0" + -parseInt(tz_tmp) + ":" + -(half_hours * 6);
		else tmp_zone = "GMT_-0" + -parseInt(tz_tmp);
	}else if(tz_tmp < 0 && tz_tmp <= -10){
		if(half_hours != 0) tmp_zone = "GMT_-" + -parseInt(tz_tmp) + ":" + -(half_hours * 6);
		else tmp_zone = "GMT_-" + -parseInt(tz_tmp);
	}else{
		tmp_zone = "0";
	}*/
	
	var tmp = month_str + day_str + hours_str + minutes_str + year + "," + tmp_zone + ", ";
	makeRequest("/goform/NTPSyncWithHost", tmp);
}

function greenap_action_switch(index)
{
	var shour_e = eval("document.GreenAP.GAPSHour"+index);
	var sminute_e = eval("document.GreenAP.GAPSMinute"+index);
	var ehour_e = eval("document.GreenAP.GAPEHour"+index);
	var eminute_e = eval("document.GreenAP.GAPEMinute"+index);
	var action_e = eval("document.GreenAP.GAPAction"+index);

	shour_e.disabled = true;
	sminute_e.disabled = true;
	ehour_e.disabled = true;
	eminute_e.disabled = true;

	if (action_e.options.selectedIndex != 0)
	{
		shour_e.disabled = false;
		sminute_e.disabled = false;
		ehour_e.disabled = false;
		eminute_e.disabled = false;
	}
}
/*daylight saving time, minglin, 2010/02/02 *start*/
function OnDaylightSelecteChange(){
	if(!document.NTP.onoffdaylightsaving.checked){
		document.getElementById("tabledaylightsavingtime").style.visibility = "hidden";
		document.getElementById("tabledaylightsavingtime").style.display = "none";
		document.getElementById("onoffdaylightsavingMsg").innerHTML = _('admin disable');
	}else{
		document.getElementById("tabledaylightsavingtime").style.visibility = "visible";
		document.getElementById("tabledaylightsavingtime").style.display = style_display_on();
		document.getElementById("onoffdaylightsavingMsg").innerHTML = _('admin enable');
	}
}
/*daylight saving time, minglin, 2010/02/02 *end*/
</script>
</head>
<body onLoad="initValue()" bgcolor="#FFFFFF">

<div align="center">
 <center>

<table class="body"><tr><td>

<table width="600" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="manTitle">System Management</td>
</tr>
<tr>

<tr>
<td colspan="2">
<p class="head" id="manIntroduction">You may configure administrator account and password, NTP settings, and Dynamic DNS settings here.</p>
</td>
<tr>
</table>
<br>

<!-- ================= Langauge Settings ================= -->
<form method="post" name="Lang" action="/goform/setSysLang">
<table width="600" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" id="manLangSet">Language Settings</td>
  </tr>
  <tr>
    <td width="180" class="head" id="manSelectLang">Select Language</td>
    <td width="399">
      <select name="langSelection" id="langSelection">
        <!-- added by initValue -->
      </select>
    </td>
  </tr>
</table>
<table width="600" border="0" cellpadding="2" cellspacing="1">
  <tr id="sbutton0" align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply" id="manLangApply" onClick="sbutton_disable(sbuttonMax); restartPage_block(); return setLanguage();"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="manLangCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>

<!-- ================= Adm Settings ================= -->
<form method="post" name="Adm" action="/goform/setSysAdm">
<table width="600" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr>
    <td class="title" colspan="2" id="manAdmSet">Adminstrator Settings</td>
  </tr>
  <tr>
    <td width="180" class="head" id="manAdmAccount">Account</td>
    <td width="399"><input type="text" name="admuser" size="16" maxlength="32" value="<% getCfgGeneral(1, "Login"); %>" readonly="1"></td>
  </tr>
  <tr>
    <td class="head" id="manAdmPasswd">Password</td>
    <td><input type="password" name="admpass" size="16" maxlength="32" value="****"></td>
    <!--<td><input type="password" name="admpass" size="16" maxlength="32" value="<% getCfgGeneral(1, "Password"); %>"></td>-->
  </tr>
</table>
<table width="600" border="0" cellpadding="2" cellspacing="1">
  <tr id="sbutton1" align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply" id="manAdmApply" onClick="return AdmFormCheck();"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="manAdmCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>

<!-- ================= NTP Settings ================= -->
<form method="post" name="NTP" action="/goform/NTP">
<table width="600" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
<tbody><tr>
  <td class="title" colspan="2" id="manNTPSet">NTP Settings</td>
</tr>
<tr id="div_date">
	<td width="180" class="head"  id="manNTPCurrentTime">Current Time</td>
	<td width="399">
		<input size="24" name="ntpcurrenttime" value="<% getCurrentTimeASP(); %>" type="text" readonly="1">
		<input type="button" value="Sync with host" id="manNTPSyncWithHost" name="manNTPSyncWithHost" onClick="syncWithHost()">
	</td>
</tr>
<tr>
  <td class="head" id="manNTPTimeZone">Time Zone:</td>
  <td>
    <select name="time_zone">
      <option value="GMT_-12" id="manntpinterdateline">(GMT-12:00) International Date Line</option>
      <option value="GMT_-11" id="manntpmidisland">(GMT-11:00) Midway Island, Samoa</option>
      <option value="GMT_-10" id="manntpAdak">(GMT-10:00) Adak</option>
      <option value="GMT1_-10" id="manntphawaii">(GMT-10:00) Hawaii</option>
	  <option value="GMT_-09" id="manntpalaska">(GMT-09:00) Alaska</option>
	  <option value="GMT_-08" id="manntppacific">(GMT-08:00) Pacific Time (US & Canada); Tijuana</option>
	  <option value="GMT_-07" id="manntparizona">(GMT-07:00) Arizona</option>
	  <option value="GMT1_-07" id="manntpChihuahua">(GMT-07:00) Chihuahua, La Paz, Mazatlan</option>
	  <option value="GMT2_-07" id="manntpmountain">(GMT-07:00) Mountain Time (US & Canada)</option>
	  <option value="GMT3_-06" id="manntpmidus">(GMT-06:00) Central America</option>
	  <option value="GMT4_-06" id="manntpcentral">(GMT-06:00) Central Time (US & Canada)</option>
	  <option value="GMT5_-06" id="manntpGuadalajara">(GMT-06:00) Guadalajara, Mexico City, Monterrey</option>
	  <option value="GMT6_-06" id="manntpGuatemala">(GMT-06:00) Guatemala</option>
	  <option value="GMT7_-06" id="manntpSaskatchewan">(GMT-06:00) Saskatchewan</option>
	  <option value="GMT_-05" id="manntpBogota">(GMT-05:00) Bogota, Lima, Quito</option>
	  <option value="GMT1_-05" id="manntpCubaStandardTime">(GMT-05:00) Cuba Standard Time</option>
	  <option value="GMT2_-05" id="manntpeastern">(GMT-05:00) Eastern Time (US & Canada)</option>
	  <option value="GMT_-04:30" id="manntpVenezuela">(GMT-04:30) Venezuela</option>
	  <option value="GMT_-04" id="manntpAtlanticTime">(GMT-04:00) Atlantic Time (Canada)</option>
	  <option value="GMT1_-04" id="manntpFalklandIslands">(GMT-04:00) Falkland Islands</option>
	  <option value="GMT2_-04" id="manntpLaPaz">(GMT-04:00) La Paz</option>
	  <option value="GMT3_-04" id="manntpParaguay">(GMT-04:00) Paraguay</option>
	  <option value="GMT4_-04" id="manntpSanLuisArgentina">(GMT-04:00) San Luis, Argentina</option>
	  <option value="GMT5_-04" id="manntpSantiago">(GMT-04:00) Santiago</option>
	  <option value="GMT_-03:30" id="manntpNewfoundland">(GMT-03:30) Newfoundland</option>
	  <option value="GMT_-03" id="manntpArgentina">(GMT-03:00) Argentina</option>
	  <option value="GMT1_-03" id="manntpBrasilia">(GMT-03:00) Brasilia</option>
	  <option value="GMT2_-03" id="manntpGeorgetown">(GMT-03:00) Georgetown, South America Eastern Time</option>
	  <option value="GMT3_-03" id="manntpGreenland">(GMT-03:00) Greenland</option>
	  <option value="GMT4_-03" id="manntpStPierre">(GMT-03:00) St Pierre and Miquelon</option>
	  <option value="GMT5_-03" id="manntpUruguay">(GMT-03:00) Uruguay</option>
	  <option value="GMT_-02" id="manntpmidatlan">(GMT-02:00) Mid-Atlantic</option>
	  <option value="GMT_-01" id="manntpazores">(GMT-01:00) Azores</option>
	  <option value="GMT1_-01" id="manntpCape">(GMT-01:00) Cape Verde Is.</option>
	  <option value="GMT_000" id="manntpDublin">(GMT) Dublin, Edinburgh, Lisbon, London</option>
	  <option value="GMT1_000" id="manntpGreenwich">(GMT) Greenwich Mean Time</option>
	  <option value="GMT2_000" id="manntpMorocco">(GMT) Morocco</option>
	  <option value="GMT_001" id="manntpAmsterdam">(GMT+01:00) Amsterdam, Berlin, Bern, Rome, Stockholm, Vienna</option>
	  <option value="GMT1_001" id="manntpBelgrade">(GMT+01:00) Belgrade, Bratislava, Budapest, Ljubljana, Prague</option>
	  <option value="GMT2_001" id="manntpBrussels">(GMT+01:00) Brussels, Copenhagen, Madrid, Paris</option>
	  <option value="GMT3_001" id="manntpNamibia">(GMT+01:00) Namibia</option>
	  <option value="GMT4_001" id="manntpSarajevo">(GMT+01:00) Sarajevo, Skopje, Warsaw, Zagreb</option>
	  <option value="GMT5_001" id="manntpWestCentralAfrica">(GMT+01:00) West Central Africa</option>
	  <option value="GMT_002" id="manntpAthens">(GMT+02:00) Athens, Beirut, Istanbul, Minsk</option>
	  <option value="GMT1_002" id="manntpBeirutAthens">(GMT+02:00) BeirutAthens, Beirut, Istanbul, Minsk</option>
	  <option value="GMT2_002" id="manntpBucharest">(GMT+02:00) Bucharest</option>
	  <option value="GMT3_002" id="manntpCairo">(GMT+02:00) Cairo</option>
	  <option value="GMT4_002" id="manntpHarare">(GMT+02:00) Harare, Pretoria</option>
	  <option value="GMT5_002" id="manntpHelsinki">(GMT+02:00) Helsinki, Kyiv, Riga, Sofia, Tallinn, Vilnius</option>
	  <option value="GMT6_002" id="manntpJordan">(GMT+02:00) Jordan</option>
	  <option value="GMT7_002" id="manntpSyria">(GMT+02:00) Syria</option>
	  <option value="GMT8_002" id="manntpTelAviv">(GMT+02:00) Tel Aviv, Jerusalem</option>
	  <option value="GMT9_002" id="manntpTurkey">(GMT+02:00) Turkey</option>
	  <option value="GMT_003" id="manntpBaghdad">(GMT+03:00) Baghdad</option>
	  <option value="GMT1_003" id="manntpKuwait">(GMT+03:00) Kuwait, Riyadh</option>
	  <option value="GMT2_003" id="manntpMoscow">(GMT+03:00) Moscow, St. Petersburg, Volgograd</option>
	  <option value="GMT3_003" id="manntpNairobi">(GMT+03:00) Nairobi</option>
	  <option value="GMT_003:30" id="manntpTehran">(GMT+03:30) Tehran</option>
	  <option value="GMT_004" id="manntpAbuDhabi">(GMT+04:00) Abu Dhabi, Muscat</option>
	  <option value="GMT1_004" id="manntpBaku">(GMT+04:00) Baku</option>
	  <option value="GMT2_004" id="manntpMauritius">(GMT+04:00) Mauritius</option>
	  <option value="GMT3_004" id="manntpTbilisi">(GMT+04:00) Tbilisi</option>
	  <option value="GMT4_004" id="manntpYerevan">(GMT+04:00) Yerevan</option>
	  <option value="GMT_004:30" id="manntpKabul">(GMT+04:30) Kabul</option>
	  <option value="GMT_005" id="manntpEkaterinburg">(GMT+05:00) Ekaterinburg</option>
	  <option value="GMT1_005" id="manntpPakistan">(GMT+05:00) Pakistan</option>
	  <option value="GMT2_005" id="manntpTashkent">(GMT+05:00) Tashkent</option>
	  <option value="GMT_005:30" id="manntpChennai">(GMT+05:30) Chennai, Kolkata, Mumbai, New Delhi</option>
	  <option value="GMT_005:45" id="manntpKathmandu">(GMT+05:45) Kathmandu</option>
	  <option value="GMT_006" id="manntpAlmaty">(GMT+06:00) Almaty, Novosibirsk</option>
	  <option value="GMT1_006" id="manntpBangladesh">(GMT+06:00) Bangladesh</option>
	  <option value="GMT2_006" id="manntpCentralsia">(GMT+06:00) Central Asia</option>
	  <option value="GMT_006:30" id="manntpRangoon">(GMT+06:30) Rangoon</option>
	  <option value="GMT_007" id="manntpBangkok">(GMT+07:00) Bangkok, Hanoi, Jakarta</option>
	  <option value="GMT1_007" id="manntpKrasnoyarsk">(GMT+07:00) Krasnoyarsk</option>
	  <option value="GMT_008" id="manntpBeijing">(GMT+08:00) Beijing, Chongqing, Hong Kong, Urum</option>
	  <option value="GMT1_008" id="manntpIrkutsk">(GMT+08:00) Irkutsk</option>
	  <option value="GMT2_008" id="manntpKualaLumpur">(GMT+08:00) Kuala Lumpur, Singapore</option>
	  <option value="GMT3_008" id="manntpPerth">(GMT+08:00) Perth</option>
	  <option value="GMT4_008" id="manntpTaipei">(GMT+08:00) Taipei</option>
	  <option value="GMT5_008" id="manntpUlaanbaatar">(GMT+08:00) Ulaanbaatar</option>
	  <option value="GMT_009" id="manntpOsaka">(GMT+09:00) Osaka, Sapporo, Tokyo</option>
	  <option value="GMT1_009" id="manntpSeoul">(GMT+09:00) Seoul</option>
	  <option value="GMT2_009" id="manntpYakutsk">(GMT+09:00) Yakutsk</option>
	  <option value="GMT_009:30" id="manntpAdelaide">(GMT+09:30) Adelaide</option>
	  <option value="GMT1_009:30" id="manntpDarwin">(GMT+09:30) Darwin</option>
	  <option value="GMT_010" id="manntpBrisbane">(GMT+10:00) Brisbane</option>
	  <option value="GMT1_010" id="manntpCanberra">(GMT+10:00) Canberra, Melbourne, Sydney</option>
	  <option value="GMT2_010" id="manntpGuam">(GMT+10:00) Guam, Port Moresby</option>
	  <option value="GMT3_010" id="manntpHobart">(GMT+10:00) Hobart</option>
	  <option value="GMT4_010" id="manntpVladivostok">(GMT+10:00) Vladivostok</option>
	  <option value="GMT_011" id="manntpMagadan">(GMT+11:00) Magadan</option>
	  <option value="GMT1_011" id="manntpsolomon">(GMT+11:00) Solomon Is., New Caledonia</option>
	  <option value="GMT_011:30" id="manntpNorfolk">(GMT+11:30) Norfolk Island</option>
	  <option value="GMT_012" id="manntpAuckland">(GMT+12:00) Auckland, Wellington</option>
	  <option value="GMT1_012" id="manntpFiji">(GMT+12:00) Fiji, Marshall Is.</option>
	  <option value="GMT2_012" id="manntpKamchatka">(GMT+12:00) Kamchatka</option>
	  <option value="GMT_013" id="manntpNuku">(GMT+13:00) Nuku'alofa</option>
	  <option value="0" id="manntpmsg">Please select your Time Zone.</option>
    </select>
  </td>
</tr>
<tr id="ds_tr" >
	<td class="head" id="mandaylightsavingtime">Daylight saving time</td>
	<td>
	<input size="1" maxlength="32" type="checkbox" name="onoffdaylightsaving" id="onoffdaylightsaving" onClick="OnDaylightSelecteChange()" >
	&nbsp;<font id="onoffdaylightsavingMsg" ></font><br />
	<table width="400" border="1" id="tabledaylightsavingtime" name="tabledaylightsavingtime" >
	  <tr>
	    <td id="mandaylightsavingtimeoffset" >offset</td>
	    <td colspan="2">
		<select name="daylightsavingtimeOffset" name="daylightsavingtimeOffset" >
			<option value="-2:00" >-2:00</option>
	        <option value="-1:00" >-1:00</option>
	        <option value="1:00" >+1:00</option>
	        <option value="2:00" >+2:00</option>
	    </select></td>
	  </tr>
	  <tr>
	    <td width="88" id="mandaylightsavingtimestarttime" >start time</td>
	    <td colspan="2">
		&nbsp;
		<select name="daylightsavingtimeStartMonth" name="daylightsavingtimeStartMonth" >
	      <option value="01" >1</option>
	      <option value="02" >2</option>
	      <option value="03" >3</option>
	      <option value="04" >4</option>
	      <option value="05" >5</option>
	      <option value="06" >6</option>
	      <option value="07" >7</option>
	      <option value="08" >8</option>
	      <option value="09" >9</option>
	      <option value="10" >10</option>
	      <option value="11" >11</option>
	      <option value="12" >12</option>
	      </select>
		  &nbsp;
	      <select name="daylightsavingtimeStartDay" name="daylightsavingtimeStartDay" >
	        <option value="01" >1</option>
	        <option value="02" >2</option>
	        <option value="03" >3</option>
	        <option value="04" >4</option>
	        <option value="05" >5</option>
	        <option value="06" >6</option>
	        <option value="07" >7</option>
	        <option value="08" >8</option>
	        <option value="09" >9</option>
	        <option value="10" >10</option>
	        <option value="11" >11</option>
	        <option value="12" >12</option>
	        <option value="13" >13</option>
	        <option value="14" >14</option>
	        <option value="15" >15</option>
	        <option value="16" >16</option>
	        <option value="17" >17</option>
	        <option value="18" >18</option>
	        <option value="19" >19</option>
	        <option value="20" >20</option>
	        <option value="21" >21</option>
	        <option value="22" >22</option>
	        <option value="23" >23</option>
	        <option value="24" >24</option>
	        <option value="25" >25</option>
	        <option value="26" >26</option>
	        <option value="27" >27</option>
	        <option value="28" >28</option>
	        <option value="29" >29</option>
	        <option value="30" >30</option>
	        <option value="31" >31</option>
	       </select>   
			&nbsp;
			<select name="daylightsavingtimeStartTime" name="daylightsavingtimeStartTime" >
	          <option value="01" >1 am</option>
	          <option value="02" >2 am</option>
	          <option value="03" >3 am</option>
	          <option value="04" >4 am</option>
	          <option value="05" >5 am</option>
	          <option value="06" >6 am</option>
	          <option value="07" >7 am</option>
	          <option value="08" >8 am</option>
	          <option value="09" >9 am</option>
	          <option value="10" >10 am</option>
	          <option value="11" >11 am</option>
			  <option value="12" >12 pm</option>
			  <option value="13" >1 pm</option>
	          <option value="14" >2 pm</option>
	          <option value="15" >3 pm</option>
	          <option value="16" >4 pm</option>
	          <option value="17" >5 pm</option>
	          <option value="18" >6 pm</option>
	          <option value="19" >7 pm</option>
	          <option value="20" >8 pm</option>
	          <option value="21" >9 pm</option>
	          <option value="22" >10 pm</option>
	          <option value="23" >11 pm</option>
			  <option value="24" >12 am</option>
	        </select></td>
	  </tr>
	  <tr>
	    <td width="88" id="mandaylightsavingtimeendtime" >end time</td>
	    <td colspan="2">
		&nbsp;
		<select name="daylightsavingtimeEndMonth" name="daylightsavingtimeEndMonth" >
	      <option value="01" >1</option>
	      <option value="02" >2</option>
	      <option value="03" >3</option>
	      <option value="04" >4</option>
	      <option value="05" >5</option>
	      <option value="06" >6</option>
	      <option value="07" >7</option>
	      <option value="08" >8</option>
	      <option value="09" >9</option>
	      <option value="10" >10</option>
	      <option value="11" >11</option>
	      <option value="12" >12</option>
	      </select>
		  &nbsp;
	      <select name="daylightsavingtimeEndDay" name="daylightsavingtimeEndDay" >
	        <option value="01" >1</option>
	        <option value="02" >2</option>
	        <option value="03" >3</option>
	        <option value="04" >4</option>
	        <option value="05" >5</option>
	        <option value="06" >6</option>
	        <option value="07" >7</option>
	        <option value="08" >8</option>
	        <option value="09" >9</option>
	        <option value="10" >10</option>
	        <option value="11" >11</option>
	        <option value="12" >12</option>
	        <option value="13" >13</option>
	        <option value="14" >14</option>
	        <option value="15" >15</option>
	        <option value="16" >16</option>
	        <option value="17" >17</option>
	        <option value="18" >18</option>
	        <option value="19" >19</option>
	        <option value="20" >20</option>
	        <option value="21" >21</option>
	        <option value="22" >22</option>
	        <option value="23" >23</option>
	        <option value="24" >24</option>
	        <option value="25" >25</option>
	        <option value="26" >26</option>
	        <option value="27" >27</option>
	        <option value="28" >28</option>
	        <option value="29" >29</option>
	        <option value="30" >30</option>
	        <option value="31" >31</option>
	        </select>
			&nbsp;
		<select name="daylightsavingtimeEndTime" name="daylightsavingtimeEndTime" >
	          <option value="01" >1 am</option>
	          <option value="02" >2 am</option>
	          <option value="03" >3 am</option>
	          <option value="04" >4 am</option>
	          <option value="05" >5 am</option>
	          <option value="06" >6 am</option>
	          <option value="07" >7 am</option>
	          <option value="08" >8 am</option>
	          <option value="09" >9 am</option>
	          <option value="10" >10 am</option>
	          <option value="11" >11 am</option>
			  <option value="12" >12 pm</option>
			  <option value="13" >1 pm</option>
	          <option value="14" >2 pm</option>
	          <option value="15" >3 pm</option>
	          <option value="16" >4 pm</option>
	          <option value="17" >5 pm</option>
	          <option value="18" >6 pm</option>
	          <option value="19" >7 pm</option>
	          <option value="20" >8 pm</option>
	          <option value="21" >9 pm</option>
	          <option value="22" >10 pm</option>
	          <option value="23" >11 pm</option>
			  <option value="24" >12 am</option>
	        </select></td>
	    </tr>
	  </table></td>
</tr>
<tr>
  <td class="head" id="manNTPServer">NTP Server</td>
  <td><input size="32" maxlength="64" name="NTPServerIP" value="<% getCfgGeneral(1, "NTPServerIP"); %>" type="text">
	<br>&nbsp;&nbsp;<font color="#808080">ex:&nbsp;time.nist.gov</font>
	<br>&nbsp;&nbsp;<font color="#808080">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;ntp0.broad.mit.edu</font>
	<br>&nbsp;&nbsp;<font color="#808080">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;time.stdtime.gov.tw</font>
  </td>
</tr>
<tr>
  <td class="head" id="manNTPSync">NTP synchronization</td>
  <td><input size="4" name="NTPSync" value="<% getCfgGeneral(1, "NTPSync"); %>" type="text"> </td>
</tr>
</tbody></table>

<table width="600" border="0" cellpadding="2" cellspacing="1">
  <tr id="sbutton2" align="center">
    <td>
      <input type=submit style="{width:120px;}" value="Apply" id="manNTPApply" onClick="return NTPFormCheck();"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel"id="manNTPCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>

<!-- ================= GreenAP ================= -->
<form method="post" name="GreenAP" action="/goform/GreenAP">
<table id="div_greenap" width="600" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr width="600">
    <td class="title" colspan="3" id="manGAPTitle">Green AP</td>
  </tr>
  <tr align="center" width="600">
    <td class="head" id="manGAPTime">Time</td>
    <td  class="head" id="manGAPAction">Action</td>
  </tr>
  <script language="JavaScript" type="text/javascript">
  for(var j=1;j<=4;j++)
  {
	  var item = "<tr align=\"center\"><td  width=\"299\"><select name=\"GAPSHour"+j+"\">";
	  for(var i=0;i<24;i++)
	  {
		  if (i < 10)
			  item += "<option value=\""+i+"\">0"+i+"</option>";
		  else
			  item += "<option value=\""+i+"\">"+i+"</option>";
	  }
	  item += "</select>&nbsp;:&nbsp;";
	  document.write(item);

	  var item = "<select name=\"GAPSMinute"+j+"\">";
	  for(var i=0;i<60;i++)
	  {
		  if (i < 10)
			  item += "<option value=\""+i+"\">0"+i+"</option>";
		  else
			  item += "<option value=\""+i+"\">"+i+"</option>";
	  }
	  item += "</select>&nbsp;~&nbsp;";
	  document.write(item);

	  var item = "<select name=\"GAPEHour"+j+"\">";
	  for(var i=0;i<24;i++)
	  {
		  if (i < 10)
			  item += "<option value=\""+i+"\">0"+i+"</option>";
		  else
			  item += "<option value=\""+i+"\">"+i+"</option>";
	  }
	  item += "</select>&nbsp;:&nbsp;";
	  document.write(item);

	  var item = "<select name=\"GAPEMinute"+j+"\">";
	  for(var i=0;i<60;i++)
	  {
		  if (i < 10)
			  item += "<option value=\""+i+"\">0"+i+"</option>";
		  else
			  item += "<option value=\""+i+"\">"+i+"</option>";
	  }
	  item += "</select></td>";
	  item += "<td  width=\"280\"><select name=\"GAPAction"+j+"\" onChange=\"greenap_action_switch('"+j+"')\">";
	  item += "<option value=\"Disable\" id=\"manGAPActionDisable"+j+"\">Disable</option>";
	  item += "<option value=\"WiFiOFF\">WiFi TxPower OFF</option>";
	  item += "<option value=\"TX25\">WiFi TxPower 25%</option>";
	  item += "<option value=\"TX50\">WiFi TxPower 50%</option>";
	  item += "<option value=\"TX75\">WiFi TxPower 75%</option";
	  item += "</select></td></tr>";
	  document.write(item);
  }
  </script> 
</table>
<table id="div_greenap_submit" width="600" border="0" cellpadding="2" cellspacing="1">
  <tr id="sbutton3" align="center">
    <td  width="600">
      <input type="submit" style="{width:120px;}" value="Apply" id="manGreenAPApply" onClick="return GreenAPFormCheck();" >&nbsp;&nbsp;
      <input type="reset" style="{width:120px;}" value="Cancle" id="manGreenAPCancle" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>

<!-- ================= DDNS  ================= -->
<form method="post" name="DDNS" action="/goform/DDNS">
<table id="div_ddns" width="600" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
<tbody width="600"><tr>
  <td class="title" colspan="2" id="manDdnsSet">DDNS Settings</td>
</tr>
<tr>
  <td width="180" class="head" id="DdnsProvider">Dynamic DNS Provider</td>
  <td width="420">
    <select onChange="DDNSupdateState()" name="DDNSProvider">
      <option value="none" id="manDdnsNone"> None </option>
      <option value="dyndns.org"> Dyndns.org </option>
      <option value="freedns.afraid.org"> freedns.afraid.org </option>
      <option value="zoneedit.com"> www.zoneedit.com </option>
      <option value="no-ip.com"> www.no-ip.com </option>
    </select>
  </td>
</tr>
<tr width="600">
  <td class="head" id="manDdnsAccount"  width="180">Account</td>
  <td width="420"><input size="16" name="Account" value="<% getCfgGeneral(1, "DDNSAccount"); %>" type="text"> </td>
</tr>
<tr width="600">
  <td class="head" id="manDdnsPasswd"  width="180">Password</td>
  <td width="420"><input size="16" name="Password" value="<% getCfgGeneral(1, "DDNSPassword"); %>" type="password"> </td>
</tr>
<tr width="600">
  <td class="head" id="manDdns"  width="180">DDNS</td>
  <td width="420"><input size="32" name="DDNS" value="<% getCfgGeneral(1, "DDNS"); %>" type="text"> </td>
</tr>
</tbody></table>
<br>
<table id="div_ddns_submit" width="600" border="0" cellpadding="2" cellspacing="1">
  <tr id="sbutton4" align="center">
    <td  width="600">
      <input type=submit style="{width:120px;}" value="Apply" id="manDdnsApply" onClick="return DDNSFormCheck(); "> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="manDdnsCancel" onClick="window.location.reload()">
    </td>
  </tr>
</table>
</form>


</td></tr></table>
 </center>
</div>
</body></html>
