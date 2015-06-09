-- Screen written by Kyzentun, so if it's confusingly advanced/simple, that's
-- why.

-- The names of the elements in this table will be used to know the names of
-- the preferences they affect.
local valtex_buttons= {
	CenterImageAddWidth= {"d", "a"},
	CenterImageAddHeight= {"w", "s"},
	CenterImageTranslateX= {"l", "j"},
	CenterImageTranslateY= {"k", "i"},
}

-- valtex will be used to store the actors that show the current values for
-- easy access.
local valtex= {}

-- change_center_val is a convenience function so that things that need to
-- change a value only need one line of code, and so that if the method for
-- changing a value changes, it only needs to be changed here.
local function change_center_val(valname, amount)
	local new_val= PREFSMAN:GetPreference(valname) + amount
	PREFSMAN:SetPreference(valname, new_val)
	valtex[valname]:settext(new_val)
	update_centering()
end

-- valtext_frames will store the ActorFrames that each name and value actor
-- is inside.  Then each ActorFrame can update widest_name and tell all the
-- frames to adjust their position for the new width.  This is how the text
-- is reasonably centered.
local valtex_frames= {}
local widest_name= 0

-- valtexact is a convenience function because each preference needs the same
-- actors: one for the name and buttons, and one for the value.  This way,
-- there are fewer lines of code, and they all behave the same.
-- The value is left aligned and the name right aligned so they don't overlap.
-- Normally, all text displayed should be translated by using THEME:GetString,
-- but in this case, these are the exact names of the preferences, so it's
-- important for people to be able to find them easily in Preferences.ini by
-- recognizing the name shown on the screen.
local function valtexact(name, x, y, c)
	return Def.ActorFrame{
		InitCommand= function(self)
			valtex_frames[name]= self
			local name_w= self:GetChild("Name"):GetWidth()
			if name_w > widest_name then widest_name= name_w end
			self:y(y)
			for frame_name, frame in pairs(valtex_frames) do
				frame:playcommand("recenter")
			end
		end,
		recenterCommand= function(self)
			self:x(x + (widest_name / 2))
		end,
		Def.BitmapText{
			Font= "Common Normal", Name= "Value", InitCommand= function(self)
				valtex[name]= self
				self:x(4):settext(PREFSMAN:GetPreference(name)):horizalign(left)
			end,
		},
		Def.BitmapText{
			Font= "Common Normal", Name= "Name", InitCommand= function(self)
				self:x(-4):settext(
					name .. "(" .. valtex_buttons[name][1] .. " or " ..
						valtex_buttons[name][2] .. "): "):horizalign(right):diffuse(c)
			end,
		},
	}
end

-- This is the callback that processes input.  It ignores releases so only
-- first press and repeat events are processed.  It uses valtex_buttons to
-- figure out what value was modified.  This way, all the prefs have the same
-- interface, and the definition of that interface is short and simple.
-- One button to increase the value by one, one button to decrease by one.
local function input(event)
	if event.type == "InputEventType_Release" then return false end
	local button= ToEnumShortString(event.DeviceInput.button)
	for valname, button_set in pairs(valtex_buttons) do
		if button == button_set[1] then
			change_center_val(valname, 1)
		elseif button == button_set[2] then
			change_center_val(valname, -1)
		end
	end
end

-- Another convenience function, this time for a simple aligned quad.
local function quaid(x, y, w, h, c, ha, va)
	return Def.Quad{
		InitCommand= function(self)
			self:xy(x, y):setsize(w, h):diffuse(c):horizalign(ha):vertalign(va)
		end
	}
end

-- Simple colors.  Note that the quads and the text are color coded: the text
-- for the prefs that affect the width and the vertical quads on the sides are
-- the same color.  The text for the prefs that affect the height is the same
-- color as the quads at the top and bottom.
local red= color("#ff0000")
local blue= color("#0000ff")

local args= {
	OnCommand= function(self)
		SCREENMAN:GetTopScreen():AddInputCallback(input)
	end,
	-- Note that the quads are aligned.  This way, they are drawn entirely
	-- inside the screen, and their edge starts at the edge of the screen.
	-- This is important because the user needs them to verify that their
	-- settings bring everything into view.
	quaid(0, 0, _screen.w, 1, red, left, top),
	quaid(0, _screen.h, _screen.w, 1, red, left, bottom),
	quaid(0, 0, 1, _screen.h, blue, left, top),
	quaid(_screen.w, 0, 1, _screen.h, blue, right, top),
	-- The text is centered to minimize the chance of some of it being cut off.
	valtexact("CenterImageAddHeight", _screen.cx, _screen.cy-36, red),
	valtexact("CenterImageAddWidth", _screen.cx, _screen.cy-12, blue),
	valtexact("CenterImageTranslateX", _screen.cx, _screen.cy+12, blue),
	valtexact("CenterImageTranslateY", _screen.cx, _screen.cy+36, red),
}

return Def.ActorFrame(args)
