local t = Def.ActorFrame{};

-- todo: add event mode indicators and such
if GAMESTATE:IsEventMode() then
	t[#t+1] = LoadFont("Common normal")..{
		Text=Screen.String("EventMode");
		InitCommand=function(self)
			self:CenterX();
			self:y(SCREEN_BOTTOM - 72);
			self:zoom(0.75);
			self:diffuse(HSV(56,0.8,1));
			self:shadowlength(1);
		end;
	};
end;

return t;