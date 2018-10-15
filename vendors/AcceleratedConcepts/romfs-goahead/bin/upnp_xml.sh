#!/bin/sh

if [ ! -e "/etc/linuxigd" ]; then
    mkdir /etc/linuxigd
fi
cp /etc_ro/linuxigd/ligd.gif /etc/linuxigd
if [ ! -n "$1" ]; then
    echo "insufficient arguments!"
    echo "Usage: $0 <device ip address>"
    echo "Example: $0 10.10.10.254"
    exit 0
fi

echo "<?xml version=\"1.0\"?>
<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">
  <specVersion>
    <major>1</major>
    <minor>0</minor>
  </specVersion>
  <actionList>
    <action>
      <name>SetConnectionType</name>
      <argumentList>
        <argument>
          <name>NewConnectionType</name>
          <direction>in</direction>
          <relatedStateVariable>ConnectionType</relatedStateVariable>
        </argument>
      </argumentList>
    </action>
    <action>
    <name>GetConnectionTypeInfo</name>
      <argumentList>
        <argument>
          <name>NewConnectionType</name>
          <direction>out</direction>
          <relatedStateVariable>ConnectionType</relatedStateVariable>
        </argument>
        <argument>
          <name>NewPossibleConnectionTypes</name>
          <direction>out</direction>
          <relatedStateVariable>PossibleConnectionTypes</relatedStateVariable>
        </argument>
      </argumentList>
    </action>
    <action>
      <name>RequestConnection</name>
    </action>
    <action>
      <name>ForceTermination</name>
    </action>
    <action>
     <name>GetStatusInfo</name>
      <argumentList>
        <argument>
          <name>NewConnectionStatus</name>
          <direction>out</direction>
          <relatedStateVariable>ConnectionStatus</relatedStateVariable>
        </argument>
        <argument>
          <name>NewLastConnectionError</name>
                  <direction>out</direction>
          <relatedStateVariable>LastConnectionError</relatedStateVariable>
        </argument>
        <argument>
          <name>NewUptime</name>
          <direction>out</direction>
          <relatedStateVariable>Uptime</relatedStateVariable>
        </argument>
      </argumentList>
    </action>
    <action>
      <name>GetNATRSIPStatus</name>
      <argumentList>
        <argument>
          <name>NewRSIPAvailable</name>
          <direction>out</direction>
          <relatedStateVariable>RSIPAvailable</relatedStateVariable>
        </argument>
        <argument>
          <name>NewNATEnabled</name>
          <direction>out</direction>
          <relatedStateVariable>NATEnabled</relatedStateVariable>
        </argument>
      </argumentList>
    </action>
    <action>
      <name>GetGenericPortMappingEntry</name>
      <argumentList>
        <argument>
          <name>NewPortMappingIndex</name>
          <direction>in</direction>
          <relatedStateVariable>PortMappingNumberOfEntries</relatedStateVariable>
        </argument>
        <argument>
          <name>NewRemoteHost</name>
          <direction>out</direction>
          <relatedStateVariable>RemoteHost</relatedStateVariable>
        </argument>
        <argument>
          <name>NewExternalPort</name>
          <direction>out</direction>
          <relatedStateVariable>ExternalPort</relatedStateVariable>
        </argument>
        <argument>
          <name>NewProtocol</name>
          <direction>out</direction>
          <relatedStateVariable>PortMappingProtocol</relatedStateVariable>
        </argument>
        <argument>
          <name>NewInternalPort</name>
          <direction>out</direction>
          <relatedStateVariable>InternalPort</relatedStateVariable>
        </argument>
        <argument>
          <name>NewInternalClient</name>
          <direction>out</direction>
          <relatedStateVariable>InternalClient</relatedStateVariable>
        </argument>
        <argument>
          <name>NewEnabled</name>
          <direction>out</direction>
          <relatedStateVariable>PortMappingEnabled</relatedStateVariable>
        </argument>
        <argument>
          <name>NewPortMappingDescription</name>
          <direction>out</direction>
          <relatedStateVariable>PortMappingDescription</relatedStateVariable>
        </argument>
        <argument>
          <name>NewLeaseDuration</name>
          <direction>out</direction>
          <relatedStateVariable>PortMappingLeaseDuration</relatedStateVariable>
        </argument>
      </argumentList>
    </action>
    <action>
      <name>GetSpecificPortMappingEntry</name>
      <argumentList>
        <argument>
          <name>NewRemoteHost</name>
          <direction>in</direction>
          <relatedStateVariable>RemoteHost</relatedStateVariable>
        </argument>
        <argument>
          <name>NewExternalPort</name>
          <direction>in</direction>
          <relatedStateVariable>ExternalPort</relatedStateVariable>
        </argument>
        <argument>
          <name>NewProtocol</name>
          <direction>in</direction>
          <relatedStateVariable>PortMappingProtocol</relatedStateVariable>
        </argument>
        <argument>
          <name>NewInternalPort</name>
          <direction>out</direction>
          <relatedStateVariable>InternalPort</relatedStateVariable>
        </argument>
        <argument>
          <name>NewInternalClient</name>
          <direction>out</direction>
          <relatedStateVariable>InternalClient</relatedStateVariable>
        </argument>
        <argument>
          <name>NewEnabled</name>
          <direction>out</direction>
          <relatedStateVariable>PortMappingEnabled</relatedStateVariable>
        </argument>
        <argument>
          <name>NewPortMappingDescription</name>
          <direction>out</direction>
          <relatedStateVariable>PortMappingDescription</relatedStateVariable>
        </argument>
        <argument>
          <name>NewLeaseDuration</name>
          <direction>out</direction>
          <relatedStateVariable>PortMappingLeaseDuration</relatedStateVariable>
        </argument>
      </argumentList>
    </action>
    <action>
      <name>AddPortMapping</name>
      <argumentList>
        <argument>
          <name>NewRemoteHost</name>
          <direction>in</direction>
          <relatedStateVariable>RemoteHost</relatedStateVariable>
        </argument>
        <argument>
          <name>NewExternalPort</name>
          <direction>in</direction>
          <relatedStateVariable>ExternalPort</relatedStateVariable>
        </argument>
        <argument>
          <name>NewProtocol</name>
          <direction>in</direction>
          <relatedStateVariable>PortMappingProtocol</relatedStateVariable>
        </argument>
        <argument>
          <name>NewInternalPort</name>
          <direction>in</direction>
          <relatedStateVariable>InternalPort</relatedStateVariable>
        </argument>
        <argument>
          <name>NewInternalClient</name>
          <direction>in</direction>
          <relatedStateVariable>InternalClient</relatedStateVariable>
        </argument>
        <argument>
          <name>NewEnabled</name>
          <direction>in</direction>
          <relatedStateVariable>PortMappingEnabled</relatedStateVariable>
        </argument>
        <argument>
          <name>NewPortMappingDescription</name>
          <direction>in</direction>
          <relatedStateVariable>PortMappingDescription</relatedStateVariable>
        </argument>
        <argument>
          <name>NewLeaseDuration</name>
          <direction>in</direction>
          <relatedStateVariable>PortMappingLeaseDuration</relatedStateVariable>
        </argument>
      </argumentList>
    </action>
    <action>
      <name>DeletePortMapping</name>
      <argumentList>
        <argument>
          <name>NewRemoteHost</name>
          <direction>in</direction>
          <relatedStateVariable>RemoteHost</relatedStateVariable>
        </argument>
        <argument>
          <name>NewExternalPort</name>
          <direction>in</direction>
          <relatedStateVariable>ExternalPort</relatedStateVariable>
        </argument>
        <argument>
          <name>NewProtocol</name>
          <direction>in</direction>
          <relatedStateVariable>PortMappingProtocol</relatedStateVariable>
        </argument>
      </argumentList>
    </action>
    <action>
    <name>GetExternalIPAddress</name>
      <argumentList>
        <argument>
          <name>NewExternalIPAddress</name>
          <direction>out</direction>
        <relatedStateVariable>ExternalIPAddress</relatedStateVariable>
        </argument>
      </argumentList>
    </action>
  </actionList>
  <serviceStateTable>
    <stateVariable sendEvents=\"no\">
      <name>ConnectionType</name>
      <dataType>string</dataType>
      <defaultValue>Unconfigured</defaultValue>
    </stateVariable>
    <stateVariable sendEvents=\"yes\">
      <name>PossibleConnectionTypes</name>
      <dataType>string</dataType>
      <allowedValueList>
        <allowedValue>Unconfigured</allowedValue>
        <allowedValue>IP_Routed</allowedValue>
        <allowedValue>IP_Bridged</allowedValue>
      </allowedValueList>
    </stateVariable>
    <stateVariable sendEvents=\"yes\">
      <name>ConnectionStatus</name>
      <dataType>string</dataType>
      <defaultValue>Unconfigured</defaultValue>
      <allowedValueList>
        <allowedValue>Unconfigured</allowedValue>
          <allowedValue>Connecting</allowedValue>
          <allowedValue>Authenticating</allowedValue>
        <allowedValue>PendingDisconnect</allowedValue>
        <allowedValue>Disconnecting</allowedValue>
        <allowedValue>Disconnected</allowedValue>
        <allowedValue>Connected</allowedValue>
      </allowedValueList>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>Uptime</name>
      <dataType>ui4</dataType>
      <defaultValue>0</defaultValue>
      <allowedValueRange>
        <minimum>0</minimum>
        <maximum></maximum>
        <step>1</step>
      </allowedValueRange>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>RSIPAvailable</name>
      <dataType>boolean</dataType>
      <defaultValue>0</defaultValue>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>NATEnabled</name>
      <dataType>boolean</dataType>
      <defaultValue>1</defaultValue>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>LastConnectionError</name>
      <dataType>string</dataType>
      <defaultValue>ERROR_NONE</defaultValue>
      <allowedValueList>
        <allowedValue>ERROR_NONE</allowedValue>
        <allowedValue>ERROR_ISP_TIME_OUT</allowedValue>
        <allowedValue>ERROR_COMMAND_ABORTED</allowedValue>
        <allowedValue>ERROR_NOT_ENABLED_FOR_INTERNET</allowedValue>
        <allowedValue>ERROR_BAD_PHONE_NUMBER</allowedValue>
        <allowedValue>ERROR_USER_DISCONNECT</allowedValue>
        <allowedValue>ERROR_ISP_DISCONNECT</allowedValue>
        <allowedValue>ERROR_IDLE_DISCONNECT</allowedValue>
        <allowedValue>ERROR_FORCED_DISCONNECT</allowedValue>
        <allowedValue>ERROR_SERVER_OUT_OF_RESOURCES</allowedValue>
        <allowedValue>ERROR_RESTRICTED_LOGON_HOURS</allowedValue>
        <allowedValue>ERROR_ACCOUNT_DISABLED</allowedValue>
        <allowedValue>ERROR_ACCOUNT_EXPIRED</allowedValue>
        <allowedValue>ERROR_PASSWORD_EXPIRED</allowedValue>
        <allowedValue>ERROR_AUTHENTICATION_FAILURE</allowedValue>
        <allowedValue>ERROR_NO_DIALTONE</allowedValue>
        <allowedValue>ERROR_NO_CARRIER</allowedValue>
        <allowedValue>ERROR_NO_ANSWER</allowedValue>
            <allowedValue>ERROR_LINE_BUSY</allowedValue>
            <allowedValue>ERROR_UNSUPPORTED_BITSPERSECOND</allowedValue>
            <allowedValue>ERROR_TOO_MANY_LINE_ERRORS</allowedValue>
            <allowedValue>ERROR_IP_CONFIGURATION</allowedValue>
            <allowedValue>ERROR_UNKNOWN</allowedValue>
      </allowedValueList>
    </stateVariable>
    <stateVariable sendEvents=\"yes\">
      <name>ExternalIPAddress</name>
      <dataType>string</dataType>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>RemoteHost</name>
      <dataType>string</dataType>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>ExternalPort</name>
      <dataType>ui2</dataType>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>InternalPort</name>
      <dataType>ui2</dataType>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>PortMappingProtocol</name>
      <dataType>string</dataType>
      <allowedValueList>
        <allowedValue>TCP</allowedValue>
        <allowedValue>UDP</allowedValue>
      </allowedValueList>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>InternalClient</name>
      <dataType>string</dataType>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>PortMappingDescription</name>
      <dataType>string</dataType>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>PortMappingEnabled</name>
      <dataType>boolean</dataType>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>PortMappingLeaseDuration</name>
      <dataType>ui4</dataType>
    </stateVariable>
    <stateVariable sendEvents=\"yes\">
      <name>PortMappingNumberOfEntries</name>
      <dataType>ui2</dataType>
    </stateVariable>
  </serviceStateTable>
