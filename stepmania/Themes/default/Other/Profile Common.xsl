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
		<xsl:attribute name="cellpadding">2</xsl:attribute>
		<xsl:attribute name="cellspacing">0</xsl:attribute>
		<xsl:attribute name="border">0</xsl:attribute>
	</xsl:attribute-set>

	<xsl:attribute-set name="EntityTableAttr">
		<xsl:attribute name="cellpadding">3</xsl:attribute>
		<xsl:attribute name="cellspacing">1</xsl:attribute>
		<xsl:attribute name="border">0</xsl:attribute>
	</xsl:attribute-set>

	<xsl:attribute-set name="CompactTableAttr">
		<xsl:attribute name="cellpadding">0</xsl:attribute>
		<xsl:attribute name="cellspacing">0</xsl:attribute>
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



</xsl:stylesheet>

