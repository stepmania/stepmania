return Def.ActorFrame {
	FOV=90;
--[[ 	LoadActor("shot") .. {
		InitCommand=cmd(diffusealpha,0;zoom,2;blend,'BlendMode_Add');
		MilestoneCommand=cmd(diffusealpha,0.75;rotationz,0;accelerate,2.5;diffusealpha,0;rotationz,360;zoom,2.5);
	}; --]]
--[[ 	LoadActor("shot") .. {
		InitCommand=cmd(diffusealpha,0;zoom,2;zoomx,-2;blend,'BlendMode_Add');
		MilestoneCommand=cmd(diffusealpha,0.75;rotationz,-360;x,0;linear,2.5;diffusealpha,0;rotationz,0;zoom,2.5);
	}; --]]
	LoadActor(THEME:GetPathG("Combo","100Milestone"));
};
