#!/bin/bash
#
# Dumb wrapper script to be called by vgetty for emailing of incoming
# voice-messages as MP3. Also does housekeeping such as renaming file 
# using sensible date format.
#
# contributed by Stroller <Linux.Luser@myrealbox.com>
#
# $Log: voice2email.sh,v $
# Revision 1.1  2005/11/27 08:36:47  gert
# contributed script to convert incoming voice calls to MP3 e-mail attachments
#
#
# ------------------------------------------------------------------------
#
# Set paths - clearly these need to be installed on your computer
MAKEMIME=/usr/bin/makemime
#SENDMAIL=/usr/sbin/sendmail
# Postfix > 2.1
#   requires this "-t" flag to extract recipients from makemime's headers
SENDMAIL="/usr/sbin/sendmail -t"
RMDTOPVF=/usr/bin/rmdtopvf
# PVFTOWAV=/usr/bin/pvftowav
#   We no longer use thisi
#   - see http://www.xasa.com/grupos/de/article/21337/de.alt.comm.mgetty
#   Additional coversion to au -> wav step required to work around bug
PVFTOAU=/usr/bin/pvftoau
SOX=/usr/bin/sox
LAME=/usr/bin/lame
DIRNAME=/bin/dirname	# requires GNU coreutils
FILE=/usr/bin/file

# We anticipate this script being called by vgetty
#   (see the line of /etc/mgetty+sendfax/voice.conf beginning "message_program"
# with ONE command-line argument.
if [ -z "$1" ]
then
   echo "ERROR! This script requires a commandline argument, silly!" >&2
   exit 1
fi

# That command-line argument is the filename wot is to be operated upon.
if [ ! -e $1 ]
then
   echo "ERROR! That file doesn't exist, silly!" >&2
   exit 1
fi

# Check file is appropriate type
#
IFREQ='raw modem data (Rockwell / compression type 0x0200)'
if [[ `$FILE -b $1` != $IFREQ ]]
then 
   echo "ERROR! That file isn't a Rockwell raw data file, silly!" >&2
   exit 1
fi 


# Set some useful date fields
LOGDATE="`date '+%d-%m-%Y %X'`"
SHORTDATE="`date +%Y-%m-%d_%X`"	# for file-naming - sorts better
MIMEDATE="`date +%Y-%m-%d_%H.%M.%S`"  # don't ask  :(
LONGDATE="`date '+%-l:%M%P %A, %-e %B %Y'`"   # for email subject

# Set arguments to makemime command
#TYPE="audio/x-wav"
TYPE="audio/mpeg"
ENCODING=base64		# 'cause it's a binary attachment, of course

FROM="Voicemail <>"
# I have Postmaster aliased to myself, but you could change to foo@bar-inc.com
RECIPIENT="Postmaster <postmaster>"

if [[ $CALLER_ID = "P" ]] || [[ -z $CALLER_ID ]]	# $CALLER_ID is inherited from vgetty 
then				# "P" indicates anonymous CLID
    SUBJECT="Message received at $LONGDATE"
    FILENAME="$SHORTDATE"
    MIMENAME="$MIMEDATE"
    LOGMESSAGE="$LOGDATE anonymous CLID - message renamed $FILENAME.rmd"
else
    SUBJECT="Message from $CALLER_ID received at $LONGDATE"
    FILENAME="$SHORTDATE"_"$CALLER_ID"
    MIMENAME="$MIMEDATE"_"$CALLER_ID"
    LOGMESSAGE="$LOGDATE $CALLER_ID    -   renamed $FILENAME.rmd"
fi


# MANY ATTEMPTS TO IMPLEMENT THIS WAY FAILED. I BELIEVE RELATED TO QUOTING.
#MAKEMIMEARGS="-c $TYPE -e $ENCODING -N $NAME -a SUBJECT: $SUBJECT -a FROM: $FROM - "
#echo $MAKEMIME $MAKEMIMEARGS
#cat $1 | $MAKEMIME $MAKEMIMEARGS | $SENDMAIL stroller

# NITTY GRITTY - pipe from hell.
#
$RMDTOPVF $1 | $PVFTOAU -16 2>/dev/null | $SOX -t au - -t wav - 2>/dev/null \
		| $LAME -b 16 - - 2>/dev/null \
		| $MAKEMIME -c $TYPE -e $ENCODING -N $MIMENAME.mp3 \
		-a "Subject: $SUBJECT" -a "From: $FROM"  -a "To: $RECIPIENT" \
		-a "Mime-Version: 1.0" - \
		| $SENDMAIL

# Rename file more sensibly now we've finished with it
#    (`dirname` is part of GNU coreutils)
mv $1 `$DIRNAME $1`/$FILENAME.rmd
echo $LOGMESSAGE >&2
