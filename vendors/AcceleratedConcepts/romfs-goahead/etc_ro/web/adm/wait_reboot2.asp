<html>
<head>
<title></title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">

<script type="text/javascript" src="/lang/b28n.js"></script>
<script language="JavaScript">
Butterlate.setTextDomain("admin");

setTimeout( "makeRequest();", 10 * 1000);
var timerid_timeout = null;
var number_of_errors = 0;

function check_DUT_sta() {
    if (http_reflash_page.readyState == 4) {
        if (http_reflash_page.status == 200) {
			if (number_of_errors > 0) {
				var result = http_reflash_page.responseTest;
				if(result = ""){
				}else{
					// refresh
					window.location.replace("http://" + window.location.hostname + "/adm/upgrade_success.asp");
					return true;
				}
			}
        } else { // http_reflash_page.status != 200
			number_of_errors++;
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
    http_reflash_page.open('GET', "/adm/upgrade_success.asp", true);
	http_reflash_page.setRequestHeader("If-Modified-Since", "0");
    http_reflash_page.send(null);
	setTimeout( "makeRequest();", 3 * 1000);
}

var start_counter = 42;
var counter = 42;

function savingbar(){
	if (counter <= 100) {
		document.getElementById("waitinner").style.width = counter+"%";
     	counter++;
		if (counter > 100)
			counter = start_counter;
   }
}

function initTranslation()
{
	var e = document.getElementById("id_wait_firmware");
	e.innerHTML = _("config firmware wait");
	e = document.getElementById("waitinner");
	e.innerHTML = _("admin flashing");
	// start_counter = e.innerHTML.lenth;
	// counter = start_counter;
}

function initValue()
{
	initTranslation();
	document.getElementById("waitouter").style.display="block";
    if (timerid_timeout == null)
        timerid_timeout = setInterval("savingbar()", 100);
}
</script>
</head>

<body onLoad="initValue()">
<table width="100%" border="0" cellspacing="0" cellpadding="10" height="100%">
  <tr> 
    <td> 
      <div align="center"> 
        <p><b><font color="#FF0000" id="id_wait_firmware"><br>
          Firmware Upgrade in progress, please wait a moment.</font></b></p>
        <p>&nbsp;</p>
      </div>
<!-- 20090505 Gord Add Saving Bar-->
<div id="waitouter" style="width: 500px; height: 20px; border: 6px; display:none;">
   <div id="waitinner" style="position: relative; height: 20px; background-color: #2C5EA4; 
   width: 20%; font-size:15px; font-weight:bolder; color:#FFFFF0;
   ">Programming Flash Now...</div>
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
