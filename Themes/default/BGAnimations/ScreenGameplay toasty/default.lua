return Def.ActorFrame {
	StartTransitioningCommand=function(self)
		MESSAGEMAN:Broadcast("Toasty",{ Time = math.random(1,3) });
	end
};