</scpd>
" > /etc/linuxigd/gateconnSCPD.xml 

echo "<?xml version=\"1.0\"?>
<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">
        <specVersion>
                <major>1</major>
                <minor>0</minor>
        </specVersion>
        <actionList>
    <action>
      <name>GetCommonLinkProperties</name>
      <argumentList>
        <argument>
          <name>NewWANAccessType</name>
          <direction>out</direction>
          <relatedStateVariable>WANAccessType</relatedStateVariable>
        </argument>
        <argument>
          <name>NewLayer1UpstreamMaxBitRate</name>
          <direction>out</direction>
          <relatedStateVariable>Layer1UpstreamMaxBitRate</relatedStateVariable>
        </argument>
        <argument>
          <name>NewLayer1DownstreamMaxBitRate</name>
          <direction>out</direction>
          <relatedStateVariable>Layer1DownstreamMaxBitRate</relatedStateVariable>
        </argument>
        <argument>
          <name>NewPhysicalLinkStatus</name>
          <direction>out</direction>
          <relatedStateVariable>PhysicalLinkStatus</relatedStateVariable>
        </argument>
      </argumentList>
    </action>
    <action>
    <name>GetTotalBytesSent</name>
      <argumentList>
        <argument>
          <name>NewTotalBytesSent</name>
          <direction>out</direction>
          <relatedStateVariable>TotalBytesSent</relatedStateVariable>
        </argument>
      </argumentList>
    </action>
    <action>
    <name>GetTotalBytesReceived</name>
      <argumentList>
        <argument>
          <name>NewTotalBytesReceived</name>
          <direction>out</direction>
          <relatedStateVariable>TotalBytesReceived</relatedStateVariable>
        </argument>
      </argumentList>
    </action>
    <action>
    <name>GetTotalPacketsSent</name>
      <argumentList>
        <argument>
          <name>NewTotalPacketsSent</name>
          <direction>out</direction>
          <relatedStateVariable>TotalPacketsSent</relatedStateVariable>
        </argument>
      </argumentList>
    </action>
    <action>
    <name>GetTotalPacketsReceived</name>
      <argumentList>
        <argument>
          <name>NewTotalPacketsReceived</name>
          <direction>out</direction>
         <relatedStateVariable>TotalPacketsReceived</relatedStateVariable>
        </argument>
      </argumentList>
    </action>

    <action>
    <name>X_GetICSStatistics</name>
      <argumentList>
        <argument>
          <name>NewTotalBytesSent</name>
          <direction>out</direction>
          <relatedStateVariable>TotalBytesSent</relatedStateVariable>
        </argument>
        <argument>
          <name>NewTotalBytesReceived</name>
          <direction>out</direction>
          <relatedStateVariable>TotalBytesReceived</relatedStateVariable>
        </argument>
        <argument>
          <name>NewTotalPacketsSent</name>
          <direction>out</direction>
          <relatedStateVariable>TotalPacketsSent</relatedStateVariable>
        </argument>
        <argument>
          <name>NewTotalPacketsReceived</name>
          <direction>out</direction>
          <relatedStateVariable>TotalPacketsReceived</relatedStateVariable>
        </argument>
        <argument>
          <name>NewX_Uptime</name>
          <direction>out</direction>
          <relatedStateVariable>X_Uptime</relatedStateVariable>
        </argument>
      </argumentList>
    </action>

        </actionList>
        <serviceStateTable>
                <stateVariable sendEvents=\"no\">
                        <name>WANAccessType</name>
                        <dataType>string</dataType>
                        <allowedValueList>
                                <allowedValue>DSL</allowedValue>
                                <allowedValue>POTS</allowedValue>
                                <allowedValue>Cable</allowedValue>
                                <allowedValue>Ethernet</allowedValue>
                                <allowedValue>Other</allowedValue>
                        </allowedValueList>
                </stateVariable>
                <stateVariable sendEvents=\"no\">
                        <name>Layer1UpstreamMaxBitRate</name>
                        <dataType>ui4</dataType>
                </stateVariable>
                <stateVariable sendEvents=\"no\">
                        <name>Layer1DownstreamMaxBitRate</name>
                        <dataType>ui4</dataType>
                </stateVariable>
                <stateVariable sendEvents=\"yes\">
                        <name>PhysicalLinkStatus</name>
                        <dataType>string</dataType>
          <allowedValueList>
        <allowedValue>Up</allowedValue>
        <allowedValue>Down</allowedValue>
        <allowedValue>Initializing</allowedValue>
        <allowedValue>Unavailable</allowedValue>
      </allowedValueList>
                </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>WANAccessProvider</name>
      <dataType>string</dataType>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>MaximumActiveConnections</name>
      <dataType>ui2</dataType>
      <allowedValueRange>
        <minimum>1</minimum>
        <maximum></maximum>
        <step>1</step>
      </allowedValueRange>
    </stateVariable>

    <stateVariable sendEvents=\"no\">
      <name>X_Uptime</name>
      <dataType>ui4</dataType>
    </stateVariable>

    <stateVariable sendEvents=\"no\">
      <name>TotalBytesSent</name>
      <dataType>ui4</dataType>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>TotalBytesReceived</name>
      <dataType>ui4</dataType>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>TotalPacketsSent</name>
      <dataType>ui4</dataType>
    </stateVariable>
    <stateVariable sendEvents=\"no\">
      <name>TotalPacketsReceived</name>
      <dataType>ui4</dataType>
    </stateVariable>
        </serviceStateTable>
