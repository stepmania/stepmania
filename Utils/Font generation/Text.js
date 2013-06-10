/* This file was written by Glenn Maynard, and is in the public domain.
 *
 * This is a Photoshop javascript to generate Kanji fonts.
 *
 * You'll need the plugin:
 *
 *    http://www.adobe.com/support/downloads/detail.jsp?ftpID=1536
 *
 * Load this script from File->Automation->Scripts, browse.  Expect it to take
 * a minute or two.
 *
 * Minimally, you'll need to edit the directories below.
 *
 * Each font page in FontPages is generated for each font in Fonts.
 *
 * I can't figure out a way to force a font to fixed-width, so this currently
 * only works with fixed-width fonts.  I'm using this primarily for Japanese
 * fonts, so that's OK.  If we could output Unicode, we could get around this,
 * by outputting each glyph in a separate layer and aligning them individually,
 * though that'd take a very long time.
 */

/* Okay, this is a stupid way to pass by reference, but I don't speak JavaScript ... */
function Integer(i) { this.val = i; }

var TemplateDir = "c:/temp/stepmania/utils/Font generation/Templates/";
var OutputDir = "c:/temp/stepmania/utils/Font generation/Output/";

/* We can't send Unicode into the font ourself for some reason.  So, we have to
 * store prototypes for each font page.  These should just be PSD files that contain
 * a single text layer with the characters we want.  The orientation, font, etc.
 * don't matter; they'll be overridden. */
var JAFontPages = Array("main", "kanji 1", "kanji 2","kanji 3","kanji 4");
var KRFontPages = Array("jamo 1", "jamo 2", "jamo 3", "jamo 4");

var Fonts = new Array(
	/* name                       font         pages        frame sz  font sz  AA                border   bold */
	new FontDef("_japanese 16px", "MS-Gothic", JAFontPages, 32,       20,      AntiAlias.STRONG, 2,       true),
	new FontDef("_japanese 24px", "MS-Gothic", JAFontPages, 32,       26,      AntiAlias.SMOOTH, 2,       true)
	//new FontDef("_korean 16px",   "ArialUnicodeMS", KRFontPages, 32,       20,      AntiAlias.SMOOTH, 2,       true),
	//new FontDef("_korean 24px",   "ArialUnicodeMS", KRFontPages, 32,       28,      AntiAlias.SMOOTH, 2,       true)
		);
	

var sel, doc;

var strtRulerUnits = preferences.rulerUnits;
var strtTypeUnits = preferences.typeUnits;
preferences.rulerUnits = Units.PIXELS;
preferences.typeUnits = TypeUnits.PIXELS;
displayDialogs = DialogModes.NO;

go();

preferences.rulerUnits = strtRulerUnits;
preferences.typeUnits = strtTypeUnits;

function FontDef(Name, /* filename prefix */
	      Font, /* actual font name */
	      Pages, /* list of font pages */
	      FrameSize, /* frame size in pixels */
	      FontSize, /* font character size in pixels (horiz) */
	      AAMethod, /* antialiasing */
	      Border, /* pixels or 0 for off */
	      Bold /* boolean */
	) 
{
	this.Name = Name;
	this.Font = Font;
	this.Pages = Pages;

	this.FrameSize = FrameSize;
	/* This is the width of each character, as output by the font renderer.
	 * Changing this changes the size of the character. */
	 
	this.HorizSize = FontSize;
	/* This is the actual vertical size of characters, as output by the font renderer. 
	 * For MS Gothic, it always seems to be 4/3 of the vertical size. */
	this.VertSize = FontSize* 4 / 3;
	this.AAMethod = AAMethod;
	this.Border = Border;
	this.Bold = Bold;
}

function go()
{
	for(var f = 0; f < Fonts.length; ++f)
	{
		for(var i = 0; i < Fonts[f].Pages.length; ++i)
		{
			MakeFontPage(TemplateDir + Fonts[f].Pages[i] + " template.psd",
					OutputDir + Fonts[f].Name + " [" + Fonts[f].Pages[i] + "]",
					Fonts[f]);
		}
	}
}

