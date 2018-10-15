<!-- Copyright (c), Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<script type="text/javascript" src="/lang/b28n.js"></script>
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">

<title>Disk Partition</title>
<script language="JavaScript" type="text/javascript">
Butterlate.setTextDomain("usb");
var count = 1;
var maxvol = parseInt('<% getMaxVol(); %>');

function initTranslation()
{
	var e = document.getElementById("adddirName");
	e.innerHTML = _("smb adddir name");

	e = document.getElementById("formatApply");
	e.value = _("usb apply");
	e = document.getElementById("formatCancel");
	e.value = _("usb cancel");
}
	
function initValue()
{
	// initTranslation();
	document.disk_parted.part2_vol.disabled = true;
	document.disk_parted.part3_vol.disabled = true;
}

function checkData()
{
	switch(document.disk_parted.max_part.value)
	{
		case "4":
		case "3":
			if (document.disk_parted.part3_vol.value == "")
			{
				alert("please fill Parttion 3's Volume");
				document.disk_parted.part3_vol.focus();
				document.disk_parted.part3_vol.select();
				return false;
			}
			if (document.disk_parted.part3_vol.value <= 0)
			{
				alert("Input value must be greater than zero!");
				document.disk_parted.part3_vol.focus();
				document.disk_parted.part3_vol.select();
				return false;
			}
			maxvol -= parseInt(document.disk_parted.part3_vol.value);
		case "2":
			if (document.disk_parted.part2_vol.value == "")
			{
				alert("please fill Parttion 2's Volume");
				document.disk_parted.part2_vol.focus();
				document.disk_parted.part2_vol.select();
				return false;
			}
			if (document.disk_parted.part2_vol.value <= 0)
			{
				alert("Input value must be greater than zero!");
				document.disk_parted.part2_vol.focus();
				document.disk_parted.part2_vol.select();
				return false;
			}
			maxvol -= parseInt(document.disk_parted.part2_vol.value);
		default:
			if (document.disk_parted.part1_vol.value == "")
			{
				alert("please fill Parttion 1's Volume");
				document.disk_parted.part1_vol.focus();
				document.disk_parted.part1_vol.select();
				return false;
			}
			if (document.disk_parted.part1_vol.value <= 0)
			{
				alert("Input value must be greater than zero!");
				document.disk_parted.part1_vol.focus();
				document.disk_parted.part1_vol.select();
				return false;
			}
			maxvol -= parseInt(document.disk_parted.part1_vol.value);
	}
	if (0 >= maxvol)
	{
		alert("total volume is greater than Max volume");
		return false;
	}

	return true;
}

function parted_count_switch()
{
	document.disk_parted.part2_vol.disabled = true;
	document.disk_parted.part3_vol.disabled = true;
	switch(document.disk_parted.max_part.value)
	{
		case "4":
		case "3":
			document.disk_parted.part3_vol.disabled = false;
		case "2":
			document.disk_parted.part2_vol.disabled = false;
		default:
			document.disk_parted.part1_vol.disabled = false;
	}
}

function partedClose()
{
	opener.location.reload();
}

function submit_apply()
{
	if (checkData() == true) {
		document.disk_parted.submit();
	}
	window.opener.initValue();
	window.close();
}
</script>
</head>

<body onLoad="initValue()" onUnload="partedClose()">
<table class="body"><tr><td>
<form method=post name="disk_parted" action="/goform/storageDiskPart">
<table width="540" border="1" cellspacing="1" cellpadding="3" vspace="2" hspace="2" bordercolor="#9BABBD">
  <tr>
    <td class="title" id="partedMaxVol" width="150px">Max. Availabled Volume</td>
<script language="JavaScript" type="text/javascript">
document.write("<td>"+maxvol+" MB</td>");
</script>
  <tr>
    <td class="title" id="partedNum" width="150px">Partition Number</td>
    <td>
      <select name="max_part" onChange="parted_count_switch()">
        <option value="1" selected>1
        <option value="2">2
        <option value="3">3
        <option value="4">4
      </select>
    </td>
  </tr>
  <tr>
    <td class="title" id="partedIndex" width="150px">Partition</td>
    <td class="title" id="partedVol">Volume</td>
  </tr>
  <tr>
    <td align="center" bgcolor="#E8F8FF" width="150px">Partition 1</td>
    <td>
      <input type="text" size="5" maxlength="5" name="part1_vol">M
    </td>
  </tr>
  <tr>
    <td align="center" bgcolor="#E8F8FF" width="150px">Partition 2</td>
    <td>
      <input type="text" size="5" maxlength="5" name="part2_vol">M
    </td>
  </tr>
  <tr>
    <td align="center" bgcolor="#E8F8FF" width="150px">Partition 3</td>
    <td>
      <input type="text" size="5" maxlength="5" name="part3_vol">M
    </td>
  </tr>
  <tr>
    <td align="center" bgcolor="#E8F8FF" width="150px">Partition 4</td>
    <td>
      <input type="text" size="5" maxlength="5" name="part4_vol" value="rest" readonly>
    </td>
  </tr>
</table>
<hr />
<br />
<table width = "540" border = "0" cellpadding = "2" cellspacing = "1">
  <tr align="center">
    <td>
      <input type=button style="{width:120px;}" value="Apply" id="partedApply" onClick="submit_apply()"> &nbsp; &nbsp;
      <input type=reset  style="{width:120px;}" value="Cancel" id="partedCancel" onClick="window.close()">
    </td>
  </tr>
</table>
</form>
</body>
</html>


