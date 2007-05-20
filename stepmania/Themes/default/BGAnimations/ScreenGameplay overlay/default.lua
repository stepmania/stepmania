local t = Def.ActorFrame {
};

if GAMESTATE:IsExtraStage() then
	t[#t+1] = LoadActor("_extra score frame") .. {
		InitCommand=cmd(x,SCREEN_CENTER_X+0;y,SCREEN_CENTER_Y-208);
		OnCommand=cmd(addy,-100;sleep,0.5;linear,1;addy,100);
		OffCommand=cmd(linear,1;addy,-100);
	};
else
	local sFile = GAMESTATE:GetPlayMode() == "PlayMode_Oni" and "_oni score frame" or "_score frame";
	t[#t+1] = LoadActor(sFile) .. {
		InitCommand=cmd(x,SCREEN_CENTER_X+0;y,SCREEN_CENTER_Y+208);
		OnCommand=cmd(addy,100;linear,0.5;addy,-100);
		OffCommand=cmd(linear,0.5;addy,100);
	};
end

return t;
