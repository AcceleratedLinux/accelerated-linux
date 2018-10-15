<html>
<head>
<title></title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">

<script type="text/javascript" src="/lang/b28n.js"></script>
<script language="JavaScript">
Butterlate.setTextDomain("admin");

var	 wanipAssi = "<% getCfgGeneral(1, "wan_ip_assignment"); %>";
/* static, Dynamic, pppoe, 3G, pptp, l2tp */
var  delayTime = Array("45", "45", "55", "100", "45", "55"); 
//setTimeout("window.location.href=\"/adm/status.asp\";", delayTime[wanipAssi] * 1000);
setTimeout( "makeRequest();", 10 * 1000);
var secs;
var timerID = null;
var timerRunning = false;
var ajax, timerid_timeout = null;
var time_to_wait = 500;

function check_DUT_sta() {
    if (http_reflash_page.readyState == 4) {
        if (http_reflash_page.status == 200) {
			var result = http_reflash_page.responseTest;
			if(result = ""){
			}else{
				// refresh
				window.location.replace("http://" + window.location.hostname + "/adm/status.asp");
				return true;
			}
        } else { // http_reflash_page.status != 200
        }
    }
}
var http_reflash_page = false;
function makeRequest() {
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_reflash_page = new XMLHttpRequest();
        if (http_reflash_page.overrideMimeType) {
            http_reflash_page.overrideMimeType('text/xml');
        }
    } else if (window.ActiveXObject) { // IE
        try {
            http_reflash_page = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_reflash_page = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_reflash_page) {
        //alert('Giving up :( Cannot create an XMLHTTP instance');
        return false;
    }
    http_reflash_page.onreadystatechange = check_DUT_sta;
    http_reflash_page.open('GET', "/adm/status.asp", true);
	http_reflash_page.setRequestHeader("If-Modified-Since", "0");
    http_reflash_page.send(null);
	setTimeout( "makeRequest();", 3 * 1000);
}
function ajax_create()
{
    var obj;
    if (window.ActiveXObject)
    {
        try 
        {
            obj = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e)
        {
            try
            {
                obj = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e2) {
                obj = null;
            }
        }
    } else if (window.XMLHttpRequest)
    {
        obj = new XMLHttpRequest();
    } else {
        obj = null;
    }
    return obj;
}

function StartTheTimer(){
	if (secs==0){
		TimeoutReload(5);
		parent.web_page_selected('3','1');
	}else{
		self.status = secs;
		secs = secs - 1;
		timerRunning = true;
		timerID = self.setTimeout("StartTheTimer()", 1000);
	}
}

function TimeoutReload(timeout)
{
	secs = timeout;
	if(timerRunning)
		clearTimeout(timerID);
	timerRunning = false;
	StartTheTimer();
}

//20090505 Gord# add for testing saving bar
var counter = 20;

function startProgress(){
	document.getElementById("waitouter").style.display="block";
	savingbar();
}

function savingbar(){
	if (counter < 101){
   	document.getElementById("waitinner").style.width = counter+"%";
     	counter++;

		if(counter == 101)
			counter = 20;
//     	setTimeout("savingbar()",200);
   }
}

function GotoPrevious()
{
    window.location.replace(document.referrer);
}

function time_count()
{
    var mesg = "";
    var i, noword = "...";

    mesg = time_to_wait;
    time_to_wait --;
    if (time_to_wait < 1)
    {
        i = ((time_to_wait*-1) % 3)
        mesg = '.' + noword.substr(0, i);
    }
//	bar_paint(mesg);
	savingbar();
}

function timeout_error()
{
    ajax.abort();
    bootcheck_start();
}

function bootcheck_start()
{
    ajax.onreadystatechange = bootcheck_cb;
    ajax.open ("GET", "/", true);
    ajax.setRequestHeader("If-Modified-Since","0");
    ajax.send ("");
    setTimeout("timeout_error()", 5000);
}

function bootcheck_cb()
{
    if (ajax.readyState == 4)
  	{
    	if (ajax.status == 200)
            window.open("/", "_top");
  	}
}

function initTranslation()
{
	var e = document.getElementById("id_wait_reboot");
	e.innerHTML = _("config reboot wait");
	e = document.getElementById("waitinner");
	e.innerHTML = _("admin booting");
}

function initValue()
{
	initTranslation();
	//ajax = ajax_create();
	document.getElementById("waitouter").style.display="block";
	//setTimeout("bootcheck_start()", 10000);
    if (timerid_timeout == null)
        timerid_timeout = setInterval("time_count()", 100);
//	startProgress();
//	TimeoutReload(60);
	//parent.web_page_redirect('6', '4');
}
</script>
</head>

<body onLoad="initValue()">
<table width="100%" border="0" cellspacing="0" cellpadding="10" height="100%">
  <tr> 
    <td> 
      <div align="center"> 
        <p><b><font color="#FF0000" id="id_wait_reboot"><br>
          Rebooting, please wait a moment.</font></b></p>
        <p>&nbsp;</p>
      </div>
<!-- 20090505 Gord Add Saving Bar-->
<div id="waitouter" style="width: 500px; height: 20px; border: 6px; display:none;">
   <div id="waitinner" style="position: relative; height: 20px; background-color: #2C5EA4; 
   width: 20%; font-size:15px; font-weight:bolder; color:#FFFFF0; ">Booting Now...</div>
</div>
<!-- Gord End -->
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
</body>
</html>
