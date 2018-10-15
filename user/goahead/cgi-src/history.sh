#!/bin/sh

# define
HISTORY="./History"

# HTTP header
echo "Content-type: text/html"
echo ""

# web header
echo "<html><head><title>RT288x SDK History</title>"

echo '<META HTTP-EQUIV="Pragma" CONTENT="no-cache">'
echo '<META HTTP-EQUIV="Expires" CONTENT="-1">'
echo '<meta http-equiv="content-type" content="text/html; charset=iso-8859-1">'
echo '<link rel="stylesheet" href="/style/normal_ws.css" type="text/css">'
echo '</head>'

# web body
echo "<body><table class=body><tr><td>"

# parse History file
sed -e '
s_\(^RT288.*\)_<h1>\1</h1><hr />_
s_\(^Version.*\)_<h3>\1</h3>_
s_^- \(\[.*\]\)\(.*\)_- <b>\1</b>\2 <br />_
s_\(^$\)_\1 <br />_
s_\(^==.*\)__
' $HISTORY

echo "</td></tr></table>"
echo "</body></html>"
