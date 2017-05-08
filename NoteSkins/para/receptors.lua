local tns_to_react_to= {
	TapNoteScore_W1= true,
	TapNoteScore_W2= true,
	TapNoteScore_W3= true,
	TapNoteScore_W4= true,
	TapNoteScore_W5= true,
}
return function(button_list)
	local rots= {ParaLeft= 90, ParaUpLeft= 135, ParaUp= 180, ParaUpRight= 225, ParaRight= 270}
	local ret= {}
	for i, button in ipairs(button_list) do
		ret[i]= Def.Sprite{
			Texture= "Down Go Receptor", InitCommand= function(self)
				self:draworder(notefield_draw_order.receptor)
					:rotationz(rots[button])
					:effectclock("beat"):diffuseramp():effectcolor1{.5, .5, .5, 1}
					:effectcolor2{1, 1, 1, 1}
			end,
			WidthSetCommand= function(self, param)
				param.column:set_layer_fade_type(self, "FieldLayerFadeType_Receptor")
			end,
			BeatUpdateCommand= function(self, param)
				-- Uh, the old metrics didn't seem to have a press or lift command,
				-- so I guess the keypress layer is how it reacts? -Kyz
			end,
			ColumnJudgmentCommand= function(self, param)
				-- The old metrics had commands for W1 through W5, but the actor file
				-- didn't load them.  Putting them here anyway. -Kyz
				if tns_to_react_to[param.tap_note_score] then
					self:stoptweening():zoom(1.25):linear(.11):zoom(1)
				end
			end,
		}
	end
	return ret
end
