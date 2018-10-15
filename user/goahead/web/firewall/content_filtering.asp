<html>
<head>
<title>Content Filter Settings</title>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">
<script type="text/javascript" src="/lang/b28n.js"></script>
<script type="text/javascript" src="/common.js"></script>

<script language="JavaScript" type="text/javascript">
restartPage_init();

var sbuttonMax=5;
var filter_urllist, filter_hostlist;

Butterlate.setTextDomain("firewall");


var URLFilterNum = 0;
var HostFilterNum = 0;

function deleteClick()
{
    return true;
}

function formCheck()
{
   return true;
}

function initTranslation()
{
	var e = document.getElementById("ContentFilterTitle");
	e.innerHTML = _("content filter title");
	e = document.getElementById("ContentFilterIntrodution");
	e.innerHTML = _("content filter introduction");
	e = document.getElementById("WebsContentFilter");
	e.innerHTML = _("content filter webs content filter");
	e = document.getElementById("WebsContentFilterFilter");
	e.innerHTML = _("content filter webs content filter filter");
	e = document.getElementById("WebsContentFilterApply");
	e.value = _("content filter webs content filter apply");	
	e = document.getElementById("WebsContentFilterReset");
	e.value = _("content filter webs content filter reset");	
	/*e = document.getElementById("WebURLFilterTitle");
	e.innerHTML = _("content filter webs URL filter title");*/
	e = document.getElementById("WebsURLFilterKeyword");
	e.innerHTML = _("content filter webs url fitler url");	
	e = document.getElementById("WebURLFilterCurrent");
	e.innerHTML = _("content filter webs url filter current");	
	e = document.getElementById("WebURLFilterNo");
	e.innerHTML = _("content filter webs url fitler No");	
	e = document.getElementById("WebURLFilterURL");
	e.innerHTML = _("content filter webs url fitler url");	
	e = document.getElementById("WebURLFilterDel");
	e.value = _("content filter webs url fitler del");
	e = document.getElementById("WebURLFilterReset");
	e.value = _("content filter webs url fitler reset");
	e = document.getElementById("WebURLFilterAddTitle");
	e.innerHTML = _("content filter webs url fitler add title");	
	e = document.getElementById("WebURLFilterAdd");
	e.value = _("content filter webs url fitler add");		
	e = document.getElementById("WebURLFilterReset2");
	e.value = _("content filter webs url fitler reset");		
	/*e = document.getElementById("WebsHostFilterTitle");
	e.innerHTML = _("content filter webs host fitler title");*/
	e = document.getElementById("WebsHostFilterCurrent");
	e.innerHTML = _("content filter webs host fitler current");	
	e = document.getElementById("WebsHostFilterNo");
	e.innerHTML = _("content filter webs host fitler no");	
	e = document.getElementById("WebsHostFilterHost");
	e.innerHTML = _("content filter webs host fitler host");	
	e = document.getElementById("WebsHostFilterDel");
	e.value = _("content filter webs host fitler del");	
	e = document.getElementById("WebsHostFilterReset");
	e.value = _("content filter webs host fitler reset");	
	e = document.getElementById("WebsHostFilterAddTitle");
	e.innerHTML = _("content filter webs host fitler add title");	
	e = document.getElementById("WebsHostFilterKeyword");
	e.innerHTML = _("content filter webs host fitler keyword");	
	e = document.getElementById("WebsHostFilterAdd");
	e.value = _("content filter webs host fitler add");	
	e = document.getElementById("WebsHostFilterReset2");
	e.value = _("content filter webs host fitler reset");	

}

function updateState()
{
	initTranslation();
	if (document.webContentFilter.websFilterProxy.value == "1")
		document.webContentFilter.websFilterProxy.checked = true;
	if (document.webContentFilter.websFilterJava.value == "1")
		document.webContentFilter.websFilterJava.checked = true;
	if (document.webContentFilter.websFilterActivex.value == "1")
		document.webContentFilter.websFilterActivex.checked = true;
/*
	if (document.webContentFilter.websFilterCookies.value == "1")
		document.webContentFilter.websFilterCookies.checked = true;
*/
}

