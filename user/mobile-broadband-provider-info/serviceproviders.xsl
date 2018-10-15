<xsl:stylesheet
xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
version="1.0">

<xsl:strip-space elements="*"/>
<xsl:output method="text"/>

<xsl:template match="/">
<xsl:apply-templates select="serviceproviders/country/provider/gsm/apn"/>
</xsl:template>

<xsl:template match="apn">
<xsl:value-of select="../../../@code"/>
<xsl:text>,</xsl:text>

<xsl:value-of select="../../name"/>
<xsl:text>,</xsl:text>

<xsl:text> </xsl:text>
<xsl:for-each select="../network-id">
<xsl:value-of select="@mcc"/>
<xsl:value-of select="@mnc"/>
<xsl:text> </xsl:text>
</xsl:for-each>

<xsl:text>,</xsl:text>

<xsl:text> </xsl:text>
<xsl:for-each select="../network-id">
<xsl:value-of select="@mnc"/>
<xsl:text> </xsl:text>
</xsl:for-each>

<xsl:text>,</xsl:text>

<xsl:value-of select="@value"/> 
<xsl:text>,</xsl:text>
<xsl:value-of select="usage/@type"/> 
<xsl:text>,dns='</xsl:text>
<xsl:for-each select="dns">
<xsl:value-of select="."/>
<xsl:text> </xsl:text>
</xsl:for-each>
<xsl:text>' </xsl:text>
<xsl:text>&#xA;</xsl:text>
</xsl:template>
</xsl:stylesheet>
