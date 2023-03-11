<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
    <xsl:output method="text" />

    <xsl:template match="/">
        <xsl:call-template name="header" />
        printborder
        <xsl:apply-templates select="rmsscharacter/data" mode="top" /> 
        <xsl:apply-templates select="rmsscharacter/stats" mode="top" />
        <xsl:apply-templates select="rmsscharacter/rrs" mode="top" />
        <xsl:call-template name="special" />
        <xsl:apply-templates select="rmsscharacter/skills" />
        <xsl:call-template name="footer" />
    </xsl:template>
    
  <xsl:template match="data" mode="top" >
    col1
    (<xsl:value-of select="./name" />)
    (<xsl:choose><xsl:when test="experience/level =
    '1'">1st</xsl:when><xsl:when test="experience/level =
    '2'">2nd</xsl:when><xsl:when test="experience/level =
    '3'">3rd</xsl:when><xsl:otherwise><xsl:value-of
    select="experience/level"/>th</xsl:otherwise></xsl:choose>)
    (<xsl:value-of select="fixed/race" />) 
    (<xsl:value-of select="fixed/profession" />)
    (<xsl:value-of select="experience/experiencepoints" />) titleshow
  </xsl:template>
    
  <xsl:template match="stats" mode="top">
    beginstats
    <xsl:apply-templates select="stat" />
  </xsl:template>
  
  <xsl:template match="stat">
    (<xsl:value-of select="name"/> (<xsl:value-of select="abbr" />))
    (<xsl:value-of select="temp"/>) (<xsl:value-of select="total"/>) stat
  </xsl:template>
  
  <xsl:template match="rrs" mode="top" >
    beginrrs 
    <xsl:apply-templates select="rr" />
  </xsl:template>
  <xsl:template match="rr">
    <xsl:if test="name='Poison'"><xsl:text>topline</xsl:text></xsl:if>
    (<xsl:value-of select="name"/>) (<xsl:value-of select="totalbonus"/>) rr
  </xsl:template>
  

  <xsl:template match="skills">
    beginskills
    <xsl:apply-templates select="category" mode="skills" />
  </xsl:template>
  
  
  <xsl:template match="category" mode="skills" >
    (<xsl:value-of select="name" />) (<xsl:value-of select="ranks" />) (<xsl:value-of select="total" />) cat
    <xsl:apply-templates select="skill|skillgroup" mode="cat" />
  </xsl:template>

  <xsl:template match="skill" mode="cat">
    <xsl:if test="ranks > 0">
      (<xsl:value-of select="name" />) (<xsl:value-of select="ranks" />) (<xsl:value-of select="total" />) skill
    </xsl:if>
  </xsl:template>
  <xsl:template match="skillgroup" mode="cat">
    <xsl:if test="sum(skill/ranks) > 0">
    (<xsl:value-of select="name" />) skillgroup
    <xsl:apply-templates mode="skillgroup" />
    </xsl:if>
  </xsl:template>
  <xsl:template match="skill" mode="skillgroup">
    <xsl:if test="ranks > 0">
      (<xsl:value-of select="name" />) (<xsl:value-of select="ranks" />) (<xsl:value-of select="total" />) skillgroupitem
    </xsl:if>
  </xsl:template>
    
  <xsl:template name="special">
    <xsl:variable name="hits">
      <xsl:value-of select=
        "/rmsscharacter/skills/category[child::name='Body Development']
              /skill[child::name='Body Development']/total"/>
    </xsl:variable>
    <xsl:variable name="pp">
      <xsl:value-of select=
        "/rmsscharacter/skills/category[child::name='Power Point Development']
              /skill[child::name='Power Point Development']/total"/>
    </xsl:variable>
    <xsl:variable name="rstat">
      <xsl:value-of select=
        "sum(/rmsscharacter/stats/stat[child::realmstat]/total) div 
                count(/rmsscharacter/stats/stat[child::realmstat]/total)" />
    </xsl:variable>
    <xsl:variable name="constitution">
      <xsl:value-of select=
        "sum(/rmsscharacter/stats/stat[child::name='Constitution']/total)"/> 
    </xsl:variable>
    <xsl:variable name="quickness">
      <xsl:value-of select=
        "sum(/rmsscharacter/stats/stat[child::name='Quickness']/total)"/> 
    </xsl:variable>
  
    newtopcol
    (Hits: <xsl:value-of select="$hits" />) printline
     ( (-10) <xsl:value-of select="round($hits div 4 * 3)" />) printline
     ( (-20) <xsl:value-of select="round($hits div 2)" />) printline 
     ( (-30) <xsl:value-of select="round($hits div 4)" />) printline 
     topline
    (PP: <xsl:value-of select="$pp" />) printline
     ( (-10) <xsl:value-of select="round($pp div 4 * 3)" />) printline
     ( (-20) <xsl:value-of select="round($pp div 2)" />) printline 
     ( (-30) <xsl:value-of select="round($pp div 4)" />) printline 
     topline
     beginrecover
     (1 Hits/3 hour active) printline
     (<xsl:value-of select="round($constitution div 2)" /> Hits/hour resting) printline
     (<xsl:value-of select="$constitution * 2" /> Hits/sleep cycle) printline
     (1 PP/3 hours active) printline
     (<xsl:value-of select="round($rstat div 2)" /> PP/hour) printline
     (<xsl:value-of select="round($pp div 2)" /> PP/sleep cycle) printline
     endrecover

    newtopcol
    /colwidth colwidth 8 add def
    <xsl:if test="/rmsscharacter/items/armor">
    (AT: <xsl:value-of select="/rmsscharacter/items/armor/at"/>) printline
    </xsl:if>
    (DB: <xsl:value-of select="$quickness * 3"/>) printline
    (  Surprised: <xsl:value-of select="($quickness * 3) - 20"/>) printline
    (  Flanked: <xsl:value-of select="($quickness * 3) - 15"/>) printline
    (  Rear: <xsl:value-of select="($quickness * 3) - 20"/>) printline
    topline
    <xsl:call-template name="primary-skills" />


  </xsl:template>
    
 <!-- Kill everything else -->
  <xsl:template match="*" mode="skillgroup" />

    <xsl:template name="primary-skills">
        /tmpcolwidth colwidth def
        /colwidth topcolwidth def
        /tmpindent indent def
        /indent 0 def
        bold setfont
        (Skill) (Ranks) (Bonus) cat
        norm setfont
        <xsl:apply-templates 
                        select="/rmsscharacter/skills//skill[child::primary]"
                        mode="cat" />
        /colwidth tmpcolwidth def
        /indent tmpindent def
    </xsl:template>
