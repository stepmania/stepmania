local gc = Var("GameCommand");
--
return Def.ActorFrame {
	--
	LoadFont("Common Normal") .. {
		Name="ScrollerItem";
		Text=THEME:GetString("ScreenTitleMenu",gc:GetText());
		InitCommand=function(self)
			ActorUtil.LoadAllCommandsAndSetXY(self,Var "LoadingScreen");
		end;
  };
};