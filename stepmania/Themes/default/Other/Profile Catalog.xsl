<?xml version="1.0" encoding="UTF-8" ?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:import href="Common.xsl"/>


	<!-- Main Template -->

	<xsl:template match="/Catalog">
		<xsl:call-template name="MainTemplate">
			<xsl:with-param name="FullHeader" select="0" />
			<xsl:with-param name="DocName" select="name()" />
			<xsl:with-param name="Content">
				<xsl:apply-templates select="/Catalog/Songs" />
				<xsl:apply-templates select="/Catalog/Courses" />
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>
	
	
	
	<!-- TopScores for SongScores and CourseScores -->
	
	<xsl:template match="/Catalog/Songs | /Catalog/Courses">
		<xsl:variable name="Type" select="name()" />
		<xsl:variable name="SubType">
			<xsl:if test="$Type='Song'">Steps</xsl:if>
			<xsl:if test="$Type='Course'">Trail</xsl:if>
		</xsl:variable>
		<xsl:call-template name="CollapsibleTopSection">
			<xsl:with-param name="title">
				<xsl:value-of select="$Type" />
			</xsl:with-param>
			<xsl:with-param name="text">
				<xsl:apply-templates select="Song | Course" />
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>		

	<xsl:template match="Song | Course">
		<xsl:variable name="Dir" select="@Dir" />
		<xsl:variable name="Path" select="@Path" />
		<xsl:variable name="MainTitle" select="/Catalog/*/*[@Dir=$Dir or @Path=$Path]/MainTitle" />
		<xsl:variable name="SubTitle" select="/Catalog/*/*[@Dir=$Dir or @Path=$Path]/SubTitle" />
		<xsl:call-template name="CollapsibleSubSection">
			<xsl:with-param name="title">
				<xsl:apply-templates select="@Dir | @Path" />
			</xsl:with-param>
			<xsl:with-param name="text">
				<xsl:apply-templates select="Steps | Trail" />
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>		
	
	<xsl:template match="Steps | Trail">
		<xsl:call-template name="CollapsibleSubSection">
			<xsl:with-param name="title">
				<xsl:apply-templates select="." mode="AttributeTitleGenerator" />
			</xsl:with-param>
			<xsl:with-param name="text">
				
				<xsl:element name="table" use-attribute-sets="EntityTableAttr">
					<xsl:call-template name="DataTableGenerator">
						<xsl:with-param name="cols" select="1" />
						<xsl:with-param name="nodeset" select="*[text()]" />
					</xsl:call-template>
				</xsl:element>

				<hr />
				<xsl:for-each select="./*[count(*) &gt; 0]">
					<xsl:element name="table" use-attribute-sets="EntityTableAttr">
						<xsl:call-template name="DataTableGenerator">
							<xsl:with-param name="cols" select="2" />
							<xsl:with-param name="nodeset" select="*[text()]" />
						</xsl:call-template>
					</xsl:element>
				</xsl:for-each>
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>		

	
	<xsl:template match="HighScoreList">
		<xsl:element name="table" use-attribute-sets="EntityTableAttr">
			<xsl:call-template name="DataTableGenerator">
				<xsl:with-param name="cols" select="2" />
				<xsl:with-param name="nodeset" select="*[text()]" />
			</xsl:call-template>
		</xsl:element>
		<xsl:apply-templates select="HighScore" />
	</xsl:template>		

	<xsl:template match="HighScore">
		<xsl:call-template name="CollapsibleSubSection">
			<xsl:with-param name="title">
				<xsl:apply-templates select="PercentDP"/><xsl:text> </xsl:text><xsl:apply-templates select="Grade"/><xsl:text> </xsl:text><xsl:value-of select="Name"/>
			</xsl:with-param>
			<xsl:with-param name="text">
				<xsl:element name="table" use-attribute-sets="EntityTableAttr">
					<xsl:call-template name="DataTableGenerator">
						<xsl:with-param name="cols" select="2" />
						<xsl:with-param name="nodeset" select="*[text()]" />
					</xsl:call-template>
				</xsl:element>

				<xsl:call-template name="CollapsibleSubSection">
					<xsl:with-param name="title">
						Details
					</xsl:with-param>
					<xsl:with-param name="text">

						<xsl:for-each select="./*[count(*) &gt; 0]">
							<h2>
								<xsl:if test="name()='TapNoteScores'">Tap Scores</xsl:if>
								<xsl:if test="name()='HoldNoteScores'">Hold Scores</xsl:if>
								<xsl:if test="name()='RadarValues'">Stats</xsl:if>
							</h2>
							<xsl:element name="table" use-attribute-sets="EntityTableAttr">
								<xsl:call-template name="DataTableGenerator">
									<xsl:with-param name="cols" select="2" />
									<xsl:with-param name="nodeset" select="*[text()]" />
								</xsl:call-template>
							</xsl:element>
							<xsl:if test="position()!=last()">
								<hr />
							</xsl:if>
						</xsl:for-each>

					</xsl:with-param>
				</xsl:call-template>		
			</xsl:with-param>
		</xsl:call-template>		
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
		<xsl:variable name="node" select="/Catalog/Types/*[name()=$Type and text()=$Value]" />
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
		<xsl:variable name="SongOrCourse" select="/Catalog/*/*[@Dir=$DirOrPath or @Path=$DirOrPath]" />
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
			<xsl:with-param name="cals" select="." />
		</xsl:call-template>
	</xsl:template>

	<xsl:template name="PrintPercentage">
		<xsl:param name="cals" />
		<xsl:value-of select="format-number($cals*100,'00.00')" />%
	</xsl:template>

	<xsl:template match="//*[(contains(name(),'Total') or contains(name(),'Num') or name()='Score' or contains(name(),'Combo') or contains(name(..),'TapNoteScores') or name(..)='HoldNoteScores' or (name(..)='RadarValues' and not(contains(text(),'.')))) and text()]">
		<xsl:call-template name="PrintDecimal">
			<xsl:with-param name="cals" select="." />
		</xsl:call-template>
	</xsl:template>
	
	<xsl:template name="PrintDecimal">
		<xsl:param name="cals" />
		<xsl:value-of select="format-number($cals,'#,##0')" />
	</xsl:template>

	<xsl:template match="//*[contains(name(),'Calories') or contains(name(),'Seconds')]">
		<xsl:call-template name="PrintCalories">
			<xsl:with-param name="cals" select="." />
		</xsl:call-template>
	</xsl:template>
	
	<xsl:template match="//*[name(..)='RadarValues' and contains(text(),'.')]">
		<xsl:call-template name="PrintPercentage">
			<xsl:with-param name="cals" select="." />
		</xsl:call-template>
	</xsl:template>
	
	<xsl:template name="PrintCalories">
		<xsl:param name="cals" />
		<xsl:value-of select="format-number($cals,'#,##0.0')" />
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
		<xsl:if test="/Catalog/InternetRankingViewGuidUrl != ''">
			<xsl:element name="a">
				<xsl:attribute name="href">
					<xsl:value-of select="/Catalog/InternetRankingViewGuidUrl" />?Guid=<xsl:value-of select="." />
				</xsl:attribute>
				<xsl:attribute name="target">
					_new
				</xsl:attribute>
				<xsl:value-of select="." />
			</xsl:element>
		</xsl:if>
		<xsl:if test="/Catalog/InternetRankingViewGuidUrl = ''">
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
				<xsl:element name="table" use-attribute-sets="EntityTableAttr">
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
		<xsl:element name="table" use-attribute-sets="EntityTableAttr">
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