function webContentFilterClick()
{
	document.webContentFilter.websFilterProxy.value = document.webContentFilter.websFilterProxy.checked ? "1": "0";
	document.webContentFilter.websFilterJava.value = document.webContentFilter.websFilterJava.checked ? "1": "0";
	document.webContentFilter.websFilterActivex.value = document.webContentFilter.websFilterActivex.checked ? "1": "0";
	document.webContentFilter.websFilterCookies.value = document.webContentFilter.websFilterCookies.checked ? "1": "0";
	return true;
}

function deleteWebsURLClick()
{
	for(i=0; i< URLFilterNum; i++){
		var tmp = eval("document.websURLFilterDelete.DR"+i);
		if(tmp.checked == true)
		{
			sbutton_disable(sbuttonMax); 
			restartPage_block(); 
			
			return true;
		}
	}
	alert(_("content filter alert please select the rule to be deleted"));
	return false;
}

function is_valid_url(url)
{
	var str, i, domainname;
	/*i = url.indexOf("http://");
	if (i != 0) {
		return false;
	}*/
	i = url.indexOf("?");
	if (i == -1)
		i = url.length;
	domainname = url.substr(0, i);
	if (!is_valid_domainname(domainname) || domainname == "") {
		return false;
	}
	return true;
}

function AddWebsURLFilterClick()
{
	var cols, i;
// ml 
	if(URLFilterNum >= 32){
		alert(_("content filter alert the maximum rule count is 32"));
		return false;  
	}
//
	if(document.websURLFilter.addURLFilter.value == ""){
		alert(_("content filter alert please enter a url filter"));
		document.websURLFilter.addURLFilter.focus();
		return false;
	}
	if (!is_valid_url(document.websURLFilter.addURLFilter.value)){
		alert(_("content filter alert please enter a valid url"));
		document.websURLFilter.addURLFilter.focus();
		return false;
	}
	cols = filter_urllist.split(';');
	if (cols.length != 0) {
		for (i = 0; i < cols.length; i++) {
			if (cols[i] == document.websURLFilter.addURLFilter.value) {
				alert(_("content filter alert duplicated url string"));
				document.websURLFilter.addURLFilter.focus();
				return false;
			}
		}
	}
	sbutton_disable(sbuttonMax); 
	restartPage_block();
	return true;
}

function deleteWebsHostClick()
{
	for(i=0; i< HostFilterNum; i++){
		var tmp = eval("document.websHostFilterDelete.DR"+i);
		if(tmp.checked == true)
		{
			sbutton_disable(sbuttonMax); 
			restartPage_block(); 
			
			return true;
		}
			
	}
	alert(_("content filter alert please select the rule to be deleted"));
	
	return false;
}

function AddWebsHostFilterClick()
{
	var cols, i;
// ml	
	if(HostFilterNum >= 32){
		alert(_("content filter alert the maximum rule count is 32"));
		return false;
	}
//
	if(document.websHostFilter.addHostFilter.value == ""){
		alert(_("content filter alert please enter a host filter"));
		return false;
	}
	if(
		!is_valid_hostname(document.websHostFilter.addHostFilter.value) ||
		document.websHostFilter.addHostFilter.value.indexOf("..") > 0 ||
		document.websHostFilter.addHostFilter.value.substr(0, 1) == '.' ||
                document.websHostFilter.addHostFilter.value.substr(document.websHostFilter.addHostFilter.value.length-1, 1) == '.'
		){
		alert(_("content filter alert invalid host name"))
		document.websHostFilter.addHostFilter.focus();
		return false;
	}
	cols = filter_hostlist.split(';');
	if (cols.length != 0) {
		for (i = 0; i < cols.length; i++) {
			if (cols[i] == document.websHostFilter.addHostFilter.value) {
				alert(_("content filter alert duplicated host string"));
				document.websHostFilter.addHostFilter.focus();
				return false;
			}
		}
	}
	sbutton_disable(sbuttonMax); 
	restartPage_block();
	
	return true;
}

