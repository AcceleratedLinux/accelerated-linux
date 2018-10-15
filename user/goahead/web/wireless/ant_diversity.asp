<html><head><title>Antenna Diversity</title>

<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<style type="text/css">
<!--
#loading {
       width: 250px;
       height: 200px;
       background-color: #3399ff;
       position: absolute;
       left: 50%;
       top: 50%;
       margin-top: -150px;
       margin-left: -250px;
       text-align: center;
}
-->
</style>

<script language="JavaScript" type="text/javascript">
document.write('<div id="loading" style="display: none;"><br><br><br>Uploading firmware <br><br> Please be patient and don\'t remove usb device if it presented...</div>');
Butterlate.setTextDomain("wireless");

var secs
var timerID = null
var timerRunning = false
var timeout = 2
var delay = 1000

function InitializeTimer(){
    // Set the length of the timer, in seconds
    secs = timeout
    StopTheClock()
    StartTheTimer()
}

function StopTheClock(){
    if(timerRunning)
        clearTimeout(timerID)
    timerRunning = false
}

function StartTheTimer(){

    if (secs==0){
        StopTheClock()

        timerHandler();

        secs = timeout
        StartTheTimer()
    }else{
        self.status = secs
        secs = secs - 1
        timerRunning = true
        timerID = self.setTimeout("StartTheTimer()", delay)
    }
}

function timerHandler(){

	makeRequest("/goform/getAntenna", "n/a");
}

var http_request = false;
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
        alert('Giving up :( Cannot create an XMLHTTP instance');
        return false;
    }
    http_request.onreadystatechange = alertContents;
    http_request.open('GET', url, true);
    http_request.send(content);
}

function alertContents() {
	if (http_request.readyState == 4) {
		if (http_request.status == 200) {
			updateAntennaStatus( http_request.responseText);
		} else {
			//alert('There was a problem with the request.');
		}
	}
}


function updateAntennaStatus(str)
{
	document.getElementById("UpdateAntenna").innerHTML = "Ant" + str;
}


function initTranslation()
{

}

function pageInit(){
	initTranslation();

	makeRequest("/goform/getAntenna", "n/a");
	InitializeTimer();

	mode = "<% getCfgGeneral(1, "AntennaDiversity"); %>"
	if(mode == "Disable"){
		document.AntennaDiversity.ADSelect.options.selectedIndex = 0;
	}else if(mode == "Enable_Algorithm1"){
		document.AntennaDiversity.ADSelect.options.selectedIndex = 1;
	}else if(mode == "Antenna0"){
		document.AntennaDiversity.ADSelect.options.selectedIndex = 2;
	}else if(mode == "Antenna2"){
		document.AntennaDiversity.ADSelect.options.selectedIndex = 3;
	}else{
		document.AntennaDiversity.ADSelect.options.selectedIndex = 0;
	}
}
</script></head><body onLoad="pageInit()">
<table class="body"><tbody><tr><td>
<h1 id="AntennaDiversityIntroStr">Antenna Diversity</h1>
<p><font id="AntennaDiversityIntro2Str">Configure the Antenna Diversity setting to increase the performance. </font></p>

<form method="post" name="AntennaDiversity" action="/goform/AntennaDiversity">
<table border="1" cellpadding="2" cellspacing="1" width="95%">
<tbody><tr>
	<td class="title" colspan="2" id="AntennaDiversity">Antenna Diversity</td>
</tr>
<tr>
	<td class="head" id="AntennaDiversityModeStr">Mode:</td>
	<td>
		<select id="ADSelect" name="ADSelect">
			<option value="Disable">Disable</option>
			<option value="Enable_Algorithm1">Enable Algorithm1(BBP)</option>
			<option value="Antenna0">fixed at Ant0</option>
			<option value="Antenna2">fixed at Ant2</option>
		</select>
	</td>
</tr>
<tr>
	<td class="head" id="AntennaDiversityAntennaStr">Antenna:</td>
	<td id="UpdateAntenna"> &nbsp; </td>
</tr>
</tbody>
</table>
<input value="Apply" id="AntennaDiversitySubmitStr" name="AntennaDiversitySubmitStr" type="submit"> &nbsp;&nbsp;
</form>

</td></tr></tbody></table>
</body></html>
