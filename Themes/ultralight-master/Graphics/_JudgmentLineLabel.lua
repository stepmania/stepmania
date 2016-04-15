local jl = Var "JudgmentLine";

return Def.ActorFrame {
	LoadFont("Common Normal") .. {
		InitCommand=cmd(zoom,0.75;settext,string.upper(JudgmentLineToLocalizedString(jl));diffuse,JudgmentLineToColor(jl);strokecolor,JudgmentLineToStrokeColor(jl);shadowlength,0;maxwidth,180);
	};
};