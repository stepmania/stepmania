local jl = Var "JudgmentLine";

return Def.ActorFrame {
	LoadActor("_frame") .. {
		InitCommand=cmd(zoomy,0.75;diffuse,JudgmentLineToColor(jl));
	};
	LoadFont("Common Normal") .. {
		InitCommand=cmd(zoom,0.75;settext,string.upper(JudgmentLineToLocalizedString(jl));diffuse,JudgmentLineToColor(jl);strokecolor,JudgmentLineToStrokeColor(jl);shadowlength,0;maxwidth,180);
	};
};