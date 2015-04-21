-- A simple RollingNumbers example, which takes 10 seconds to go from 0 to 1000.
-- RollingNumbers are interesting (and kind of lame) in the sense that
-- the way they are loaded is through metrics.
-- To test this, put this in your metrics ini:

[RollingNumbersExample]
Fallback="RollingNumbers"

TextFormat="%04.0f"
ApproachSeconds=10
Commify=true
LeadingZeroMultiplyColor=Color.Orange

-- Be sure to reload the metrics if StepMania is currently open.

-- Let's go over the parts:
-- TextFormat: How many trailing zeroes you want, using the format "xx.0f", where xx
-- is the number.
-- ApproachSeconds: How many seconds it takes to reach the target number (explained later)
-- Commify: Whether or not to use commas every 3 digits, i.e. 1,000 vs 1000.
	-- Commify can only be true or false.
-- LeadingZeroMultiplyColor: The color of the closest zero to the number.
-- For example, if you were to make LeadingZeroMultiplyColor red, and
-- your RollingNumber was at 100 with a TextFormat of "%04.0f", then
-- then the zero before the 1 (0100) would be red.

-- Now, for the actual thing:
Def.RollingNumbers{
	-- RollingNumbers derives from BitmapText, so you're gonna need a font.
	Font="Common Normal",

	-- The Text field is ignored.

	-- Let's load the metrics and set our target number
	-- If you don't call Load, your RollingNumbers actor will not be rendered
	-- and you won't be able to set the target number.
	OnCommand= function(self)
		self:Load("RollingNumbersExample"):targetnumber(1000)
			:xy(_screen.cx, _screen.cy - 100)
	end

}
-- Your RollingNumber should start at zero, then take 10 seconds to reach 1000.
