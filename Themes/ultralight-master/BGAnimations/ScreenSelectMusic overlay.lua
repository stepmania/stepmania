-- this is only used for post-selection so far
local t = Def.ActorFrame{};

t[#t+1] = Def.Banner{
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_TOP-128;visible,false);
	SetCommand=function(self)
		if GAMESTATE:IsCourseMode() then
			if GAMESTATE:GetCurrentCourse() then
				self:LoadFromCourse(GAMESTATE:GetCurrentCourse());
			end;
		else
			if GAMESTATE:GetCurrentSong() then
				self:LoadFromSong(GAMESTATE:GetCurrentSong());
			end;
		end;

		local w, h = self:GetWidth(), self:GetHeight();
		local aspect = w/h;
		-- notable banner aspect ratios:
		-- 3.2 (256x80 [ddr]; 512x160 doublesized)
		-- 1.5 (300x200; 204x153 real [pump])
		-- 3.0 (300x100 [pump pro])
		-- 2.54878 (418x164 [itg])
		if h >= 128 then
			-- banner height may be too tall and obscure information, scale it
			local newZoomY = scale(h, 128,200, 80,100);
			local newZoomX = self:GetWidth() * newZoomY/self:GetHeight();
			self:zoomto(newZoomX,newZoomY);
		else
			self:zoomto(w,h)
		end;
	end;
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	ShowPressStartForOptionsCommand=cmd(visible,true;sleep,0.4;decelerate,1;y,SCREEN_CENTER_Y*0.75);
	OffCommand=cmd(sleep,0.75;bouncebegin,0.375;zoomx,0);
};

t[#t+1] = LoadFont("Common normal")..{
	Text=THEME:GetString("ScreenSelectMusic","OptionsMessage");
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y*1.35;vertalign,bottom;NoStroke;shadowlength,1;shadowcolor,color("0,0,0,0.375"));
	OnCommand=cmd(visible,false);
	ShowPressStartForOptionsCommand=cmd(hibernate,0.5;visible,true;zoom,1.5;decelerate,1;zoom,1);
	ShowEnteringOptionsCommand=cmd(settext,THEME:GetString("ScreenSelectMusic","EnteringOptions"););
	OffCommand=cmd(sleep,0.8;bouncebegin,0.375;zoomy,0);
};

-- todo: add player information?

t[#t+1] = LoadFont("Common normal")..{
	Name="Title";
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+12;diffusealpha,0;zoom,1.25;valign,1;strokecolor,color("#00000000"));
	SetCommand=function(self)
		local SongOrCourse, text = nil, "";
		if GAMESTATE:IsCourseMode() then
			SongOrCourse = GAMESTATE:GetCurrentCourse();
		else
			SongOrCourse = GAMESTATE:GetCurrentSong();
		end;
		if SongOrCourse then
			text = SongOrCourse:GetDisplayFullTitle();
		end;
		self:settext(text);
	end;
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	ShowPressStartForOptionsCommand=cmd(linear,1;diffusealpha,1;zoom,1);
	OffCommand=cmd(sleep,1.5;bouncebegin,0.5;zoomy,0);
};

-- todo: localize stage text stuff
t[#t+1] = LoadFont("Common normal")..{
	Name="Secondary";
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+10;diffusealpha,0;zoom,1;valign,0;strokecolor,color("#00000000"));
	SetCommand=function(self)
		local SongOrCourse, text = nil, "";
		if GAMESTATE:IsCourseMode() then
			SongOrCourse = GAMESTATE:GetCurrentCourse();
			if SongOrCourse then
				local stages = SongOrCourse:GetEstimatedNumStages();
				if stages == 1 then
					text = string.format(ScreenString("%i stage"),stages)
				else
					text = string.format(ScreenString("%i stages"),stages)
				end;
			end;
		else
			SongOrCourse = GAMESTATE:GetCurrentSong();
			if SongOrCourse then
				text = SongOrCourse:GetDisplayArtist()
			end;
		end;
		self:settext(text);
	end;
	CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
	CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
	ShowPressStartForOptionsCommand=cmd(linear,1;diffusealpha,1;zoom,0.8);
	OffCommand=cmd(sleep,1.55;bouncebegin,0.5;zoomy,0);
};

return t;