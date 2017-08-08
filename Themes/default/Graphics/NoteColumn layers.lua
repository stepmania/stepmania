local offset= 52
local hns_states= {
	HoldNoteScore_Held= 0,
	HoldNoteScore_LetGo= 1,
	HoldNoteScore_MissedHold= 1,
}

return {
	Def.Sprite{
		Texture= THEME:GetPathG("HoldJudgment", "label"),
		InitCommand= function(self)
			self:draworder(notefield_draw_order.under_field)
				:y(offset):animate(false):diffusealpha(0)
		end,
		WidthSetCommand= function(self, param)
			scale_to_fit(self, param.width, param.width)
			param.column:set_layer_transform_type(self, "FieldLayerTransformType_PosOnly")
			:set_layer_fade_type(self, "FieldLayerFadeType_Explosion")
		end,
		PlayerStateSetCommand= function(self, param)
			if player_config:get_data(param.PlayerNumber).JudgmentUnderField then
				self:draworder(notefield_draw_order.under_field)
			else
				self:draworder(notefield_draw_order.over_field)
			end
		end,
		ColumnJudgmentCommand= function(self, param)
			local hns= param.hold_note_score
			if hns_states[hns] then
				self:stoptweening()
					:setstate(hns_states[hns])
					:linear(.1):diffusealpha(1)
					:sleep(.5)
					:linear(.2):diffusealpha(0)
			elseif hns then
				lua.ReportScriptError("No sprite state for hns " .. tostring(hns))
			end
		end,
		ReverseChangedCommand= function(self, param)
			self:y(offset * param.sign)
		end,
	},
}
