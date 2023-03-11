<?xml version="1.0"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text" />
 
  <xsl:template match="/">

	  <!-- <xsl:include  href="spelllistbase.ps" /> -->
#include "spelllistbase.ps"
  
		<xsl:apply-templates select="spelllist|spellgroup|spellbook" />

    showpage
	</xsl:template>

	<xsl:template match="spellbook">
		<xsl:apply-templates select="spellgroup" />
	</xsl:template>

	<xsl:template match="spellgroup">
		<xsl:apply-templates select="spelllist" />
	</xsl:template>
	
	<xsl:template match="spelllist">
    % The spell list name
    (<xsl:value-of select="name" />) printtitle
  
	  <!--
    (<xsl:value-of select="normalize-space(spelllist/description)" />)
        parprint
   	-->

    % The spellist summary table
    beginspellsummary
    <xsl:apply-templates select="spell" mode="summary" /> 
    endspellsummary
    
    % Spell descriptions 
    <xsl:apply-templates select="spell" mode="long" />

    % The notes: if any
    <xsl:apply-templates select="notes" mode="notes" />

  </xsl:template>
  
  <xsl:template match="spell" mode="summary">
    (<xsl:value-of select="normalize-space(level)" />)
    (<xsl:value-of select="normalize-space(name)" />
         <xsl:apply-templates select="nopp" />
     <xsl:apply-templates select="instantaneous" />)
    (<xsl:value-of select="normalize-space(areaofeffect)" />)
    (<xsl:value-of select="normalize-space(duration)" />)
    (<xsl:value-of select="normalize-space(range)" />)
    (<xsl:value-of select="normalize-space(type)" />)
    printsummary
  </xsl:template>
  
  <xsl:template match="spell" mode="long">
    (<xsl:value-of select="level" />) 
    (<xsl:value-of select="name" />)
    (<xsl:value-of select="normalize-space(description)" 
        /><xsl:text> </xsl:text>) 
        printspell
  </xsl:template>

  <xsl:template match="nopp"><xsl:text> \267</xsl:text></xsl:template>
  <xsl:template match="instantaneous">*</xsl:template>

  <!-- Spell list notes: Autonumbered too ;-) -->
  <xsl:template match="notes" mode="notes">
    boldfont setfont (Notes:) print normalfont setfont
    <xsl:for-each select="note">
      (<xsl:number value="position()" format="1. "/>) print
      (<xsl:value-of select="normalize-space(.)" />) printhere
      
    </xsl:for-each>
  </xsl:template>

</xsl:stylesheet>

<!-- vim:set sw=2 : -->
