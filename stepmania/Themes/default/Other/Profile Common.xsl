<?xml version="1.0" encoding="UTF-8" ?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	
	<!-- Global Definitions -->
	
	<xsl:output method="html" omit-xml-declaration="no" encoding="iso-8859-1" indent="no"/>

	<xsl:strip-space elements="*" />
		
	<xsl:attribute-set name="TopLevelTableAttr">
		<xsl:attribute name="cellpadding">4</xsl:attribute>
		<xsl:attribute name="cellspacing">1</xsl:attribute>
		<xsl:attribute name="border">0</xsl:attribute>
		<xsl:attribute name="width">100%</xsl:attribute>
	</xsl:attribute-set>

	<xsl:attribute-set name="DataTableAttr">
		<xsl:attribute name="align">left</xsl:attribute>
		<xsl:attribute name="cellpadding">2</xsl:attribute>
		<xsl:attribute name="cellspacing">0</xsl:attribute>
		<xsl:attribute name="border">0</xsl:attribute>
	</xsl:attribute-set>

	<xsl:attribute-set name="EntityTableAttr">
		<xsl:attribute name="cellpadding">3</xsl:attribute>
		<xsl:attribute name="cellspacing">1</xsl:attribute>
		<xsl:attribute name="border">0</xsl:attribute>
	</xsl:attribute-set>

	<xsl:attribute-set name="CollapsableTableAttr">
		<xsl:attribute name="cellpadding">3</xsl:attribute>
		<xsl:attribute name="cellspacing">1</xsl:attribute>
		<xsl:attribute name="border">0</xsl:attribute>
		<xsl:attribute name="width">100%</xsl:attribute>
	</xsl:attribute-set>

	<xsl:attribute-set name="CompactTableAttr">
		<xsl:attribute name="align">left</xsl:attribute>
		<xsl:attribute name="class">CompactTableAttr</xsl:attribute>
		<xsl:attribute name="cellpadding">3</xsl:attribute>
		<xsl:attribute name="cellspacing">1</xsl:attribute>
		<xsl:attribute name="border">0</xsl:attribute>
	</xsl:attribute-set>
	
	<xsl:attribute-set name="ToggleLinkAttr">
		<xsl:attribute name="onClick">JavaScript: toggleLinkClicked(event);</xsl:attribute>
		<xsl:attribute name="onSelectStart">JavaScript: return false;</xsl:attribute>
		<xsl:attribute name="onMouseOver">JavaScript: toggleLinkOnMouseOver(event);</xsl:attribute>
		<xsl:attribute name="onMouseOut">JavaScript: toggleLinkOnMouseOut(event);</xsl:attribute>
		<xsl:attribute name="style">cursor: hand;</xsl:attribute>		
	</xsl:attribute-set>





	<!-- Global Variables -->
	<xsl:variable name="Catalog" select="document('Catalog.xml')/Catalog" />





	<!-- Main Template -->

	<xsl:template name="MainTemplate">
		<xsl:param name="FullHeader" />
		<xsl:param name="DocName" />
		<xsl:param name="Content" />
		<html>
			<head>
				<title><xsl:value-of select="$Catalog/ProductTitle" /><xsl:text> </xsl:text><xsl:value-of select="$DocName" /></title>

				<script language="JavaScript">

					var imgOpenedSymbol = "images/arrow_down.gif";
					var imgClosedSymbol = "images/arrow_right.gif";
					
					function setDisplayAll( image ) {
						var elems = document.getElementsByName("toggleImage");
						for ( var i = 0; i &lt; elems.length; i++ ) {
							if ( elems[i].src != image ) {
								elems[i].src = image;
								toggleImageChanged( elems[i] );
							}
						}
					}
					function toggleLinkClicked(e) {
						var tg;
						if (!e) var e = window.event;
						if (e.target) tg = e.target;
						else if (e.srcElement) tg = e.srcElement;
						if (tg.nodeType == 3) // defeat Safari bug
							tg = targ.parentNode;
	
						var obj = tg;
						if ( obj.name != "toggleImage" ) {
							obj = obj.childNodes.item('toggleImage');
						}
						obj.src.search(imgOpenedSymbol) > -1 ? obj.src = imgClosedSymbol : obj.src = imgOpenedSymbol;
						toggleImageChanged(obj);
					}
					function toggleImageChanged(img) {
						var tg = img;
	
						var disp;
						tg.src.search(imgOpenedSymbol) > -1 ? disp = "" : disp = "none";
						var obj = tg.parentNode.parentNode.parentNode.parentNode;
						obj.rows[1].style.display = disp;
						
						var cell = img.parentNode;
						var spanOpen = cell.childNodes[1];
						spanOpen.style.display = disp;
						var spanClosed = cell.childNodes[2];
						spanClosed.style.display = (disp=="" ? "none" : "");
					}
					function toggleLinkOnMouseOver(e) {
						var tg;
						if (!e) var e = window.event;
						if (e.target) tg = e.target;
						else if (e.srcElement) tg = e.srcElement;
						if (tg.nodeType == 3) // defeat Safari bug
							tg = targ.parentNode;
	
						var obj = tg;
						obj.tagName == 'IMG' ? obj = obj.parentNode : null;
						obj.tagName == 'SPAN' ? obj = obj.parentNode : null;
						if ( obj.className == 'main' ) {
							obj.style.backgroundColor = '#C0C1C4';
						} else {
							obj.style.backgroundColor = '#C0C1C4';
						}
					}
					function toggleLinkOnMouseOut(e) {
						var tg;
						if (!e) var e = window.event;
						if (e.target) tg = e.target;
						else if (e.srcElement) tg = e.srcElement;
						if (tg.nodeType == 3) // defeat Safari bug
							tg = targ.parentNode;
	
						var obj = tg;
						obj.tagName == 'IMG' ? obj = obj.parentNode : null;
						obj.tagName == 'SPAN' ? obj = obj.parentNode : null;
						if ( obj.className == 'main' ) {
							obj.style.backgroundColor = '#DBE0E5';
						} else {
							obj.style.backgroundColor = '#DBE0E5';
						}
					}
					function startItUp() {
						setDisplayAll( imgClosedSymbol );
					}
					function navButtonMouseOver() {
						this.src = 'images/button_' + this.imagename + '_focus.gif';
					}
					function navButtonMouseOut() {
						this.src = 'images/button_' + this.imagename + '_up.gif';
					}
					function navButtonMouseDown() {
						this.src = 'images/button_' + this.imagename + '_down.gif';
					}
					function navButtonMouseUp() {
						this.src = 'images/button_' + this.imagename + '_focus.gif';
					}
				</script>
				<style type='text/css'>
