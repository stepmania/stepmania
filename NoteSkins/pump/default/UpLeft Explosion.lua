return Def.ActorFrame {
	NOTESKIN:LoadActor(Var "Button", "Tap Note") .. {
		InitCommand=cmd(blend,"BlendMode_Add";zoom,0;queuecommand,"Glow");
		W1Command=cmd(playcommand,"Glow");
		W2Command=cmd(playcommand,"Glow");
		W3Command=cmd(playcommand,"Glow");
		W4Command=cmd();
		W5Command=cmd();
		HoldingOnCommand=cmd(playcommand,"Glow");
		HitMineCommand=cmd(playcommand,"Glow");
		HeldCommand=cmd(playcommand,"Glow");
		GlowCommand=cmd(setstate,0;finishtweening;diffusealpha,0.95;zoom,1;linear,0.15;zoom,1.15;linear,0.15;diffusealpha,0;zoom,1.30);
	};
	LoadActor("_flash")..{
		InitCommand=cmd(blend,"BlendMode_Add";zoom,0;queuecommand,"Glow");
		W1Command=cmd(playcommand,"Glow");
		W2Command=cmd(playcommand,"Glow");
		W3Command=cmd(playcommand,"Glow");
		W4Command=cmd();
		W5Command=cmd();
		HoldingOnCommand=cmd(playcommand,"Glow");
		HitMineCommand=cmd(playcommand,"Glow");
		HeldCommand=cmd(playcommand,"Glow");
		GlowCommand=cmd(setstate,0;finishtweening;diffusealpha,0.95;zoom,0.6;linear,0.15;zoom,0.65;linear,0.15;diffusealpha,0;zoom,0.7);
	};
	--this is supposed to be the "press" effect
	--[[LoadActor("_receptor parts")..{
		InitCommand=cmd(pause;diffusealpha,0.5;setstate,2);
		ColumnJudgmentMessageCommand=function(self,params)
			self:finishtweening();
			if params.TapNoteScore == 'TapNoteScore_W2' then
				self:zoom(2);
				self:linear(0.3);
				self:zoom(0);
			end
			--cmd(finishtweening;zoom,2;linear,0.3;zoom,1);
		end;
	};]]
	Def.Quad {
		InitCommand=cmd(diffuse,1,1,1,0;zoomto,SCREEN_WIDTH*100,SCREEN_HEIGHT*100;zoomz,SCREEN_WIDTH*SCREEN_HEIGHT);
		HitMineCommand=cmd(finishtweening;diffusealpha,1;linear,0.3;diffusealpha,0);
	};
}