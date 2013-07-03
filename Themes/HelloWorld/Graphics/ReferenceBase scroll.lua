local gc = Var("GameCommand")
local itemName = gc:GetName()

local t = Def.ActorFrame{
	-- focused frame
	LoadActor( THEME:GetPathG("ReferenceBase","ItemFocused") )..{
		GainFocusCommand=function(self)
			self:decelerate(0.25);
			self:diffusealpha(1);
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

	Def.ActorFrame{
		LoadFont("_frutiger roman 24px")..{
			Name="SectionName";
			Text=ScreenString(itemName);
			InitCommand=function(self)
				self:x(-250);
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
			Text=ScreenString(itemName.."Desc");
			InitCommand=function(self)
				self:x(-248);
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
	};
};

return t;