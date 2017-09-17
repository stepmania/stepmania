if IsNetSMOnline() then
	-- don't show "Ready" online; it will obscure the immediately-starting steps.
	return Def.ActorFrame{}
end

return LoadActor("ready") .. {
	InitCommand=cmd(Center;draworder,105);
	StartTransitioningCommand=cmd(zoom,1.3;diffusealpha,0;bounceend,0.25;zoom,1;diffusealpha,1;linear,0.15;glow,BoostColor(Color("Orange"),1.75);decelerate,0.3;glow,1,1,1,0;sleep,1-0.45;linear,0.25;diffusealpha,0;);
};