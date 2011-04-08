return LoadFont("Common", "Normal")..{
	Text="EXIT";
	InitCommand=cmd(CenterX);
	GainFocusCommand=cmd(diffuse,color("#FF0000"));
	LoseFocusCommand=cmd(diffuse,color("#FFFFFF"));
}