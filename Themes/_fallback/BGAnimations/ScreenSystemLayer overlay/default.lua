local function CreditsText( pn )
	local text = LoadFont(Var "LoadingScreen","credits") .. {
		InitCommand=function(self)
			self:name("Credits" .. PlayerNumberToString(pn))
			ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen");
		end;
		UpdateTextCommand=function(self)
			local str = ScreenSystemLayerHelpers.GetCreditsMessage(pn);
			self:settext(str);
		end;
		UpdateVisibleCommand=function(self)
			local screen = SCREENMAN:GetTopScreen();
			local bShow = true;
			if screen then
				local sClass = screen:GetName();
				bShow = THEME:GetMetric( sClass, "ShowCreditDisplay" );
			end

			self:visible( bShow );
		end
	};
	return text;
end;
--
local t = Def.ActorFrame {}
	-- Aux
t[#t+1] = LoadActor(THEME:GetPathB("ScreenSystemLayer","aux"));
	-- Credits
t[#t+1] = Def.ActorFrame {
 	CreditsText( PLAYER_1 );
	CreditsText( PLAYER_2 ); 
};
	-- Text
t[#t+1] = Def.ActorFrame {
	Def.Quad {
		InitCommand=function(self)
			self:zoomtowidth(SCREEN_WIDTH);
			self:zoomtoheight(30);
			self:horizalign(left);
			self:vertalign(top);
			self:y(SCREEN_TOP);
			self:diffuse(color("0,0,0,0"));
		end;
		OnCommand=function(self)
			self:finishtweening();
			self:diffusealpha(0.85);
		end;
		OffCommand=function(self)
			self:sleep(3);
			self:linear(0.5);
			self:diffusealpha(0);
		end;
	};
	LoadFont("Common","Normal") .. {
		Name="Text";
		InitCommand=function(self)
			self:maxwidth(750);
			self:horizalign(left);
			self:vertalign(top);
			self:y(SCREEN_TOP + 10);
			self:x(SCREEN_LEFT + 10);
			self:shadowlength(1);
			self:diffusealpha(0);
		end;
		OnCommand=function(self)
			self:finishtweening();
			self:diffusealpha(1);
			self:zoom(0.5);
		end;
		OffCommand=function(self)
			self:sleep(3);
			self:linear(0.5);
			self:diffusealpha(0);
		end;
	};
	SystemMessageMessageCommand = function(self, params)
		self:GetChild("Text"):settext( params.Message );
		self:playcommand( "On" );
		if params.NoAnimate then
			self:finishtweening();
		end
		self:playcommand( "Off" );
	end;
	HideSystemMessageMessageCommand = function(self)
		self:finishtweening();
	end;
};

return t;
