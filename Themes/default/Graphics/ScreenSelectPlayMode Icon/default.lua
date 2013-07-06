local gc = Var("GameCommand");
local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
	GainFocusCommand=function(self)
		self:stoptweening();
		self:bob(); -- the heck is this?
		self:effectmagnitude(0, 6, 0);
		self:decelerate(0.05);
		self:zoom(1);
	end;
	LoseFocusCommand=function(self)
		self:stoptweening();
		self:stopeffect();
		self:decelerate(0.1);
		self:zoom(0.6);
	end;

	LoadActor("_background base")..{
		InitCommand=function(self)
			self:diffuse(ModeIconColors[gc:GetName()]);
		end;
	};
	LoadActor("_background effect");
	LoadActor("_gloss");
	LoadActor("_stroke");
	LoadActor("_cutout");

	-- todo: generate a better font for these.
	LoadFont("_helveticaneuelt std extblk cn 42px")..{
		InitCommand=function(self)
			self:y(-12);
			self:zoom(1.1);
			self:diffuse(color("#000000"));
			self:uppercase(true);
			self:settext(gc:GetText());
		end;
		GainFocusCommand=function(self)
			self:diffuse(Color.Black);
			self:stopeffect();
		end;
		LoseFocusCommand=function(self)
			self:diffuse(Color.Black);
			self:stopeffect();
		end;
	};
	LoadFont("_helveticaneuelt std extblk cn 42px")..{
		InitCommand=function(self)
			self:y(27.5);
			self:zoom(0.45);
			self:maxwidth(320 * 1.6);
			self:uppercase(true);
			self:settext(THEME:GetString(Var "LoadingScreen", gc:GetName().."Explanation"));
		end;
		GainFocusCommand=function(self)
			self:diffuse(Color.White);
			self:stopeffect();
		end;
		LoseFocusCommand=function(self)
			self:diffuse(Color.White);
			self:stopeffect();
		end;
	};
	LoadActor("_background base") .. {
		DisabledCommand=function(self)
			self:diffuse(color("0,0,0,0.5"));
		end;
		EnabledCommand=function(self)
			self:diffuse(color("1,1,1,0"));
		end;
	};
};
return t