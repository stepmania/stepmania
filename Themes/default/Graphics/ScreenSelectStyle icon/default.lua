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
		InitCommand=cmd(x,SCREEN_CENTER_X-150;y,SCREEN_CENTER_Y+190;vertalign,bottom);
		OnCommand=cmd(cropbottom,1;fadebottom,1;linear,.2;cropbottom,0;fadebottom,0);
		OffCommand=cmd(croptop,0;fadetop,0;linear,.2;croptop,1;fadetop,1);
		GainFocusCommand=cmd(visible,true);
		LoseFocusCommand=cmd(visible,false);
	};
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X+156;y,SCREEN_CENTER_Y+52;);
		OnCommand=cmd(addy,280;decelerate,.22;addy,-280);
		OffCommand=cmd(accelerate,.22;addy,280);
		GainFocusCommand=cmd(visible,true);
		LoseFocusCommand=cmd(visible,false);
		LoadActor( "card frame " .. st );
		LoadFont( "_sf square head 26px" ) .. {
			InitCommand=cmd(y,-51;settext,string.upper(gc:GetText());maxwidth,500;shadowlength,0;);
		};
		LoadFont( "_venacti bold 15px" ) .. {
			InitCommand=cmd(horizalign,left;x,-12;y,-2;maxwidth,300;shadowlength,0;diffuse,color("#000000"););
			BeginCommand=function(self)
				if st == "StyleType_OnePlayerTwoSides" then
					self:settext("ONE PLAYER USES\nTWO CONTROLLERS");
				else
					self:settext("EACH PLAYER USES\nONE CONTROLLER");
				end
			end;
		};
		LoadFont( "_venacti bold 15px" ) .. {
			InitCommand=cmd(horizalign,right;x,98;y,42;settext,"MAX STAGE/";maxwidth,300;shadowlength,0;diffuse,color("#32d545"););
		};
		LoadFont( "_venacti bold 15px" ) .. {
			InitCommand=cmd(horizalign,right;x,120;y,42;settext,max_stages;maxwidth,100;shadowlength,0;diffuse,color("#259c33"););
		};
		LoadActor( "card " .. pad_file ) .. {
			InitCommand=cmd(x,-82;y,8;);
		};
	};
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X+20+gc:GetIndex()*106;y,SCREEN_CENTER_Y-90;);
		OnCommand=cmd(runcommandsonleaves,cmd(diffusealpha,0;sleep,gc:GetIndex()/12;linear,.16;diffusealpha,1;););
		OffCommand=cmd(runcommandsonleaves,cmd(diffusealpha,1;sleep,gc:GetIndex()/12;linear,.16;diffusealpha,0;););
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
		LoadFont( "_terminator two 18px" ) .. {
			InitCommand=cmd(y,-28;settext,string.upper(gc:GetText());shadowlength,0;maxwidth,80;);
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