<?xml version="1.0"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text" />
 
  <xsl:template match="/">

    <xsl:call-template name="preamble" />
  
    % The spell list name
    (<xsl:value-of select="spelllist/name" />) printtitle
    
    % The spellist summary table
    beginspellsummary
    <xsl:apply-templates select="spelllist/spell" mode="summary" /> 
    endspellsummary
    
    % Spell descriptions 
    <xsl:apply-templates select="spelllist/spell" mode="long" />


    <xsl:call-template name="footer" />
  </xsl:template>
  
  <xsl:template match="spell" mode="summary">
    (<xsl:value-of select="normalize-space(level)" />)
    (<xsl:value-of select="normalize-space(name)" />
        <xsl:text> </xsl:text>
         <xsl:apply-templates select="nopp" />
     <xsl:apply-templates select="instantaneous" />)
    (<xsl:value-of select="normalize-space(areaofeffect)" />)
    (<xsl:value-of select="normalize-space(duration)" />)
    (<xsl:value-of select="normalize-space(range)" />)
    (<xsl:value-of select="normalize-space(type)" />)
    printsummary
  </xsl:template>
  
  <xsl:template match="spell" mode="long">
    [(<xsl:value-of select="level" />.) 
        /boldfont (<xsl:value-of select="name" />)
        /normalfont (\320)
        <xsl:apply-templates select="description" mode="spelldescription" />]
        splitwords print
  </xsl:template>
  
  <xsl:template match="description" mode="spelldescription">
    (<xsl:apply-templates select="ref|text()" />)
  </xsl:template>
  <xsl:template match="ref">
    <xsl:text>) /italicfont (</xsl:text>
    <xsl:value-of select="." />
    <xsl:text>) /normalfont (</xsl:text>
  </xsl:template>

  <xsl:template match="nopp">\267</xsl:template>
  <xsl:template match="instantaneous">*</xsl:template>

  <xsl:template name="preamble">
        <xsl:text>%!
%%Pages: 1
%
% Base.ps
%
% A Postscript file for printing characters and spellists (and
% maybe other things).
%
% Generally the data should be able to be appended after %Page: 1 1 mark,
% but before trailer
%

%
% Font information
/fontsize 8 def
/lineskill fontsize 1 add def
/titlefontsize 16 def
/normalfont /Times-Roman findfont fontsize scalefont def
/boldfont /Times-Bold findfont fontsize scalefont def
/italicfont /Times-Italic findfont fontsize scalefont def
/titlefont /Helvitica findfont titlefontsize scalefont def

%/Em /italicfont setfont def

/wordbreak ( ) def

% This may not be used
/spacewidth normalfont setfont wordbreak stringwidth pop def

% 
% Page infomation
/topofpage 800 def
/bottomofpage 50 def
/pagewidth 595 def
/marginwidth 20 def

/parindent -5 def

%
% Functions
/newpage {
        /yline topofpage def
        /leftmargin marginwidth def
        /rightmargin pagewidth marginwidth sub 2 div def
} def
/newcol {
        /yline topofpage def 
        /leftmargin rightmargin marginwidth add def
        /rightmargin pagewidth marginwidth sub def
} def

/newline {
        yline bottomofpage lt {
                newcol
        } if 
        /wordcount 0 def
        /yline yline lineskill sub def
        leftmargin yline moveto
        /currentpos leftmargin def
        mark
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

%%%
% splitwords
%%%
% Splits an array of strings (and others) into individual words
% split on spaces
% [ (string) /normalfont (string) ... ] splitwords [ (word) (word)...]
%%%
/splitwords {
        /inputarray exch def
        
        mark % start the output array
        
        % Loop over each element
        inputarray {
                /sw_string exch def
                sw_string type /stringtype ne {
                        sw_string
                } {
                        sw_string splitstring
                } ifelse
        } forall

        ] % close the array
} def
%%%
% splitstring
%%%
% Splits a single string into words based on the array
% (multiple words) splitstring (multiple) (words)
%%
/splitstring {
        /ss_string exch def

        {
                ss_string wordbreak search {
                        /nextword exch def pop
                        /ss_string exch def 
                
                        % If it is length 0 throw it away
                        nextword length 0 ne {
                                nextword
                        } if
                }
                {
                        pop exit
                } ifelse
        } loop

        % Put the last item if it is longer then 0
        ss_string length 0 ne {
                ss_string
        } if
} def

%%%
% print
%%%
% Formats a string, justifies it, then displays it
% [printingarray] printjustify -
%%%
/print { 
        /inputstring exch def 
      
        % Start the paragraph (including indent)
        /wordcount 0 def
        /currentpos leftmargin parindent add def
        [ % Start the newline
        currentfont % So we can recover it
        currentpos yline moveto
        
        
        inputstring { printfunc } forall
        rightmargin printline
} def
/printfunc {
        /string exch def
       
        string type /stringtype ne {
                string cvx exec setfont 
        } {
                /wordcount wordcount 1 add def
                /thiswidth string stringwidth pop def
       
                /currentpos currentpos thiswidth add 
                        ( ) stringwidth pop add def
       
                currentpos rightmargin gt {
                        currentpos thiswidth sub printline 
                        /currentpos currentpos thiswidth add spacewidth add def
                } if
        } ifelse
        string
} def
/printline {
        /pl_linewidth exch def
        
        ] % Close the array

        /pl_array exch def
        rightmargin pl_linewidth ne {
                /pl_space rightmargin pl_linewidth sub 
                wordcount 1 sub div def
        } {
                /pl_space 0 def
        } ifelse
        
        pl_array {  
                /pl_string exch def
                pl_string type /stringtype ne {
                        pl_string cvx exec setfont 
                } {
                        pl_string show
                        ( ) show
                        pl_space  0 rmoveto
                } ifelse
        } forall

        newline
        % save the current font for the next line
        currentfont
        
} def

%
% printtitle
%
% Prints the Spell list name - centered in current column
% (name) [printtitle] -
/printtitle {
        /title exch def
        
        titlefont setfont
        title stringwidth
        pop 2 div 
        leftmargin rightmargin add 2 div
        exch sub
        yline moveto title show
        /yline yline titlefontsize 2 add sub def 
        normalfont setfont
} def


% Spell list summary
%
% printsummary
% (lvl) (spellname) (aoe) (Duration) (Range) (Type)
/printsummary {
        /spelltype exch def
        /spellrange exch def
        /spellduration exch def
        /spellarea exch def
        /spellname exch def
        /spelllvl exch def

        /lvlwidth 15 def
        /namewidth 100 def
        /areawidth 50 def
        /durwidth 50 def
        /rangewidth 30 def
        /typewidth 15 def
        

        /lvlstart 20 def  % leftmargin
        /namestart lvlstart lvlwidth add def 
        /areastart namestart namewidth add def 
        /durstart areastart areawidth add def
        /rangestart durstart durwidth add def
        /typestart rangestart rangewidth add def

        (6) spelllvl eq 
        (11) spelllvl eq 
        (16) spelllvl eq 
        (21) spelllvl eq  or or or
        {  /yline yline fontsize 2 div sub def } if


        lvlstart lvlwidth spelllvl justifyright % Should be justify right
        currentfont
        boldfont setfont
        namestart 4 add yline moveto spellname show % Note different
        setfont % restore previous font
        areastart areawidth spellarea justifyword
        durstart durwidth spellduration justifyword
        rangestart rangewidth spellrange justifyword
        typestart typewidth spelltype justifyword

        /yline yline fontsize 1 add sub def

} def
/beginspellsummary {
        /yline yline 3 sub def
        boldfont setfont
        (Lvl) (Name) (Area Of Effect) (Duration) (Range) (Type) printsummary
        normalfont setfont
        /yline yline 1 sub def
} def
/endspellsummary {
        /yline yline fontsize sub def

} def
% 
% Justifies a single word or phrase in a box 
% Will not overlap other justifyword printings
% (pos) (width) (word) [justifyword] -
/justifyword {
        /word exch def
        /boxwidth exch def
        /pos exch def 
        
        % Fixme
        /thiswidth word stringwidth pop def
        pos boxwidth 2 div add % Center of box
        thiswidth 2 div sub % position correctly
        yline moveto
        word show
} def

% Justifies a single word or phrase in a box 
% Will not overlap other justifyword printings
% (pos) (width) (word) [justifyright] -
/justifyright {
        /word exch def
        /boxwidth exch def
        /pos exch def 
        
        % Fixme
        /thiswidth word stringwidth pop def
        pos boxwidth add % Center of box
        thiswidth sub % position correctly
        yline moveto
        word show
} def


% Set some basic state
normalfont setfont

%%Page: 1 1
newpage</xsl:text>
        </xsl:template>        


  <xsl:template name="footer">
<xsl:text>
showpage
</xsl:text>
        </xsl:template>

</xsl:stylesheet>
