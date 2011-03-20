local curGameName = GAMESTATE:GetCurrentGame():GetName();

local t = Def.ActorFrame{
	LoadFont("Common Normal") .. {
		InitCommand=cmd(horizalign,left;zoom,0.5;);
		BeginCommand=function(self)
			self:settextf( Screen.String("CurrentGametype"), curGameName );
		end;
	};
};
return t;