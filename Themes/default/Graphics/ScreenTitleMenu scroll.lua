local gc = Var("GameCommand");
local s = THEME:GetString( 'ScreenTitleMenu', gc:GetText() );
local t = Def.ActorFrame {
	LoadFont("_venacti bold 24px") ..{
		InitCommand=cmd(uppercase,true;settext,s;horizalign,center;shadowlengthx,0;shadowlengthy,2;strokecolor,color("#00000044"););
		GainFocusCommand=cmd(stoptweening;diffuseshift;effectperiod,0.5;effectcolor1,0.5,1,0.5,1;effectcolor2,0.25,0.5,0.25,1;);
		LoseFocusCommand=cmd(stoptweening;stopeffect);
		DisabledCommand=cmd(diffuse,0.5,0.5,0.5,1);
	};
};

return t;
