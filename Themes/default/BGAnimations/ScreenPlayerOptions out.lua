if PREFSMAN:GetPreference( "ShowSongOptions" ) ~= "Maybe_Ask" then
	return LoadActor( THEME:GetPathB("Screen", "out") );
end

local t = Def.ActorFrame {
	LoadActor( THEME:GetPathB("Screen", "out") );

	LoadFont( "common normal" ) .. {
		InitCommand=function(self)
			self:settext("Press &START; for more options");
			self:x(SCREEN_CENTER_X);
			self:y(SCREEN_CENTER_Y + 100);
			self:visible(false);
		end;
		AskForGoToOptionsCommand=function(self)
			self:visible(true);
			self:diffusealpha(0);
			self:linear(0.15);
			self:zoomy(1);
			self:diffusealpha(1);
			self:sleep(1);
			self:linear(0.15);
			self:diffusealpha(0);
			self:zoomy(0);
		end;
		GoToOptionsCommand=function(self)
			self:visible(false);
		end;
	};
	LoadFont( "common normal" ) .. {
		InitCommand=function(self)
			self:settext("entering options...");
			self:x(SCREEN_CENTER_X);
			self:y(SCREEN_CENTER_Y + 100);
			self:visible(false);
		end;
		AskForGoToOptionsCommand=function(self)
			self:visible(false);
			self:linear(0.15);
			self:zoomy(1);
			self:diffusealpha(1);
			self:sleep(1);
			self:linear(0.15);
			self:diffusealpha(0);
			self:zoomy(0);
		end;
		GoToOptionsCommand=function(self)
			self:visible(true);
		end;
	};
};

return t;