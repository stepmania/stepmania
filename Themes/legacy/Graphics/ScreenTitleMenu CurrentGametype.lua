local curGameName = GAMESTATE:GetCurrentGame():GetName();

local t = LoadFont("Common Normal") .. {
	BeginCommand=function(self)
		self:settextf( Screen.String("CurrentGametype"), curGameName );
	end;
};
return t;