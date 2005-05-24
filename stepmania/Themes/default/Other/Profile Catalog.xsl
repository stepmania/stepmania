<?xml version="1.0" encoding="UTF-8" ?>

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:import href="Common.xsl"/>


	<!-- Main Template -->

	<xsl:template match="/Catalog">
		<xsl:call-template name="MainTemplate">
			<xsl:with-param name="FullHeader" select="0" />
			<xsl:with-param name="DocName" select="name()" />
			<xsl:with-param name="Content">
				<xsl:apply-templates select="/Catalog/Totals" />
				<xsl:apply-templates select="/Catalog/Songs" />
				<xsl:apply-templates select="/Catalog/Courses" />
			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>
	
	
	
	<!-- Totals -->
	
	<xsl:template match="/Catalog/Totals">
		<xsl:call-template name="CollapsibleTopSection">
			<xsl:with-param name="title">
				Totals
			</xsl:with-param>
			<xsl:with-param name="text">

				<xsl:element name="table" use-attribute-sets="EntityTableAttr">
					<xsl:call-template name="DataTableGenerator">
						<xsl:with-param name="cols" select="2" />
						<xsl:with-param name="nodeset" select="*[text()]" />
					</xsl:call-template>
				</xsl:element>
				
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
		<xsl:call-template name="SubSectionCompact">
			<xsl:with-param name="title">
				<xsl:apply-templates select="." mode="AttributeTitleGenerator" />
			</xsl:with-param>
			<xsl:with-param name="text">
				
				<xsl:element name="table" use-attribute-sets="EntityTableAttr">
					<xsl:call-template name="DataTableGenerator">
						<xsl:with-param name="cols" select="1" />
						<xsl:with-param name="nodeset" select="*[text()] | ./*/*[text()]" />
					</xsl:call-template>
				</xsl:element>

			</xsl:with-param>
		</xsl:call-template>
	</xsl:template>		

	
	<!-- Main Categories End - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -->


	<!-- That's it -->
	
</xsl:stylesheet>

