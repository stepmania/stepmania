return Def.ActorFrame {
	BeginCommand=function(self)
		self:visible(false);
		if SCREENMAN:GetTopScreen():HaveProfileToSave() then
			self:visible(true);
			self:sleep(1);
		end;
		self:queuecommand("Save");
	end;
	SaveCommand=function(self)
		SCREENMAN:GetTopScreen():Continue();
	end;
	--
	Draw.Box(SCREEN_WIDTH,50)..{
		InitCommand=function(self) self:Center(); end;
	};
	LoadFont("_arial black 20px")..{
		Text="Saving Profiles...";
		InitCommand=function(self)
			self:Center();
			self:skewx(-0.1);
			self:diffuse(color("#000000"));
			self:Stroke(color("#ffffff"));
		end;
	};
}