<!--
    The file header 
-->
<xsl:template name="header"> %!PS

/pagewidth 595 def
/pageheight 840 def


/margin 20 def

/topofpage pageheight margin sub def
/indent 8 def

/fontsize 9 def
/titlefontsize 12 def

/topofpage pageheight margin sub def

/colwidth pagewidth margin margin add sub 3 div def
/topcolwidth pagewidth margin margin add sub 4 div def

/norm /Times-Roman findfont fontsize scalefont def
/bold /Times-Bold findfont fontsize scalefont def
/italic /Times-Italic findfont fontsize scalefont def
/titlefontbold /Times-Bold findfont titlefontsize scalefont def
/titlefontnorm /Times-Roman findfont titlefontsize scalefont def


/col1 {
        /xpos margin def
        /ypos topofpage def
} def
/newcol {
        /xpos xpos colwidth add def
        /ypos topofpage def

               stroke
         (Cat/Skill) (Ranks) (Bonus) cat       
        
        xpos 3 sub ypos fontsize add 2 sub moveto
        colwidth 0 rlineto
        stroke
        
        /ypos ypos 2 sub def
} def
/newline {
        /ypos ypos fontsize sub def
        ypos margin lt {
                newcol
        } if
} def

/rightshow {
        /rs_string exch def
        rs_string stringwidth
        pop
        neg 0 rmoveto
        rs_string show
} def

% Cat
% (name) (ranks) (bonus) 
/cat {
        /bonus exch def
        /ranks exch def

        bold setfont
        xpos ypos moveto 
        &lt;c2b7&gt; (\267) substitute
        (\342\200\242) (\267) substitute show
        shownumbers
        
        norm setfont
        newline
} def
/stat {
        /bonus exch def
        /ranks exch def

        xpos ypos moveto 
        show
        xpos topcolwidth 38 sub add ypos moveto
               ranks rightshow
        xpos topcolwidth 8 sub add ypos moveto
                 bonus rightshow

        newline
} def
/rr {
        /bonus exch def
        xpos ypos moveto
        show 
        
        xpos topcolwidth 8 sub add ypos moveto
                bonus rightshow
        newline
} def


/skill {
        /bonus exch def
        /ranks exch def

        xpos indent add ypos moveto
        show
        shownumbers        
        newline
} def
/skillgroup {
        xpos indent add ypos moveto
        italic setfont
        show
        norm setfont
        newline
} def
/skillgroupitem {
        /bonus exch def
        /ranks exch def
        xpos indent indent add add ypos moveto
        show
        shownumbers        
        newline
} def

/shownumbers {
        xpos colwidth 38 sub add ypos moveto
                ranks rightshow
        xpos colwidth 8 sub add ypos moveto
                bonus rightshow
} def

