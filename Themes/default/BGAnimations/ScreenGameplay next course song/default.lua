local t = Def.ActorFrame{};

if not GAMESTATE:IsCourseMode() then return t; end;

t[#t+1] = Def.Sprite {
	InitCommand=function(self)
		self:Center();
	end;
	BeforeLoadingNextCourseSongMessageCommand=function(self) self:LoadFromSongBackground( SCREENMAN:GetTopScreen():GetNextCourseSong() ) end;
	ChangeCourseSongInMessageCommand=function(self)
		self:scale_or_crop_background();
	end;
	StartCommand=function(self)
		self:diffusealpha(0);
		self:decelerate(0.5);
		self:diffusealpha(1);
	end;
	FinishCommand=function(self)
		self:linear(0.1);
		self:glow(Color.Alpha(Color("White"),0.5));
		self:decelerate(0.4);
		self:glow(Color("Invisible"));
		self:diffusealpha(0);
	end;
};

t[#t+1] = Def.ActorFrame {
	InitCommand=function(self)
		self:Center();
	end;
	OnCommand=function(self)
		self:stoptweening();
		self:addx(30);
		self:linear(3);
		self:addx(-30);
	end;
	LoadFont("Common Normal") .. {
		InitCommand=function(self)
			self:strokecolor(Color("Outline"));
			self:y(-10);
		end;
		BeforeLoadingNextCourseSongMessageCommand=function(self)
			local NextSong = SCREENMAN:GetTopScreen():GetNextCourseSong();
			self:settext( NextSong:GetDisplayFullTitle() );
		end;
		StartCommand=function(self)
			self:faderight(1);
			self:diffusealpha(0);
			self:linear(0.5);
			self:faderight(0);
			self:diffusealpha(1);
			self:sleep(1.5);
			self:linear(0.5);
			self:diffusealpha(0);
		end;
	};
	LoadFont("Common Normal") .. {
		InitCommand=function(self)
			self:strokecolor(Color("Outline"));
			self:diffuse(Color("Orange"));
			self:diffusebottomedge(Color("Yellow"));
			self:zoom(0.75);
			self:y(10);
		end;
		BeforeLoadingNextCourseSongMessageCommand=function(self)
			local NextSong = SCREENMAN:GetTopScreen():GetNextCourseSong();
			self:settext( SecondsToMSSMsMs( NextSong:MusicLengthSeconds() ) );
		end;
		StartCommand=function(self)
			self:faderight(1);
			self:diffusealpha(0);
			self:linear(0.5);
			self:faderight(0);
			self:diffusealpha(1);
			self:sleep(1.5);
			self:linear(0.5);
			self:diffusealpha(0);
		end;
	};
};

return t;