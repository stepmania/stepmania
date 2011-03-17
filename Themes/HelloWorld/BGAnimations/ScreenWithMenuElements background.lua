-- this file returns a single quad that represents the background of most screens in the theme.
return Def.Quad{
	-- FullScreen is defined in Themes/_fallback/Scripts/02 Actor.lua in sm-ssc.

	-- colors can either be entered as hex ("#FFFFFFFF") or 0..1 ("1.0,1.0,1.0,1.0")
	-- where the values are Red, Green, Blue, and Alpha (transparency).
	InitCommand=cmd(FullScreen;diffuse,color("#fffdf2"));
};