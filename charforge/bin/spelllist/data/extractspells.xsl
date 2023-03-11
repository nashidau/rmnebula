<?xml version="1.0"?>

<!--
  Given a chracter extracts a list of known spells and the level the list is
  known too.

  So 
    character.xml
      ||
      || extractspells.xsl
      \/
    knownlists.xml

  Output format is:
    <spelllistsummary>
      <list name="list name" level="ranks/max level known" />
    </spelllistsummary>

  Note the output format is optimsied for easy parins in XSLTs, not for
  general use.  It is designed as intermediate format.
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="xml" />

  <xsl:template match="/">
    <spelllistsummary>
      <xsl:for-each select="/rmsscharacter/skills/category">
	<xsl:if test="starts-with(name, 'Spells ')">
	  <xsl:apply-templates select="skill" />
	</xsl:if>
      </xsl:for-each>
    </spelllistsummary>
  </xsl:template>

  <xsl:template match="skill">
    <xsl:if test="ranks &gt; 0">
      <xsl:element name="list">
	<xsl:attribute name="name">   
	  <xsl:value-of select="name" />
	</xsl:attribute>
	<xsl:attribute name="level">
	  <xsl:value-of select="ranks" />
	</xsl:attribute>
      </xsl:element>
    </xsl:if>
  </xsl:template>

</xsl:stylesheet>

<!-- vim:set sw=2: -->
