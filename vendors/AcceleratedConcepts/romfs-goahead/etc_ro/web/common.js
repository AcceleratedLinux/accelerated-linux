function restartPage_init()
{
	document.write('<div class=\"restart\" id=\"restart\" style=\"display: none;\">');
	document.write('<iframe name=\"mainFrame\" src=\"/wait.asp\" align=\"top\" border=\"0\" frameborder=\"0\" width=\"100%\" height=\"100%\" bgcolor=\"#DBE2EC\"></iframe>');
	document.write('</div>');
}

function restartPage_block()
{
	document.getElementById("restart").style.display="block";
}

function sbutton_disable(max)
{
	for (i=0; i < max; i++)
	{
		document.getElementById("sbutton"+i).style.visibility = "hidden";
   		document.getElementById("sbutton"+i).style.display = "none";
	}
}

function is_contained_char(c, str)
{
    var i;
    for (i = 0;i < str.length; i++)
    {
        if (str.substr(i, 1) == c)
            return true;
    }
}

function is_valid_hostname(hostname)
{
	var num = "0123456789";
	var ascii = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	var extra = "1234567890_-.";
    var i;
    var valid_string = num + ascii + extra;

    if (hostname.indexOf("..") > -1)
        return false;
    if (hostname.length > 32 || hostname.length < 0)
        return false;
    for (i = 0; i < hostname.length; i++)
        if (!is_contained_char(hostname.substr(i, 1), valid_string))
            return false;
    return true;
}

function is_valid_domainname(domainname)
{
	var num = "0123456789";
	var ascii = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	var extra = "_-.";
    var i;
    var valid_string = num + ascii + extra;

    if (
        domainname.indexOf("..") > -1 ||
        domainname.substr(0,1) == '.' ||
        domainname.substr(domainname.length - 1, 1) == '.'
        )
        return false;
    if (domainname.length > 256 || domainname.length < 0)
        return false;
    for (i = 0; i < domainname.length; i++)
        if (!is_contained_char(domainname.substr(i, 1), valid_string))
            return false;
    return true;
}

function is_ip(str)
{
    var re=/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/
    if (re.test(str))
    {
        if (RegExp.$1<256 && RegExp.$2<256 && RegExp.$3<256 && RegExp.$4<256)
        return true;
    }
    return false;
}

function d2b(value)
{
	var num1;
	var num2=null;
	var currnum;
	currnum = 128;

	num1 = eval(value);
	if(num1 >= currnum)
	{
		num2 = "1";
		num1 = num1 - currnum;
		currnum = currnum / 2;
	}
	else
	{
		num2 = "0";
		currnum = currnum / 2;
	}		

	for (p = 1; p <= 7; p++)
	{
		if(num1 >= currnum)
		{
			num2 = num2 + "1";
			num1 = num1 - currnum;
			currnum = currnum / 2;
		}
		else
		{
			num2 = num2 + "0";
			currnum = currnum / 2;
		}
	}
	return num2;
}

function checkIPinSubnetMask(ip,subnet)
{
	var ip_b=new Array(atoi(ip,1), atoi(ip,2), atoi(ip,3), atoi(ip,4));
	var subnet_b=new Array(atoi(subnet,1), atoi(subnet,2), atoi(subnet,3), atoi(subnet,4));
	
	if( (ip_b[0] >= subnet_b[0]) ||
		(ip_b[0] < subnet_b[0] && ip_b[1] >= subnet_b[1]) ||
		(ip_b[0] < subnet_b[0] && ip_b[1] < subnet_b[1] && ip_b[2] >= subnet_b[2]) ||
		(ip_b[0] < subnet_b[0] && ip_b[1] < subnet_b[1] && ip_b[2] < subnet_b[2] && ip_b[3] >= subnet_b[3]) ){
		return true;
	} else {
		return false;
	}
}

function checkIPwithSubnetMask(ip,subnet)
{
	var i,j,vi=0,vj=0,zero=0,one=0;
	var ip_b=new Array(d2b(atoi(ip,1)), d2b(atoi(ip,2)), d2b(atoi(ip,3)), d2b(atoi(ip,4)));
	var subnet_b=new Array(d2b(atoi(subnet,1)), d2b(atoi(subnet,2)), d2b(atoi(subnet,3)), d2b(atoi(subnet,4)));

	for(i=0;i<4;i++) {
		for(j=0;j<8;j++) {
			if(subnet_b[i].charAt(j) == "0") {
				vi = i;
				vj = j;
				break;
			}
		}
	}

	for(i=vi;i<4;i++) {
		if (i==vi) {
			for(j=vj;j<8;j++) {
				if(ip_b[i].charAt(j) == "0")
					zero++;
				else if(ip_b[i].charAt(j) == "1")
					one++;
				else 
					return false;
			}
		} else {
			for(j=0;j<8;j++) {
				if(ip_b[i].charAt(j) == "0")
					zero++;
				else if(ip_b[i].charAt(j) == "1")
					one++;
				else 
					return false;
			}
		}
	}

	if (zero == 0 && one != 0)	{
		alert("Error, IP & Subnetmask !");
		return false;
	} else if (zero != 0 && one == 0) {
		alert("Error, IP & Subnetmask !");
		return false;
	} else if (zero == 0 && one == 0) {
		return false;
	}

	return true;
}