A, A:visited	{
	color: #0072C2;
}
A:hover, A:active	{
	color: #FF6E00;
}

BODY	{
	font-family: Arial,Verdana,sans-serif;
	font-size: 11px;
	color: #3D5066;
	background: #A1ACB9;
}

TD	{
	background: #FFFFFF;
	vertical-align: top;
}

.pageheader	{
	background: #E1E5EA;
}

.pagefooter {
	background: #E1E5EA;
}

.clear	{
	background: #F4F6F7;
}

.valuename	{
	margin-bottom: 5px;
}

.visible {
}

.hidden {
	display:none
}

TABLE	{
	font-size: 11px;
	margin-top: 2px;
	margin-bottom: 5px;
	background: #DBE0E5;
}

SELECT	{
	font-size: 11px;
	background: #F4F6F7;
}

TH	{
	font-size: 11px;
	font-weight: bold;
	text-align: left;
	color: #3D5066;
}

TH.main	{
	font-size: 12px;
	font-weight: bold;
	text-align: left;
	color: #3D5066;
}

H1	{
	margin-top: 0px;
	margin-bottom: 8px;
	font-size: 16px;
	font-weight: bold;
	color: #005CB1;
}

H2	{
	margin-top: 0px;
	margin-bottom: 5px;
	font-size: 12px;
	font-weight: bold;
	color: #3D5066;
}

H3	{
	margin-top: 0px;
	margin-bottom: 2px;
	font-size: 11px;
	font-weight: bold;
	color: #3D5066;
}

HR	{
	height: 1px;
	color: #DBE0E5;
}

.dyndata	{
	color: #005CB1;
}

.titlename	{
	font-size: 10px;
}

.titlevalue {
	font-size: 10px;
	font-weight: bold;
}

