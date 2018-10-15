<html>
<head>
<title></title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<meta HTTP-EQUIV="REFRESH" target=\"topFrame\" content="120; url=/home.asp">
</head>
<script type="text/javascript" src="/lang/b28n.js"></script>
<script language="JavaScript">
/*Butterlate.setTextDomain("admin");*/
var	 wanipAssi = "<% getCfgGeneral(1, "wan_ip_assignment"); %>";
/* static, Dynamic, pppoe, 3G, pptp, l2tp */
var  delayTime = Array("45", "45", "55", "100", "45", "55"); 
//setTimeout("window.location.reload();", delayTime[wanipAssi] * 1000);
setTimeout( "makeRequest();", 10 * 1000);
var http_reflash_page = false;

function check_DUT_sta() {
    if (http_reflash_page.readyState == 4) {
        if (http_reflash_page.status == 200) {
			var result = http_reflash_page.responseTest;
			if(result = ""){
			}else{
				// refresh
				window.location.replace("http://" + window.location.hostname + "/home.asp");
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

function initTranslation()
{
	/*var e = document.getElementById("uploadSucess1");
	e.innerHTML = _("uploadSucess1");
	e = document.getElementById("uploadSucess2");
	e.innerHTML = _("uploadSucess2");*/
}

function initValue()
{
	initTranslation();
}
</script>


<body  bgcolor="#FFFFFF" onLoad="initValue()">
<div align="center">
  <center>
<table width="872" border="0" cellspacing="0" cellpadding="10" height="600" style="border-collapse: collapse" bgcolor="#4F7599">
  <tr> 
    <td align="center"> 
      <div align="center"> 
        <p><b><font color="#FFFFFF" id="uploadSucess1" size="4" >Upgrade Successfully.<br>
        </font><font color="#FFFFFF" id="id_wait_reboot"> <br>
          </font><font color="#FF0000" id="uploadSucess2" size="4" >Rebooting, please wait a moment.</font></b></p>
        <p>&nbsp;</p>
      </div>
    </td>
  </tr>
 
</table>
  </center>
</div>
</body>
</html>