function checkSameSubnet(ipa,ipb,gateway) {
	var i,j;
	var a= new Array(d2b(atoi(ipa,1)),d2b(atoi(ipa,2)),d2b(atoi(ipa,3)),d2b(atoi(ipa,4)));
	var b= new Array(d2b(atoi(ipb,1)),d2b(atoi(ipb,2)),d2b(atoi(ipb,3)),d2b(atoi(ipb,4)));
	var gw = new Array(d2b(atoi(gateway,1)),d2b(atoi(gateway,2)),d2b(atoi(gateway,3)),d2b(atoi(gateway,4)));

	for(i=0;i<4;i++) {
		for(j=0;j<8;j++) {
			if(gw[i].charAt(j) == "1")
				if(a[i].charAt(j) != b[i].charAt(j))
					return false;
		}
	}
	return true;
}

function isSameClass(ip1,ip2,mask)
{
   var test1=ip1.split(".");
   var test2=ip2.split(".");
   var myMask=mask.split(".");
   
   if (test1.length!=4 || test2.length!=4 || myMask.length!=4)
   {
     //alert("Not IP or Mask format!");
     return false;
   }
   
   for(var i=0; i<=3; i++)   
   {
      if ( (test1[i]&myMask[i])!=(test2[i]&myMask[i]) )
        return false;
   }
   return true;
}

function checkIPAddr_sub(ip1)
{
   var test1=ip1.split(".");
   
   if (test1.length!=4 )
     return false;
   
   for(var i=0; i<=3; i++)   
   {
      if ( test1[i].length > 3 )
        return false;
   }
   return true;
}

/*
 * ipv4_to_bytearray
 *	Convert an IPv4 address dotted string to a byte array
 */
function ipv4_to_bytearray(ipaddr)
{
	var ip = ipaddr + "";
	var got = ip.match (/^\s*(\d{1,3})\s*[.]\s*(\d{1,3})\s*[.]\s*(\d{1,3})\s*[.]\s*(\d{1,3})\s*$/);
	if (!got) {
		return 0;
	}
	var a = [];
	var q = 0;
	for (var i = 1; i <= 4; i++) {
		q = parseInt(got[i],10);
		if (q < 0 || q > 255) {
			return 0;
		}
		a[i-1] = q;
	}
	return a;
}

/*
 * is_ipv4_valid
 *	Check is an IP address dotted string is valid.
 */
function is_ipv4_valid(ipaddr)
{
	var ip = ipv4_to_bytearray(ipaddr);
	if (ip === 0) {
		return false;
	}
	return true;
}

function subnet_check(ip1, netmask, ip2)
{
	//split the ip address and netmask
	var ip1Array = ip1.split(".");
	var netmaskArray = netmask.split(".");
	var ip2Array = ip2.split(".");
	var ip1_1=ip1Array[0]*1, ip1_2=ip1Array[1]*1, ip1_3=ip1Array[2]*1, ip1_4=ip1Array[3]*1; //ip1 = ip1_1.ip1_2.ip1_3.ip1_4
	var netmask1=netmaskArray[0]*1, netmask2=netmaskArray[1]*1, netmask3=netmaskArray[2]*1, netmask4=netmaskArray[3]*1;
	var ip2_1=ip2Array[0]*1, ip2_2=ip2Array[1]*1, ip2_3=ip2Array[2]*1, ip2_4=ip2Array[3]*1; //ip2 = ip2_1.ip2_2.ip2_3.ip2_4
	
	//get the subnet
	var subnet1_1=ip1_1&netmask1, subnet1_2=ip1_2&netmask2, subnet1_3=ip1_3&netmask3, subnet1_4=ip1_4&netmask4;
	var subnet2_1=ip2_1&netmask1, subnet2_2=ip2_2&netmask2, subnet2_3=ip2_3&netmask3, subnet2_4=ip2_4&netmask4;
	
	//compare the subnet
	if((subnet1_1!=subnet2_1) || 
	   (subnet1_2!=subnet2_2) ||
	   (subnet1_3!=subnet2_3) ||
	   (subnet1_4!=subnet2_4)) {
			return false;
	}
	
	return true;
}

function checkDhcpStartEnd(start, end)
{
	if(atoi(start, 4) > atoi(end, 4))
		return false;
	return true;
}

/**	Function **/
function XMLHttpGetSetting(content)
{
    var http_req = false;
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_req = new XMLHttpRequest();
        if (http_req.overrideMimeType) {
            http_req.overrideMimeType('text/xml');
        }
    }else if (window.ActiveXObject) { // IE
        try {
            http_req = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_req = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_req) {
        alert(_("wan not create XMLHTTP instance"));
        return false;
    }
    http_req.open('POST', "/goform/readConfigure", true);
    http_req.send(content);
	return http_req;
}

function alertContents() {
	var htp = XMLHttpGetSetting("lan_ipaddr");
    if (htp.readyState == 4) {
        if (htp.status == 200) {
			setTimeout("alert(\"%s\", htp.responseText)", 2000);
        } else {
            //alert('There was a problem with the request.');
        }
    }
}

/**
 * if return 1 then server is online, otherwise is offline
 */
function chk_server_status()
{
    var http_req = false;
    if (window.XMLHttpRequest) { // Mozilla, Safari,...
        http_req = new XMLHttpRequest();
        if (http_req.overrideMimeType) {
            http_req.overrideMimeType('text/xml');
        }
    }else if (window.ActiveXObject) { // IE
        try {
            http_req = new ActiveXObject("Msxml2.XMLHTTP");
        } catch (e) {
            try {
            http_req = new ActiveXObject("Microsoft.XMLHTTP");
            } catch (e) {}
        }
    }
    if (!http_req) {
        alert(_("wan not create XMLHTTP instance"));
        return false;
    }
    http_req.open('POST', "/goform/chkServeState", true);
    http_req.send(content);
	
	if (http_req.readyState == 4) {
        if (http_req.status == 200) {
			return 1;
        } else {
            return 0;
        }
    }
}