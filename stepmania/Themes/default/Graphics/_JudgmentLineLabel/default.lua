local jl = Var "JudgmentLine";

return Def.ActorFrame {
	LoadActor("frame") .. {
		InitCommand=cmd(y,4;);
	};
	LoadFont("_sf square head 32") .. {
		InitCommand=cmd(settext,string.upper(JudgmentLineToLocalizedString(jl));diffuse,JudgmentLineToColor(jl);strokecolor,JudgmentLineToStrokeColor(jl);shadowlength,0;);
	};
};