</script>
</head>


                         <!--     body      -->
<body onload="updateState()" bgcolor="#FFFFFF">

<div align="center">
 <center>

<table class="body"><tr><td>

<table width="540" border="1" cellpadding="2" cellspacing="1">

<tr>
  <td class="title" colspan="2" id="ContentFilterTitle">Content Filter  Settings </td>
<% checkIfUnderBridgeModeASP(); %>
</tr>
<tr>
<td colspan="2">
<p class="head" id="ContentFilterIntrodution"></p>
</td>
</tr>

</table>

<br>

<form method=post name="webContentFilter" action=/goform/webContentFilter style="display:none">
<table width="540" border="1" cellpadding="2" cellspacing="1">
<tr>
  <td class="title" colspan="2" id="WebsContentFilter" width="530">Webs Content Filter</td>
</tr>
<tr>
	<td id="WebsContentFilterFilter" width="72" bgcolor="#DBE2EC">Filter</td>
	<td width="465">
		<input type=checkbox name=websFilterProxy value="<% getCfgZero(1, "websFilterProxy"); %>" > Proxy
		<input type=checkbox name=websFilterJava value="<% getCfgZero(1, "websFilterJava"); %>" > Java
		<input type=checkbox name=websFilterActivex value="<% getCfgZero(1, "websFilterActivex"); %>" > ActiveX
<!--	<input type=checkbox name=websFilterCookies value="<% getCfgZero(1, "websFilterCookies"); %>" > Cookies  -->
	</td>
</tr>
</table>

<table width="540" border="0" cellpadding="2" cellspacing="1">
<tr id="sbutton0" align="center">
	<td>
		<input type="submit" value="Apply" id="WebsContentFilterApply" name="addFilterPort" onClick="sbutton_disable(sbuttonMax); restartPage_block(); return webContentFilterClick(); "> &nbsp;&nbsp;
		<input type="reset" value="Reset" id="WebsContentFilterReset" name="reset">
	</td>
</tr>
</table>	
</form>

<br><hr>
<!--<h1 id="WebURLFilterTitle">Webs URL Filter Settings </h1>-->
<form action=/goform/websURLFilterDelete method=POST name="websURLFilterDelete">
<table width="540" border="1" cellpadding="2" cellspacing="1">	
	<tr>
		<td class="title" colspan="5" id="WebURLFilterCurrent" width="530">Current Webs URL Filters: </td>
	</tr>

	<tr>
		<td id="WebURLFilterNo" width="72" bgcolor="#DBE2EC">No.</td>
		<td id="WebURLFilterURL" width="465" bgcolor="#DBE2EC">URL</td>
	</tr>

	<script language="JavaScript" type="text/javascript">
	var i;
	var entries = new Array();
	var all_str = "<% getCfgGeneral(1, "websURLFilters"); %>";
	filter_urllist = all_str;

	if(all_str.length){
		entries = all_str.split(";");
		for(i=0; i<entries.length; i++){
			document.write("<tr><td>");
			document.write(i+1);
			document.write("<input type=checkbox name=DR"+i+"></td>");
	
			document.write("<td>"+ entries[i] +"</td>");
			document.write("</tr>\n");
		}

		URLFilterNum = entries.length;
	}
	</script>
</table>

<table width="540" border="0" cellpadding="2" cellspacing="1">
<tr id="sbutton1" align="center">
	<td>
		<input type="submit" value="Delete" id="WebURLFilterDel" name="deleteSelPortForward" onClick="return deleteWebsURLClick();">&nbsp;&nbsp;
		<input type="reset" value="Reset" id="WebURLFilterReset" name="reset">
	</td>
