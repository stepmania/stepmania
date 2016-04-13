-- detail labels (because I'm lazy (again))

local labelZoom = 0.5;
local labelBeginY = met("ScreenEvaluation","DetailFrameRowYBegin");
local labelOffsetY = met("ScreenEvaluation","DetailFrameRowYOffset");

local t = Def.ActorFrame{};

local function DetailForPlayer(pn)
	local beginX = pn == PLAYER_1 and met("ScreenEvaluation","JudgmentP1X") or met("ScreenEvaluation","JudgmentP2X");
	return Def.ActorFrame{
		InitCommand=cmd(x,beginX-112);
		BeginCommand=function(self)
			self:visible(IsPlayerValid(pn));
		end;
		Font("mentone","24px")..{
			InitCommand=cmd(y,labelBeginY+labelOffsetY;halign,0;shadowlength,1;zoom,labelZoom;draworder,200;strokecolor,color("0,0,0,0"));
			Text=THEME:GetString("RadarCategory","Taps");
		};
		Font("mentone","24px")..{
			InitCommand=cmd(y,labelBeginY+(labelOffsetY*2);halign,0;shadowlength,1;zoom,labelZoom;draworder,200;strokecolor,color("0,0,0,0"));
			Text=THEME:GetString("RadarCategory","Jumps");
		};
		Font("mentone","24px")..{
			InitCommand=cmd(y,labelBeginY+(labelOffsetY*3);halign,0;shadowlength,1;zoom,labelZoom;draworder,200;strokecolor,color("0,0,0,0"));
			Text=THEME:GetString("RadarCategory","Holds");
		};
		Font("mentone","24px")..{
			InitCommand=cmd(y,labelBeginY+(labelOffsetY*4);halign,0;shadowlength,1;zoom,labelZoom;draworder,200;strokecolor,color("0,0,0,0"));
			Text=THEME:GetString("RadarCategory","Mines");
		};
		Font("mentone","24px")..{
			InitCommand=cmd(y,labelBeginY+(labelOffsetY*5);halign,0;shadowlength,1;zoom,labelZoom;draworder,200;strokecolor,color("0,0,0,0"));
			Text=THEME:GetString("RadarCategory","Hands");
		};
		Font("mentone","24px")..{
			InitCommand=cmd(y,labelBeginY+(labelOffsetY*6);halign,0;shadowlength,1;zoom,labelZoom;draworder,200;strokecolor,color("0,0,0,0"));
			Text=THEME:GetString("RadarCategory","Rolls");
		};
	};
end;

for pn in ivalues(PlayerNumber) do
	if IsPlayerValid(pn) then
		t[#t+1] = DetailForPlayer(pn);
	end;
end

return t;