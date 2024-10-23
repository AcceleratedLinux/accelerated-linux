mstpd: Multiple Spanning Tree Protocol daemon

-MSTPD is reported to be compliant with IXIA ANVL RSTP test suite, with the notable exception of looped-back BPDUs
-Important note! MSTP part of the code (as opposed to STP/RSTP part) is mainly untested, so I believe it will behave unexpectedly in many situations. Don't use it in production!
https://github.com/mstpd/mstpd/wiki/ImplementationFeatures

mstpd -d -s -v 4 &
mstpctl addbridge br0
mstpctl showbridge

# monitor stp & rstp protocol
tcpdump  -vvv -p -n -i br0 stp

#RSTP Interface Settings
  # bridges forward delay in seconds (forward_delay)
  mstpctl setfdelay br0 <value> # range:4-30, limitation: 2*(value-1) >= max_age
  # bridges max age in seconds (max_age)
  mstpctl setmaxage br0 <value> # range:6-40, default:20
  # MAC address aging time in seconds (aging_time)
  mstpctl setageing br0 <value> # range:10-1000000, default:300
  # bridges max hops (max_hops)
  mstpctl setmaxhops br0 <value> # range:6-40, default:20
  # hello time in seconds (hello_time)
  mstpctl sethello br0 <value>  # range:1-10, default: 2
  # transmit hold count
  mstpctl settxholdcount br0 <value> # range:1-10, default:6

# Per Port Settings
# Port cost
mstpctl setportpathcost br0 lan <value> # 0 - auto
#Edge Port - enable,disable,auto
mstpctl setportautoedge br0 lan yes
# Initial Edge State
mstpctl setportadminedge br0 lan yes
# point to point detection modem
mstpctl setportp2p br0 lan auto
mstpctl setportnetwork br0 lan yes
mstpctl settreeportcost br0 lan 0 2001

# SHOW
mstpctl showportdetail br0
br0:lan CIST info
  enabled            yes                     role                 Designated
  port id            8.001                   state                forwarding
  external port cost 20000                   admin external cost  0
  internal port cost 20000                   admin internal cost  0
  designated root    8.000.00:27:04:39:A4:1F dsgn external cost   0
  dsgn regional root 8.000.00:27:04:39:A4:1F dsgn internal cost   0
  designated bridge  8.000.00:27:04:39:A4:1F designated port      8.001
  admin edge port    no                      auto edge port       yes
  oper edge port     yes                     topology change ack  no
  point-to-point     yes                     admin point-to-point auto
  restricted role    no                      restricted TCN       no
  port hello time    2                       disputed             no
  bpdu guard port    no                      bpdu guard error     no
  network port       no                      BA inconsistent      no
  bpdu filter port   no                      Num RX BPDU Filtered 0
  Num TX BPDU        1683                    Num TX TCN           0
  Num RX BPDU        0                       Num RX TCN           0
  Num Transition FWD 1                       Num Transition BLK   1
  Rcvd BPDU          no                      Rcvd STP             no
  Rcvd RSTP          no                      Send RSTP            yes
  Rcvd TC Ack        no                      Rcvd TCN             no

mstpctl showport br0
  lan   8.001 forw 8.000.00:27:04:34:5E:4B 8.000.00:27:04:34:5E:4B 8.001 Root
E wan   8.002 forw 8.000.00:27:04:34:5E:4B 8.000.00:27:04:39:A4:1F 8.002 Desg

mstpctl showbridge br0
br0 CIST info
  enabled         yes
  bridge id       8.000.00:27:04:39:A4:1F
  designated root 8.000.00:27:04:39:A4:1F
  regional root   8.000.00:27:04:39:A4:1F
  root port       none
  path cost     0          internal path cost   0
  max age       20         bridge max age       20
  forward delay 15         bridge forward delay 15
  tx hold count 6          max hops             20
  hello time    2          ageing time          300
  force protocol version     rstp
  time since topology change 3424
  topology change count      0
  topology change            no
  topology change port       None
  last topology change port  None

-------------------------------------------------------------------------------------

# Test setup (STP kernel implementation)
I used two DAL devices with at least two ethernet ports each and my PC with two
ethernet ports (running VirtualDAL bridged to both ethernets).

DAL device 1:
        lan, wan bridged static ip 192.168.21.1/24
DAL device 2:
        lan, wan bridged static ip 192.168.21.2/24
VirtualDAL:
        lan, wan bridged static ip 192.168.21.3/24

cable connections:
        DAL device 1:lan to DAL device 2:lan
        DAL device 2:wan to VirtualDAL:lan
        VirtualDAL:wan to DAL device 1:wan

NOTE: starting with stp enabled until we have everything setup (stops packet storms)

I should be able to ping each device from any other device.
# check status of kernel stp
brctl showstp br0
# when you unplug a network cable kernel STP will detect and reroute on alternate path
# to continue to work with a small downtime (seen 5s to 30s)

# This is STP kernel implementation and can be tested with this setup.
# The "STP 802.1d" string should be able to be seen in the tcpdump output.

------------------------------------------------------------------------------------

# Testing RSTP (mstpd daemon implementation)
# Use same setup as above except
# start mstpd daemon on each device & addbridge
mstpd &
mstpctl addbridge br0
# Then disable STP in config so that daemon will take over.
# You should see "STP 802.1w" string in tcpdump output.


