return function(button_list, stepstype, skin_parameters)
	local ret= {}
	local rots= {
		Left= 90, Down= 0, Up= 180, Right= 270,
		UpLeft= 90, UpRight= 180, 
		DownLeft= 0, DownRight= 270,  Center= 0
	}
	local tap_redir= {
		Left= "down", Right= "down", Down= "down", Up= "down", 
		UpLeft= "DownLeft", UpRight= "DownLeft", -- shared for dance and pump
		DownLeft= "DownLeft", DownRight= "DownLeft", Center= "Center"
	}
	for i, button in ipairs(button_list) do
		ret[i]= Def.Sprite{
			Texture= tap_redir[button].." receptor (doubleres).png", InitCommand= function(self)
			self:rotationz(rots[button] or 0):SetAllStateDelays(1)
					:effectclock("beat"):diffuseramp()
					:effectcolor1(1,1,1,1):effectcolor2(.8,.8,.8,1)
					:effectperiod(0.5):effecttiming(0.25,0.50,0,0.25):effectoffset(-0.25)
			end,
			ColumnJudgmentCommand= function(self)
				self.none = false
			end,
			BeatUpdateCommand= function(self, param)
				if param.pressed then
					self:zoom(.9):diffusealpha(0.7)
				elseif param.lifted then
					self:stoptweening():zoom(.9):linear(.09):zoom(1):diffusealpha(1)
				end
			end,
		}
	end
	return ret
end
