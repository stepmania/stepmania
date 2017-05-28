local diffuse = {
	TapNoteScore_W1 = {1.0,1.0,1.0,0.875},
	TapNoteScore_W2 = {1.0,1.0,0.3,0.875},
	TapNoteScore_W3 = {0.0,1.0,0.4,0.875},
	TapNoteScore_W4 = {0.3,0.8,1.0,0.875},
	TapNoteScore_W5 = {0.8,0.0,0.6,0.875}
}

return function(button_list, stepstype, skin_parameters)
	local rots =  {Left= 90, Down= 0, Up= 180, Right= 270, UpLeft= 135, UpRight= 225, DownLeft= 45, DownRight= 315}
	local t = {}
	for i, button in ipairs(button_list) do
		t[i] = Def.ActorFrame{
			InitCommand=cmd(draworder,notefield_draw_order.explosion),
			ColumnJudgmentCommand=function(self,param)
				
				local score = param.tap_note_score
				
				local child
					if param.bright then child = 'bright' else child ='dim' end
					
				if param.hold_note_score then
					self:GetChild(child):blend('BlendMode_Add'):diffuse(diffuse['TapNoteScore_W2']):zoom(1):linear(0.01):zoom(1.1):linear(0.01):diffusealpha(0)
				
				elseif score and diffuse[score] then
					self:GetChild(child):blend('BlendMode_Add'):diffuse(diffuse[score]):zoom(1):linear(0.06):zoom(1.1):linear(0.08):diffusealpha(0)
				end
			end,
			LoadActor("dim explosion")..{
				Name = 'dim',
				InitCommand=cmd(diffusealpha,0;rotationz,rots[button]),
				HoldCommand=function(self,param)
					if param.start then self:blend('BlendMode_Add'):diffuse(diffuse['TapNoteScore_W2']):zoom(1):linear(0.01):zoom(1.1):linear(0.01):diffusealpha(0) end
				end
			},
			LoadActor("bright explosion")..{
				Name = 'bright',
				InitCommand=cmd(diffusealpha,0),
				HoldCommand=function(self,param)
					if param.start then
						self:blend('BlendMode_Add'):pulse():effectmagnitude(1,1.125,1):effectperiod(0.05):effectclock('beatnooffset'):diffusealpha(1)
					elseif param.finished then
						self:stopeffect():diffusealpha(0)
					end
				end
			},
		}
	end
	return t
end
