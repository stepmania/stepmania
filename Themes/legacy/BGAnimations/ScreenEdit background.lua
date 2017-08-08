local t = Def.ActorFrame {
   InitCommand=cmd(fov,90);
};
t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
	LoadActor( THEME:GetPathB("ScreenWithMenuElements","background/_bg top") ) .. {
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT);
	};
	Def.Quad{
		InitCommand=cmd(scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;diffuse,color("0.2,0.2,0.2,0"));
		OnCommand=function(self)
			local topScreen = SCREENMAN:GetTopScreen()
			if topScreen then
				local screenName = topScreen:GetName()
				if screenName == "ScreenEdit" or screenName == "ScreenPractice" then
					self:diffusealpha(0.95)
				else
					self:diffusealpha(0.65)
				end;
			end;
		end;
		EditorShowMessageCommand=cmd(stoptweening;linear,0.5;diffusealpha,0.95);
		EditorHideMessageCommand=cmd(stoptweening;linear,0.5;diffusealpha,0.65);
	};
};

return t;
