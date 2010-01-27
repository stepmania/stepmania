local t = Def.ActorFrame{};

if not GAMESTATE:IsCourseMode() then return t; end;

t[#t+1] = Def.Sprite {
	InitCommand=cmd(Center);
	BeforeLoadingNextCourseSongMessageCommand=function(self) self:LoadFromSongBackground( SCREENMAN:GetTopScreen():GetNextCourseSong() ) end;
	StartCommand=cmd(cropleft,1;fadeleft,1;cropright,0;faderight,0;scaletoclipped,SCREEN_WIDTH,SCREEN_HEIGHT;linear,1;fadeleft,0;cropleft,0;sleep,1);
	FinishCommand=cmd(linear,1;cropright,1;faderight,1;diffusealpha,PREFSMAN:GetPreference("BGBrightness"));
};

t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(Center);
  OnCommand=cmd(stoptweening;addx,30;linear,3;addx,-30);
	LoadFont("Common Normal") .. {
		Text=GAMESTATE:IsCourseMode() and GAMESTATE:GetCurrentCourse():GetDisplayFullTitle() or GAMESTATE:GetCurrentSong():GetDisplayFullTitle();
		InitCommand=cmd(strokecolor,Color("Outline");y,-10);
		OnCommand=cmd(faderight,1;diffusealpha,0;linear,0.5;faderight,0;diffusealpha,1;sleep,1.5;linear,0.5;diffusealpha,0);
	};
--[[ 	LoadFont("Common Normal") .. {
		Text=GAMESTATE:IsCourseMode() and GAMESTATE:GetCurrentCourse():GetCourseType() or GAMESTATE:GetCurrentSong():GetDisplayArtist();
		InitCommand=cmd(strokecolor,Color("Outline");zoom,0.75);
		OnCommand=cmd(faderight,1;diffusealpha,0;linear,0.5;faderight,0;diffusealpha,1;sleep,1.5;linear,0.5;diffusealpha,0);
	}; --]]
	LoadFont("Common Normal") .. {
		InitCommand=cmd(strokecolor,Color("Outline");diffuse,Color("Orange");diffusebottomedge,Color("Yellow");zoom,0.75;y,10);
		BeginCommand=function(self)
			local NextSong = SCREENMAN:GetTopScreen():GetNextCourseSong();
			self:settext( SecondsToMSSMsMs( NextSong:MusicLengthSeconds() ) );
		end;
		OnCommand=cmd(faderight,1;diffusealpha,0;linear,0.5;faderight,0;diffusealpha,1;sleep,1.5;linear,0.5;diffusealpha,0);
	};
};

return t;