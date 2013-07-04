return Def.ActorFrame{
	LoadActor("tapmessage") .. {
		InitCommand=function(self)
			self:x(SCREEN_CENTER_X + 160);
			self:y(SCREEN_CENTER_Y + 40);
			self:diffusealpha(0);
		end;
		ShowCommand=function(self)
			self:linear(0);
			self:diffusealpha(1);
			self:sleep(2);
			self:linear(0);
			self:diffusealpha(0);
		end;
	};
};