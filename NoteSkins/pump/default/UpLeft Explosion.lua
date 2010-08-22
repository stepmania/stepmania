return Def.ActorFrame {
	--note graphic
	NOTESKIN:LoadActor(Var "Button", "Tap Note") .. {
		InitCommand=cmd(blend,"BlendMode_Add";playcommand,"Glow");
		W1Command=cmd(playcommand,"Glow");
		W2Command=cmd(playcommand,"Glow");
		W3Command=cmd(playcommand,"Glow");
		W4Command=cmd();
		W5Command=cmd();
		--HoldingOnCommand=cmd(playcommand,"Glow");
		HitMineCommand=cmd(playcommand,"Glow");
		GlowCommand=cmd(setstate,0;finishtweening;diffusealpha,1.0;zoom,1.0;linear,0.15;diffusealpha,0.9;zoom,1.15;linear,0.15;diffusealpha,0.0;zoom,1.3);
		HeldCommand=cmd(playcommand,"Glow");
	};
	NOTESKIN:LoadActor(Var "Button", "Ready Receptor")..{
		Name="Tap";
		Frames = { { Frame = 2 } };
		InitCommand=cmd(zoom,1;diffusealpha,0);
		TapCommand=cmd(finishtweening;diffusealpha,1;zoom,1;linear,0.2;diffusealpha,0;zoom,1.2);
		
		--W1Command=cmd(playcommand,"Tap");
		--W2Command=cmd(playcommand,"Tap");
		--W3Command=cmd(playcommand,"Tap");
		--W4Command=cmd(playcommand,"Tap");
		--W5Command=cmd(playcommand,"Tap");
		
		--HitMineCommand=cmd(playcommand,"Tap");
		HeldCommand=cmd(playcommand,"Tap");
		ColumnJudgmentMessageCommand=cmd(playcommand,"Tap");
		TapNoneCommand=cmd(playcommand,"Tap");
		
		--NONECommand=cmd(playcommand,"Tap");
		--HeldCommand=NOTESKIN:GetMetricA(Var "Button", "TapHeldCommand");
		
	};
	--explosion
	LoadActor("_flash")..{
		InitCommand=cmd(blend,"BlendMode_Add";playcommand,"Glow");
		W1Command=cmd(playcommand,"Glow");
		W2Command=cmd(playcommand,"Glow");
		W3Command=cmd(playcommand,"Glow");
		W4Command=cmd();
		W5Command=cmd();
		--HoldingOnCommand=cmd(playcommand,"Glow");
		HitMineCommand=cmd(playcommand,"Glow");
		HeldCommand=cmd(playcommand,"Glow");
		GlowCommand=cmd(setstate,0;finishtweening;diffusealpha,1;zoom,1;linear,0.2;diffusealpha,0;zoom,1.2);
	};
	Def.Actor {
		HitMineCommand=function(self) MESSAGEMAN:BroadCast("AMineWasHit"); end;
	};
	--[[Def.Quad {
		InitCommand=function(self)
			local style = GAMESTATE:GetCurrentStyle()GetStyleType()
			local maxzoom = SCREEN_WIDTH
			if style == 'StyleType_OnePlayerTwoSides' then
				maxzoom = 320
			end
			
			self:diffuse(1,1,1,0);
			self:zoomx(maxzoom);
			self:zoomy(SCREEN_HEIGHT*5)
		end;
		--cmd(diffuse,1,1,1,0;zoomy,SCREEN_HEIGHT*100);
		HitMineCommand=function(self) MESSAGEMAN:BroadCast("AMineWasHit"); end;
		--cmd(finishtweening;diffusealpha,1;linear,0.3;diffusealpha,0);
	};]]
}