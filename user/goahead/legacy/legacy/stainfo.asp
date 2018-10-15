<!-- Copyright 2004, Ralink Technology Corporation All Rights Reserved. -->
<html>
<head>
<META HTTP-EQUIV="Pragma" CONTENT="no-cache">
<META HTTP-EQUIV="Expires" CONTENT="-1">
<META http-equiv="Content-Type" content="text/html; charset=utf-8">
<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">
<title>Station List</title>

<script language="JavaScript" type="text/javascript">
</script>
</head>


<body>
<table class="body"><tr><td>

<h1>Station List</h1>
<p> You could monitor stations which associated to this Legacy AP here. </p>
<hr />

<table width="540" border="1" cellspacing="1" cellpadding="3" bordercolor="#9BABBD">
  <tr> 
    <td class="title" colspan="3">Wireless Network</td>
  </tr>
  <tr>
    <td bgcolor=#E8F8FF>MAC Address</td>
    <td bgcolor=#E8F8FF>Aid</td>
    <td bgcolor=#E8F8FF>PSM</td>
  </tr>
  <% getLegacyStaInfo(); %>
</table>

</td></tr></table>
</body>
</html>

