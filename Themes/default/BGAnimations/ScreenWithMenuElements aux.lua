local function Clock()
	function UpdateVisible(self)
		local screen = SCREENMAN:GetTopScreen()
		local bShow = true
		if screen then
			local sClass = screen:GetName()
			bShow = THEME:GetMetric(sClass, "ShowClock")
		end
		if bShow then
			self:smooth(0.25)
			self:y(12)
		else
			self:smooth(0.25)
			self:y(-56)
		end
	end
	-- clock
	clock = Def.ActorFrame {
		Name="Clock",
		InitCommand=cmd(x,50;y,12;playcommand,"Update"),
		ScreenChangedMessageCommand=UpdateVisible,
		UpdateCommand=cmd(runcommandsonleaves,cmd(queuecommand,"Update")),
		Def.RoundedBox(90,26)..{ InitCommand=cmd(x,-22;y,-4) },
		Def.ActorFrame {
			Name="ClockText",
			InitCommand=cmd(y,-2),
			LoadFont("Common", "normal")..{
				Text="00:00:",
				InitCommand=cmd(horizalign,right;shadowlength,0;diffusebottomedge,color("0.9,0.9,0.9")),
				UpdateCommand=function(self)
					local hour, min = Hour(), Minute()
					if hour > 12 and GetUserPrefB("Use12HourClock") then
						hour = hour - 12
					elseif hour == 0 and GetUserPrefB("Use12HourClock") then
						hour = 12
					end	
					self:settext(string.format('%02i:%02i:', hour, min))
					self:sleep(1)
					self:queuecommand("Update")
				end
			},
			LoadFont("Common", "normal")..{
				Text="00",
				InitCommand=cmd(horizalign,left;shadowlength,0;diffusebottomedge,color("0.9,0.9,0.9")),
				UpdateCommand=function(self)
					local sec = Second()
					self:settext(string.format('%02i', sec))
					self:sleep(1)
					self:queuecommand("Update")
				end,
			},
			LoadFont("Common", "normal")..{
				Text="",
				InitCommand=cmd(x,28;y,-3;horizalign,left;shadowlength,0;diffusebottomedge,color("0.9,0.9,0.9");visible,false;zoom,0.75),
				UpdateCommand=function(self)
					if not GetUserPrefB("Use12HourClock") then
						self:visible(false)
						return
					end
					local hour = Hour()
					if hour < 12 then
						self:settext("AM")
						self:diffuse(color("1,0.85,0,1"))
						self:diffusebottomedge(color("0.75,0.55,0,1"))
					else
						self:settext("PM")
						self:diffuse(color("0,0.85,1,1"))
						self:diffusebottomedge(color("0,0.55,1,1"))
					end
					self:visible(true)
					self:sleep(1)
					self:queuecommand("Update")
				end
			}
		}
	}
	return clock;
end
local t = Def.ActorFrame {};
return t;