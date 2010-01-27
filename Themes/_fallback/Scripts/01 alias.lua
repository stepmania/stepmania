--[[ sm-ssc aliases (non-compatibility)
This is mainly here for making commands case-insensitive without needing to
clutter up the C++ code. It can also be used to add custom functions that
wouldn't otherwise belong somewhere else.
--]]

-- in fact, this probably belongs in Sprite.lua...
function Sprite:cropto(w,h)
	self:CropTo(w,h);
end;