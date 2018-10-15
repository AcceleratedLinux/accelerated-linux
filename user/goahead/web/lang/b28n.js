// "Butterfat internationalization" (b28n.js)
//  Released under the GNU GPL  by bmuller@buttterfat.net- http://www.gnu.org/licenses/gpl.txt
//  Modified by winfred_lu@ralinktech.com.tw for our need
/****************************************************************************************************

This script provides i18n support via a basic replica of GNU's gettext api.  To use it, pick a domain
("messages" is the traditional domain).  For each language, create a file named <domain>_<lang>.xml,
where lang is one of the two letter abbreviations found on http://www.loc.gov/standards/iso639-2/langcodes.html
(for instance, messages_en.xml is a good start).  The script will first look for a cookie with the name
"language", and if it is not set then it uses the browser langague.  It should be noted that this "browser
langague" is just the "navigator.language" or "navigator.browserLanguage" value, and NOT the "Accept-Language"
header sent by the browser.  This is something that should be set by the server, such as in a cookie.  The 
preferable method is for the server to take the "Accept-Language" header value and store it in a permanent cookie,
allowing this script to henceforth know which po file to use.  The text within the msgid and msgstr should, obviously
be html encoded.  Your javascript does not have to be html encoded.  For example, the call _("test\"test") with 
a message of "<message msgid="test&quot;test" msgstr="test&quot;test&quot;test" />" would return "test\"test\"test".

The po file should have the following syntax:

<po creationdate ="2005-10-12 12:08-0500" translator="FULL NAME EMAIL@ADDRESS">
  <message msgid="this is english" msgstr="this is spanish" />
</po>

The attributes for <po> are optional.  The html file needs only to include the b28n.js file
and then set the text domain via Butterlate.setTextDomain(<domain>,<location>).  <domain> is the domain
you used to create the above xml file, and <location> is the HTTP accessable directory of that xml
file (so, "http://<FQDN>/messages_en.xml").  Whenever you are showing text, just call the function
"_(<text>)" or "Butterlate.gettext(<text>)" and Butterlate will handle the rest.  The following
example will produce a page that says "this is spanish" (assuming you set up the setTextDomain correctly
and make the above xml available).

<html>
  <head>
    <title>I18N Test</title>
    <script type="text/javascript" src="b28n.js"></script>
    <script type="text/javascript">
      Butterlate.setTextDomain("messages","http://butterfat.net/~bmuller/i18n");
      function startUp() {
        var test = document.getElementById("test");
        test.innerHTML = _("this is english");
      }
    </script>
  </head>
  <body onload="startUp();">
    <div id="test">
  </body>
</html>

There is also the ability to have variables in your translations.  For instance, if you have a message
that says "hello <username>, how are you?" and the translation ends up being "blah blah <username> blah"
you need a way of handling the variable.  In the xml file, just use "%s" to denote a variable's placement-
for instance: " <message msgid="hello %s, how are you?" msgstr="blah blah %s blah" /> ".  Then in your 
javascript call either the function "__(<msgid>,<array of translations>)" or "vgettext(<msgid>,<array of translations>)".
The array of translations is just an array of strings in the same order as your %s's.  If there are more array
strings than variables, the extras are ignored.  If there are more %s's than array strings, the last string
in the array is simply repeated as many times as necessary.  If the array is empty, then the msgstr from the
xml file will be returned as-is.

*NOTE*: If the browser is set to a language for which you do not have an xml file, the original 
text will just be displayed and there will be no error messages.  The same goes for any text you
did not define in the xml file if it does exist.

*****************************************************************************************************/

var Butterlate = new Butterlation();
window._ = function(key) { return Butterlate.gettext(key); };
window.__ = function(key,replacements) { return Butterlate.vgettext(key,replacements); };

function Butterlation() {
  this.dict = new ButterDictionary();
  this.getLang = function() {
    var one, two, end;
// Nagy commented out if { and } to assure return of "en"
//    if((one=document.cookie.indexOf("language"))==-1) {
      //return ((navigator.language) ? navigator.language : navigator.browserLanguage).substring(0,2);   
      return "en";
//    }
    end = (document.cookie.indexOf(';',one)!=-1) ? document.cookie.indexOf(';',one) : document.cookie.length;
    return unescape(document.cookie.substring(one+9,end));
  };
  this.lang = this.getLang();
  this.setTextDomain = function(domain) { this.po=window.location.protocol+"//"+window.location.host+"/lang/"+this.lang+"/"+domain+".xml"; this.initializeDictionary(); }
  this.initializeDictionary = function() {
    var request;
    try { request = new XMLHttpRequest(); } catch(e1) {
      try { request = new ActiveXObject("Msxml2.XMLHTTP"); } catch(e2) {
        try { request = new ActiveXObject("Microsoft.XMLHTTP"); } catch(e3) { return; }}};
    request.open("GET",this.po,false); 
    request.send(null);
    if(request.status==200) { 
      var pos = request.responseXML.documentElement.getElementsByTagName("message");
      for(var i=0; i<pos.length; i++) this.dict.set(pos[i].getAttribute("msgid"),pos[i].getAttribute("msgstr"));
    }
  };
  this.gettext = function(key) { return this.dict.get(key); };
  this.vgettext = function(key,replacements) { 
    var nkey=this.gettext(key); var index; var count=0;
    if(replacements.length==0) return nkey;
    while((index=nkey.indexOf('%s'))!=-1) { 
      nkey=nkey.substring(0,index)+replacements[count]+nkey.substring(index+2,nkey.length); 
      count = ((count+1)==replacements.length) ? count : (count+1);
    }
    return nkey;
  };
}
      
function ButterDictionary() {
  this.keys = new Array();
  this.values = new Array();
  this.set = function(key,value) { 
    var index = this.getIndex(key);
    if(index==-1) { this.keys.push(key); this.values.push(value); }
    else this.values[index]=value;
  };
  this.get = function(key) {
    var index;
    if((index=this.getIndex(key))!=-1) return this.values[index];
    return key;
  };
  this.getIndex = function(key) {
    var index=-1;
    for(var i=0; i<this.keys.length; i++) if(this.keys[i]==key) { index=i; break; }
    return index;
  };
  this.keyExists = function(key) { return (this.getIndex(key)!=1); };
  this.deleteKey = function(key) { 
    var index = getIndex(key);
    if(index!=-1) { this.keys.splice(index,1); this.values.splice(index,1); }
  };
}


