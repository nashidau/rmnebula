<?xml version='1.0'?>

<!--
        This XSLT converts a spelllist produced by Bent Dalager
        http://www.pvv.org/~bcd/rolemaster/
        to the format used by myself.
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="xml" />

  <xsl:template match="/">
    <spelllist>
      <name><xsl:value-of select="list/@name" /></name>
      <listtype>
        <xsl:value-of select="list/@realm" /><xsl:text> </xsl:text>
        <xsl:value-of select="list/@type"  /><xsl:text> </xsl:text>
      </listtype>
      
      <author>Bent Dalager</author>

      <xsl:copy-of select="list/description" />

      <xsl:apply-templates select="list/spell" />

      <xsl:if test="count(list/specialnote) &gt; 0">
        <notes>
          <xsl:apply-templates select="list/specialnote" />
        </notes>
      </xsl:if>
      
    </spelllist>
  </xsl:template>

  <xsl:template match="spell">
    <spell>
      <level><xsl:value-of select="@level" /></level>
      <name><xsl:value-of select="@name" /></name>
      <areaofeffect><xsl:value-of select="area" /></areaofeffect>
      <xsl:copy-of select="duration" />
      <xsl:copy-of select="range" />
      <xsl:copy-of select="type" />
      <xsl:copy-of select="description" />
      <xsl:copy-of select="annotation" />
    </spell>
  </xsl:template>
 
  <xsl:template match="specialnote">
    <note><xsl:value-of select="." /></note>
  </xsl:template>

</xsl:stylesheet>
