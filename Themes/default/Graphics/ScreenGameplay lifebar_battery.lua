local pn = ...
local life_meter_width = 400
local life_meter_height = 20
local life_meter_outline = 0

local function CreateSeperators()
	local t = Def.ActorFrame {}

	for i = 1, 50 do
		t[#t+1] = Def.ActorFrame {
			SetCommand=function(self)
				local life_meter = SCREENMAN:GetTopScreen():GetLifeMeter(pn)
				local num_items = life_meter:GetTotalLives()

				local function position(index)
					return scale(index/num_items,0,1,-life_meter_width/2,life_meter_width/2)
				end

				self:x(position(i))
				self:visible(i < num_items)
			end,
			OnCommand=cmd(playcommand,"Set"),
			--
			Def.Quad {
				InitCommand=cmd(zoomto,2,life_meter_height),
				OnCommand=cmd(diffuse,Color.Black)
			}
		}
	end

	return t
end

local t = Def.ActorFrame {}

t[#t+1] = Def.ActorFrame {
	HealthStateChangedMessageCommand=function(self,param)
		local c = self:GetChildren()

		if param.PlayerNumber == pn then
			if param.HealthState ~= param.OldHealthState then
				local state_name = ToEnumShortString(param.HealthState)
				self:playcommand(state_name)
			end
		end
	end,
	LifeChangedMessageCommand=function(self,param)
		local c = self:GetChildren()
		if param.Player == pn then
			local life = param.LifeMeter:GetLife()
			c.Fill:zoomtowidth( life_meter_width * life )
		end
	end,
	-- Outline
	Def.Quad {
		Name="Outline",
		InitCommand=cmd(zoomto,life_meter_width+life_meter_outline,life_meter_height+life_meter_outline),
		OnCommand=cmd()
	},
	-- Background 
	Def.Quad {
		Name="Background",
		InitCommand=cmd(zoomto,life_meter_width,life_meter_height),
		OnCommand=cmd(diffuse,color("#32373E")),
		AliveCommand=cmd(stopeffect;diffuse,color("#32373E")),
		DangerCommand=cmd(diffuseshift;effectcolor2,ColorMidTone(Color.Red);effectcolor1,ColorDarkTone(Color.Red)),
		DeadCommand=cmd(stopeffect;diffuse,color("#000000")),
	},
	Def.Quad {
		Name="Fill",
		InitCommand=cmd(x,-life_meter_width/2;zoomto,life_meter_width,life_meter_height;horizalign,left),
		OnCommand=cmd(diffuse,PlayerColor(pn)),
		--
		HotCommand=cmd(glowshift;effectclock,'beat'),
		AliveCommand=cmd(stopeffect),
		DangerCommand=cmd(diffuseshift;effectclock,'beat';effectcolor1,PlayerColor(pn);effectcolor2,PlayerDarkColor(pn)),
		DeadCommand=cmd(stopeffect)
	},
	LoadFont("Common Normal") .. {
		Text="",
		OnCommand=cmd(diffuse,Color.Black)
	},
	CreateSeperators()
}

return t