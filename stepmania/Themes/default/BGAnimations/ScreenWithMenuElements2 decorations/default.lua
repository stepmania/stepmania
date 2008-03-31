local t = LoadActor( THEME:GetPathB('ScreenWithMenuElements','decorations') ) .. {
	LoadActor( THEME:GetPathB('','_standard decoration'), "LeftFrame", "LeftFrame" );
	LoadActor( THEME:GetPathB('','_standard decoration'), "RightFrame", "RightFrame" );
	
	LoadActor( "top" ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_TOP;vertalign,top);
	};
	Def.ActorFrame {
		InitCommand=cmd(x,SCREEN_CENTER_X-284;y,SCREEN_TOP+30);
		LoadActor( "bar" ) .. {
			InitCommand=cmd(horizalign,left;y,4);
		};
		LoadFont( "_sf sports night ns upright 52" ) .. {
			InitCommand=cmd(x,64;y,-7;horizalign,left;shadowlength,0;zoom,0.5;settext,"Select Style";skewx,-0.15);
		};
		LoadFont( "_venacti 24" ) .. {
			InitCommand=cmd(x,64;y,12;horizalign,left;shadowlength,0;zoom,0.5;settext,"I can't wait to see your next moves!");
		};
		LoadActor( "arrow" ) .. {
		};
		LoadActor( "ring" ) .. {
			InitCommand=cmd(x,-1.5);
		};
	};
};

local style = GAMESTATE:GetCurrentStyle();
if style then
	local master_pn = GAMESTATE:GetMasterPlayerNumber();
	local st = style:GetStyleType();
	local pad_file = "";
	if st == "StyleType_OnePlayerOneSide"  or  st == "StyleType_OnePlayerTwoSides" then
		pad_file = st .. " " .. master_pn;
	elseif st == "StyleType_TwoPlayersTwoSides" then
		pad_file = st;
	end

	t[#t+1] = LoadActor( "style " .. pad_file ) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X+170;y,SCREEN_CENTER_Y-202;);
	};
end

return t;