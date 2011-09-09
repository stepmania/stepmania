return Def.ActorFrame {
-- 	InitCommand=cmd(x,SCREEN_RIGHT;y,SCREEN_BOTTOM;draworder,101);
	StartTransitioningCommand=function(self)
		MESSAGEMAN:Broadcast("Toasty",{ Time = math.random(1,3) });
	end
};