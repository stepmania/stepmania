#!/usr/bin/python
# WARNING: This only works in GIMP 2.4.7 and later. 2.4.6 had a nasty memory leak in
# gimp_text_fontname that breaks this script in all sorts of bad ways.
# confirmed working on Linux, and broken on OS X.

from math import ceil
from gimpfu import *
 
def smfontgen(fontname, fontsize, glyphsizex, glyphsizey, sstartcode, sendcode, charsperrow, rowspertex, border, filename):
	startcode = int(sstartcode, 16)
	endcode = int(sendcode, 16)
	texindex = 0
	metricsfd = 0
	atglyph = startcode
	
	metrics = open(filename+".ini", "w")
	metrics.write("\xEF\xBB\xBF") # Unicode BOM
	metrics.write("[common]\n")
	# These are all guesses. They can easily be changed by the user so we don't have to get it right.
	metrics.write("Baseline="+str(fontsize)+"\n")
	metrics.write("Top=0\n")
	metrics.write("Linespacing="+str(glyphsizey)+"\n")
	metrics.write("DrawExtraPixelsLeft=0\n")
	metrics.write("DrawExtraPixelsRight=0\n")
	metrics.write("AdvanceExtraPixels=0\n")
	metrics.write("\n")
	
	while(atglyph <= endcode):
		atglyph = maketex(texindex, fontname, fontsize, glyphsizex, glyphsizey, atglyph, endcode, charsperrow, rowspertex, metrics, border, filename)
		texindex+=1
	# XXX Wrap up metrics here

