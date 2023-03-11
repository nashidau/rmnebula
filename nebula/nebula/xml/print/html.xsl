<?xml version="1.0"?>

<!--
  ToDo:
        Need to fix 'half rank' and 'hidden flags' in cats/skills


-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="html" />

  <xsl:template match="/">
    <html>
      <head>
        <title><xsl:value-of select='/rmsscharacter/data/name' /></title>
        <link rel="stylesheet" type="text/css" 
                href="rmsscharacter.css"
                title="RMSS Character Style" />
      </head>
      <body>
        <h1><xsl:value-of select="/rmsscharacter/data/name" /></h1>
        <h2>Level 
           <xsl:value-of select="/rmsscharacter/data/experience/level" />
           <xsl:text>  </xsl:text>
           <xsl:value-of select="/rmsscharacter/data/fixed/profession" />
        </h2>
        <xsl:apply-templates select="/rmsscharacter/stats"/>
        <xsl:apply-templates select="/rmsscharacter/rrs"/>
        <xsl:apply-templates select="/rmsscharacter/skills"/>
      </body>
    </html>
  </xsl:template>


  <xsl:template match="data">
     <xsl:apply-templates mode="data"/>
  </xsl:template>

  <!-- Do the stats table -->
  <xsl:template match="stats">
    <table>
        <tr>
          <th>Stat</th>
          <th>Abbr</th>
          <th>Temp</th>
          <th>Pot</th>
          <th>Basic</th>
          <th>Racial</th>
          <th>Special</th>
          <th>Total</th>
        </tr>
        <xsl:apply-templates mode="stat" />
    </table>
  </xsl:template>
  <xsl:template match="stat" mode="stat" >
    <tr>
      <xsl:apply-templates mode="stat" />
    </tr>
  </xsl:template>
  <!-- FIXME: Should handle each column independantly -->
  <xsl:template match="name" mode="stat">
    <td><xsl:value-of select="." /></td>
  </xsl:template>
  <xsl:template match="abbr" mode="stat">
    <td class="abbr"><xsl:value-of select="." /></td>
  </xsl:template>
  <xsl:template match="*" mode="stat">
    <td class="numeric"><xsl:value-of select="." /></td>
  </xsl:template>

  <!-- RRs -->
  <xsl:template match="rrs">
    <table>
      <tr>
          <th>Resistance Roll</th>
          <th>Stats</th>
          <th>Stat Bonus</th>
          <th>Racial</th>
          <th>Special</th>
          <th>Total</th>
      </tr>
      <xsl:apply-templates select="rr" mode="rr"/>
    </table>
  </xsl:template>
  <xsl:template match="rr" mode="rr">
    <tr>
      <td><xsl:value-of select="name" /></td>  <!-- skill -->
      <td><xsl:value-of select="stats" /></td> 
      <td class="numeric"><xsl:value-of select="statbonus" /></td>
      <td class="numeric"><xsl:value-of select="racialbonus" /></td>
      <td class="numeric"><xsl:value-of select="specialbonus" /></td>
      <td class="numeric"><xsl:value-of select="totalbonus" /></td>
    </tr>
  </xsl:template>


  <!-- Skills -->
  <xsl:template match="skills">
    <table>
      <tr>
        <th>Skill</th>
        <th>Stats</th>
        <th>Class</th>
        <th>Progression</th>
        <th>Cost</th>
        <th>Prof</th>
        <th>Stat</th>
        <th>Spec</th>
        <th>Ranks</th>
        <th>Rank</th>
        <th>Bonus</th>
      </tr>
      <xsl:apply-templates mode="skills" />
    </table>
  </xsl:template>
  <xsl:template match="category" mode="skills" >
      <tr class="category">
        <td class="title"><xsl:value-of select="name" /></td>  <!-- skill -->
        <td><xsl:value-of select="statbonuses" /></td> 
        <td></td>
        <td><xsl:value-of select="progression" /></td>
        <td class="numeric"><xsl:value-of select="cost" /></td>
        <td class="numeric"><xsl:value-of select="professionbonus" /></td>
        <td class="numeric"><xsl:value-of select="statbonus" /></td>
        <td class="numeric"><xsl:value-of select="specialbonus" /></td>
        <td class="numeric"><xsl:value-of select="ranks" /></td>
        <td class="numeric"><xsl:value-of select="rankbonus" /></td>
        <td class="numeric"><xsl:value-of select="total" /></td>
      </tr>
    <xsl:apply-templates select="skill|skillgroup" mode="cat" />
  </xsl:template>
  <xsl:template match="skill" mode="cat">
    <tr>
      <td class="skill"><xsl:value-of select="name" /></td>  
      <td> </td> 
      <td><xsl:value-of select="class" /></td>
      <td> </td>
      <td class="numeric"><xsl:value-of select="cost" /></td>
      <td> </td>
      <td> </td>
      <td class="numeric"><xsl:value-of select="specialbonus" /></td>
      <td class="numeric"><xsl:value-of select="ranks" /></td>
      <td class="numeric"><xsl:value-of select="rankbonus" /></td>
      <td class="numeric"><xsl:value-of select="total" /></td>
    </tr>
  </xsl:template>

  <xsl:template match="skillgroup" mode="cat">
    <tr>
      <xsl:apply-templates mode="skillgroup" />
    </tr>
  </xsl:template>
  <xsl:template match="name" mode="skillgroup">
    <td class="skillgroup" colspan="11"><xsl:value-of select="." /></td>
  </xsl:template>
  <!-- Kill everything else -->
  <xsl:template match="*" mode="skillgroup" />
  
  <xsl:template match="skill" mode="skillgroup">
    <tr>
      <td class="skillgroupmember"><xsl:value-of select="name" /></td>  
      <td><xsl:value-of select="statbonuses" /></td> 
      <td><xsl:value-of select="class" /></td>
      <td class="numeric"><xsl:value-of select="progression" /></td>
      <td class="numeric"><xsl:value-of select="cost" /></td>
      <td class="numeric"><xsl:value-of select="professionbonus" /></td>
      <td class="numeric"><xsl:value-of select="statbonus" /></td>
      <td class="numeric"><xsl:value-of select="specialbonus" /></td>
      <td class="numeric"><xsl:value-of select="ranks" /></td>
      <td class="numeric"><xsl:value-of select="rankbonus" /></td>
      <td class="numeric"><xsl:value-of select="total" /></td>
    </tr>
  </xsl:template>
  <xsl:template match="name" mode="skill2">
    <td class="skillgroupmember"><xsl:value-of select="." /></td>
  </xsl:template>

</xsl:stylesheet>
