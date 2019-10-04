-- This is used for the life/timing difficulty displays on the title menu,
-- as well as the current gametype.
local params = ...
return Def.ActorFrame {
	-- Base
	-- todo; make getting the base's image less stupid
	LoadActor(THEME:GetPathG("","ScreenSelectPlayMode Icon/_background base")) .. {
		InitCommand=cmd(zoomto,70,70;diffuse,params.base_color;diffusebottomedge,ColorMidTone(params.base_color))
	},
	-- The wanted value
	LoadFont("Common Normal") .. {
		InitCommand=cmd(diffuse,color("#FFFFFF");diffusealpha,0.85),
		OnCommand=function(self)
			self:settext( params.value_text )
			self:zoom(string.len(params.value_text) > 3 and 0.6 or 1.5)
		end
	},
	-- Label
	LoadFont("_roboto condensed Bold 48px") .. {
		InitCommand=cmd(x,50;zoom,0.35;diffuse,color("#512232");horizalign,left;uppercase,true),
		OnCommand= function(self)
			self:shadowlength(0):settext(params.label_text)
		end,
	}
}
