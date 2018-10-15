<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Ralink Wireless Station AMPDU Originator Add</title>
<script language="JavaScript" type="text/javascript">

function initValue()
{
	<% setRefreshSta11nCfg(); %>
}

function getConnectedBSSID()
{
	var tmp = "<% getStaConnectedBSSID(); %>";
	document.write(tmp);
}

function submit_apply()
{
	document.sta_org_add.submit();
	opener.location.reload();
	window.close();
}

function selectedBSSID(tmp)
{
	document.sta_org_add.selectedbssid.value = tmp;

	document.sta_org_add.mpdu_apply.disabled = true;
	if (document.sta_org_add.mac.checked)
		document.sta_org_add.mpdu_apply.disabled = false;
}

</script>
</head>


<body onLoad="initValue()">
<table class="body"><tr><td>

<h1>Add AMPDU Originator</h1>
<hr />

<form method=post name="sta_org_add" action="/goform/setStaOrgAdd">
<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td class="title" colspan="2">MPDU Aggregation</td>
  </tr>
  <tr>
    <td class="head">TID</td>
    <td>
      <select name="tid" size="1">
	<option value=0 selected>0</option>
	<option value=1>1</option>
	<option value=2>2</option>
	<option value=3>3</option>
	<option value=4>4</option>
	<option value=5>5</option>
	<option value=6>6</option>
	<option value=7>7</option>
      </select>
    </td>
  </tr>
  <tr>
    <td class="head">BA Window size</td>
    <td><input type=text name="ba_window_size" value=32>
    </td>
  </tr>
</table>
<br />

<table width="540" border="1" cellpadding="2" cellspacing="1">
  <tr>
    <td class="title" colspan="2">Connected BSSIDs</td>
  </tr>
  <tr>
    <td class="head">BSSID</td>
  </tr>
  <% getStaConnectedBSSID(); %>
</table>
<br />

<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td >
      <input type="button" name="mpdu_apply" style="{width:120px;}" value="Apply" onClick="submit_apply()"> &nbsp; &nbsp;
      <input type="button" style="{width:120px;}" value="Cancel" onClick="window.close()"> &nbsp; &nbsp;
    </td>
  </tr>
</table>
<input type=hidden name=selectedbssid value="">
</form>


</td></tr></table>
</body>
</html>

