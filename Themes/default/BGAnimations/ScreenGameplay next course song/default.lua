local t = Def.ActorFrame{};

if not GAMESTATE:IsCourseMode() then return t; end;

t[#t+1] = Def.Sprite {
	InitCommand=cmd(Center);
	BeforeLoadingNextCourseSongMessageCommand=function(self) self:LoadFromSongBackground( SCREENMAN:GetTopScreen():GetNextCourseSong() ) end;
	ChangeCourseSongInMessageCommand=cmd(scale_or_crop_background);
	StartCommand=cmd(diffusealpha,0;decelerate,0.5;diffusealpha,1;);
	FinishCommand=cmd(linear,0.1;glow,Color.Alpha(Color("White"),0.5);decelerate,0.4;glow,Color("Invisible");diffusealpha,0);
};

t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
  OnCommand=cmd(stoptweening;addx,30;linear,3;addx,-30);
	LoadFont("Common Normal") .. {
		InitCommand=cmd(strokecolor,Color("Outline");y,-10);
		BeforeLoadingNextCourseSongMessageCommand=function(self)
			local NextSong = SCREENMAN:GetTopScreen():GetNextCourseSong();
			self:settext( NextSong:GetDisplayFullTitle() );
		end;
		StartCommand=cmd(faderight,1;diffusealpha,0;linear,0.5;faderight,0;diffusealpha,1;sleep,1.5;linear,0.5;diffusealpha,0);
	};
--[[ 	LoadFont("Common Normal") .. {
		Text=GAMESTATE:IsCourseMode() and GAMESTATE:GetCurrentCourse():GetCourseType() or GAMESTATE:GetCurrentSong():GetDisplayArtist();
		InitCommand=cmd(strokecolor,Color("Outline");zoom,0.75);
		OnCommand=cmd(faderight,1;diffusealpha,0;linear,0.5;faderight,0;diffusealpha,1;sleep,1.5;linear,0.5;diffusealpha,0);
	}; --]]
	LoadFont("Common Normal") .. {
		InitCommand=cmd(strokecolor,Color("Outline");diffuse,Color("Orange");diffusebottomedge,Color("Yellow");zoom,0.75;y,10);
		BeforeLoadingNextCourseSongMessageCommand=function(self)
			local NextSong = SCREENMAN:GetTopScreen():GetNextCourseSong();
			self:settext( SecondsToMSSMsMs( NextSong:MusicLengthSeconds() ) );
		end;
		StartCommand=cmd(faderight,1;diffusealpha,0;linear,0.5;faderight,0;diffusealpha,1;sleep,1.5;linear,0.5;diffusealpha,0);
	};
};

return t;