%%%
% substitute
%%%
% Substitutes all occurances of a substring in a string
% (string) (search) (replace) substitute (string)
%%%
/substitute {
        /replacement exch def
        /searchstr exch def
        /string exch def
        
        /diff searchstr length replacement length sub def

        % Begin loop...
        {
                string searchstr search
                % String found:
                {
                        /prefix exch def
                        /match exch def
                        /post exch def
         
                        % Make room for the new array
                        /moved 
                                string 
                                prefix length searchstr length add
                                post length getinterval
                        def
                        string
                                prefix length replacement length add
                                moved putinterval
                        % Copy in replacement
                        string prefix length replacement putinterval
                        % Space out any surplus
                        diff 0 gt {
                                1 1 diff {
                                        /loopcounter exch def
                                        string
                                                string length loopcounter sub
                                                32 put
                                } for
                        } if
                }
                % Not found
                {
                        pop
                        % not found
                        exit % Exit loop
                } ifelse
        } loop

        % return the string
        string
} def


/printborder {
        /leftedge margin 4 sub def
        /rightedge pagewidth margin sub 4 add def
        /topedge pageheight margin sub def
        /bottomedge margin 4 sub def
       
        2 setlinewidth
       
        leftedge topedge moveto
        rightedge topedge        lineto
        rightedge bottomedge lineto
        leftedge bottomedge lineto
        closepath

        stroke
} def

/titleshow {
        /xp exch def
        /prof exch def
        /race exch def
        /level exch def
        /name exch def
       
        titlefontbold setfont
        margin ypos titlefontsize sub moveto
        name show
        titlefontnorm setfont
        (, ) show 
        level show ( level ) show 
        race show ( ) show
        prof show
        10 0 rmoveto
        italic setfont
        xp show ( xp) show
        norm setfont

        1 setlinewidth
        margin 4 sub ypos titlefontsize sub 2 sub moveto
        pagewidth margin sub 4 add ypos titlefontsize sub 2 sub  lineto
        stroke
        
        /ypos ypos titlefontsize sub 2 sub fontsize sub def
        /topofpage ypos def
       
} def
/beginstats {
        norm setfont
        /endoftop topofpage def

        
        bold setfont
                (Stat (abbr)) (Temp) (Bonus) stat
        norm setfont

        /ypos ypos 2 sub def
} def
/beginrrs {
        newtopcol

        bold setfont
                (Resistance Roll) (Bonus) rr
        norm setfont
        /ypos ypos 2 sub def
} def
/newtopcol {
        ypos endoftop lt {
                /endoftop ypos def
        } if
      
        /xpos xpos topcolwidth add def 
        /ypos topofpage def
} def
/topline {
        1 setlinewidth
        xpos 3 sub ypos fontsize add 2 sub moveto
        xpos 3 sub topcolwidth add ypos fontsize add 2 sub lineto
        stroke

        /ypos ypos 2 sub def
} def
/beginskills {
        ypos endoftop lt {
                /endoftop ypos def
        } if
      
        % Draw the line accross
        margin 4 sub endoftop fontsize add 2 sub moveto
        pagewidth margin sub 4 add endoftop fontsize add 2 sub lineto
        stroke
       
        % Draw lines up to divide the 4 columns
        /tpos topcolwidth margin add 3 sub def
        tpos topofpage fontsize add moveto
        tpos endoftop fontsize add 2 sub lineto
        /tpos tpos topcolwidth add def
        tpos topofpage fontsize add moveto
        tpos endoftop fontsize add 2 sub lineto
        /tpos tpos topcolwidth add def
        tpos topofpage fontsize add moveto
        tpos endoftop fontsize add 2 sub lineto
        stroke
 
        stroke
       
        /topofpage endoftop 3 sub def

        
        col1

        % Print the 3 borders
        xpos colwidth add 3 sub dup  ypos fontsize add 1 add moveto
                margin 4 sub lineto
        xpos colwidth 2 mul add 3 sub dup  ypos fontsize add 1 add moveto
                margin 4 sub lineto


        (Cat/Skill) (Ranks) (Bonus) cat
        margin 4 sub ypos fontsize add 2 sub moveto
        colwidth 0 rlineto
        stroke
        /ypos ypos 2 sub def
        
} def
/printline {
        xpos ypos moveto
        show
        newline
} def

/beginrecover {
        xpos ypos moveto
        (Recover: ) show
        currentpoint pop
        /oldxpos xpos def
        /xpos exch def
} def
/endrecover {
        /xpos oldxpos def
} def

%%Page: 1 1
</xsl:template>

<xsl:template name="footer">
showpage

%%Trailer
</xsl:template>


</xsl:stylesheet>
  
