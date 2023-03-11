<?xml version="1.0"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="html" />
 
  <!-- Known represents what level the list is known too -->
  <xsl:param name="knownlevel">0</xsl:param>
  <!-- Condensed version puts the description in the table -->
  <xsl:param name="condensed">0</xsl:param>
  <!-- Should I show any annotations in spell lists? -->
  <xsl:param name="annotate">0</xsl:param>
  
  

  <xsl:template match="/">
    <html>
      <head>
        <title><xsl:value-of select='/spelllist/name' /></title>
        <link rel="stylesheet" type="text/css" 
                href="http://www.nash.nu/Rolemaster/XML/spelllist/spelllist.css"
                title="Spell List Style" />
      </head>
      <body>
         <h1><xsl:value-of select='/spelllist/name' /></h1>
         <h2><xsl:value-of select='/spelllist/id' />
             <xsl:text> </xsl:text>
         </h2>
         <h3>By <xsl:value-of select='/spelllist/author' /></h3>
       
         <xsl:if test="count(/spelllist/description) &gt; 0">
           <p><xsl:value-of select="/spelllist/description" /></p>
         </xsl:if>
       
         <table>
           <tr>
             <th></th> <!-- have list? -->
             <th class="numeric">Lvl</th> 
             <th class="name">Name</th>
             <th>Area of Effect</th>
             <th>Duration</th>
             <th>Range</th>
             <th>Type</th>
           </tr>
           <xsl:apply-templates select="spelllist/spell" mode="summary" />
         </table>
         
         <xsl:if test="$condensed != 1">
           <xsl:apply-templates select="spelllist/spell" mode="long" />
         </xsl:if>

         <xsl:apply-templates select="spelllist/notes" />
      </body>
    </html>
  </xsl:template>
  
  <xsl:template match="spell" mode="summary">
    <tr class="summary">
      <xsl:variable name="level">
        <xsl:value-of select="level" />
      </xsl:variable>
      <xsl:choose>

        <xsl:when test='$level&lt;=$knownlevel' >  
          <td><img src="http://www.nash.nu/Rolemaster/XML/spelllist/yes.png" alt="X" /></td>
        </xsl:when>

        <xsl:otherwise>
          <td>&#160;</td>
        </xsl:otherwise>
      </xsl:choose>

      <td class="numeric"><xsl:value-of select="level" /></td>
      <xsl:choose>
        <xsl:when test="$condensed=1">
          <td class="namebold">
            <xsl:value-of select="name" />
            <xsl:apply-templates select="nopp|instantaneous" />
          </td>
        </xsl:when>
        <xsl:otherwise>
          <td class="name">
            <a href="#{name}"><xsl:value-of select="name" /></a>
          </td>
        </xsl:otherwise>
      </xsl:choose>
      <td><xsl:value-of select="areaofeffect" /></td>
      <td><xsl:value-of select="duration" /></td>
      <td><xsl:value-of select="range" /></td>
      <td><xsl:value-of select="type" /></td>
    </tr>
    <xsl:if test="$condensed=1">
      <tr>
        <td colspan="2"></td>
        <td colspan="6">
          <p class="spell">
            <xsl:apply-templates select="rrmod" />
            <xsl:apply-templates select="description" />
          </p>
        </td>
      </tr>
    </xsl:if>
  </xsl:template>
  <xsl:template match="nopp">&#160;&#x2022;</xsl:template>
  <xsl:template match="instantaneous">*</xsl:template>


  <xsl:template match="spell" mode="long">
    <p class="spell"><xsl:value-of select="level" />. 
      <a name="{name}"></a>
      <span class="spellname"><xsl:value-of select="name" /></span>
      &#8212;
      <xsl:apply-templates select="rrmod" />
      <xsl:apply-templates select="description" />
    </p>
    <xsl:apply-templates select="annotation" />
  </xsl:template>

  <xsl:template match="annotation">
    <p class="annotation">
      <xsl:value-of select="." />
    </p>
  </xsl:template>
  <xsl:template match="rrmod">
    &#160;[RR Mod: <xsl:value-of select="." />]&#160;
  </xsl:template>


  <!-- Description with <ref> tags -->
  <xsl:template match="description">
    <xsl:apply-templates select="ref|text()" />
  </xsl:template>
  <xsl:template match="ref">
    <a href="#{.}" class="reference"><xsl:value-of select="." /></a>
  </xsl:template>


  <xsl:template match="notes">
     <h3>Notes</h3>
     <ol>
       <xsl:apply-templates select="note" />
     </ol>
  </xsl:template>

  <xsl:template match="note">
     <li> <xsl:value-of select="." /> </li>
  </xsl:template>
</xsl:stylesheet>
