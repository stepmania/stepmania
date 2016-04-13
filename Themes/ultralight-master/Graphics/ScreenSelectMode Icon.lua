-- I'm really only using this for an explanation (right now, anyways)
local gc = Var("GameCommand");

local t = Def.ActorFrame{
	Font("mentone","24px")..{
		Name="Explanation";
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y*1.5625;zoom,0.8;strokecolor,color("0,0,0,0");diffusebottomedge,HSV(192,0.3,0.9));
		GainFocusCommand=function(self)
			self:stoptweening();
			self:sleep(0.125);
			self:diffusealpha(0);
			self:settext( THEME:GetString("ScreenSelectMode",gc:GetName().."Explanation") );
			self:bounceend(0.25);
			self:diffusealpha(1);
		end;
		LoseFocusCommand=cmd(stoptweening;bouncebegin,0.25;diffusealpha,0;);
		OffCommand=cmd(linear,0.1;zoomx,0.5;linear,0.3825;rotationy,-80;zoomx,0;y,SCREEN_BOTTOM+SCREEN_CENTER_Y);
	};
};

return t;