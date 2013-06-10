local t = Def.ActorFrame { };

t[#t+1] = LoadActor( THEME:GetPathB("ScreenWithMenuElements","underlay") );

t[#t+1] = LoadActor( THEME:GetPathB("", "_moveon") ) .. {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y);
};

t[#t+1] = Def.Actor {
	BeginCommand=function(self)
		if SCREENMAN:GetTopScreen():HaveProfileToSave() then
			self:sleep(0.3);
		end;
		self:queuecommand("Save");
	end;
	SaveCommand=function()
		SCREENMAN:GetTopScreen():Continue();
	end;
};

return t;

