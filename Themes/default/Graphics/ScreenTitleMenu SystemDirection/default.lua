local t = Def.ActorFrame {};
local tInfo = {
	{"EventMode","Stages"},
	{"LifeDifficulty","Life"},
	{"TimingDifficulty","Difficulty"},
};
local fSpacingX = 72;

local function MakeIcon( sTarget )
	local t = Def.ActorFrame {
		LoadActor(THEME:GetPathG("MenuTimer","Frame"));
		LoadFont("Common Normal") .. {
			Text=sTarget[2];
			InitCommand=cmd(y,24+2;zoom,0.5;shadowlength,1);
		};
		--
		LoadFont("Common Normal") .. {
			Text="TEST";
-- 			Text=( PREFSMAN:GetPreference("EventMode") ) and "∞" or PREFSMAN:GetPreference("SongsPerPlay");
			OnCommand=cmd(settext,"4");
			Condition=sTarget[1] == "EventMode";
		};
		--
--[[ 		for i=1,8 do
			t[#t+1] = Def.Quad {
				InitCommand=cmd(vertalign,bottom;zoomto,4,10+(i*4));
			};
		end --]]
	};
	return t
end;

for i=1,#tInfo do
	t[#t+1] = MakeIcon( tInfo[i] ) .. {
		InitCommand=cmd(x,(i-1)*fSpacingX);
	};
end

return t

