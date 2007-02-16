function CreditsText( pn )
	function update(self)
		local str = ScreenSystemLayerHelpers.GetCreditsMessage(pn);
		self:settext(str);
	end

	local text = Def.BitmapText {
		Font=THEME:GetPathF("ScreenManager","credits");
		InitCommand=cmd(shadowlength,0);
		RefreshCreditTextMessageCommand=update;
		CoinInsertedMessageCommand=update;
		PlayerJoinedMessageCommand=update;
	};
	return text;
end

local t = Def.ActorFrame {
	children = {
		Def.BitmapText {
			Font=THEME:GetPathF(Var "LoadingScreen","message");
			Text="";
			InitCommand=cmd(maxwidth,750;
			horizalign,left;vertalign,top;
			zoom,0.8;shadowlength,2;
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
			InitCommand=cmd(x,SCREEN_LEFT+120;y,SCREEN_BOTTOM-6);
		};
		CreditsText( PLAYER_2 ) .. {
			InitCommand=cmd(x,SCREEN_RIGHT-120;y,SCREEN_BOTTOM-6);
		};

	};
};
return t;
