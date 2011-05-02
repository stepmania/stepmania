return Def.ActorFrame {
	BeginCommand=function(self)
		self:visible(false);
		if SCREENMAN:GetTopScreen():HaveProfileToLoad() then
			self:visible(true);
			self:sleep(1);
		end;
		self:queuecommand("Load");
	end;
	LoadCommand=function(self)
		SCREENMAN:GetTopScreen():Continue();
	end;
	--
	Draw.Box(SCREEN_WIDTH,50)..{
		InitCommand=cmd(Center);
	};
	LoadFont("_arial black 20px")..{
		Text="Loading Profiles...";
		InitCommand=cmd(Center;skewx,-0.1;diffuse,color("#000000");Stroke,color("#ffffff"));
	};
}