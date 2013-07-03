local gc = Var("GameCommand")
local itemName = gc:GetName()
local url = gc:GetUrl()

local t = Def.ActorFrame{
	-- focused frame
	LoadActor( THEME:GetPathG("SectionMenu","ItemFocused") )..{
		GainFocusCommand=function(self)
			self:decelerate(0.25);
			self:diffusealpha,(1);
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:decelerate(0.25);
			self:diffusealpha(0);
		end;
	};
	-- unfocused
	LoadActor( THEME:GetPathG("SectionMenu","ItemUnfocused") )..{
		GainFocusCommand=function(self)
			self:decelerate(0.25);
			self:diffusealpha(0);
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:decelerate(0.25);
			self:diffusealpha(1);
		end;
	};

	LoadActor( THEME:GetPathG("_section", "Reference") )..{
		InitCommand=function(self)
			self:x(-240);
			self:y(-23);
			self:Real();
			self:halign(0);
		end;
		GainFocusCommand=function(self)
			self:decelerate(0.25);
			self:diffusealpha(0.9);
		end;
		LoseFocusCommand=function(self)
			self:stoptweening();
			self:decelerate(0.25);
			self:diffusealpha(0.45);
		end;
	};

	Def.ActorFrame{
		LoadFont("_frutiger roman 24px")..{
			Name="SectionName";
			Text=THEME:GetString("ScreenReferenceMain",itemName);
			InitCommand=function(self)
				self:x(-246);
				self:y(-6);
				self:halign(0);
				self:diffuse(color("#222222FF"));
			end;
			GainFocusCommand=function(self)
				self:decelerate(0.25);
				self:diffusealpha(1);
			end;
			LoseFocusCommand=function(self)
				self:stoptweening();
				self:decelerate(0.25);
				self:diffusealpha(0.5);
			end;
		};
		LoadFont("_frutiger roman 24px")..{
			Name="SectionDesc";
			Text=THEME:GetString("ScreenReferenceMain",itemName.."Desc");
			InitCommand=function(self)
				self:x(-244);
				self:y(10);
				self:align(0, 0);
				self:diffuse(color("#222222FF"));
				self:zoom(0.5);
			end;
			GainFocusCommand=function(self)
				self:decelerate(0.25);
				self:diffusealpha(1);
			end;
			LoseFocusCommand=function(self)
				self:stoptweening();
				self:decelerate(0.25);
				self:diffusealpha(0.5);
			end;
		};
		LoadActor(THEME:GetPathG("_tango","webbrowser"))..{
			Name="UrlIcon";
			InitCommand=function(self)
				self:x(224);
				self:y(4);
				self:zoom(0.75);
				self:visible(url~="");
			end;
			GainFocusCommand=function(self)
				self:decelerate(0.25);
				self:diffusealpha(1);
			end;
			LoseFocusCommand=function(self)
				self:stoptweening();
				self:decelerate(0.25);
				self:diffusealpha(0.5);
			end;
		};
	};
};

return t;