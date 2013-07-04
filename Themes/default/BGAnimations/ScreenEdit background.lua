local t = Def.ActorFrame {
	InitCommand=function(self)
		self:fov(90);
	end;
};
t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:Center();
	end;
	LoadActor( THEME:GetPathB("ScreenWithMenuElements","background/_bg top") ) .. {
		InitCommand=function(self)
			self:scaletoclipped(SCREEN_WIDTH, SCREEN_HEIGHT);
		end;
	};
	Def.Quad{
		InitCommand=function(self)
			self:scaletoclipped(SCREEN_WIDTH, SCREEN_HEIGHT);
			self:diffuse(color("0.2,0.2,0.2,0"));
		end;
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
		EditorShowMessageCommand=function(self)
			self:stoptweening();
			self:linear(0.5);
			self:diffusealpha(0.95);
		end;
		EditorHideMessageCommand=function(self)
			self:stoptweening();
			self:linear(0.5);
			self:diffusealpha(0.95);
		end;
	};
};

return t;
