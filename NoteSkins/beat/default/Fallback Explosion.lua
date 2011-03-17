local t = Def.ActorFrame { };
t[#t+1] = NOTESKIN:LoadActor( Var "Button", "Hold Explosion" ) .. {
	HoldingOnCommand=cmd(visible,true);
	HoldingOffCommand=cmd(visible,false);
	InitCommand=cmd(visible,false;finishtweening;blend,"BlendMode_Add");
	Frames = Sprite.LinearFrames(
		NOTESKIN:GetMetricF( "GhostArrowDim", "HoldFrames" ),
		NOTESKIN:GetMetricF( "GhostArrowDim", "HoldSeconds" ) );
};

t[#t+1] = NOTESKIN:LoadActor( Var "Button", "Roll Explosion" ) .. {
	RollOnCommand=cmd(visible,true);
	RollOffCommand=cmd(visible,false);
	InitCommand=cmd(visible,false;finishtweening;blend,"BlendMode_Add");
	Frames = Sprite.LinearFrames(
		NOTESKIN:GetMetricF( "GhostArrowDim", "HoldFrames" ),
		NOTESKIN:GetMetricF( "GhostArrowDim", "HoldSeconds" ) );
};

t[#t+1] = NOTESKIN:LoadActor( Var "Button", "Tap Explosion Dim" ) .. {
	InitCommand=cmd(diffusealpha,0;blend,"BlendMode_Add");
	HeldCommand=cmd(zoom,1;linear,0.06;zoom,1.1;linear,0.06;diffusealpha,0);
	ColumnJudgmentCommand=function(self, params)
		if params.TapNoteScore == "TapNoteScore_HitMine" then return; end

		(cmd(finishtweening;loop,0;diffusealpha,1;setstate,0;sleep,self:GetAnimationLengthSeconds()-0.001;diffusealpha,0))(self);
	end;
	Frames = Sprite.LinearFrames(
		NOTESKIN:GetMetricF( "GhostArrowDim", "JudgmentFrames" ),
		NOTESKIN:GetMetricF( "GhostArrowDim", "JudgmentSeconds" ) );
};

local mine = NOTESKIN:LoadActor( Var "Button", "HitMine Explosion" ) .. {
	InitCommand=cmd(diffusealpha,0);
	Frames = Sprite.LinearFrames(
		NOTESKIN:GetMetricF( "GhostArrowDim", "MineFrames" ),
		NOTESKIN:GetMetricF( "GhostArrowDim", "MineSeconds" ) );
};

local Next = "1";
t[#t+1] = Def.ActorFrame {
	mine .. { Name="1"; };
	mine .. { Name="2"; };

	ColumnJudgmentCommand=function(self, params)
		if params.TapNoteScore ~= "TapNoteScore_HitMine" then return; end

		local c = self:GetChild(Next);
		Next = Next == "1" and "2" or "1";
		(cmd(stoptweening;setstate,0;diffusealpha,1;sleep,self:GetAnimationLengthSeconds()-0.001;diffusealpha,0))(c);
	end;
};

return t;
