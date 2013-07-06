local jl = Var "JudgmentLine";

return Def.ActorFrame {
	Def.Quad {
		InitCommand=function(self)
			self:horizalign(right);
			self:zoomto(256,18);
		end;
		OnCommand=function(self)
			self:diffuse(Color("Black"));
			self:fadeleft(1);
		end;
	};
	Def.Quad {
		InitCommand=function(self)
			self:horizalign(left);
			self:zoomto(256,18);
		end;
		OnCommand=function(self)
			self:diffuse(Color("Black"));
			self:faderight(1);
		end;
	};
	
	LoadActor("_frame") .. {
		InitCommand=function(self)
			self:diffuse(JudgmentLineToColor(jl));
		end;
	};
	LoadFont("Common Normal") .. {
		InitCommand=function(self)
			self:zoom(0.675);
			self:settext(string.upper(JudgmentLineToLocalizedString(jl)));
			self:diffuse(JudgmentLineToColor(jl));
			self:shadowlength(1);
			self:maxwidth(180);
		end;
	};
};