</scpd>
" > /etc/linuxigd/gateicfgSCPD.xml

echo "<?xml version=\"1.0\"?>
<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">
        <specVersion>
                <major>1</major>
                <minor>0</minor>
        </specVersion>
        <actionList>
                <action>
                        <name>SetDefaultConnectionService</name>
                        <argumentList>
                                <argument>
                                        <name>NewDefaultConnectionService</name>
                                        <direction>in</direction>
                                        <relatedStateVariable>DefaultConnectionService</relatedStateVariable>
                                </argument>
                        </argumentList>
                </action>
                <action>
                        <name>GetDefaultConnectionService</name>
                        <argumentList>
                                <argument>
                                        <name>NewDefaultConnectionService</name>
                                        <direction>out</direction>
                                        <relatedStateVariable>DefaultConnectionService</relatedStateVariable>
                                </argument>
                        </argumentList>
                </action>
        </actionList>
        <serviceStateTable>
                <stateVariable sendEvents=\"yes\">
                        <name>DefaultConnectionService</name>
                        <dataType>string</dataType>
                </stateVariable>
        </serviceStateTable>
</scpd>
" > /etc/linuxigd/l3fwdSCPD.xml

WEB_ADDR="http://$1"
echo "<?xml version=\"1.0\"?>
<root xmlns=\"urn:schemas-upnp-org:device-1-0\">
        <specVersion>
                <major>1</major>
                <minor>0</minor>
        </specVersion>
        <device>
                <deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType>
                <presentationURL>$WEB_ADDR</presentationURL>
                <friendlyName>WR6202 Device</friendlyName>
                <manufacturer>Accton Wireless Broadband Corp.</manufacturer>
                <manufacturerURL>http://www.awbnetworks.com/</manufacturerURL>
                <modelDescription>Wifi Router 6202</modelDescription>
                <modelName>WR6202</modelName>
                <modelNumber>WR6202</modelNumber>
                <serialNumber>1.00</serialNumber>
                <UDN>uuid:75802409-bccb-40e7-8e6c-fa095ecce13e</UDN>
                <iconList>
                        <icon>
                                <mimetype>image/gif</mimetype>
                                <width>118</width>
                                <height>119</height>
                                <depth>8</depth>
                                <url>/ligd.gif</url>
                        </icon>
                </iconList>
                <serviceList>
                        <service>
                                <serviceType>urn:schemas-upnp-org:service:Layer3Forwarding:1</serviceType>
                                <serviceId>urn:upnp-org:serviceId:L3Forwarding1</serviceId>
                                <controlURL>/upnp/control/L3Forwarding1</controlURL>
                                <eventSubURL>/upnp/control/L3Forwarding1</eventSubURL>
                                <SCPDURL>/l3fwdSCPD.xml</SCPDURL>
                        </service>
                </serviceList>
                <deviceList>
                        <device>
                                <deviceType>urn:schemas-upnp-org:device:WANDevice:1</deviceType>
                                <friendlyName>WANDevice</friendlyName>
                                <manufacturer>Linux UPnP IGD Project</manufacturer>
                                <manufacturerURL>http://linux-igd.sourceforge.net</manufacturerURL>
                                <modelDescription>WAN Device on Linux IGD</modelDescription>
                                <modelName>Linux IGD</modelName>
                                <modelNumber>1.00</modelNumber>
                                <modelURL>http://linux-igd.sourceforge.net</modelURL>
                                <serialNumber>1.00</serialNumber>
                                <UDN>uuid:c4131cbd-0408-463d-91b0-38dd1ee22fd6</UDN>
                                <UPC>Linux IGD</UPC>
                                <serviceList>
                                        <service>
                                                <serviceType>urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1</serviceType>
                                                <serviceId>urn:upnp-org:serviceId:WANCommonIFC1</serviceId>
                                                <controlURL>/upnp/control/WANCommonIFC1</controlURL>
                                                <eventSubURL>/upnp/control/WANCommonIFC1</eventSubURL>
                                                <SCPDURL>/gateicfgSCPD.xml</SCPDURL>
                                        </service>
                                </serviceList>
                                <deviceList>
                                        <device>
                                                <deviceType>urn:schemas-upnp-org:device:WANConnectionDevice:1</deviceType>
                                                <friendlyName>WANConnectionDevice</friendlyName>
                                                <manufacturer>Linux UPnP IGD Project</manufacturer>
                                                <manufacturerURL>http://linux-igd.sourceforge.net</manufacturerURL>
                                                <modelDescription>WanConnectionDevice on Linux IGD</modelDescription>
                                                <modelName>Linux IGD</modelName>
                                                <modelNumber>0.95</modelNumber>
                                                <modelURL>http://linux-igd.sourceforge.net</modelURL>
                                                <serialNumber>0.95</serialNumber>
                                                <UDN>uuid:5f75f342-9f96-4ecc-9dbd-6f981efc3f14</UDN>
                                                <UPC>Linux IGD</UPC>
                                                <serviceList>
                                                        <service>
                                                                <serviceType>urn:schemas-upnp-org:service:WANIPConnection:1</serviceType>
                                                                <serviceId>urn:upnp-org:serviceId:WANIPConn1</serviceId>
                                                                <controlURL>/upnp/control/WANIPConn1</controlURL>
                                                                <eventSubURL>/upnp/control/WANIPConn1</eventSubURL>
                                                                <SCPDURL>/gateconnSCPD.xml</SCPDURL>
                                                        </service>
                                                </serviceList>
                                        </device>
                                </deviceList>
                        </device>
                </deviceList>
        </device>
</root>
" > /etc/linuxigd/gatedesc.xml
