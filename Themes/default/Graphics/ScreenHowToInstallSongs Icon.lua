local gc = Var "GameCommand";
local Name = gc:GetName();
local Index = gc:GetIndex();

local previewWidth = SCREEN_CENTER_X*0.825;
local previewHeight = SCREEN_CENTER_Y;

local t = Def.ActorFrame{
	Name="PreviewFrame";
	InitCommand=function(self)
		self:x(SCREEN_CENTER_X * 1.5);
		self:y(SCREEN_CENTER_Y * 0.85);
	end;
	OffCommand=function(self)
		self:bouncebegin(0.25);
		self:addx(SCREEN_CENTER_X);
	end;
};

local function TitleMenuItem(text,focused)
	if focused == nil then focused = false; end;
	local textColor = focused and color("#FFFFFF") or color("#888888");
	return LoadFont("Common Normal")..{
		Text=text;
		InitCommand=function(self)
			self:zoom(0.45);
			self:strokecolor(Color("Outline"));
			self:diffuse(textColor);
		end;
		GainFocusCommand=function(self)
			self:stoptweening();
			self:decelerate(0.5);
			self:diffusealpha(1);
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:accelerate(0.5);
			self:diffusealpha(0);
		end;
	};
end;

local previews = {
	WhereToFind = Def.ActorFrame{
		LoadActor(THEME:GetPathG("_howto","find"))..{
			InitCommand=function(self)
				self:zoomto(previewWidth, previewHeight);
			end;
			GainFocusCommand=function(self)
				self:stoptweening();
				self:decelerate(0.5);
				self:diffusealpha(1);
			end;
			LoseFocusCommand=function(self)
				self:stoptweening();
				self:accelerate(0.5);
				self:diffusealpha(0);
			end;
		};
	};
	HowToInstall = Def.ActorFrame{
		LoadActor(THEME:GetPathG("_howto","install"))..{
			InitCommand=function(self)
				self:zoomto(previewWidth, previewHeight);
			end;
			GainFocusCommand=function(self)
				self:stoptweening();
				self:decelerate(0.5);
				self:diffusealpha(1);
			end;
			LoseFocusCommand=function(self)
				self:stoptweening();
				self:accelerate(0.5);
				self:diffusealpha(0);
			end;
		};
	};
	AdditionalFolders = Def.ActorFrame{
		Def.Quad{
			InitCommand=function(self)
				self:zoomto(previewWidth, previewHeight);
			end;
			GainFocusCommand=function(self)
				self:stoptweening();
				self:decelerate(0.5);
				self:diffusealpha(1);
			end;
			LoseFocusCommand=function(self)
				self:stoptweening();
				self:accelerate(0.5);
				self:diffusealpha(0);
			end;
		};
		Def.Quad{
			InitCommand=function(self)
				self:y(-previewHeight * 0.45);
				self:diffuse(color("#E0F0F0"));
				self:zoomto(previewWidth, previewHeight * 0.1);
			end;
			GainFocusCommand=function(self)
				self:stoptweening();
				self:decelerate(0.5);
				self:diffusealpha(1);
			end;
			LoseFocusCommand=function(self)
				self:stoptweening();
				self:accelerate(0.5);
				self:diffusealpha(0);
			end;
		};
		LoadFont("Common normal")..{
			InitCommand=function(self)
				self:x(SCREEN_CENTER_X * -0.4);
				self:y(SCREEN_CENTER_Y * -0.475);
				self:zoom(0.625);
				self:halign(0);
				self:valign(0);
				self:diffuse(color("#000000"));
			end;
			BeginCommand=function(self)
				local text = "Preferences.ini";
				self:settext(text);
			end;
			GainFocusCommand=function(self)
				self:stoptweening();
				self:decelerate(0.5);
				self:diffusealpha(1);
			end;
			LoseFocusCommand=function(self)
				self:stoptweening();
				self:accelerate(0.5);
				self:diffusealpha(0);
			end;
		};
		LoadFont("Common normal")..{
			Text="[Options]\nAdditionalCourseFolders=\nAdditionalFolders=\nAdditionalSongFolders=";
			InitCommand=function(self)
				self:x(SCREEN_CENTER_X * -0.4);
				self:y(SCREEN_CENTER_Y * -0.35);
				self:zoom(0.75);
				self:halign(0);
				self:valign(0);
				self:diffuse(color("#000000"));
			end;
			GainFocusCommand=function(self)
				self:stoptweening();
				self:decelerate(0.5);
				self:diffusealpha(1);
			end;
			LoseFocusCommand=function(self)
				self:stoptweening();
				self:accelerate(0.5);
				self:diffusealpha(0);
			end;
		};
	};
	ReloadSongs = Def.ActorFrame{
		LoadActor(THEME:GetPathB("ScreenTitleMenu","background/_bg"))..{
			InitCommand=function(self)
				self:zoomto(previewWidth, previewHeight);
				self:halign(0.5);
				self:valign(0.5);
			end;
			GainFocusCommand=function(self)
				self:stoptweening();
				self:decelerate(0.5);
				self:diffusealpha(1);
			end;
			LoseFocusCommand=function(self)
				self:stoptweening();
				self:accelerate(0.5);
				self:diffusealpha(0);
			end;
		};
		LoadFont("Common normal")..{
			InitCommand=function(self)
				self:zoom(0.4);
				self:maxwidth((previewWidth * 1.6) - 8);
			end;
			BeginCommand=function(self)
				local song = SONGMAN:GetRandomSong();
				self:settext("Loading songs...\n"..song:GetGroupName().."\n"..song:GetDisplayFullTitle());
			end;
			GainFocusCommand=function(self)
				self:stoptweening();
				self:decelerate(0.5);
				self:diffusealpha(1);
			end;
			LoseFocusCommand=function(self)
				self:stoptweening();
				self:accelerate(0.5);
				self:diffusealpha(0);
			end;
		};
	};
	Exit = Def.ActorFrame{
		LoadActor(THEME:GetPathB("ScreenTitleMenu","background/_bg"))..{
			InitCommand=function(self)
				self:zoomto(previewWidth, previewHeight);
				self:halign(0.5);
				self:valign(0.5);
			end;
			GainFocusCommand=function(self)
				self:stoptweening();
				self:decelerate(0.5);
				self:diffusealpha(1);
			end;
			LoseFocusCommand=function(self)
				self:stoptweening();
				self:accelerate(0.5);
				self:diffusealpha(0);
			end;
		};
		LoadActor(THEME:GetPathG("ScreenTitleMenu","logo"))..{
			InitCommand=function(self)
				self:y(-28);
				self:zoom(0.35);
				self:propagate(true);
			end;
			GainFocusCommand=function(self)
				self:stoptweening();
				self:decelerate(0.5);
				self:diffusealpha(1);
			end;
			LoseFocusCommand=function(self)
				self:stoptweening();
				self:accelerate(0.5);
				self:diffusealpha(0);
			end;
		};
		TitleMenuItem("Game Start",true)..{
			InitCommand=function(self)
				self:y(20);
			end;
		};
		TitleMenuItem("Options")..{
			InitCommand=function(self)
				self:y(32);
			end;
		};
		TitleMenuItem("Edit/Share")..{
			InitCommand=function(self)
				self:y(44);
			end;
		};
		TitleMenuItem("Exit")..{
			InitCommand=function(self)
				self:y(56);
			end;
		};
	};
};

t[#t+1] = previews[Name];

t[#t+1] = LoadFont("Common normal")..{
	Name="Explanation";
	--Text="The quick brown fox jumps over the lazy dog ".. Index .." times.";
	Text=Screen.String("Explanation-"..Name);
	-- was x,-(SCREEN_CENTER_X*0.4);y,SCREEN_CENTER_Y*0.525;
	InitCommand=function(self)
		self:x(SCREEN_CENTER_X * -0.8);
		self:y(SCREEN_CENTER_Y * 0.8);
		self:halign(0);
		self:valign(0);
		self:zoom(0.65);
		self:wrapwidthpixels((SCREEN_WIDTH * 0.55) * 1.75);
		self:NoStroke();
		self:shadowlength(1);
	end;
	GainFocusCommand=function(self)
		self:stoptweening();
		self:decelerate(0.5);
		self:diffusealpha(1);
	end;
	LoseFocusCommand=function(self)
		self:stoptweening();
		self:accelerate(0.5);
		self:diffusealpha(0);
	end;
};

return t;