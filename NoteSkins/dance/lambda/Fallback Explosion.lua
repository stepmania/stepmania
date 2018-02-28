--If a Command has "NOTESKIN:GetMetricA" in it, that means it gets the command from the metrics.ini, else use cmd(); to define command.
--If you dont know how "NOTESKIN:GetMetricA" works here is an explanation.
--NOTESKIN:GetMetricA("The [Group] in the metrics.ini", "The actual Command to fallback on in the metrics.ini");

--The NOTESKIN:LoadActor() just tells us the name of the image the Actor redirects on.
--Oh and if you wonder about the "Button" in the "NOTESKIN:LoadActor( )" it means that it will check for that direction.
--So you dont have to do "Down" or "Up" or "Left" etc for every direction which will save space ;)
local t = Def.ActorFrame {
	--Hold Explosion Commands
	NOTESKIN:LoadActor( Var "Button", "Hold Explosion" ) .. {
		HoldingOnCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "HoldingOnCommand");
		HoldingOffCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "HoldingOffCommand");
		InitCommand=cmd(playcommand,"HoldingOff";finishtweening);
	};
	--Roll Explosion Commands
	NOTESKIN:LoadActor( Var "Button", "Hold Explosion" ) .. {
		RollOnCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "RollOnCommand");
		RollOffCommand=NOTESKIN:GetMetricA("HoldGhostArrow", "RollOffCommand");
		InitCommand=cmd(playcommand,"RollOff";finishtweening);
	};
	--Dim Explosion Commands
	NOTESKIN:LoadActor( Var "Button", "Tap Explosion Dim" ) .. {
		InitCommand=cmd(diffusealpha,0);
		W5Command=NOTESKIN:GetMetricA("GhostArrowDim", "W5Command");
		W4Command=NOTESKIN:GetMetricA("GhostArrowDim", "W4Command");
		W3Command=NOTESKIN:GetMetricA("GhostArrowDim", "W3Command");
		W2Command=NOTESKIN:GetMetricA("GhostArrowDim", "W2Command");
		W1Command=NOTESKIN:GetMetricA("GhostArrowDim", "W1Command");
		JudgmentCommand=cmd(finishtweening);
		BrightCommand=cmd(visible,false);
		DimCommand=cmd(visible,true);
	};
	--yes yes I know I could do it in another way but I'm lazy and it works doesnt it ;>
	--This code give the Hold OK explosion Dim the same images as Bright
	NOTESKIN:LoadActor( Var "Button", "Tap Explosion Bright" ) .. {
		InitCommand=cmd(diffusealpha,0);
		HeldCommand=NOTESKIN:GetMetricA("GhostArrowDim", "HeldCommand");
		JudgmentCommand=cmd(finishtweening);
		BrightCommand=cmd(visible,false);
		DimCommand=cmd(visible,true);
	};
	--Bright Explosion Commands
	NOTESKIN:LoadActor( Var "Button", "Tap Explosion Bright" ) .. {
		InitCommand=cmd(diffusealpha,0);
		W5Command=NOTESKIN:GetMetricA("GhostArrowBright", "W5Command");
		W4Command=NOTESKIN:GetMetricA("GhostArrowBright", "W4Command");
		W3Command=NOTESKIN:GetMetricA("GhostArrowBright", "W3Command");
		W2Command=NOTESKIN:GetMetricA("GhostArrowBright", "W2Command");
		W1Command=NOTESKIN:GetMetricA("GhostArrowBright", "W1Command");
		HeldCommand=NOTESKIN:GetMetricA("GhostArrowBright", "HeldCommand");
		JudgmentCommand=cmd(finishtweening);
		BrightCommand=cmd(visible,true);
		DimCommand=cmd(visible,false);

	};
	--Mine Explosion Commands
	NOTESKIN:LoadActor( Var "Button", "HitMine Explosion" ) .. {
		InitCommand=cmd(blend,"BlendMode_Add";diffusealpha,0);
		HitMineCommand=NOTESKIN:GetMetricA("GhostArrowBright", "HitMineCommand");
	};
}
return t;