.CompactTableAttr {
	margin: 2px 2px 2px 2px;
}
			</style>  

			</head>


			<body topmargin="3" bottommargin="3" leftmargin="3" rightmargin="3" onLoad="JavaScript: startItUp();">

				<table cellpadding="0" cellspacing="0" border="0" width="100%" height="100%">
					<tr>
						<td colspan="3" class="pageheader">
							<table cellpadding="10" cellspacing="0" border="0" width="100%" >
								<tr>
									<td nowrap="nowrap" class="pageheader"><h1><xsl:value-of select="$Catalog/ProductTitle" /><xsl:text> </xsl:text><xsl:value-of select="$DocName" /></h1></td>
									<td width="100%" class="pageheader"></td>
									<td nowrap="nowrap" class="pageheader"><h1>&lt; &lt; &lt; &lt; &lt;</h1></td>
								</tr>
							</table>
						</td>
					</tr>
					
					<tr>
						<td width="100%" height="100%" valign="top" name="masterContainer">
							<br />
							<table cellpadding="10" cellspacing="0" border="0" width="100%">
								<xsl:if test="$FullHeader = 1">
									<tr>
										<td nowrap="nowrap">
											<input type="button" value="Expand All" name="navButton" onClick="JavaScript:setDisplayAll( imgOpenedSymbol );" />
											<xsl:text> </xsl:text>
											<input type="button" value="Collapse All" name="navButton" onClick="JavaScript:setDisplayAll( imgClosedSymbol );" />
											<xsl:text> </xsl:text>
											<xsl:if test="$Catalog/InternetRankingHomeUrl != ''">
												<xsl:element name="input">
													<xsl:attribute name="type">button</xsl:attribute>
													<xsl:attribute name="value">Internet Ranking</xsl:attribute>
													<xsl:attribute name="name">navButton</xsl:attribute>
													<xsl:attribute name="onClick">JavaScript: window.location = '<xsl:value-of select="$Catalog/InternetRankingHomeUrl" />';</xsl:attribute>
												</xsl:element> 
											</xsl:if>
											<xsl:text> </xsl:text>
											<xsl:if test="$Catalog/InternetRankingUploadUrl != ''">
												<xsl:element name="input">
													<xsl:attribute name="type">button</xsl:attribute>
													<xsl:attribute name="value">Upload Stats</xsl:attribute>
													<xsl:attribute name="name">navButton</xsl:attribute>
													<xsl:attribute name="onClick">JavaScript: window.location = '<xsl:value-of select="$Catalog/InternetRankingUploadUrl" />&amp;stats_xml=' + window.location;</xsl:attribute>
												</xsl:element> 
											</xsl:if>
										</td>
										<td align="right" nowrap="nowrap" valign="top">
											<xsl:if test="/Stats/GeneralData/IsMachine = 1">
												<font class="titlename">Machine: </font><font class="titlevalue"><xsl:value-of select="/Stats/GeneralData/Guid" /></font>
											</xsl:if>
											<xsl:if test="/Stats/GeneralData/IsMachine = 0">
												<font class="titlename">Name: </font><font class="titlevalue"><xsl:value-of select="/Stats/GeneralData/DisplayName" /></font>
											</xsl:if>
											<br/>
											<font class="titlename">Last Played: </font><font class="titlevalue"><xsl:value-of select="/Stats/GeneralData/LastPlayedDate" /></font>
										</td>
									</tr>
								</xsl:if>
								<tr>
									<td colspan="2">
										<xsl:copy-of select="$Content" />
									</td>
								</tr>
							</table>
	
						</td>		
					</tr>

					<tr>
						<td colspan="3" class="pagefooter">
							<table cellpadding="10" cellspacing="0" border="0" width="100%" >
								<tr>
									<td nowrap="nowrap" class="pagefooter"><h1>&gt; &gt; &gt; &gt; &gt;</h1></td>
									<td width="100%" class="pagefooter"></td>
									<td nowrap="nowrap" class="pagefooter">
										<a>
											<xsl:attribute name="href"><xsl:value-of select="$Catalog/FooterLink" /></xsl:attribute>
											<xsl:value-of select="$Catalog/FooterText" />
										</a>
									</td>
								</tr>
							</table>
						</td>
					</tr>

				</table>
			</body>
		</html>
	</xsl:template>


	<!-- Main Categories End - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -->
	

	<!-- General Name Templates -->	
	
	<xsl:template match="*" mode="name" priority="-1">
		<!-- Strip the trailing 's' -->	
		<xsl:variable name="Type" select="substring(name(..),1,string-length(name(..))-1)" />
		<xsl:call-template name="Translate">
			<xsl:with-param name="Type" select="$Type" />
			<xsl:with-param name="Value" select="name()" />
		</xsl:call-template>
	</xsl:template>

	<xsl:template match="StepsType | @StepsType | Difficulty | @Difficulty | CourseDifficulty | @CourseDifficulty | Grade | @Grade | PlayMode | @PlayMode | Meter | @Meter">
		<xsl:call-template name="Translate">
			<xsl:with-param name="Type" select="name()" />
			<xsl:with-param name="Value" select="." />
		</xsl:call-template>
	</xsl:template>
	
	<xsl:template match="LastDifficulty">
		<xsl:call-template name="Translate">
			<xsl:with-param name="Type" select="'Difficulty'" />
			<xsl:with-param name="Value" select="." />
		</xsl:call-template>
	</xsl:template>
	
	<xsl:template match="LastCourseDifficulty">
		<xsl:call-template name="Translate">
			<xsl:with-param name="Type" select="'CourseDifficulty'" />
			<xsl:with-param name="Value" select="." />
		</xsl:call-template>
	</xsl:template>
	
	<xsl:template name="Translate">
		<xsl:param name="Type" />
		<xsl:param name="Value" />
		<xsl:variable name="node" select="$Catalog/Types/*[name()=$Type]/*[name()=$Type and text()=$Value]" />
		<xsl:if test="count($node) &gt; 0">
			<xsl:value-of select="$node/@DisplayAs" />
		</xsl:if>
		<xsl:if test="count($node) = 0">
			<xsl:value-of select="$Value" />
		</xsl:if>
	</xsl:template>

	<xsl:template match="Style">
		<xsl:call-template name="Translate">
			<xsl:with-param name="Type" select="name(@Style)" />
			<xsl:with-param name="Value" select="@Style" />
		</xsl:call-template>
	</xsl:template>

	<xsl:template match="Dir | @Dir | LastSong | Path | @Path | LastCourse">
		<xsl:variable name="DirOrPath" select="translate(., ' ', ' ')" />
		<xsl:variable name="SongOrCourse" select="$Catalog/*/*[@Dir=$DirOrPath or @Path=$DirOrPath]" />
		<xsl:if test="count($SongOrCourse) &gt; 0">
			<xsl:value-of select="$SongOrCourse/MainTitle" />
			<xsl:text> </xsl:text>
			<xsl:value-of select="$SongOrCourse/SubTitle" />
		</xsl:if>
		<xsl:if test="count($SongOrCourse) = 0">
			<xsl:value-of select="." />
		</xsl:if>
	</xsl:template>

	<xsl:template match="//*[contains(name(),'Modifiers')]">
		<xsl:call-template name="TranslateModifiers">
			<xsl:with-param name="mods" select="." />
		</xsl:call-template>
	</xsl:template>

	<xsl:template name="TranslateModifiers">
		<xsl:param name="mods" />
		<xsl:variable name="before" select="substring-before($mods,', ')" />
		<xsl:variable name="after" select="substring-after($mods,', ')" />
		<xsl:if test="$before = ''">
			<xsl:call-template name="Translate">
				<xsl:with-param name="Type" select="'Modifier'" />
				<xsl:with-param name="Value" select="$mods" />
			</xsl:call-template>
		</xsl:if>
		<xsl:if test="$before != ''">
			<xsl:call-template name="Translate">
				<xsl:with-param name="Type" select="'Modifier'" />
				<xsl:with-param name="Value" select="$before" />
			</xsl:call-template>
			<xsl:text>, </xsl:text>
			<xsl:call-template name="TranslateModifiers">
				<xsl:with-param name="mods" select="$after" />
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

	<xsl:template match="PercentDP | @PercentDP">
		<xsl:apply-templates mode="percentage" select="." />
	</xsl:template>
	
	<xsl:template mode="percentage" match="//*">
		<xsl:call-template name="PrintPercentage">
			<xsl:with-param name="num" select="." />
		</xsl:call-template>
	</xsl:template>

	<xsl:template name="PrintPercentage">
		<xsl:param name="num" />
		<xsl:value-of select="format-number($num*100,'00.00')" />%
	</xsl:template>

	<xsl:template match="//*[text() and not(contains(name(),'Calories')) and not(contains(name(),'Seconds')) and (contains(name(),'Total') or contains(name(),'Num') or name()='Score' or contains(name(),'Combo') or contains(name(..),'TapNoteScores') or name(..)='HoldNoteScores' or (name(..)='RadarValues' and not(contains(text(),'.'))))]">
		<xsl:call-template name="PrintDecimal">
			<xsl:with-param name="num" select="." />
		</xsl:call-template>
	</xsl:template>
	
	<xsl:template name="PrintDecimal">
		<xsl:param name="num" />
		<xsl:value-of select="format-number($num,'#,##0')" />
	</xsl:template>

	<xsl:template match="//*[contains(name(),'Calories')]">
		<xsl:call-template name="PrintCalories">
			<xsl:with-param name="num" select="." />
		</xsl:call-template>
	</xsl:template>
	
	<xsl:template name="PrintCalories">
		<xsl:param name="num" />
		<xsl:value-of select="format-number($num,'#,##0.0')" />
	</xsl:template>

	<xsl:template match="//*[contains(name(),'Seconds')]">
		<xsl:call-template name="PrintSeconds">
			<xsl:with-param name="num" select="." />
		</xsl:call-template>
	</xsl:template>
	
	<xsl:template name="PrintSeconds">
		<xsl:param name="num" />
		
		<xsl:variable name="seconds" select="floor($num mod 60)" />
		<xsl:variable name="minutes" select="floor(($num div 60) mod 60)" />
		<xsl:variable name="hours"  select="floor(($num div 3600))" />
		
		<xsl:value-of select="format-number($hours,'#,##0')" />
		<xsl:text>:</xsl:text>
		<xsl:value-of select="format-number($minutes,'00')" />
		<xsl:text>:</xsl:text>
		<xsl:value-of select="format-number($seconds,'00')" />
	</xsl:template>

	<xsl:template match="//*[name(..)='RadarValues' and contains(text(),'.')]">
		<xsl:call-template name="PrintPercentage">
			<xsl:with-param name="num" select="." />
		</xsl:call-template>
	</xsl:template>
	
	<xsl:template match="//*[contains(name(),'Is') or contains(name(),'Using')]">
		<xsl:if test=".!='0'">
			true
		</xsl:if>
		<xsl:if test=".='0'">
			false
		</xsl:if>
	</xsl:template>

	<xsl:template match="//*[contains(name(),'Guid')]">
		<xsl:if test="$Catalog/InternetRankingViewGuidUrl != ''">
			<xsl:element name="a">
				<xsl:attribute name="href">
					<xsl:value-of select="$Catalog/InternetRankingViewGuidUrl" />Guid=<xsl:value-of select="." />
				</xsl:attribute>
				<xsl:attribute name="target">
					_new
				</xsl:attribute>
				<xsl:value-of select="." />
			</xsl:element>
		</xsl:if>
		<xsl:if test="$Catalog/InternetRankingViewGuidUrl = ''">
			<xsl:value-of select="." />
		</xsl:if>
	</xsl:template>
	
	
	


	<!-- DataTable Elements -->
	
	<xsl:template match="*" mode="DataTableElement" priority="-1">
		<xsl:param name="nowrap" />
		<tr>
			<xsl:element name="td">
				<xsl:attribute name="class">valuename</xsl:attribute>										
				<xsl:if test="$nowrap != ''">
					<xsl:attribute name="nowrap">nowrap</xsl:attribute>
				</xsl:if>
				<xsl:apply-templates select="." mode="name" />
			</xsl:element>
			<td>&#160;</td>
			<xsl:element name="td">
				<xsl:attribute name="class">dyndata</xsl:attribute>										
				<xsl:if test="$nowrap != ''">
					<xsl:attribute name="nowrap">nowrap</xsl:attribute>
				</xsl:if>
				<xsl:apply-templates select="." />
			</xsl:element>
			<td>&#160;&#160;&#160;</td>
		</tr>
	</xsl:template>

	
	<xsl:template name="PrintHorizontalDataTable">
		<xsl:param name="text" />
		<xsl:element name="table" use-attribute-sets="EntityTableAttr">
			<tr>
				<xsl:copy-of select="$text" />
			</tr>
		</xsl:element>
	</xsl:template>


	<xsl:template name="PrintHorizontalDataCell">
		<xsl:param name="name" />
		<xsl:param name="value" />
		<td>
			<xsl:element name="table" use-attribute-sets="DataTableAttr">
				<tr>
					<xsl:element name="td">
						<xsl:attribute name="class">valuename</xsl:attribute>										
						<xsl:copy-of select="$name" />
					</xsl:element>
					<td>&#160;&#160;</td>
					<xsl:element name="td">
						<xsl:attribute name="class">dyndata</xsl:attribute>										
						<xsl:copy-of select="$value" />
					</xsl:element>
				</tr>
			</xsl:element>
		</td>
	</xsl:template>

	<xsl:template name="PrintVerticalDataTable">
		<xsl:param name="text" />
		<xsl:element name="table" use-attribute-sets="EntityTableAttr">
			<tr>
				<td>
					<xsl:element name="table" use-attribute-sets="DataTableAttr">
						<xsl:copy-of select="$text" />
					</xsl:element>
				</td>
			</tr>
		</xsl:element>
	</xsl:template>

	<xsl:template name="PrintVerticalDataRow">
		<xsl:param name="rank" select="''" />
		<xsl:param name="name" />
		<xsl:param name="value" select="''" />
		<tr>
			<xsl:if test="$rank != ''">
				<xsl:element name="td">
					<xsl:attribute name="class">valuename</xsl:attribute>										
					<xsl:copy-of select="$rank" />
				</xsl:element>
				<td>&#160;&#160;</td>
			</xsl:if>
			<xsl:element name="td">
				<xsl:attribute name="class">valuename</xsl:attribute>										
				<xsl:copy-of select="$name" />
			</xsl:element>
			<xsl:if test="$value != ''">
				<td>&#160;&#160;</td>
				<xsl:element name="td">
					<xsl:attribute name="class">dyndata</xsl:attribute>										
					<xsl:copy-of select="$value" />
				</xsl:element>
			</xsl:if>
		</tr>
	</xsl:template>




	<!-- Table Generator -->

	<xsl:template name="TableGenerator">
		<xsl:param name="cols" select="2" />
		<xsl:param name="nodeset" />
		<xsl:param name="nodecounter" select="1" />
		<xsl:param name="trid" />
		<tr name="{$trid}">
			<xsl:apply-templates select="$nodeset[position() >= $nodecounter and position() &lt; $nodecounter + $cols]" mode="TableGeneratorColumn">
				<xsl:with-param name="rownode" select="$nodecounter" />
				<xsl:with-param name="total" select="count($nodeset)" />
			</xsl:apply-templates>
			<xsl:if test="$nodecounter > 1 and ( $nodecounter + $cols > count($nodeset) ) and count($nodeset) mod $cols != 0">
				<xsl:apply-templates select="$nodeset[1]" mode="TableGeneratorColumn">
					<xsl:with-param name="emptycounter" select="$cols - ( count($nodeset) mod $cols )" />
				</xsl:apply-templates>
			</xsl:if>
		</tr>
		<xsl:if test="$nodecounter + $cols &lt;= count($nodeset)">
			<xsl:call-template name="TableGenerator">
				<xsl:with-param name="cols" select="$cols" />
				<xsl:with-param name="nodecounter" select="$nodecounter + $cols" />
				<xsl:with-param name="nodeset" select="$nodeset" />
				<xsl:with-param name="trid" select="$trid" />
			</xsl:call-template>
		</xsl:if>
	</xsl:template>

	<xsl:template match="*" mode="TableGeneratorColumn">
		<xsl:param name="emptycounter" select="0" />
		<xsl:param name="rownode" select="1" />
		<xsl:param name="total" select="1" />
		<xsl:variable name="current" select="$rownode + position() - 1" />
		<td>
			<xsl:choose>
				<xsl:when test="$emptycounter > 0">
					&#160;
				</xsl:when>
				<xsl:otherwise>
					<xsl:apply-templates select=".">
						<xsl:with-param name="current" select="$current" />
						<xsl:with-param name="total" select="$total" />
					</xsl:apply-templates>
				</xsl:otherwise>
			</xsl:choose>
		</td>
		<xsl:if test="$emptycounter > 1">
			<xsl:apply-templates select="." mode="TableGeneratorColumn">
				<xsl:with-param name="emptycounter" select="$emptycounter - 1" />
			</xsl:apply-templates>
		</xsl:if>
	</xsl:template>
	
	
	
	
	
	


	<!-- Title Generator -->

	<xsl:template mode="AttributeTitleGenerator" match="*">
		<xsl:for-each select="@*">
			<xsl:apply-templates select="." />
			<xsl:text> </xsl:text>
		</xsl:for-each>
	</xsl:template>



	<!-- DataTable Generator -->

	<xsl:template name="DataTableGenerator">
		<xsl:param name="cols" select="2" />
		<xsl:param name="nodeset" />
		<xsl:param name="nodecounter" select="1" />
		<xsl:variable name="skip" select="ceiling( count($nodeset) div $cols )" />
		<td valign="top">
			<xsl:element name="table" use-attribute-sets="DataTableAttr">
				<xsl:apply-templates select="$nodeset[(position() >= $nodecounter) and (position() &lt; ($nodecounter + $skip))]" mode="DataTableElement" />
			</xsl:element>
		</td>
		<xsl:if test="($nodecounter + $skip) &lt;= count($nodeset)">
			<xsl:call-template name="DataTableGenerator">
				<xsl:with-param name="cols" select="$cols" />
				<xsl:with-param name="nodecounter" select="$nodecounter + $skip" />
				<xsl:with-param name="nodeset" select="$nodeset" />
				<xsl:with-param name="skip" select="$skip" />
			</xsl:call-template>
		</xsl:if>
	</xsl:template>
	
	
	
	<!-- CollapsibleTextTable -->
		
	<xsl:template name="CollapsibleTextTable">
		<xsl:param name="title" />
		<xsl:param name="text" />
		<xsl:call-template name="CollapsibleSubSection">
			<xsl:with-param name="title" select="$title" />
			<xsl:with-param name="text">
				<xsl:element name="table" use-attribute-sets="CollapsableTableAttr">
					<tr>
						<td>
							<xsl:copy-of select="$text" />
						</td>
					</tr>
				</xsl:element>
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>
	
	
	<xsl:template name="CollapsibleSubSection">
		<xsl:param name="title" />
		<xsl:param name="text" />
		<xsl:element name="table" use-attribute-sets="CollapsableTableAttr">
			<tr>
				<xsl:element name="th" use-attribute-sets="ToggleLinkAttr">
					<xsl:attribute name="class">main</xsl:attribute>
					<xsl:call-template name="ToggleDisplayImage" />
					<xsl:text> </xsl:text><xsl:value-of select="$title" />
				</xsl:element>
			</tr>
			<tr name="hide">
				<td style="background: #F4F6F7;">
					<xsl:copy-of select="$text" />
				</td>
			</tr>
		</xsl:element>
	</xsl:template>
	
	<xsl:template name="SubSectionCompact">
		<xsl:param name="title" />
		<xsl:param name="text" />
		<xsl:element name="table" use-attribute-sets="CompactTableAttr">
			<tr>
				<xsl:element name="th">
					<xsl:attribute name="class">main</xsl:attribute>
					<xsl:value-of select="$title" />
				</xsl:element>
			</tr>
			<tr name="hide">
				<td style="background: #F4F6F7;">
					<xsl:copy-of select="$text" />
				</td>
			</tr>
		</xsl:element>
	</xsl:template>
	
	<xsl:template name="CollapsibleTopSection">
		<xsl:param name="title" />
		<xsl:param name="text" />
		<xsl:element name="table" use-attribute-sets="TopLevelTableAttr">
			<tr>
				<xsl:element name="th" use-attribute-sets="ToggleLinkAttr">
					<xsl:attribute name="class">main</xsl:attribute>
					<xsl:call-template name="ToggleDisplayImage" />
					<xsl:text> </xsl:text><xsl:value-of select="$title" />
				</xsl:element>
			</tr>
			<tr name="hide">
				<td style="background: #F4F6F7;">
					<xsl:copy-of select="$text" />
				</td>
			</tr>
		</xsl:element>
	</xsl:template>


	
	<!-- Link for hiding & displaying data -->
		
	<xsl:template name="ToggleDisplayImage">
		<img name="toggleImage" align="absmiddle" class="hidden" /><span>▼</span><span>►</span>
	</xsl:template>

	

	<!-- That's it -->

</xsl:stylesheet>

