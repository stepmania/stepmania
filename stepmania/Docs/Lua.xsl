<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
	xmlns:sm="http://www.stepmania.com"
	exclude-result-prefixes="sm"> <!-- keep xslt from spittingout namespace info. -->
<!-- This could be xhtml 1.0 strict, but firefox is failing at xml -> xhtml, or something
 <xsl:output method="xml" encoding="UTF-8" version="1.0" standalone="yes"
	doctype-system="http://www.w3.org/TR/2000/REC-xhtml1-20000126/DTD/xhtml1-strict.dtd"
	doctype-public="-//W3C//DTD XHTML 1.0 Strict//EN" />
-->
<xsl:output method="html" encoding="UTF-8" version="4.01" standalone="yes"
	doctype-system="http://www.w3.org/TR/html4/strict.dtd"
	doctype-public="-//W3C//DTD HTML 4.01//EN" />

<xsl:template match="/">
	<html>
		<head>
			<title>StepMania LUA Information</title>
			<style type="text/css">
				th {
					background-color: #FAEBD7;
					border: 1px solid
				}
				table {
					border: 1px solid
				}
				td {
					border: 1px solid
				}
				hr {
					width: 90%
				}
				.trigger {
					cursor: pointer
				}
				.footer {
					text-align: center
				}
			</style>
			<script type="text/javascript">
				function Toggle( id )
				{
					var imgid = 'img_' + id;
					var listid = 'list_' + id;
					var img = document.getElementById( imgid );
					var list = document.getElementById( listid );
					
					if( img.getAttribute('src') == 'closed.gif' )
					{
						img.setAttribute( 'src', 'open.gif' );
						list.style.display = 'block';
					}
					else
					{
						img.setAttribute( 'src', 'closed.gif' );
						list.style.display = 'none';
					}
				}
			</script>
		</head>
		<body>
			<xsl:apply-templates />
		</body>
	</html>
</xsl:template>


<xsl:template match="sm:Lua">
	<div>
		<h2>StepMania LUA Information</h2>
		<table>
			<tr>
				<th><a href="#Singletons">Singletons</a></th>
				<th><a href="#Classes">Classes</a></th>
				<th><a href="#GlobalFunctions">Global Functions</a></th>
				<th><a href="#Enums">Enums</a></th>
				<th><a href="#Constants">Constants</a></th>
			</tr>
		</table>
	</div>
	<xsl:apply-templates select="sm:Singletons" />
	<xsl:apply-templates select="sm:Classes" />
	<xsl:apply-templates select="sm:GlobalFunctions" />
	<xsl:apply-templates select="sm:Enums" />
	<xsl:apply-templates select="sm:Constants" />
	<hr />
	<p class="footer">
		Generated for <xsl:value-of select="sm:Version" /> on
		<xsl:value-of select="sm:Date" />.
	</p>
</xsl:template>


<xsl:template match="sm:Singletons">
	<div>
		<a name="Singletons" />
		<h3>Singletons</h3>
		<ul>
			<xsl:for-each select="sm:Singleton">
				<xsl:sort select="@name" />
				<li>
					<a href="#{@class}" onclick="Toggle('{@class}')">
						<xsl:value-of select="@name" />
					</a>
				</li>
			</xsl:for-each>
		</ul>
	</div>
</xsl:template>


<xsl:template match="sm:Classes">
	<div>
		<a name="Classes" />
		<h3>Classes</h3>
		<xsl:apply-templates select="sm:Class">
			<xsl:sort select="@name" />
		</xsl:apply-templates>
	</div>
</xsl:template>

<xsl:variable name="docs" select="document('LuaDocumentation.xml')/sm:Documentation" />

<xsl:template match="sm:Class">
	<xsl:variable name="name" select="@name" />
	<div>
		<a name="{@name}" class="trigger" onclick="Toggle('{@name}')">
			<img src="closed.gif" id="img_{@name}" alt="" />
			Class <xsl:value-of select="@name" />
		</a>
		<xsl:if test="@base != ''">
			<xsl:text> : </xsl:text>
			<a href="#{@base}" onclick="Toggle('{@base}')">
				<xsl:value-of select="@base" />
			</a>
		</xsl:if>
		<div style="display: none" id="list_{@name}">
		<table>
			<tr><th>Function</th><th>Description</th></tr>
			<xsl:apply-templates select="sm:Function">
				<xsl:sort select="@name" />
				<xsl:with-param name="path" select="$docs/sm:Classes/sm:Class[@name=$name]" />
			</xsl:apply-templates>
		</table>
		<br />
		</div>
	</div>
</xsl:template>


<xsl:template match="sm:GlobalFunctions">
	<div>
		<a name="GlobalFunctions" />
		<h3>Global Functions</h3>
		<table>
			<tr><th>Function</th><th>Description</th></tr>
			<xsl:apply-templates select="sm:Function">
				<xsl:sort select="@name" />
				<xsl:with-param name="path" select="$docs/sm:GlobalFunctions" />
			</xsl:apply-templates>
		</table>
	</div>
</xsl:template>


<xsl:template match="sm:Function">
	<xsl:param name="path" />
	<xsl:variable name="name" select="@name" />
	<xsl:variable name="elmt" select="$path/sm:Function[@name=$name]" />
	<tr>
		<xsl:choose>
			<xsl:when test="string($elmt/@name)=$name"><td> <!-- The name must exist. -->
				<xsl:value-of select="$elmt/@return" />
				<xsl:text> </xsl:text>
				<xsl:value-of select="@name" />( <xsl:value-of select="$elmt/@arguments" /> )
			</td><td><xsl:value-of select="$elmt" /></td>
			</xsl:when>
			<xsl:otherwise>
				<td><xsl:value-of select="@name" /></td>
			</xsl:otherwise>
		</xsl:choose>
	</tr>
</xsl:template>

<xsl:template match="sm:Enums">
	<div>
		<a name="Enums" />
		<h3>Enums</h3>
		<xsl:apply-templates select="sm:Enum">
			<xsl:sort select="@name" />
		</xsl:apply-templates>
	</div>
</xsl:template>


<xsl:template match="sm:Enum">
	<div>
		<a class="trigger" onclick="Toggle('{@name}')">
		<img src="closed.gif" id="img_{@name}" alt="" />
		Enum <xsl:value-of select="@name" /></a>
		<div style="display: none" id="list_{@name}">
		<table>
			<tr>
				<th>Enum</th>
				<th>Value</th>
			</tr>
			<xsl:for-each select="sm:EnumValue">
				<xsl:sort data-type="number" select="@value" />
				<tr>
					<td><xsl:value-of select="@name" /></td>
					<td><xsl:value-of select="@value" /></td>
				</tr>
			</xsl:for-each>
		</table>
		<br />
		</div>
	</div>
</xsl:template>


<xsl:template match="sm:Constants">
	<div>
		<a name="Constants" />
		<h3>Constants</h3>
		<table>
			<tr>
				<th>Constant</th>
				<th>Value</th>
			</tr>
			<xsl:for-each select="sm:Constant">
				<xsl:sort select="@name" />
				<tr>
					<td><xsl:value-of select="@name" /></td>
					<td><xsl:value-of select="@value" /></td>
				</tr>
			</xsl:for-each>
		</table>
		<br />
	</div>
</xsl:template>
</xsl:stylesheet>
