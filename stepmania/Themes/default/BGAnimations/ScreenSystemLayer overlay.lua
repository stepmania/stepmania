local function CreditsText( pn )
	function update(self)
		local str = ScreenSystemLayerHelpers.GetCreditsMessage(pn);
		self:settext(str);
	end

	function UpdateVisible(self)
		local screen = SCREENMAN:GetTopScreen();
		local bShow = true;
		if screen then
			local sClass = screen:GetName();
			bShow = THEME:GetMetric( sClass, "ShowCoinsAndCredits" );
		end

		self:visible( bShow );
	end

	local text = LoadFont(Var "LoadingScreen","credits") .. {
		InitCommand=THEME:GetMetric(Var "LoadingScreen","CreditsInitCommand");
		RefreshCreditTextMessageCommand=update;
		CoinInsertedMessageCommand=update;
		PlayerJoinedMessageCommand=update;
		ScreenChangedMessageCommand=UpdateVisible;
	};
	return text;
end

local t = Def.ActorFrame {
	Def.ActorFrame {
		Def.Quad {
			InitCommand=cmd(zoomtowidth,SCREEN_WIDTH;zoomtoheight,30;horizalign,left;vertalign,top;y,SCREEN_TOP;diffusetopedge,color("#FFFFFF");diffusebottomedge,color("#000000");diffusealpha,0;);
			OnCommand=cmd(finishtweening;x,SCREEN_LEFT;diffusealpha,0.3;addx,-SCREEN_WIDTH;linear,0.5;addx,SCREEN_WIDTH;);		
			OffCommand=cmd(sleep,3;linear,0.5;diffusealpha,0;);
		};
		LoadFont(Var "LoadingScreen","SystemMessage") .. {
			Name="Text";
			InitCommand=cmd(maxwidth,750;horizalign,left;vertalign,top;y,SCREEN_TOP+10;strokecolor,color("#000000");shadowlength,0;diffusealpha,0;);
			OnCommand=cmd(finishtweening;x,SCREEN_LEFT+10;diffusealpha,1;addx,-SCREEN_WIDTH;linear,0.5;addx,SCREEN_WIDTH;);
			OffCommand=cmd(sleep,3;linear,0.5;diffusealpha,0;);
		};
		SystemMessageMessageCommand = function(self, params)
			self:GetChild("Text"):settext( params.Message );
			self:playcommand( "On" );
			if params.NoAnimate then
				self:finishtweening();
			end
			self:playcommand( "Off" );
		end;
		HideSystemMessageMessageCommand = cmd(finishtweening);
	};
	CreditsText( PLAYER_1 ) .. {
		InitCommand=cmd(x,THEME:GetMetric(Var "LoadingScreen","CreditsP1X");y,THEME:GetMetric(Var "LoadingScreen","CreditsP1Y"););
	};
	CreditsText( PLAYER_2 ) .. {
		InitCommand=cmd(x,THEME:GetMetric(Var "LoadingScreen","CreditsP2X");y,THEME:GetMetric(Var "LoadingScreen","CreditsP2Y"););
	};
};
return t;