function MakeFontPage(TemplatePath, OutputPath, Font)
{
	/* Open the template. */
	var file = new File(TemplatePath);
	if(!file.open("r"))
	{
		alert("Couldn't open " + TemplatePath);
		return;
	}
	open(file);
	file = null;

	/* Change the template to a font page. */
	var frameX = new Integer;
	var frameY = new Integer;
	ConvertCurrentDocument(frameX, frameY, Font);

	/* Save the template to the destination file and close the template. */
	var Save = new PNGSaveOptions;
	Save.interlaced = false;
	OutputPath += " " + frameX.val + "x" + frameY.val;
	OutputPath += ".png";
	activeDocument.saveAs(new File(OutputPath), Save, true, Extension.LOWERCASE);
	activeDocument.close(SaveOptions.DONOTSAVECHANGES);
}

function GetTextLayer(Doc)
{
	/* Find a text layer. */
	var TextLayer = null;
	for(var i = 0; i < activeDocument.artLayers.length; ++i)
	{
		if(activeDocument.artLayers[i].kind != LayerKind.TEXT)
			continue; /* irrelevant */

		if(TextLayer != null)
		{
			alert("More than one text layer found.  Confused.");
			return null;
		}

		TextLayer = activeDocument.artLayers[i];
	}

	if(TextLayer == null)
	{
		alert("No text layer found.  Confused.");
		return null;
	}

	return TextLayer;
}

/* We have an open document with a single text layer.  Convert it to a font
 * page.  The result will be a single RGB layer. */
function ConvertCurrentDocument( frameX, frameY, Font )
{
	// Create a new art layer and convert it to a text layer.
	// Set its contents, size and color.
	if (documents.length == 0)
	{
		alert ("You must have at least one open document to run this script.");
		return;
	}

	doc = activeDocument;
	sel = doc.selection;

	/* Find a text layer. */
	var TextLayer = GetTextLayer(doc);
	if(TextLayer == null) return;

	/* Figure out the number of frames in each dimension, based on the line count
	 * and max characters/line. */
	GetFrameCount(TextLayer.textItem.contents, frameX, frameY);

	/* Based on the frame count and frame size, figure out the dimensions of
	 * the whole page. */
	var PageWidth = frameX.val * Font.FrameSize;
	var PageHeight = frameY.val * Font.FrameSize;

	/* Resize the image down to the size required to hold the images.  This prevents
	 * errors later when we move columns around if the image is too big. */
	doc.resizeCanvas(PageWidth, PageHeight, AnchorPosition.TOPLEFT);

	{
		/* Using HorizSize as the vertical origin seems to align the text so that
		 * it's centered in the top VertSize pixels.  Adjust from there to being centered
		 * in the frame. 
		 *
		 * Add half of a pixel.  This seems to help alignment a lot. */
		var Dist = Font.FrameSize/2-Font.VertSize/2;
		TextLayer.textItem.position = Array(0, Font.HorizSize+Dist+.5);
	}

	TextLayer.textItem.font = Font.Font;
	TextLayer.textItem.antiAliasMethod = Font.AAMethod;
	var textColor = new SolidColor;
		textColor.rgb.red = 255;
		textColor.rgb.green = 255;
		textColor.rgb.blue = 255;

	TextLayer.textItem.color = textColor;

	TextLayer.textItem.useAutoLeading = 0;
	TextLayer.textItem.spaceBefore = Font.FrameSize;
	TextLayer.textItem.spaceAfter = 0;

	TextLayer.textItem.size = Font.HorizSize;

	TextLayer.textItem.fauxBold = Font.Bold;
	TextLayer.textItem.fauxItalic = false;

	TextLayer.rasterize(RasterizeType.ENTIRELAYER);

	var CurrentSpacing = Font.HorizSize;
	/* Faux bold adds half a pixel to the width of each character. */
	if(Font.Bold)
		CurrentSpacing += 0.5;

	/* We currently have a horizontal spacing of CurrentSpacing; adjust it to
	 * have a vertical spacing of FrameSize. */
	Adjust(Font.FrameSize, CurrentSpacing, frameX.val);

	if(Font.Border != 0)
		TextLayer = MakeBorder(TextLayer, doc.artLayers, Font.Border);

	{
		/* Resize the texture to have the largest possible frame count without going
		 * over a power of two.  This way, when the font page grows, the frame count 
		 * (8x8) stays constant, so we don't have to keep renaming files in CVS. */
		var MaxX = power_of_two(PageWidth);
		var MaxY = power_of_two(PageHeight);
		while(PageWidth + Font.FrameSize <= MaxX)
			PageWidth += Font.FrameSize;
		while(PageHeight + Font.FrameSize <= MaxY)
			PageHeight += Font.FrameSize;

		/* Actually, let's force the texture to be square, too, which reduces filename
		 * changes even more.  This eats a little texture memory.  It also means that
		 * if the texture is reduced at runtime, it'll be reduced equally in both
		 * dimensions, which will probably look less weird. */
		if(PageHeight < PageWidth) PageHeight = PageWidth;
		else PageWidth = PageHeight;

		frameX.val = PageWidth / Font.FrameSize;
		frameY.val = PageHeight / Font.FrameSize;
	}
	doc.resizeCanvas(PageWidth, PageHeight, AnchorPosition.TOPLEFT);
}

