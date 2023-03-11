<?xml version='1.0' ?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" 
				version="1.0">

  <xsl:output method="xml" />

  <xsl:template match="/">
    <rmssprofession>
      <description>
	FIXME: No description
      </description>
      <quote>
	FIXME: No Quote
      </quote>
      <baselists>
	<list>FIXME</list>
	<list>FIXME</list>
	<list>FIXME</list>
	<list>FIXME</list>
	<list>FIXME</list>
	<list>FIXME</list>
      </baselists>
      <xsl:copy select="profession/data" />
      <xsl:apply-templates select="profession/skills" />
      <!-- xsl:apply-templates select="stats" / -->

    </rmssprofession>
    <xsl:comment> vim:set sw=2 : </xsl:comment>
  </xsl:template>

  <xsl:template match="skills">
    <xsl:text>
    </xsl:text>
    <skills>
      <xsl:for-each select="category">
	<xsl:text>
	</xsl:text>
	<xsl:element name="category">
	  <xsl:attribute name="name">
	    <xsl:value-of select="name" />
	  </xsl:attribute>
	  <xsl:attribute name="cost">
	    <xsl:value-of select="cost" />
	  </xsl:attribute>
	  <xsl:if test="professionbonus">
	    <xsl:attribute name="bonus">
	      <xsl:value-of select="professionbonus" />
	    </xsl:attribute>
	  </xsl:if>
	  <xsl:apply-templates select="skill" />

	</xsl:element>
      </xsl:for-each>
      <xsl:text>
      </xsl:text>
    </skills>
  </xsl:template>

  <xsl:template match="skill">
    <xsl:text>
	  </xsl:text> <!-- For indenting -->
    <xsl:element name="skill">
      <xsl:attribute name="name">
	<xsl:value-of select="name" />
      </xsl:attribute>
      <xsl:attribute name="class">
	<xsl:value-of select="class" />
      </xsl:attribute>	
    </xsl:element>
  </xsl:template>

</xsl:stylesheet>

<!-- vim:set sw=2 : -->