# Generates one texture
def maketex(texindex, fontname, fontsize, glyphsizex, glyphsizey, startcode, endcode, charsperrow, rowspertex, metrics, border, filename):
	totalx = glyphsizex * charsperrow # We'll crop it down later if needed
	totaly = glyphsizey*rowspertex # See above
	
	metrics.write("["+str(texindex)+"]\n")
	
	img = pdb.gimp_image_new(totalx, totaly, RGB)
	# DEBUG: show the image
	#pdb.gimp_display_new(img)
	bglyr = pdb.gimp_layer_new(img, totalx, totaly, RGBA_IMAGE, "Background", 100, NORMAL_MODE)
	pdb.gimp_image_add_layer(img, bglyr, 0)
	glyphlyr = pdb.gimp_layer_new(img, totalx, totaly, RGBA_IMAGE, "Glyphs", 100, NORMAL_MODE)
	pdb.gimp_image_add_layer(img, glyphlyr, 0)
	pdb.gimp_drawable_fill(bglyr, TRANSPARENT_FILL)
	pdb.gimp_drawable_fill(glyphlyr, TRANSPARENT_FILL)
	pdb.gimp_context_set_foreground((255, 255,255))
	pdb.gimp_context_set_background((0, 0, 0))
	cellx = 0
	celly = 0
	glyphsintex = 0
	atglyph = startcode
	charlenout = ""
	# Write first Line X=
	metrics.write("Line 0=")
	while atglyph <= endcode:
		# Make sure nothing is selected.
		pdb.gimp_selection_none(img)
		# Attempt to render the glyph.
		charlyr = pdb.gimp_text_fontname(img, None, cellx*glyphsizex, celly*glyphsizey, unichr(atglyph), border, True, fontsize, PIXELS, fontname)
		# Aight now, y'all ready for a little magic?
		# GIMP doesn't have a way for us to test whether the font has a glyph for
		# a given character code. HOWEVER, we can test it ourselves, by first of all,
		# Checking if  gimp_text_fontname() even created a layer:
		if type(charlyr) != type(None):
			# Alpha to selection on the layer:
			pdb.gimp_selection_layer_alpha(charlyr)
			# Merge the layer down (the selection stays):
			glyphlyr = pdb.gimp_image_merge_down(img, charlyr, CLIP_TO_IMAGE)
		
		# And finally, test if the selection is empty. If it is, there's no glyph.
		# If it isn't, we got a glyph.
		# If no floating selection was created, nothing was drawn,
		# and there is no selection.
		# Make a special case for 0x20 (Space: " ")
		if atglyph == 0x20 or not pdb.gimp_selection_is_empty(img):
			metrics.write(unichr(atglyph))
			extents = pdb.gimp_text_get_extents_fontname(unichr(atglyph), fontsize, PIXELS, fontname)
			charlenout += str(glyphsintex)+"="+str(extents[0])+"\n"
			glyphsintex+=1
			cellx+=1
			if cellx >= charsperrow:
				cellx = 0
				celly+=1
				if celly >= rowspertex: break
				metrics.write("\n")
				metrics.write("Line "+str(celly)+"=")
		
		atglyph+=1
	
	# If we didn't use all the rows/columns, crop off the excess
	if(celly == 0): # Only one row
		assert atglyph >= endcode
		pdb.gimp_image_crop(img, (cellx+1)*glyphsizex, glyphsizey, 0, 0)
	elif ( (celly+1)*glyphsizey < totaly):
		assert atglyph >= endcode
		pdb.gimp_image_crop(img, totalx, (celly+1)*glyphsizey, 0, 0)
	
	# Here is where you put your code for dressing the glyphs up. gimp_selection_layer_alpha is your friend.
	
	# Outline
	#otlnlyr = pdb.gimp_layer_copy(glyphlyr, False)
	#pdb.gimp_drawable_set_name(otlnlyr, "Outlines")
	#pdb.gimp_image_add_layer(img, otlnlyr, 1)
	# pdb.gimp_image_set_active_layer(img, otlnlyr)
	#pdb.gimp_selection_layer_alpha(otlnlyr)
	#pdb.gimp_selection_grow(img, 3)
	#pdb.gimp_edit_bucket_fill(otlnlyr, BG_BUCKET_FILL, NORMAL_MODE,100, 255, False, 0, 0)
	
	# Rename the ("new") glyph layer
	pdb.gimp_drawable_set_name(glyphlyr, "Glyphs")
	
	# Save the texture.
	if(celly == rowspertex): # Full texture
		finalfilename = filename+" ["+str(texindex)+"] "+str(charsperrow)+"x"+str(rowspertex)+".png"
	elif(celly > 0): # Came short a few
		finalfilename = filename+" ["+str(texindex)+"] "+str(charsperrow)+"x"+str(celly+1)+".png"
	else: # Only one row
		finalfilename = filename+" ["+str(texindex)+"] "+str(cellx+1)+"x1.png"
	
	drwbl = pdb.gimp_image_merge_visible_layers(img, CLIP_TO_IMAGE)
	pdb.gimp_file_save(img, drwbl, finalfilename, finalfilename)
	
	# All saved up. Blow it out of RAM.
	pdb.gimp_image_delete(img)
	
	# Wrap up metrics for this tex.
	metrics.write("\n")
	metrics.write("\n")
	metrics.write(charlenout)
	metrics.write("\n")
	
	charlenout = ""
	
	return atglyph

register(
	"python_fu_stepmania_font",
	"Generate a bitmap font for StepMania.\nBe patient, this can take a LONG time.",
	"Generate a bitmap font for StepMania",
	"Ben Anderson",
	"Ben Anderson",
	"2008",
	"Generate StepMania font",
	"",
	[
		(PF_FONT,		"fontname",		"Source font",									"Sans"),
		(PF_INT,		"fontsize",		"Size to render font at (in pixels)",			48),
		(PF_INT,		"glyphsizex",		"Cell Size (X) (powers of 2 recommended)",	64),
		(PF_INT,		"glyphsizey",		"Cell Size (Y) (powers of 2 recommended)",	64),
		(PF_STRING,	"sstartcode",		"Character code of first glyph to render",	"0"),
		(PF_STRING,	"sendcode",		"Character code of last glyph to render",		"7F"),
		(PF_INT,		"charsperrow",	"Number of characters per row",				16),
		(PF_INT,		"rowspertex",		"Number of rows per texture",					16),
		(PF_INT,		"border",			"border param to gimp-text-fontname",			6),
		(PF_STRING,	"filename",		"Folder+prefix to save to",						"")
	],
	[],
	smfontgen,
	"<Image>/File/Create/Misc/")

main()