function GetFrameCount(str, x, y)
{
	/* Strip blank lines off the end. */
	for(var c = str.length-1; c >= 0; --c)
	{
		if(c != '\r' && c != ' ')
			break;
		str = str.substr(0, str.length-1);
	}

	var NumLines = 0, MaxLength = 0, CurLength = 0;
	
	for(var c = 0; c < str.length; ++c)
	{
		if(str[c] == '\r')
		{
			CurLength = 0;
			NumLines++;
		}
		else
		{
			CurLength++;
			if(CurLength > MaxLength)
				MaxLength = CurLength;
		}
	}
	if(CurLength)
		NumLines++;

	x.val = MaxLength;
	y.val = NumLines;
}

function power_of_two(n)
{
	var value = 1;

	while (value < n)
		value *= 2;

	return value;
}

/* The current document is in columns of width NewWidth.  Space it out so the columns
 * are of width OldWidth. */
function Adjust(NewWidth, OldWidth, Columns)
{
	// col = 7;
	for(var col = Columns-1; col >= 0; --col)
	{
		var selBounds = Array(
			Array(col*OldWidth,0),
			Array((col+1)*OldWidth, 0),
			Array((col+1)*OldWidth, activeDocument.height),
			Array(col*OldWidth, activeDocument.height));
		sel.select(selBounds);

		var OldXCenter = (col + .5) * OldWidth;
		var NewXCenter = (col + .5) * NewWidth;
		var Distance = NewXCenter-OldXCenter;
		sel.translate(Distance);
	}
	sel.deselect();
}

function SelectTransparencyFromCurrentLayer()
{
	// Voodoo from the action recorder, since I couldn't find a way to
	// do this normally.
	var id15 = charIDToTypeID( "setd" );
	    var desc3 = new ActionDescriptor();
	    var id16 = charIDToTypeID( "null" );
		var ref1 = new ActionReference();
		var id17 = charIDToTypeID( "Chnl" );
		var id18 = charIDToTypeID( "fsel" );
		ref1.putProperty( id17, id18 );
	    desc3.putReference( id16, ref1 );
	    var id19 = charIDToTypeID( "T   " );
		var ref2 = new ActionReference();
		var id20 = charIDToTypeID( "Chnl" );
		var id21 = charIDToTypeID( "Chnl" );
		var id22 = charIDToTypeID( "Trsp" );
		ref2.putEnumerated( id20, id21, id22 );
	    desc3.putReference( id19, ref2 );
	executeAction( id15, desc3, DialogModes.NO );
}

/* Add a border to the current layer. */
function MakeBorder(Layer, Layers, Width)
{
	SelectTransparencyFromCurrentLayer();
	sel.expand(Width);

	var BorderLayer = Layers.add();
	BorderLayer.moveAfter(Layer);

	var borderColor = new SolidColor;
		borderColor.rgb.red = 0;
		borderColor.rgb.green = 0;
		borderColor.rgb.blue = 0;

	sel.fill(borderColor);
	sel.deselect();

	Layer.merge();

	return BorderLayer;
}


