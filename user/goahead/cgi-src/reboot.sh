#!/bin/sh
echo "Content-type: text/html"
echo ""
echo "<body>rebooting</body>"
flashconfig.sh save
reboot &

