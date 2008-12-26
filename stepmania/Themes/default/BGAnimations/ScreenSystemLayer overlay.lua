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
	LoadFont(Var "LoadingScreen","message") .. {
		InitCommand=cmd(maxwidth,750;
		horizalign,left;vertalign,top;
		shadowlength,2;shadowcolor,color("#000000");
		y,SCREEN_TOP+20;
		diffusealpha,0
		);

		SystemMessageMessageCommand = function(self, params)
			self:settext( params.Message );
			local f = cmd(finishtweening;x,SCREEN_LEFT+20;diffusealpha,1;addx,-SCREEN_WIDTH;linear,0.5;addx,SCREEN_WIDTH); f(self);
			self:playcommand( "On" );
			if params.NoAnimate then
				self:finishtweening();
			end
			f = cmd(sleep,5;linear,0.5;diffusealpha,0); f(self);
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
