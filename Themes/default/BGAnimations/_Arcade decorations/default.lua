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
	LoadActor(THEME:GetPathB("_frame","3x1"),"rounded fill", 250-32) .. {
		OnCommand=cmd(diffuse,color("#8C1940");diffusealpha,1);
	};
	LoadFont("Common Italic Condensed") .. {
		InitCommand=cmd(zoom,1;shadowlength,1;strokecolor,Color("Outline");diffuse,color("#FAB56B");diffusetopedge,color("#F2D5A2");uppercase,true);
		Text="...";
		OnCommand=cmd(playcommand,"Refresh");
		CoinInsertedMessageCommand=cmd(playcommand,"Refresh");
		CoinModeChangedMessageCommand=cmd(playcommand,"Refresh");
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