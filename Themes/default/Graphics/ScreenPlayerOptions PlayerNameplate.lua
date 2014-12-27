local PlayerNumber = ...
assert( PlayerNumber )

local bpm_text_zoom = 0.875

local t = Def.ActorFrame {
	LoadActor(THEME:GetPathB("_frame","3x1"),"rounded fill", 192-8) .. {
		OnCommand=cmd(diffuse,color("#333333");diffusealpha,0.875);
	};
	LoadActor(THEME:GetPathB("_frame","3x1"),"rounded gloss", 192-8) .. {
		OnCommand=cmd(diffusealpha,0.125);
	};
	LoadFont("Common Normal") .. {
		Text=ToEnumShortString(PlayerNumber);
		Name="PlayerShortName",
		InitCommand=cmd(x,-104;maxwidth,32),
		OnCommand=cmd(diffuse,PlayerColor(PlayerNumber);shadowlength,1)
	},
	LoadFont("Common Normal") .. {
		Text="50 - 10000";
		Name="BPMRangeOld",
		InitCommand=cmd(x,-40;maxwidth,88/bpm_text_zoom),
		OnCommand=cmd(shadowlength,1;zoom,bpm_text_zoom)
	},
	LoadActor(THEME:GetPathG("_StepsDisplayListRow","arrow")) .. {
		Name="Seperator",
		InitCommand=cmd(x,14)
	},
	LoadFont("Common Normal") .. {
		Text="100 - 200000";
		Name="BPMRangeNew",
		InitCommand=cmd(x,68;maxwidth,88/bpm_text_zoom),
		OnCommand=cmd(shadowlength,1;zoom,bpm_text_zoom;queuecommand,"BPMWillChange"),
		BPMWillNotChangeCommand=cmd(stopeffect),
		BPMWillChangeCommand=cmd(diffuseshift;effectcolor1,Color.White;effectcolor2,Color.Orange)
	}
}

return t