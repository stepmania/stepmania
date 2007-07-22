return Def.ActorFrame {
	BeginCommand=cmd(queuecommand,"Load");
	LoadCommand=function()
		SCREENMAN:GetTopScreen():Continue();
	end;
};
