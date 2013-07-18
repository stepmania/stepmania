local gc = Var("GameCommand");
local squareSize = 8; -- was 18

return Def.ActorFrame {
	Def.Quad{
		InitCommand=cmd(x,-12;zoom,squareSize;rotationz,45;diffuse,color("#222222"));
		GainFocusCommand=cmd(stoptweening;accelerate,0.25;zoom,squareSize;rotationz,45;);
		LoseFocusCommand=cmd(stoptweening;decelerate,0.25;zoom,0;rotationz,360+45);
	};
	LoadFont("Common Normal") .. {
		Text=gc:GetText();
		InitCommand=cmd(halign,0;zoom,0.625);
		GainFocusCommand=cmd(stoptweening;decelerate,0.25;diffuse,color("1,1,1,1"));
		LoseFocusCommand=cmd(stoptweening;accelerate,0.25;diffuse,color("0.5,0.5,0.5,1"));
	};
};