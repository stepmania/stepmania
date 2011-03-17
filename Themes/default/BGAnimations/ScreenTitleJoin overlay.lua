local t = Def.ActorFrame{};

-- todo: add event mode indicators and such
if GAMESTATE:IsEventMode() then
	t[#t+1] = LoadFont("Common normal")..{
		Text=Screen.String("EventMode");
		InitCommand=cmd(CenterX;y,SCREEN_BOTTOM-72;zoom,0.75;diffuse,HSV(56,0.8,1));
	};
end;

return t;