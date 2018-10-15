#!./perl
# Tests to ensure that we don't unexpectedly change prototypes of builtins

BEGIN {
    chdir 't' if -d 't';
    @INC = '../lib';
}

BEGIN { require './test.pl'; }
plan tests => 234;

while (<DATA>) {
    chomp;
    my ($keyword, $proto) = split;
    if ($proto eq 'undef') {
	ok( !defined prototype "CORE::".$keyword, $keyword );
    }
    elsif ($proto eq 'unknown') {
	eval { prototype "CORE::".$keyword };
	like( $@, qr/Can't find an opnumber for/, $keyword );
    }
    else {
	is( "(".prototype("CORE::".$keyword).")", $proto, $keyword );
    }
}

# the keyword list :

__DATA__
abs (;$)
accept (**)
alarm (;$)
and ()
atan2 ($$)
bind (*$)
binmode (*;$)
bless ($;$)
caller (;$)
chdir (;$)
chmod (@)
chomp undef
chop undef
chown (@)
chr (;$)
chroot (;$)
close (;*)
closedir (*)
cmp unknown
connect (*$)
continue unknown
cos (;$)
crypt ($$)
dbmclose (\%)
dbmopen (\%$$)
defined undef
delete undef
die (@)
do undef
dump ()
each (\%)
else undef
elsif undef
endgrent ()
endhostent ()
endnetent ()
endprotoent ()
endpwent ()
endservent ()
eof (;*)
eq ($$)
err unknown
eval undef
exec undef
exists undef
exit (;$)
exp (;$)
fcntl (*$$)
fileno (*)
flock (*$)
for undef
foreach undef
fork ()
format undef
formline ($@)
ge ($$)
getc (;*)
getgrent ()
getgrgid ($)
getgrnam ($)
gethostbyaddr ($$)
gethostbyname ($)
gethostent ()
getlogin ()
getnetbyaddr ($$)
getnetbyname ($)
getnetent ()
getpeername (*)
getpgrp (;$)
getppid ()
getpriority ($$)
getprotobyname ($)
getprotobynumber ($)
getprotoent ()
getpwent ()
getpwnam ($)
getpwuid ($)
getservbyname ($$)
getservbyport ($$)
getservent ()
getsockname (*)
getsockopt (*$$)
glob undef
gmtime (;$)
goto undef
grep undef
gt ($$)
hex (;$)
if undef
index ($$;$)
int (;$)
ioctl (*$$)
join ($@)
keys (\%)
kill (@)
last undef
lc (;$)
lcfirst (;$)
le ($$)
length (;$)
link ($$)
listen (*$)
local undef
localtime (;$)
lock (\$)
log (;$)
lstat (*)
lt ($$)
m undef
map undef
mkdir ($;$)
msgctl ($$$)
msgget ($$)
msgrcv ($$$$$)
msgsnd ($$$)
my undef
ne ($$)
next undef
no undef
not ($)
oct (;$)
open (*;$@)
opendir (*$)
or ()
ord (;$)
our undef
pack ($@)
package undef
pipe (**)
pop (;\@)
pos undef
print undef
printf undef
prototype undef
push (\@@)
q undef
qq undef
qr undef
quotemeta (;$)
qw undef
qx undef
rand (;$)
read (*\$$;$)
readdir (*)
readline (;*)
readlink (;$)
readpipe unknown
recv (*\$$$)
redo undef
ref (;$)
rename ($$)
require undef
reset (;$)
return undef
reverse (@)
rewinddir (*)
rindex ($$;$)
rmdir (;$)
s undef
scalar undef
seek (*$$)
seekdir (*$)
select (;*)
semctl ($$$$)
semget ($$$)
semop ($$)
send (*$$;$)
setgrent ()
sethostent ($)
setnetent ($)
setpgrp (;$$)
setpriority ($$$)
setprotoent ($)
setpwent ()
setservent ($)
setsockopt (*$$$)
shift (;\@)
shmctl ($$$)
shmget ($$$)
shmread ($$$$)
shmwrite ($$$$)
shutdown (*$)
sin (;$)
sleep (;$)
socket (*$$$)
socketpair (**$$$)
sort undef
splice (\@;$$@)
split undef
sprintf ($@)
sqrt (;$)
srand (;$)
stat (*)
study undef
sub undef
substr ($$;$$)
symlink ($$)
syscall ($@)
sysopen (*$$;$)
sysread (*\$$;$)
sysseek (*$$)
system undef
syswrite (*$;$$)
tell (;*)
telldir (*)
tie undef
tied undef
time ()
times ()
tr undef
truncate ($$)
uc (;$)
ucfirst (;$)
umask (;$)
undef undef
unless undef
unlink (@)
unpack ($$)
unshift (\@@)
untie undef
until undef
use undef
utime (@)
values (\%)
vec ($$$)
wait ()
waitpid ($$)
wantarray ()
warn (@)
while undef
write (;*)
x unknown
xor ($$)
y undef
