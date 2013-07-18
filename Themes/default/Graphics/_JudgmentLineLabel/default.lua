local jl = Var "JudgmentLine";

return Def.ActorFrame {
	Def.Quad {
		InitCommand=cmd(horizalign,right;zoomto,256,18);
		OnCommand=cmd(diffuse,Color("Black");fadeleft,1);
	};
	Def.Quad {
		InitCommand=cmd(horizalign,left;zoomto,256,18);
		OnCommand=cmd(diffuse,Color("Black");faderight,1);
	};
	
	LoadActor("_frame") .. {
		InitCommand=cmd(diffuse,JudgmentLineToColor(jl));
	};
	LoadFont("Common Normal") .. {
		InitCommand=cmd(zoom,0.675;settext,string.upper(JudgmentLineToLocalizedString(jl));diffuse,JudgmentLineToColor(jl);shadowlength,1;maxwidth,180);
	};
};