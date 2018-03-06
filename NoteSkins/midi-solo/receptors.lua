return function(button_list, stepstype, skin_parameters)
	local ret= {}
	local rots= {
		Left= 90, Down= 0, Up= 180, Right= 270,
		UpLeft= 90, UpRight= 180, 
		DownLeft= 0, DownRight= 270,
	}
	local tap_redir= {
		Left= "down", Right= "down", Down= "down", Up= "down", 
		UpLeft= "down", UpRight= "down", -- shared for dance and pump
		DownLeft= "down", DownRight= "down"
	}
	for i, button in ipairs(button_list) do
		ret[i]= Def.ActorFrame {
			BeatUpdateCommand= function(self, param)
				if param.pressed then
					self:playcommand("Press")
				elseif param.lifted then
					self:playcommand("Lift")
				end
			end;
			
			Def.ActorFrame {
				PressCommand= function(self)
					self:zoom(.85):linear(.12):zoom(1)
				end,
				
				Def.Sprite {
					Texture= tap_redir[button].." receptor (doubleres).png", 
					InitCommand= function(self)
					self:rotationz(rots[button] or 0):SetAllStateDelays(1)
					end,
				};
				
				Def.Sprite {
					Texture= tap_redir[button].." receptor (doubleres).png", 
					InitCommand= function(self)
					self:rotationz(rots[button] or 0):SetAllStateDelays(1)
							:blend('BlendMode_Add')
							:effectclock("beatnooffset"):diffuseramp()
							:effectcolor1(1,1,1,1):effectcolor2(1,1,1,0)
							:effecttiming(.2,0,.8,0)
					end,
				};
			};
			
			Def.Sprite{
				Texture= tap_redir[button].." flash.png", 
				InitCommand= function(self)
					self:rotationz(rots[button] or 0):SetAllStateDelays(1)
					:blend('BlendMode_Add'):diffusealpha(0)
				end,
				PressCommand= function(self)
					self:finishtweening():zoom(0.85):decelerate(0.12):zoom(1):diffusealpha(0.6)
				end,			
				LiftCommand= function(self)
					self:finishtweening():accelerate(0.12):zoom(1.2):diffusealpha(0)
				end,
			};
		};
	end
	return ret
end
