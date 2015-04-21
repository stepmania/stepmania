-- A simple BitmapText example. Puts "Hello World!" in the center of the screen.
-- BitmapTexts put text on the screen (pretty self-explanatory)

Def.BitmapText{
	Font="Common Normal",
	-- Chooses the font for the text. The font needs to be in the theme's Fonts
	-- folder or the fallback theme's Fonts folder. By default, Common Normal
	-- is "_open sans semibold".
	-- File is used in XML files from ITG, instead of Font. It's deprecated.
	-- You shouldn't use it.
	
	Text="Hello World!",
	-- Chooses what the BitmapText will show. Pretty straightforward.

	OnCommand= function(self) self:Center() end,
	-- Puts the text in the center in the center of the screen.
}
