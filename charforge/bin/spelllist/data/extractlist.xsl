<?xml version='1.0' ?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:variable name="spells" select="document('file:///home/nash/work/charforge/bin/spelllist/data/spells.xml')"/>
  

  <xsl:output method="xml" />

  <xsl:template match="/">
    <spellbook>
      <spellgroup>
	<xsl:apply-templates select="/spelllistsummary/list" />
      </spellgroup>
    </spellbook>
  </xsl:template>

  <xsl:template match="list">
    <xsl:variable name="listname" select="@name" />
    <xsl:variable name="maxlevel" select="@level" />

    <xsl:for-each select="$spells/spellbook/spellgroup/spelllist">
      <xsl:if test="name=$listname">
	<!-- FIXME: Should be everything != spell -->
	<spelllist>
	  <xsl:copy-of select="listtype|name|id|author" />
	  <xsl:for-each select="spell">
	    <xsl:if test="level &lt;= $maxlevel">
	      <spell>
		<xsl:copy-of select="*" />
	      </spell>
	    </xsl:if>
	  </xsl:for-each>
	  <!-- Copy the notes last so they are at the bottom -->
	  <xsl:copy-of select="notes" />
	</spelllist>
      </xsl:if>
    </xsl:for-each>
  </xsl:template>

</xsl:stylesheet>

<!-- vim:set sw=2 : -->