</tr>
</table>
</form>

<form action=/goform/websURLFilter method=POST name="websURLFilter" onsubmit="return AddWebsURLFilterClick();">
<table width="540" border="1" cellpadding="2" cellspacing="1">	
	<tr>
		<td class="title" colspan="5" id="WebURLFilterAddTitle" width="530">Add a URL Filter: </td>
	</tr>
	<tr>
		<!--<td class="head" id="WebsURLFilterKeyword" >URL</td>-->
		<td id="WebsURLFilterKeyword" width="72" bgcolor="#DBE2EC" >URL</td>
		<td width="465"> Http(s):// <input name="addURLFilter" size="16" maxlength="32" type="text"> </td>
	</tr>
</table>

<table width="540" border="0" cellpadding="2" cellspacing="1">
<tr id="sbutton2" align="center">
	<td>
		<input type="submit" value="Add" id="WebURLFilterAdd" name="addwebsurlfilter">&nbsp;&nbsp;
		<input type="reset" value="Reset" id="WebURLFilterReset2" name="reset">
	</td>
</tr>
</table>
</form>

<br><hr>
<!--<h1 id="WebsHostFilterTitle">Webs Host Filter Settings </h1>-->
<form action=/goform/websHostFilterDelete method=POST name="websHostFilterDelete">
<table width="540" border="1" cellpadding="2" cellspacing="1">	
	<tr>
		<td class="title" colspan="5" id="WebsHostFilterCurrent" width="530">Current Website Host Filters: </td>
	</tr>

	<tr>
		<td id="WebsHostFilterNo" width="72" bgcolor="#DBE2EC">No.</td>
		<td id="WebsHostFilterHost" width="465" bgcolor="#DBE2EC"> Host(Keyword)</td>
	</tr>

	<script language="JavaScript" type="text/javascript">
	var i;
	var entries = new Array();
	var all_str = "<% getCfgGeneral(1, "websHostFilters"); %>";
	filter_hostlist = all_str;

	if(all_str.length){
		entries = all_str.split(";");

		for(i=0; i<entries.length; i++){
			document.write("<tr><td>");
			document.write(i+1);
			document.write("<input type=checkbox name=DR"+i+"></td>");

			document.write("<td>"+ entries[i] +"</td>");
			document.write("</tr>\n");
		}

		HostFilterNum = entries.length;
	}
	</script>
</table>

<table width="540" border="0" cellpadding="2" cellspacing="1">
<tr id="sbutton3" align="center">
	<td>
		<input type="submit" value="Delete" id="WebsHostFilterDel" name="deleteSelPortForward" onClick="return deleteWebsHostClick();">&nbsp;&nbsp;
		<input type="reset" value="Reset" id="WebsHostFilterReset" name="reset">
	</td>
</tr>
</table>
</form>

<form action=/goform/websHostFilter method=POST name="websHostFilter" onsubmit="return AddWebsHostFilterClick();">

<table width="540" border="1" cellpadding="2" cellspacing="1">	
	<tr>
		<td class="title" colspan="5" id="WebsHostFilterAddTitle" width="530">Add a Host(keyword) Filter: </td>
	</tr>
	<tr>
		<td id="WebsHostFilterKeyword" width="72" bgcolor="#DBE2EC">Keyword</td>
		<td width="465"> <input name="addHostFilter" size="16" maxlength="32" type="text"> </td>
	</tr>
</table>

<table width="538" border="0" cellpadding="2" cellspacing="1">
<tr id="sbutton4" align="center">
	<td width="532">
		<input type="submit" value="Add" id="WebsHostFilterAdd" name="addwebscontentfilter">&nbsp;&nbsp;
		<input type="reset" value="Reset" id="WebsHostFilterReset2" name="reset">
	</td>
</tr>
</table>		
</form>

</td></tr></table>
 </center>
</div>
</body>
</html>
