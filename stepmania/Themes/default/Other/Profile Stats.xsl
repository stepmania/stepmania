<?xml version="1.0" encoding="UTF-8" ?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:import href="Common.xsl"/>
	

	<!-- Global Variables -->
	<xsl:variable name="Stats" select="/Stats" />




	<!-- Main Template -->

	<xsl:template match="/Stats">
		<xsl:call-template name="MainTemplate">
			<xsl:with-param name="FullHeader" select="1" />
			<xsl:with-param name="DocName" select="name()" />
			<xsl:with-param name="Content">
				<xsl:call-template name="Instructions" />
				<xsl:apply-templates select="/Stats/GeneralData" />
				<xsl:apply-templates mode="Popularity" select="/Stats/SongScores" />
				<xsl:apply-templates mode="Popularity" select="/Stats/CourseScores" />
				<xsl:apply-templates mode="TopScores" select="/Stats/SongScores" />
				<xsl:apply-templates mode="TopScores" select="/Stats/CourseScores"/>
				<xsl:apply-templates mode="Completeness" select="/Stats/SongScores"/>
				<xsl:apply-templates mode="Completeness" select="/Stats/CourseScores"/>
				<xsl:apply-templates select="/Stats/ScreenshotData" />
				<xsl:apply-templates select="/Stats/CalorieData" />
				<xsl:apply-templates select="/Stats/RecentSongScores" />
				<xsl:apply-templates select="/Stats/RecentCourseScores" />
				<xsl:apply-templates select="/Stats/CoinData" />
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>

	
	
	
	<!-- Instructions -->
	
	<xsl:template name="Instructions">
		<xsl:call-template name="CollapsibleTopSection">
			<xsl:with-param name="title">
				Instructions
			</xsl:with-param>
			<xsl:with-param name="text">

				<h2>Overview</h2>
				<table class="EntityTableAttr">
					<tr>
						<td>
							This section explains all the files saved to your memory card. Please read the instructions below before modifying any files on your memory card. Modifying files may result in irreversible loss of your data.
						</td>
					</tr>
				</table>
				
				<hr />
				
				<h2>Description of Files</h2>
				<table class="EntityTableAttr">
					<tr>
						<td><a href="Edits/" target="_new">Edits/</a></td>
						<td>
							This directory contains edit step files that you've created yourself or downloaded from the internet.
							<xsl:call-template name="CollapsibleTextTable">
								<xsl:with-param name="title">More Info</xsl:with-param>
								<xsl:with-param name="text">
									You can place up to 200 .edit files in this directory.  The edit file format is similar to an .sm file, except that it has only two tags: 
									<br />
									#SONG:&lt;SongDirectory&gt;;
									<br />
									#NOTES:&lt;StepsType&gt;:&lt;Description&gt;:&lt;Difficulty&gt;:&lt;Meter&gt;:&lt;RadarValues&gt;:&lt;NoteData&gt;;
								</xsl:with-param>
							</xsl:call-template>
						</td>
					</tr>
					<tr>
						<td><a href="LastGood/" target="_new">LastGood/</a></td>
						<td>
							This directory contains a backup of your last Stats.xml and signatures that were successfully loaded.
							<xsl:call-template name="CollapsibleTextTable">
								<xsl:with-param name="title">More Info</xsl:with-param>
								<xsl:with-param name="text">
									The stats on your memory card may fail to load because: 
									<ul>
										<li>The Stats.xml file or its digital signatures have become corrupt.</li>
										<li>The Stats.xml file has been modified outside of the game. This will cause the digital signature check to fail.</li>
									</ul>
									If your saved statistics fail to load, your can restore from from the "last good" data. Copy all 3 files in the files <i>LastGood</i> directory into the main profile directory (one level up from <i>LastGood</i>).
								</xsl:with-param>
							</xsl:call-template>
						</td>
					</tr>
					<tr>
						<td><a href="Screenshots/" target="_new">Screenshots/</a></td>
						<td>
							All in-game screenshots that you take are saved in this directory.
							<xsl:call-template name="CollapsibleTextTable">
								<xsl:with-param name="title">More Info</xsl:with-param>
								<xsl:with-param name="text">
									This directory contains all screenshots that you've captured while playing the game. The Screenshots section of Stats.xml shows thumbnails score details for all screens you've captured. 
									<br />
									The MD5 hash for a screenshot file can be used to verify that the screenshot has not been modified since it was first saved.
									<br />
									If your memory card is full, you can free space by deleting some of the screenshot .jpg files or moving them to another disk.
								</xsl:with-param>
							</xsl:call-template>
						</td>
					</tr>
					<tr>
						<td>Catalog.xml</td>
						<td>Contains game data used by Stats.xsl for displaying a report of your stats.</td>
					</tr>
					<tr>
						<td>DontShare.sig</td>
						<td>
							DontShare.sig is a digital signature that's required by the game when it loads your memory card data. This is a secret file that you shouldn't share with anyone else.
							<xsl:call-template name="CollapsibleTextTable">
								<xsl:with-param name="title">More Info</xsl:with-param>
								<xsl:with-param name="text">
									You can freely share Stats.xml and Stats.xml.sig with other players or submit these files for internet ranking. However, you should always keep DontShare.sig private. 
									Without the DontShare.sig, another player will not be able to load your saved data and pass it off as their own.
								</xsl:with-param>
							</xsl:call-template>
						</td>
					</tr>
					<tr>
						<td><a href="Editable.ini" target="_new">Editable.ini</a></td>
						<td>
							Holds preferences that you can edit using your home computer. You can open this file using any text editor program and save changes.
						</td>
					</tr>
					<tr>
						<td>Stats.xml</td>
						<td>You're looking at this file now. It contains all of your saved scores, statistics, and preferences. The game reads this data when you insert your memory card.</td>
					</tr>
					<tr>
						<td>Stats.xml.sig</td>
						<td>
							This is the digital signature for Stats.xml.
							<xsl:call-template name="CollapsibleTextTable">
								<xsl:with-param name="title">More Info</xsl:with-param>
								<xsl:with-param name="text">
									Digital signatures are used to verify that your data hasn't been modified outside of the game. This prevents cheaters from changing their score data and passing it off as real.
									<br />
									If any of Stats.xml, Stats.xml.sig, or DontShare.sig have been modified outside of the game, your memory card data will be ignored and overridden after the next save.  It's important that you don't modify any of these three files because doing so will render your data permanently unusable.
								</xsl:with-param>
							</xsl:call-template>
						</td>
					</tr>
				</table>
				
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>




	<!-- GeneralData-->
		
		
	<xsl:template match="/Stats/GeneralData">
		<xsl:call-template name="CollapsibleTopSection">
			<xsl:with-param name="title">
				General Data
			</xsl:with-param>
			<xsl:with-param name="text">

				<table class="EntityTableAttr">
					<xsl:call-template name="DataTableGenerator">
						<xsl:with-param name="cols" select="2" />
						<xsl:with-param name="nodeset" select="*[text() and name() != 'NumExtraStagesPassed' and name() != 'NumExtraStagesFailed' and name() != 'NumToasties'] | Song | Course" />
					</xsl:call-template>
				</table>
				
				<hr/>
				
				<xsl:call-template name="CollapsibleSubSection">
					<xsl:with-param name="title">Song Count by PlayMode</xsl:with-param>
					<xsl:with-param name="text">
						<xsl:call-template name="PrintVerticalDataTable">
							<xsl:with-param name="text">
								<xsl:variable name="NumSongsPlayedByPlayMode" select="NumSongsPlayedByPlayMode" />
								<xsl:for-each select="$Catalog/Types/PlayMode/PlayMode">
									<xsl:call-template name="PrintVerticalDataRow">
										<xsl:with-param name="name">
											<xsl:apply-templates select="." />
										</xsl:with-param>
										<xsl:with-param name="value">
											<xsl:variable name="blah" select="." />
											<xsl:value-of select="sum($NumSongsPlayedByPlayMode/*[name()=$blah])" />
										</xsl:with-param>
									</xsl:call-template>
								</xsl:for-each>
							</xsl:with-param>
						</xsl:call-template>
					</xsl:with-param>
				</xsl:call-template>
				
				<xsl:call-template name="CollapsibleSubSection">
					<xsl:with-param name="title">Song Count by Style</xsl:with-param>
					<xsl:with-param name="text">
						<xsl:call-template name="PrintVerticalDataTable">
							<xsl:with-param name="text">
								<xsl:variable name="NumSongsPlayedByStyle" select="NumSongsPlayedByStyle" />
								<xsl:for-each select="$Catalog/Types/Style/Style">
									<xsl:call-template name="PrintVerticalDataRow">
										<xsl:with-param name="name">
											<xsl:apply-templates select="." />
										</xsl:with-param>
										<xsl:with-param name="value">
											<xsl:variable name="blah" select="." />
											<xsl:value-of select="sum($NumSongsPlayedByStyle/*[@Game = $blah/@Game and @Style = $blah/@Style])" />
										</xsl:with-param>
									</xsl:call-template>
								</xsl:for-each>
							</xsl:with-param>
						</xsl:call-template>
					</xsl:with-param>
				</xsl:call-template>
				
				<xsl:call-template name="CollapsibleSubSection">
					<xsl:with-param name="title">Song Count by Difficulty</xsl:with-param>
					<xsl:with-param name="text">
						<xsl:call-template name="PrintVerticalDataTable">
							<xsl:with-param name="text">
								<xsl:variable name="NumSongsPlayedByDifficulty" select="NumSongsPlayedByDifficulty" />
								<xsl:for-each select="$Catalog/Types/Difficulty/Difficulty">
									<xsl:call-template name="PrintVerticalDataRow">
										<xsl:with-param name="name">
											<xsl:apply-templates select="." />
										</xsl:with-param>
										<xsl:with-param name="value">
											<xsl:variable name="blah" select="." />
											<xsl:value-of select="sum($NumSongsPlayedByDifficulty/*[name()=$blah])" />
										</xsl:with-param>
									</xsl:call-template>
								</xsl:for-each>
							</xsl:with-param>
						</xsl:call-template>
					</xsl:with-param>
				</xsl:call-template>
				
				<xsl:call-template name="CollapsibleSubSection">
					<xsl:with-param name="title">Song Count by Meter</xsl:with-param>
					<xsl:with-param name="text">
						<xsl:call-template name="PrintVerticalDataTable">
							<xsl:with-param name="text">
								<xsl:variable name="NumSongsPlayedByMeter" select="NumSongsPlayedByMeter" />
								<xsl:for-each select="$Catalog/Types/Meter/Meter">
									<xsl:call-template name="PrintVerticalDataRow">
										<xsl:with-param name="name">
											<xsl:apply-templates select="." />
										</xsl:with-param>
										<xsl:with-param name="value">
											<xsl:variable name="blah" select="." />
											<xsl:value-of select="sum($NumSongsPlayedByMeter/*[name()=$blah])" />
										</xsl:with-param>
									</xsl:call-template>
								</xsl:for-each>
							</xsl:with-param>
						</xsl:call-template>
					</xsl:with-param>
				</xsl:call-template>
				
				<xsl:call-template name="CollapsibleSubSection">
					<xsl:with-param name="title">Stages Passed by Grade</xsl:with-param>
					<xsl:with-param name="text">
						<xsl:call-template name="PrintVerticalDataTable">
							<xsl:with-param name="text">
								<xsl:variable name="NumStagesPassedByGrade" select="NumStagesPassedByGrade" />
								<xsl:for-each select="$Catalog/Types/Grade/Grade">
									<xsl:call-template name="PrintVerticalDataRow">
										<xsl:with-param name="name">
											<xsl:apply-templates select="." />
										</xsl:with-param>
										<xsl:with-param name="value">
											<xsl:variable name="blah" select="." />
											<xsl:value-of select="sum($NumStagesPassedByGrade/*[name()=$blah])" />
										</xsl:with-param>
									</xsl:call-template>
								</xsl:for-each>
							</xsl:with-param>
						</xsl:call-template>
					</xsl:with-param>
				</xsl:call-template>
				
				<xsl:call-template name="CollapsibleSubSection">
					<xsl:with-param name="title">Stages Passed by PlayMode</xsl:with-param>
					<xsl:with-param name="text">
						<xsl:call-template name="PrintVerticalDataTable">
							<xsl:with-param name="text">
								<xsl:variable name="NumStagesPassedByPlayMode" select="NumStagesPassedByPlayMode" />
								<xsl:for-each select="$Catalog/Types/PlayMode/PlayMode">
									<xsl:call-template name="PrintVerticalDataRow">
										<xsl:with-param name="name">
											<xsl:apply-templates select="." />
										</xsl:with-param>
										<xsl:with-param name="value">
											<xsl:variable name="blah" select="." />
											<xsl:value-of select="sum($NumStagesPassedByPlayMode/*[name()=$blah])" />
										</xsl:with-param>
									</xsl:call-template>
								</xsl:for-each>
							</xsl:with-param>
						</xsl:call-template>
					</xsl:with-param>
				</xsl:call-template>

			</xsl:with-param>
		</xsl:call-template>
					
	</xsl:template>		
		

	
	

	<!-- Popularity -->
	
	<xsl:template mode="Popularity" match="/Stats/SongScores | /Stats/CourseScores">
		<xsl:variable name="Type" select="substring-before(name(),'Scores')" />
		<xsl:variable name="TypePlural">
			<xsl:if test="$Type='Song'">Songs</xsl:if>
			<xsl:if test="$Type='Course'">Courses</xsl:if>
		</xsl:variable>
		<xsl:variable name="SubType">
			<xsl:if test="$Type='Song'">Steps</xsl:if>
			<xsl:if test="$Type='Course'">Trail</xsl:if>
		</xsl:variable>


		<xsl:call-template name="CollapsibleTopSection">
			<xsl:with-param name="title">
				<xsl:value-of select="$Type" /><xsl:text> Popularity</xsl:text>
			</xsl:with-param>
			<xsl:with-param name="text">

				<xsl:call-template name="CollapsibleSubSection">
					<xsl:with-param name="title">Ranking</xsl:with-param>
					<xsl:with-param name="text">
						<xsl:call-template name="PrintVerticalDataTable">
							<xsl:with-param name="text">
						
								<xsl:variable name="TotalPlays" select="sum(*/*/HighScoreList/NumTimesPlayed)" />
								<xsl:for-each select="Song | Course">
									<xsl:sort select="sum(*/HighScoreList/NumTimesPlayed)" data-type="number" order="descending"/>
								
									<xsl:call-template name="PrintVerticalDataRow">
										<xsl:with-param name="rank">
											<xsl:value-of select="position()"/>
										</xsl:with-param>
										<xsl:with-param name="name">
											<xsl:apply-templates select="@Dir | @Path" />
										</xsl:with-param>
										<xsl:with-param name="value">
											<xsl:call-template name="PrintPercentage" >
												<xsl:with-param name="num" select="sum(*/HighScoreList/NumTimesPlayed) div $TotalPlays" />
											</xsl:call-template>
										</xsl:with-param>
									</xsl:call-template>

								</xsl:for-each>
							</xsl:with-param>
						</xsl:call-template>
					</xsl:with-param>
				</xsl:call-template>
			
				<xsl:call-template name="CollapsibleSubSection">
					<xsl:with-param name="title">Unplayed</xsl:with-param>
					<xsl:with-param name="text">
						<xsl:call-template name="PrintVerticalDataTable">
							<xsl:with-param name="text">
								<xsl:for-each select="$Catalog/*[name(.)=$TypePlural]/*[name(.)=$Type]">
									<xsl:variable name="Dir" select="@Dir" />
									<xsl:variable name="Path" select="@Path" />
									<xsl:variable name="NumPlays" select="sum($Stats/*/*[@Dir=$Dir or @Path=$Path]/*/HighScoreList/NumTimesPlayed)" />
									<xsl:if test="$NumPlays = 0">
										<xsl:call-template name="PrintVerticalDataRow">
											<xsl:with-param name="name">
												<xsl:apply-templates select="." mode="AttributeTitleGenerator" />
											</xsl:with-param>
										</xsl:call-template>
									</xsl:if>
								</xsl:for-each>
								
							</xsl:with-param>
						</xsl:call-template>
					</xsl:with-param>
				</xsl:call-template>
			
				<xsl:call-template name="CollapsibleSubSection">
					<xsl:with-param name="title"><xsl:value-of select="$SubType" /> Ranking</xsl:with-param>
					<xsl:with-param name="text">

						<xsl:variable name="SubTypes" select="*/*" />
					
						<xsl:for-each select="$Catalog/Types/StepsType/StepsType">
							<xsl:variable name="StepsType" select="." />
							<xsl:variable name="TotalPlays" select="sum($SubTypes[@StepsType=$StepsType]/HighScoreList/NumTimesPlayed)" />
					
							<xsl:call-template name="CollapsibleSubSection">
								<xsl:with-param name="title">
									<xsl:apply-templates select="$StepsType" />
								</xsl:with-param>
								<xsl:with-param name="text">
									<table class="DataTableAttr">
										<xsl:for-each select="$SubTypes[@StepsType=$StepsType]">
										<xsl:sort select="sum(HighScoreList/NumTimesPlayed)" data-type="number" order="descending"/>
											<xsl:variable name="Dir" select="../@Dir" />
											<xsl:variable name="Path" select="../@Path" />
											<xsl:text> </xsl:text>
											<tr>
												<td>
													<xsl:value-of select="position()" />
												</td>
												<td>
													<xsl:apply-templates select=".." />
												</td>
												<td>
													<xsl:apply-templates select="." mode="AttributeTitleGenerator" />
												</td>
												<td>
													<span class="dyndata">
														<xsl:call-template name="PrintPercentage" >
															<xsl:with-param name="num" select="sum(HighScoreList/NumTimesPlayed) div $TotalPlays" />
														</xsl:call-template>
													</span>	
												</td>
											</tr>
										</xsl:for-each>
									</table>
								</xsl:with-param>
							</xsl:call-template>
							
						</xsl:for-each>
						
					</xsl:with-param>
				</xsl:call-template>
				
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>
	
	

	
	<!-- TopScores for SongScores and CourseScores -->
	
	<xsl:template mode="TopScores" match="/Stats/SongScores | /Stats/CourseScores">
		<xsl:variable name="Type" select="substring-before(name(),'Scores')" />
		<xsl:variable name="SubType">
			<xsl:if test="$Type='Song'">Steps</xsl:if>
			<xsl:if test="$Type='Course'">Trail</xsl:if>
		</xsl:variable>
		<xsl:call-template name="CollapsibleTopSection">
			<xsl:with-param name="title">
				<xsl:value-of select="$Type" /> Top Scores
			</xsl:with-param>
			<xsl:with-param name="text">
				<xsl:apply-templates mode="TopScores" select="Song | Course" />
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>		

	<xsl:template match="Song | Course">
		<xsl:variable name="Dir" select="@Dir" />
		<xsl:variable name="Path" select="@Path" />
		<xsl:variable name="MainTitle" select="$Catalog/*/*[@Dir=$Dir or @Path=$Path]/MainTitle" />
		<xsl:variable name="SubTitle" select="$Catalog/*/*[@Dir=$Dir or @Path=$Path]/SubTitle" />
		<xsl:value-of select="$MainTitle | $SubTitle" />
	</xsl:template>		
	
	<xsl:template mode="TopScores" match="Song | Course">
		<xsl:variable name="Dir" select="@Dir" />
		<xsl:variable name="Path" select="@Path" />
		<xsl:variable name="MainTitle" select="$Catalog/*/*[@Dir=$Dir or @Path=$Path]/MainTitle" />
		<xsl:variable name="SubTitle" select="$Catalog/*/*[@Dir=$Dir or @Path=$Path]/SubTitle" />
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
				<xsl:apply-templates select="HighScoreList" />
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>		

	
	<xsl:template match="HighScoreList">
		<table class="EntityTableAttr">
			<xsl:call-template name="DataTableGenerator">
				<xsl:with-param name="cols" select="2" />
				<xsl:with-param name="nodeset" select="*[text()]" />
			</xsl:call-template>
		</table>
		<xsl:apply-templates select="HighScore" />
	</xsl:template>		

	<xsl:template match="HighScore">
		<xsl:call-template name="SubSectionCompact">
			<xsl:with-param name="title">
				<xsl:apply-templates select="PercentDP"/><xsl:text> </xsl:text><xsl:apply-templates select="Grade"/><xsl:text> </xsl:text><xsl:value-of select="Name"/>
			</xsl:with-param>
			<xsl:with-param name="text">
				<table class="EntityTableAttr">
					<xsl:call-template name="DataTableGenerator">
						<xsl:with-param name="cols" select="2" />
						<xsl:with-param name="nodeset" select="*[text()]" />
					</xsl:call-template>
				</table>
			</xsl:with-param>
		</xsl:call-template>		
	</xsl:template>		

	
	

	<!-- Completeness -->
	
	<xsl:template mode="Completeness" match="/Stats/SongScores | /Stats/CourseScores">
		<xsl:variable name="ScoresName" select="translate(name(), ' ', ' ')" />
		<xsl:variable name="Type" select="substring-before(name(),'Scores')" />
		<xsl:variable name="TypePlural">
			<xsl:if test="$Type='Song'">Songs</xsl:if>
			<xsl:if test="$Type='Course'">Courses</xsl:if>
		</xsl:variable>
		<xsl:variable name="SubType">
			<xsl:if test="$Type='Song'">Steps</xsl:if>
			<xsl:if test="$Type='Course'">Trail</xsl:if>
		</xsl:variable>
		<xsl:variable name="DifficultyName">
			<xsl:if test="$Type = 'Song'">Difficulty</xsl:if>
			<xsl:if test="$Type = 'Course'">CourseDifficulty</xsl:if>
		</xsl:variable>
		<xsl:call-template name="CollapsibleTopSection">
			<xsl:with-param name="title">
				<xsl:value-of select="$Type" /> Completeness
			</xsl:with-param>
			<xsl:with-param name="text">

				<xsl:for-each select="$Catalog/Types/StepsType/StepsType">
					<xsl:variable name="StepsType" select="." />

					<xsl:call-template name="CollapsibleSubSection">
						<xsl:with-param name="title">
							<xsl:apply-templates select="$StepsType" />
						</xsl:with-param>
						<xsl:with-param name="text">


							<xsl:variable name="ActualO" select="sum($Stats/*[name()=$ScoresName]/*/*[@StepsType=$StepsType]/HighScoreList/HighScore[1]/PercentDP)" />
							<xsl:variable name="PossibleO" select="count($Catalog/*/*[name()=$Type]/*[@StepsType=$StepsType])" />
							

							<table class="EntityTableAttr">
								<tr>
									<td>
										<font size="+2">
											<xsl:if test="$PossibleO = 0">
												<xsl:call-template name="PrintPercentage">
													<xsl:with-param name="num" select="0" />
												</xsl:call-template>
											</xsl:if>
											<xsl:if test="$PossibleO != 0">
												<xsl:call-template name="PrintPercentage">
													<xsl:with-param name="num" select="$ActualO div $PossibleO" />
												</xsl:call-template>
											</xsl:if>
										</font>
									</td>
								</tr>
							</table>
							
							
							<hr />


							<table>

								<tr>
									<td> </td>
									<xsl:for-each select="$Catalog/Types/*[name()=$DifficultyName]/*[name()=$DifficultyName]">
										<xsl:variable name="Difficulty" select="." />
										<td>
											<xsl:apply-templates select="$Difficulty"/>
										</td>
									</xsl:for-each>
								</tr>
								
								<tr>
									<td>Actual</td>
									<xsl:for-each select="$Catalog/Types/*[name()=$DifficultyName]/*[name()=$DifficultyName]">
										<xsl:variable name="Difficulty" select="." />
										<xsl:variable name="Actual" select="sum($Stats/*[name()=$ScoresName]/*/*[(@Difficulty=$Difficulty or @CourseDifficulty=$Difficulty) and @StepsType=$StepsType]/HighScoreList/HighScore[1]/PercentDP)" />
										<xsl:variable name="Possible" select="count($Catalog/*/*[name()=$Type]/*[(@Difficulty=$Difficulty or @CourseDifficulty=$Difficulty) and @StepsType=$StepsType])" />
										
										<td>
											<span class="dyndata">
												<xsl:call-template name="PrintCalories">
													<xsl:with-param name="num" select="$Actual" />
												</xsl:call-template>
											</span>
										</td>
										
									</xsl:for-each>
								</tr>

								<tr>
									<td>Possible</td>
									<xsl:for-each select="$Catalog/Types/*[name()=$DifficultyName]/*[name()=$DifficultyName]">
										<xsl:variable name="Difficulty" select="." />
										<xsl:variable name="Actual" select="sum($Stats/*[name()=$ScoresName]/*/*[(@Difficulty=$Difficulty or @CourseDifficulty=$Difficulty) and @StepsType=$StepsType]/HighScoreList/HighScore[1]/PercentDP)" />
										<xsl:variable name="Possible" select="count($Catalog/*/*[name()=$Type]/*[(@Difficulty=$Difficulty or @CourseDifficulty=$Difficulty) and @StepsType=$StepsType])" />
										
										<td>
											<span class="dyndata">
												<xsl:call-template name="PrintCalories">
													<xsl:with-param name="num" select="$Possible" />
												</xsl:call-template>
											</span>
										</td>
										
									</xsl:for-each>
								</tr>

								<tr>
									<td>Percentage</td>
									<xsl:for-each select="$Catalog/Types/*[name()=$DifficultyName]/*[name()=$DifficultyName]">
										<xsl:variable name="Difficulty" select="." />
										<xsl:variable name="Actual" select="sum($Stats/*[name()=$ScoresName]/*/*[(@Difficulty=$Difficulty or @CourseDifficulty=$Difficulty) and @StepsType=$StepsType]/HighScoreList/HighScore[1]/PercentDP)" />
										<xsl:variable name="Possible" select="count($Catalog/*/*[name()=$Type]/*[(@Difficulty=$Difficulty or @CourseDifficulty=$Difficulty) and @StepsType=$StepsType])" />
										
										<td>
											<span class="dyndata">
												<xsl:if test="$Possible = 0">
													<xsl:call-template name="PrintPercentage">
														<xsl:with-param name="num" select="0" />
													</xsl:call-template>
												</xsl:if>
												<xsl:if test="$Possible != 0">
													<xsl:call-template name="PrintPercentage">
														<xsl:with-param name="num" select="$Actual div $Possible" />
													</xsl:call-template>
												</xsl:if>
											</span>
										</td>
										
									</xsl:for-each>
								</tr>

							</table>


							<hr />



							<table>

								<tr>
									<td> </td>
									<xsl:for-each select="$Catalog/Types/*[name()=$DifficultyName]/*[name()=$DifficultyName]">
										<xsl:variable name="Difficulty" select="." />
										<td><xsl:apply-templates select="$Difficulty"/></td>
									</xsl:for-each>
								</tr>
																
								<xsl:for-each select="$Catalog/*[name()=$TypePlural]/*[name()=$Type]">
									<xsl:variable name="Dir" select="@Dir" />
									<xsl:variable name="Path" select="@Path" />
									<xsl:variable name="StatsSongOrCourse" select="$Stats/*[name()=concat($Type,'Scores')]/*[@Dir=$Dir or @Path=$Path]" />
									<xsl:variable name="CatalogSongOrCourse" select="." />
									<tr>
										<td><xsl:apply-templates select="." mode="AttributeTitleGenerator" /></td>
										<xsl:for-each select="$Catalog/Types/*[name()=$DifficultyName]/*[name()=$DifficultyName]">
											<xsl:variable name="Difficulty" select="." />
											<xsl:variable name="StatsStepsOrTrail" select="$StatsSongOrCourse/*[(@Difficulty=$Difficulty or @CourseDifficulty=$Difficulty) and @StepsType=$StepsType]" />
											<xsl:variable name="CatalogStepsOrTrail" select="$CatalogSongOrCourse/*[(@Difficulty=$Difficulty or @CourseDifficulty=$Difficulty) and @StepsType=$StepsType]" />
											<td>
												<xsl:if test="count($CatalogStepsOrTrail) &gt; 0">
													<xsl:apply-templates select="$CatalogStepsOrTrail/Meter"/>
													<xsl:text> </xsl:text>
													<span class="dyndata">
														<xsl:apply-templates select="$StatsStepsOrTrail/HighScoreList/HighScore[1]/PercentDP" />
													</span>
												</xsl:if>
											</td>
										</xsl:for-each>
									</tr>
								</xsl:for-each>
								
							</table>
				
						</xsl:with-param>
					</xsl:call-template>		
				</xsl:for-each>
			</xsl:with-param>
		</xsl:call-template>		
	</xsl:template>
	
	


	<!-- RecentSongScores and RecentCourseScores -->
	
	<xsl:template match="/Stats/RecentSongScores | /Stats/RecentCourseScores">
		<xsl:variable name="Type" select="substring-after(substring-before(name(),'Scores'),'Recent')" />
		
		<xsl:call-template name="CollapsibleTopSection">
			<xsl:with-param name="title">
				<xsl:value-of select="$Type" /> Recent Scores
			</xsl:with-param>
			<xsl:with-param name="text">
				<xsl:for-each select="HighScoreForASongAndSteps | HighScoreForACourseAndTrail">
					<xsl:sort select="./*/HighScore/DateTime" order="descending" />
					<xsl:apply-templates select="." />
				</xsl:for-each>
			</xsl:with-param>
		</xsl:call-template>

	</xsl:template>		
	
	
	<xsl:template match="HighScoreForASongAndSteps | HighScoreForACourseAndTrail">
		<xsl:call-template name="SubSectionCompact">
			<xsl:with-param name="title">
				<xsl:apply-templates select="*" mode="AttributeTitleGenerator" />
			</xsl:with-param>
			<xsl:with-param name="text">
				<xsl:apply-templates select="HighScore" />
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>		
	



	
	<!-- ScreenshotData -->
		
		
	<xsl:template match="/Stats/ScreenshotData">
		<xsl:call-template name="CollapsibleTopSection">
			<xsl:with-param name="title">
				Screenshots
			</xsl:with-param>
			<xsl:with-param name="text">
				<xsl:for-each select="Screenshot">
					<xsl:sort select="HighScore/DateTime" order="descending" />
					<xsl:apply-templates select="." />
				</xsl:for-each>
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>		
	
	<xsl:template match="Screenshot">
		<table class="EntityTableAttr">
			<tr>
				<td style="background: #F4F6F7;">
					<a>
						<xsl:attribute name="href"><xsl:value-of select="concat('Screenshots/',FileName)" /></xsl:attribute>
						<xsl:attribute name="target">_new</xsl:attribute>
						<img>
							<xsl:attribute name="src"><xsl:value-of select="concat('Screenshots/',FileName)" /></xsl:attribute>
							<xsl:attribute name="width">160</xsl:attribute>
							<xsl:attribute name="height">120</xsl:attribute>
							<xsl:attribute name="style">border-width: 0</xsl:attribute>
						</img>
					</a>
				</td>
				<td style="background: #F4F6F7;">
					<table class="EntityTableAttr">
						<xsl:call-template name="DataTableGenerator">
							<xsl:with-param name="cols" select="2" />
							<xsl:with-param name="nodeset" select="*[text()]" />
						</xsl:call-template>
					</table>
					<xsl:apply-templates select="HighScore" />
				</td>
			</tr>
		</table>
	</xsl:template>		
	

	
	<!-- CoinData -->
		
		
	<xsl:template match="/Stats/CoinData">
		<xsl:call-template name="CollapsibleTopSection">
			<xsl:with-param name="title">
				Coin Counts
			</xsl:with-param>
			<xsl:with-param name="text">
			
				<xsl:for-each select="*">
				
					<xsl:call-template name="CollapsibleTopSection">
						<xsl:with-param name="title">
							<xsl:value-of select="name()" />
						</xsl:with-param>
						<xsl:with-param name="text">
						
				
							<table class="EntityTableAttr">
								<xsl:call-template name="DataTableGenerator">
									<xsl:with-param name="cols" select="2" />
									<xsl:with-param name="nodeset" select="./*[text()]" />
								</xsl:call-template>
							</table>
					
						</xsl:with-param>
					</xsl:call-template>
							
				</xsl:for-each>
			
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>		
	

	
	<!-- CalorieData -->
		
		
	<xsl:template match="/Stats/CalorieData">

		<xsl:call-template name="CollapsibleTopSection">
			<xsl:with-param name="title">
				Calories
			</xsl:with-param>
			<xsl:with-param name="text">

				<xsl:if test="../GeneralData/IsWeightSet = 0">
					WARNING: Weight is not set. Calorie counts may be inaccurate.
					<hr/>
				</xsl:if>
			
				<xsl:variable name="Cals" select="../GeneralData/TotalCaloriesBurned" />
				<xsl:variable name="NumSongs" select="sum(../GeneralData/NumSongsPlayedByPlayMode/*)" />
				<xsl:variable name="GameplaySeconds" select="../GeneralData/TotalGameplaySeconds" />

				<xsl:call-template name="PrintHorizontalDataTable">
					<xsl:with-param name="text">
						<xsl:call-template name="PrintHorizontalDataCell">
							<xsl:with-param name="name">All time</xsl:with-param>
							<xsl:with-param name="value">
								<xsl:call-template name="PrintCalories">
									<xsl:with-param name="num" select="$Cals" />
								</xsl:call-template>
							</xsl:with-param>
						</xsl:call-template>
						<xsl:call-template name="PrintHorizontalDataCell">
							<xsl:with-param name="name">Per Song</xsl:with-param>
							<xsl:with-param name="value">
								<xsl:if test="$NumSongs = 0">
									0
								</xsl:if>
								<xsl:if test="$NumSongs &gt; 0">
									<xsl:call-template name="PrintCalories">
										<xsl:with-param name="num" select="$Cals div $NumSongs" />
									</xsl:call-template>
								</xsl:if>
							</xsl:with-param>
						</xsl:call-template>
						<xsl:call-template name="PrintHorizontalDataCell">
							<xsl:with-param name="name">Per Minute of Gameplay</xsl:with-param>
							<xsl:with-param name="value">
								<xsl:if test="$GameplaySeconds = 0">
									0
								</xsl:if>
								<xsl:if test="$GameplaySeconds &gt; 0">
									<xsl:call-template name="PrintCalories">
										<xsl:with-param name="num" select="$Cals div ($GameplaySeconds div 60)" />
									</xsl:call-template>
								</xsl:if>
							</xsl:with-param>
						</xsl:call-template>
					</xsl:with-param>
				</xsl:call-template>
			
				<hr/>

				<h2>By Week</h2>
				<table>
					<tr>
						<td></td>
						<td>Sun</td>
						<td>Mon</td>
						<td>Tue</td>
						<td>Wed</td>
						<td>Thu</td>
						<td>Fri</td>
						<td>Sat</td>
					</tr>	

					<xsl:if test="count(/Stats/CalorieData/*) &gt; 0" >
						<xsl:variable name="firstDateJulian">
							<xsl:call-template name="calculate-julian-day2">
								<xsl:with-param name="date" select="/Stats/CalorieData/CaloriesBurned[1]/@Date" />
							</xsl:call-template>
						</xsl:variable>
						<xsl:variable name="lastDateJulian">
							<xsl:call-template name="calculate-julian-day2">
								<xsl:with-param name="date" select="/Stats/CalorieData/CaloriesBurned[last()]/@Date" />
							</xsl:call-template>
						</xsl:variable>
						<xsl:variable name="lastDateDayOfWeek">
							<xsl:call-template name="calculate-day-of-the-week2">
								<xsl:with-param name="date" select="/Stats/CalorieData/CaloriesBurned[last()]/@Date" />
							</xsl:call-template>
						</xsl:variable>
						<xsl:variable name="lastDateJulianRoundedToWeek" select="$lastDateJulian - $lastDateDayOfWeek" />
						
						<xsl:call-template name="PrintWeeksRecursive">
							<xsl:with-param name="beginDayJulian" select="$lastDateJulianRoundedToWeek" />
							<xsl:with-param name="stopDayJulian" select="$firstDateJulian" />
						</xsl:call-template>
					</xsl:if>
				</table>

			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>		
	
	
	<xsl:template name="PrintWeeksRecursive">
	    <xsl:param name="beginDayJulian" />
	    <xsl:param name="stopDayJulian" />
	    <xsl:variable name="endDayJulian" select="$beginDayJulian+6" />
	    <tr>
			<td>
				Week of 
				<xsl:call-template name="format-julian-day">
					<xsl:with-param name="julian-day" select="$beginDayJulian" />
				</xsl:call-template>
			</td>
			<td>
				<xsl:call-template name="CaloriesForJulianDay">
					<xsl:with-param name="dayJulian" select="$beginDayJulian+0" />
				</xsl:call-template>
			</td>
			<td>
				<xsl:call-template name="CaloriesForJulianDay">
					<xsl:with-param name="dayJulian" select="$beginDayJulian+1" />
				</xsl:call-template>
			</td>
			<td>
				<xsl:call-template name="CaloriesForJulianDay">
					<xsl:with-param name="dayJulian" select="$beginDayJulian+2" />
				</xsl:call-template>
			</td>
			<td>
				<xsl:call-template name="CaloriesForJulianDay">
					<xsl:with-param name="dayJulian" select="$beginDayJulian+3" />
				</xsl:call-template>
			</td>
			<td>
				<xsl:call-template name="CaloriesForJulianDay">
					<xsl:with-param name="dayJulian" select="$beginDayJulian+4" />
				</xsl:call-template>
			</td>
			<td>
				<xsl:call-template name="CaloriesForJulianDay">
					<xsl:with-param name="dayJulian" select="$beginDayJulian+5" />
				</xsl:call-template>
			</td>
			<td>
				<xsl:call-template name="CaloriesForJulianDay">
					<xsl:with-param name="dayJulian" select="$beginDayJulian+6" />
				</xsl:call-template>
			</td>
		</tr>
		<xsl:if test="$beginDayJulian &gt; $stopDayJulian">
			<xsl:variable name="nextBeginDayJulian" select="$beginDayJulian - 7" />
	        <xsl:call-template name="PrintWeeksRecursive">
				<xsl:with-param name="beginDayJulian" select="$nextBeginDayJulian" />
				<xsl:with-param name="stopDayJulian" select="$stopDayJulian" />
			</xsl:call-template>
	    </xsl:if>
	</xsl:template>
	
	<xsl:template name="CaloriesForJulianDay">
	    <xsl:param name="dayJulian" />
		<xsl:variable name="date">
			<xsl:call-template name="format-julian-day">
				<xsl:with-param name="julian-day" select="$dayJulian" />
			</xsl:call-template>
		</xsl:variable>
	    <xsl:variable name="cals" select="/Stats/CalorieData/CaloriesBurned[@Date=$date]" />
	    <xsl:if test="$cals">
			<xsl:call-template name="PrintCalories">
				<xsl:with-param name="num" select="$cals" />
			</xsl:call-template>
	    </xsl:if>
	</xsl:template>
	
	
	<xsl:template name="calculate-julian-day2">
	    <xsl:param name="date"/><!--2004-04-20-->

		<xsl:variable name="year"><!--2004-->
			<xsl:value-of select="substring-before($date,'-')"/>
		</xsl:variable>	
		<xsl:variable name="month-day"><!--04-20-->
			<xsl:value-of select="substring-after($date,'-')"/>
		</xsl:variable>
		<xsl:variable name="month"><!--04-->
			<xsl:value-of select="substring-before($month-day,'-')"/>
		</xsl:variable>
		<xsl:variable name="day"><!--20-->
			<xsl:value-of select="substring-after($month-day,'-')"/>
		</xsl:variable>
		
		<xsl:call-template name="calculate-julian-day">
			<xsl:with-param name="year" select="$year"/>
			<xsl:with-param name="month" select="$month"/>
			<xsl:with-param name="day" select="$day"/>
		</xsl:call-template>
		
	</xsl:template>
		
	<xsl:template name="calculate-julian-day">
		<xsl:param name="year" /><!--2004-->
		<xsl:param name="month" /><!--04-->
		<xsl:param name="day" /><!--20-->
		
		<xsl:variable name="a" select="floor((14 - $month) div 12)"/>
		<xsl:variable name="y" select="$year + 4800 - $a"/>
		<xsl:variable name="m" select="$month + 12 * $a - 3"/>
		<xsl:value-of select="$day + floor((153 * $m + 2) div 5) + $y * 365 + floor($y div 4) - floor($y div 100) + floor($y div 400) - 32045"/>
	</xsl:template>

	<xsl:template name="format-julian-day">
		<xsl:param name="julian-day"/>
		<xsl:param name="format" select="'%Y-%m-%d'"/>

		<xsl:variable name="a" select="$julian-day + 32044"/>
		<xsl:variable name="b" select="floor((4 * $a + 3) div 146097)"/>
		<xsl:variable name="c" select="$a - floor(($b * 146097) div 4)"/>

		<xsl:variable name="d" select="floor((4 * $c + 3) div 1461)"/>
		<xsl:variable name="e" select="$c - floor((1461 * $d) div 4)"/>
		<xsl:variable name="m" select="floor((5 * $e + 2) div 153)"/>

		<xsl:variable name="day" select="$e - floor((153 * $m + 2) div 5) + 1"/>
		<xsl:variable name="month" select="$m + 3 - 12 * floor($m div 10)"/>
		<xsl:variable name="year" select="$b * 100 + $d - 4800 + floor($m div 10)"/>

		<xsl:call-template name="format-date-time">
			<xsl:with-param name="year" select="$year"/>
			<xsl:with-param name="month" select="$month"/>
			<xsl:with-param name="day" select="$day"/>
			<xsl:with-param name="format" select="$format"/>
		</xsl:call-template>
		
	</xsl:template>
	

	
  <xsl:template name="calculate-day-of-the-week">
    <xsl:param name="year"/>
    <xsl:param name="month"/>
    <xsl:param name="day"/>

    <xsl:variable name="a" select="floor((14 - $month) div 12)"/>
    <xsl:variable name="y" select="$year - $a"/>
    <xsl:variable name="m" select="$month + 12 * $a - 2"/>

    <xsl:value-of select="($day + $y + floor($y div 4) - floor($y div 100) + floor($y div 400) + floor((31 * $m) div 12)) mod 7"/>

  </xsl:template>


  <xsl:template name="calculate-day-of-the-week2">
	<xsl:param name="date"/><!--2004-04-20-->
	
	<xsl:variable name="month-day"><!--04-20-->
		<xsl:value-of select="substring-after($date,'-')"/>
	</xsl:variable>
	<xsl:variable name="month"><!--04-->
		<xsl:value-of select="substring-before($month-day,'-')"/>
	</xsl:variable>
	<xsl:variable name="day"><!--20-->
		<xsl:value-of select="substring-after($month-day,'-')"/>
	</xsl:variable>
	<xsl:variable name="year"><!--2004-->
		<xsl:value-of select="substring-before($date,'-')"/>
	</xsl:variable>	

    <xsl:variable name="a" select="floor((14 - $month) div 12)"/>
    <xsl:variable name="y" select="$year - $a"/>
    <xsl:variable name="m" select="$month + 12 * $a - 2"/>

    <xsl:value-of select="($day + $y + floor($y div 4) - floor($y div 100) + floor($y div 400) + floor((31 * $m) div 12)) mod 7"/>

  </xsl:template>



  <xsl:template name="get-day-of-the-week-name">
    <xsl:param name="day-of-the-week"/>

    <xsl:choose>
      <xsl:when test="$day-of-the-week = 0">Sunday</xsl:when>
      <xsl:when test="$day-of-the-week = 1">Monday</xsl:when>
      <xsl:when test="$day-of-the-week = 2">Tuesday</xsl:when>
      <xsl:when test="$day-of-the-week = 3">Wednesday</xsl:when>
      <xsl:when test="$day-of-the-week = 4">Thursday</xsl:when>
      <xsl:when test="$day-of-the-week = 5">Friday</xsl:when>
      <xsl:when test="$day-of-the-week = 6">Saturday</xsl:when>
      <xsl:otherwise>error: <xsl:value-of select="$day-of-the-week"/></xsl:otherwise>
    </xsl:choose>

  </xsl:template>



  <xsl:template name="get-day-of-the-week-abbreviation">
    <xsl:param name="day-of-the-week"/>

    <xsl:choose>
      <xsl:when test="$day-of-the-week = 0">Sun</xsl:when>
      <xsl:when test="$day-of-the-week = 1">Mon</xsl:when>
      <xsl:when test="$day-of-the-week = 2">Tue</xsl:when>
      <xsl:when test="$day-of-the-week = 3">Wed</xsl:when>
      <xsl:when test="$day-of-the-week = 4">Thu</xsl:when>
      <xsl:when test="$day-of-the-week = 5">Fri</xsl:when>
      <xsl:when test="$day-of-the-week = 6">Sat</xsl:when>
      <xsl:otherwise>error: <xsl:value-of select="$day-of-the-week"/></xsl:otherwise>
    </xsl:choose>

  </xsl:template>



  <xsl:template name="get-month-abbreviation">
    <xsl:param name="month"/>

    <xsl:choose>
      <xsl:when test="$month = 1">Jan</xsl:when>
      <xsl:when test="$month = 2">Feb</xsl:when>
      <xsl:when test="$month = 3">Mar</xsl:when>
      <xsl:when test="$month = 4">Apr</xsl:when>
      <xsl:when test="$month = 5">May</xsl:when>
      <xsl:when test="$month = 6">Jun</xsl:when>
      <xsl:when test="$month = 7">Jul</xsl:when>
      <xsl:when test="$month = 8">Aug</xsl:when>
      <xsl:when test="$month = 9">Sep</xsl:when>
      <xsl:when test="$month = 10">Oct</xsl:when>
      <xsl:when test="$month = 11">Nov</xsl:when>
      <xsl:when test="$month = 12">Dec</xsl:when>
      <xsl:otherwise>error: <xsl:value-of select="$month"/></xsl:otherwise>
    </xsl:choose>

  </xsl:template>



  <xsl:template name="get-month-name">
    <xsl:param name="month"/>

    <xsl:choose>
      <xsl:when test="$month = 1">January</xsl:when>
      <xsl:when test="$month = 2">February</xsl:when>
      <xsl:when test="$month = 3">March</xsl:when>
      <xsl:when test="$month = 4">April</xsl:when>
      <xsl:when test="$month = 5">May</xsl:when>
      <xsl:when test="$month = 6">June</xsl:when>
      <xsl:when test="$month = 7">July</xsl:when>
      <xsl:when test="$month = 8">August</xsl:when>
      <xsl:when test="$month = 9">September</xsl:when>
      <xsl:when test="$month = 10">October</xsl:when>
      <xsl:when test="$month = 11">November</xsl:when>
      <xsl:when test="$month = 12">December</xsl:when>
      <xsl:otherwise>error: <xsl:value-of select="$month"/></xsl:otherwise>
    </xsl:choose>

  </xsl:template>



  <xsl:template name="calculate-week-number">
    <xsl:param name="year"/>
    <xsl:param name="month"/>
    <xsl:param name="day"/>

    <xsl:variable name="J">
      <xsl:call-template name="calculate-julian-day">
        <xsl:with-param name="year" select="$year"/>
        <xsl:with-param name="month" select="$month"/>
        <xsl:with-param name="day" select="$day"/>
      </xsl:call-template>
    </xsl:variable>

    <xsl:variable name="d4" select="($J + 31741 - ($J mod 7)) mod 146097 mod 36524 mod 1461"/>
    <xsl:variable name="L" select="floor($d4 div 1460)"/>
    <xsl:variable name="d1" select="(($d4 - $L) mod 365) + $L"/>

    <xsl:value-of select="floor($d1 div 7) + 1"/>

  </xsl:template>



  <xsl:template name="format-date-time">
    <xsl:param name="year"/>
    <xsl:param name="month"/>
    <xsl:param name="day"/>
    <xsl:param name="hour"/>
    <xsl:param name="minute"/>
    <xsl:param name="second"/>
    <xsl:param name="time-zone"/>
    <xsl:param name="format" select="'%Y-%m-%dT%H:%M:%S%z'"/>

    <xsl:value-of select="substring-before($format, '%')"/>

    <xsl:variable name="code" select="substring(substring-after($format, '%'), 1, 1)"/>

    <xsl:choose>

      <!-- Abbreviated weekday name -->
      <xsl:when test="$code='a'">
        <xsl:variable name="day-of-the-week">
          <xsl:call-template name="calculate-day-of-the-week">
            <xsl:with-param name="year" select="$year"/>
            <xsl:with-param name="month" select="$month"/>
            <xsl:with-param name="day" select="$day"/>
          </xsl:call-template>
        </xsl:variable>
        <xsl:call-template name="get-day-of-the-week-abbreviation">
          <xsl:with-param name="day-of-the-week" select="$day-of-the-week"/>
        </xsl:call-template>
      </xsl:when>

      <!-- Full weekday name -->
      <xsl:when test="$code='A'">
        <xsl:variable name="day-of-the-week">
          <xsl:call-template name="calculate-day-of-the-week">
            <xsl:with-param name="year" select="$year"/>
            <xsl:with-param name="month" select="$month"/>
            <xsl:with-param name="day" select="$day"/>
          </xsl:call-template>
        </xsl:variable>
        <xsl:call-template name="get-day-of-the-week-name">
          <xsl:with-param name="day-of-the-week" select="$day-of-the-week"/>
        </xsl:call-template>
      </xsl:when>

      <!-- Abbreviated month name -->
      <xsl:when test="$code='b'">
        <xsl:call-template name="get-month-abbreviation">
          <xsl:with-param name="month" select="$month"/>
        </xsl:call-template>
      </xsl:when>

      <!-- Full month name -->
      <xsl:when test="$code='B'">
        <xsl:call-template name="get-month-name">
          <xsl:with-param name="month" select="$month"/>
        </xsl:call-template>
      </xsl:when>

      <!-- Date and time representation appropriate for locale -->
      <xsl:when test="$code='c'">
        <xsl:text>[not implemented]</xsl:text>
      </xsl:when>

      <!-- Day of month as decimal number (01 - 31) -->
      <xsl:when test="$code='d'">
        <xsl:if test="$day &lt; 10">0</xsl:if>
        <xsl:value-of select="number($day)"/>
      </xsl:when>

      <!-- Hour in 24-hour format (00 - 23) -->
      <xsl:when test="$code='H'">
        <xsl:if test="$hour &lt; 10">0</xsl:if>
        <xsl:value-of select="number($hour)"/>
      </xsl:when>

      <!-- Hour in 12-hour format (01 - 12) -->
      <xsl:when test="$code='I'">
        <xsl:choose>
          <xsl:when test="$hour = 0">12</xsl:when>
          <xsl:when test="$hour &lt; 10">0<xsl:value-of select="$hour - 0"/></xsl:when>
          <xsl:when test="$hour &lt; 13"><xsl:value-of select="$hour - 0"/></xsl:when>
          <xsl:when test="$hour &lt; 22">0<xsl:value-of select="$hour - 12"/></xsl:when>
          <xsl:otherwise><xsl:value-of select="$hour - 12"/></xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <!-- Day of year as decimal number (001 - 366) -->
      <xsl:when test="$code='j'">
        <xsl:text>[not implemented]</xsl:text>
      </xsl:when>

      <!-- Month as decimal number (01 - 12) -->
      <xsl:when test="$code='m'">
        <xsl:if test="$month &lt; 10">0</xsl:if>
        <xsl:value-of select="number($month)"/>
      </xsl:when>

      <!-- Minute as decimal number (00 - 59) -->
      <xsl:when test="$code='M'">
        <xsl:if test="$minute &lt; 10">0</xsl:if>
        <xsl:value-of select="number($minute)"/>
      </xsl:when>

      <!-- Current locale's A.M./P.M. indicator for 12-hour clock -->
      <xsl:when test="$code='p'">
        <xsl:choose>
          <xsl:when test="$hour &lt; 12">AM</xsl:when>
          <xsl:otherwise>PM</xsl:otherwise>
        </xsl:choose>
      </xsl:when>

      <!-- Second as decimal number (00 - 59) -->
      <xsl:when test="$code='S'">
        <xsl:if test="$second &lt; 10">0</xsl:if>
        <xsl:value-of select="number($second)"/>
      </xsl:when>

      <!-- Week of year as decimal number, with Sunday as first day of week (00 - 53) -->
      <xsl:when test="$code='U'">
        <!-- add 1 to day -->
        <xsl:call-template name="calculate-week-number">
          <xsl:with-param name="year" select="$year"/>
          <xsl:with-param name="month" select="$month"/>
          <xsl:with-param name="day" select="$day + 1"/>
        </xsl:call-template>
      </xsl:when>

      <!-- Weekday as decimal number (0 - 6; Sunday is 0) -->
      <xsl:when test="$code='w'">
        <xsl:call-template name="calculate-day-of-the-week">
          <xsl:with-param name="year" select="$year"/>
          <xsl:with-param name="month" select="$month"/>
          <xsl:with-param name="day" select="$day"/>
        </xsl:call-template>
      </xsl:when>

      <!-- Week of year as decimal number, with Monday as first day of week (00 - 53) -->
      <xsl:when test="$code='W'">
        <xsl:call-template name="calculate-week-number">
          <xsl:with-param name="year" select="$year"/>
          <xsl:with-param name="month" select="$month"/>
          <xsl:with-param name="day" select="$day"/>
        </xsl:call-template>
      </xsl:when>

      <!-- Date representation for current locale -->
      <xsl:when test="$code='x'">
        <xsl:text>[not implemented]</xsl:text>
      </xsl:when>

      <!-- Time representation for current locale -->
      <xsl:when test="$code='X'">
        <xsl:text>[not implemented]</xsl:text>
      </xsl:when>

      <!-- Year without century, as decimal number (00 - 99) -->
      <xsl:when test="$code='y'">
        <xsl:text>[not implemented]</xsl:text>
      </xsl:when>

      <!-- Year with century, as decimal number -->
      <xsl:when test="$code='Y'">
        <xsl:value-of select="concat(substring('000', string-length(number($year))), $year)"/>
      </xsl:when>

      <!-- Time-zone name or abbreviation; no characters if time zone is unknown -->
      <xsl:when test="$code='z'">
        <xsl:value-of select="$time-zone"/>
      </xsl:when>

      <!-- Percent sign -->
      <xsl:when test="$code='%'">
        <xsl:text>%</xsl:text>
      </xsl:when>

    </xsl:choose>

    <xsl:variable name="remainder" select="substring(substring-after($format, '%'), 2)"/>

    <xsl:if test="$remainder">
      <xsl:call-template name="format-date-time">
        <xsl:with-param name="year" select="$year"/>
        <xsl:with-param name="month" select="$month"/>
        <xsl:with-param name="day" select="$day"/>
        <xsl:with-param name="hour" select="$hour"/>
        <xsl:with-param name="minute" select="$minute"/>
        <xsl:with-param name="second" select="$second"/>
        <xsl:with-param name="time-zone" select="$time-zone"/>
        <xsl:with-param name="format" select="$remainder"/>
      </xsl:call-template>
    </xsl:if>

  </xsl:template>
  
	<!-- Main Categories End - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -->
	

	<!-- That's it -->
	
</xsl:stylesheet>

