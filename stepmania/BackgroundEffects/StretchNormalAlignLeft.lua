-- Align left when cropping to 4:3.
local Color = color(Var "Color1");

local t = Def.ActorFrame {
	LoadActor(Var "File1") .. {
		OnCommand=cmd(horizalign,left;scaletocover,0,0,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,color(Color);effectclock,"music");
		GainFocusCommand=cmd(play);
		LoseFocusCommand=cmd(pause);
	};
};

return t;
