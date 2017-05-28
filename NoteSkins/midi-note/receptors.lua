return function(button_list, stepstype, skin_parameters)
	local t = {}
	local rots=  {Left= 90, Down= 0, Up= 180, Right= 270, UpLeft= 135, UpRight= 225, DownLeft= 45, DownRight= 315}
	for i, button in ipairs(button_list) do
		t[i] = Def.ActorFrame{
			InitCommand=cmd(rotationz,rots[button];effectclock,'beat';draworder,notefield_draw_order.receptor),
			WidthSetCommand= function(self, param)
				param.column:set_layer_fade_type(self, "FieldLayerFadeType_Receptor")
			end,
			BeatUpdateCommand= function(self, param)
				if param.pressed then
					self:playcommand("Press")
				elseif param.lifted then
					self:playcommand("Lift")
				end
			end,
			ColumnJudgmentCommand= function(self, param)
				local score = param.tap_note_score
				if score == 'TapNoteScore_W1' or score == 'TapNoteScore_W2' or score == 'TapNoteScore_W3' then
					self:playcommand('Lift')
				end
			end,
			
			Def.Sprite{ -- receptor
				Texture="receptor (doubleres)",
				PressCommand=cmd(stoptweening;zoom,0.85;linear,0.12;zoom,1),
				WCommand=cmd(stoptweening;zoom,0.85;linear,0.12;zoom,1)
			},
			Def.Sprite{ -- receptor overlay
				Texture="receptor (doubleres)",
				InitCommand=cmd(blend,"BlendMode_Add";effectclock,"beat";diffuseramp;effectcolor1,color("1,1,1,0");effectcolor2,color("1,1,1,1");effecttiming,.2,0,.8,0;effectoffset,.05),
				PressCommand=cmd(stoptweening;zoom,0.85;linear,0.12;zoom,1),
			},
			Def.Sprite{ -- flash
				Texture="flash",
				InitCommand=cmd(diffusealpha,0;blend,"BlendMode_Add"),
				PressCommand=cmd(finishtweening;zoom,0.85;decelerate,0.12;zoom,1;diffusealpha,0.6;),
				LiftCommand=cmd(finishtweening;diffusealpha,1;zoom,1;accelerate,0.12;diffusealpha,0;zoom,1.2)
			},
		}
	end
	
	return t
end
