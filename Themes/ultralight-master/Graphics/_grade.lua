local grade = ...
assert(grade, "needs a grade")

-- todo: grade colors and such?
return LoadFont("Common normal")..{
	Text=THEME:GetString("Grade",grade);
	InitCommand=cmd(zoom,0.75;diffuse,color("#000000");shadowlength,0;strokecolor,color("#00000000"));
};