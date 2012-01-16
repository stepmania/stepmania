local bg = Def.ActorFrame{
	Def.Quad{
		InitCommand=cmd(FullScreen;diffuse,color("0,0,0,0"));
		OnCommand=cmd(linear,5;diffusealpha,1);
	};
};

if GAMESTATE:GetPlayMode() == 'PlayMode_Rave' then
	-- find out who won the round
	local raveWin, raveFile;
	if GAMESTATE:IsWinner(PLAYER_1) then
		raveFile = THEME:GetPathG("_rave result","P1")
	elseif GAMESTATE:IsWinner(PLAYER_2) then
		raveFile = THEME:GetPathG("_rave result","P2")
	else
		raveFile = THEME:GetPathG("_rave result","draw")
	end

	raveWin = LoadActor(raveFile)..{
		InitCommand=cmd(Center;cropbottom,1;fadebottom,1;);
		OnCommand=cmd(sleep,2;linear,0.5;cropbottom,0;fadebottom,0;sleep,1.75;linear,0.25;diffusealpha,0);
	};

	table.insert(bg,raveWin)
end

return bg