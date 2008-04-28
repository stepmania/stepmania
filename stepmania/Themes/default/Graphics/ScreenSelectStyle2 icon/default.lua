local gc = Var("GameCommand");
local st = gc:GetStyle():GetStyleType();
local master_pn = GAMESTATE:GetMasterPlayerNumber();
local pad_file = "";
if st == "StyleType_OnePlayerOneSide"  or  st == "StyleType_OnePlayerTwoSides" then
	pad_file = st .. " " .. master_pn;
elseif st == "StyleType_TwoPlayersTwoSides" then
	pad_file = st;
else
	assert(0);
end
local max_stages = PREFSMAN:GetPreference( "SongsPerPlay" );

local t = Def.ActorFrame {
	LoadActor( "preview " .. st ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X-150;y,SCREEN_CENTER_Y+170;vertalign,bottom);
		GainFocusCommand=cmd(visible,true);
		LoseFocusCommand=cmd(visible,false);
	};
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X+156;y,SCREEN_CENTER_Y+52;);
		GainFocusCommand=cmd(visible,true);
		LoseFocusCommand=cmd(visible,false);
		LoadActor( "card frame " .. st ) .. {
		};
		LoadFont( "_sf square head 60" ) .. {
			InitCommand=cmd(y,-51;settext,string.upper(gc:GetText());maxwidth,500;shadowlength,0;);
		};
		LoadFont( "_venacti bold 32" ) .. {
			InitCommand=cmd(horizalign,left;x,-12;y,-2;settext,"EACH PLAYER USES\nONE CONTROLLER";maxwidth,300;shadowlength,0;diffuse,color("#000000"););
		};
		LoadFont( "_venacti bold 32" ) .. {
			InitCommand=cmd(horizalign,right;x,98;y,42;settext,"MAX STAGE/";maxwidth,300;shadowlength,0;diffuse,color("#32d545"););
		};
		LoadFont( "_sf square head 32" ) .. {
			InitCommand=cmd(horizalign,right;x,128;y,40;settext,max_stages;maxwidth,100;shadowlength,0;);
		};
		LoadActor( "card " .. pad_file ) .. {
			InitCommand=cmd(x,-82;y,8;);
		};
	};
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X+20+gc:GetIndex()*106;y,SCREEN_CENTER_Y-90;);
		LoadActor( "icon frame focus" ) .. {
			GainFocusCommand=cmd(visible,true);
			LoseFocusCommand=cmd(visible,false);
		};
		LoadActor( "icon frame nofocus" ) .. {
			GainFocusCommand=cmd(visible,false);
			LoseFocusCommand=cmd(visible,true);
		};
		LoadActor( "icon glow" ) .. {
			InitCommand=cmd(blend,"BlendMode_Add");
			GainFocusCommand=cmd(diffuseshift;visible,true);
			LoseFocusCommand=cmd(stopeffect;visible,false);
		};
		LoadFont( "_terminator two 36" ) .. {
			InitCommand=cmd(y,-30;settext,string.upper(gc:GetText());shadowlength,0;maxwidth,160;strokecolor,color("#000000FF"));
		};
		Def.ActorFrame {
			InitCommand=cmd(y,10;);
			LoadActor( "icon " .. pad_file ) .. {
				InitCommand=cmd(;);
			};
		};
	};
};

return t;