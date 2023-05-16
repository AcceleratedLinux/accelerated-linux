#!/bin/bash

FILENAME=/etc/config/support_report

if [[ -f $FILENAME ]]
then
    rm $FILENAME
fi
touch $FILENAME
if [[ $? != 0 ]]
then
    echo "Error: unable to save file "$FILENAME"."
    exit 1
fi

support-report &> $FILENAME
