local t = Def.ActorFrame {};
t.InitCommand=function(self)
	self:name("ArcadeOverlay")
	ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen")
end;
t[#t+1] = Def.ActorFrame {
	Name="ArcadeOverlay.Text";
	InitCommand=function(self)
		ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen")
	end;
	LoadActor(THEME:GetPathG("OptionRowExit","Frame")) .. {
		InitCommand=function(self)
			self:diffuse(Color("Orange"));
			self:diffusealpha(0.35);
		end;
	};
	LoadFont("Common Normal") .. {
		InitCommand=function(self)
			self:zoom(0.75);
			self:shadowlength(1);
			self:glowshift();
			self:strokecolor(Color("Outline"));
			self:diffuse(Color("Orange"));
			self:diffusetopedge(Color("Yellow"));
			self:textglowmode('TextGlowMode_Inner');
		end;
		Text="TESTING";
		OnCommand=function(self)
			self:playcommand("Refresh");
		end;
		CoinInsertedMessageCommand=function(self)
			self:playcommand("Refresh");
		end;
		CoinModeChangedMessageCommand=function(self)
			self:playcommand("Refresh");
		end;
		RefreshCommand=function(self)
			local bCanPlay = GAMESTATE:EnoughCreditsToJoin();
			local bReady = GAMESTATE:GetNumSidesJoined() > 0;
			if bCanPlay or bReady then
				self:settext(THEME:GetString("ScreenTitleJoin","HelpTextJoin"));
			else
				self:settext(THEME:GetString("ScreenTitleJoin","HelpTextWait"));
			end
		end;
	};
};
return t