return Def.ActorFrame {
	LoadActor(THEME:GetPathG("OptionRowExit","frame")) .. {
		InitCommand=function(self)
			self:diffusebottomedge(Color("Orange"));
		end;
	};
	LoadActor(THEME:GetPathG("_icon","Sort")) .. {
		InitCommand=function(self)
			self:x(-60);
			self:shadowlength(1);
			self:diffuse(Color("Orange"));
			self:diffusetopedge(Color("Yellow"));
		end;
	};
};