--Sí, ando usando judge premiere para el themekit ¿Y qué?
local player = Var "Player"
-- blah
--if not getenv then getenv = function() return false end end

--frame correspondiente de cada judgment
local TNSframe = {
	TapNoteScore_CheckpointHit = 0;
	TapNoteScore_W1 = 0;
	TapNoteScore_W2 = 0;
	TapNoteScore_W3 = 1;
	TapNoteScore_W4 = 2;
	TapNoteScore_W5 = 3;
	TapNoteScore_Miss = 4;
	TapNoteScore_CheckpointMiss = 4;
}

--frames para RG
local TNSframeReversed = {
	TapNoteScore_CheckpointHit = 4;
	TapNoteScore_W1 = 4;
	TapNoteScore_W2 = 4;
	TapNoteScore_W3 = 3;
	TapNoteScore_W4 = 2;
	TapNoteScore_W5 = 1;
	TapNoteScore_Miss = 0;
	TapNoteScore_CheckpointMiss = 0;
}

return Def.ActorFrame {
	--init
	InitCommand=function(self)
		local this = self:GetChildren()
		this.judgm:pause();
		
		--this.judgm:y(-35);
		--this.combo:y(42);
		
		this.combo:vertalign(top);
		this.label:vertalign(top);
		
		this.judgm:visible(false);
		this.combo:visible(false);
		this.label:visible(false);
		
		--self:runcommandsonleaves(cmd( SetTextureFiltering,false ))
	end;

	--judges
	LoadActor("judge")..{
		Name="judgm";
		NormalCommand=cmd(shadowlength,0;diffusealpha,1;zoomx,0.913;zoomy,1.175;decelerate,0.175;zoomx,0.62;zoomy,0.78;accelerate,0.06;zoomx,0.63;zoomy,0.82;sleep,0.04;diffusealpha,0.5;zoomx,0.97;zoomy,0.82;sleep,0.04;zoomx,0.63;zoomy,0.82;decelerate,0.175;zoomx,0.82;zoomy,0.82;diffusealpha,0)
	};
	--label
	LoadActor("label")..{
		Name="label";
		NormalCommand=cmd(shadowlength,0;diffusealpha,1;zoomx,1.58;zoomy,1.6;y,100;decelerate,0.175;zoomx,1.09;zoomy,1.1;y,70;accelerate,0.06;zoomx,1.14;zoomy,1.1;y,71;sleep,0.04;diffusealpha,0.5;zoomx,1.66;zoomy,1.1;sleep,0.04;zoomx,1.14;zoomy,1.1;decelerate,0.175;zoomx,1.34;zoomy,1.1;diffusealpha,0)
	};
	--combo
	LoadFont("combo")..{
		Name="combo";
		NormalCommand=cmd(shadowlength,0;diffusealpha,1;zoomx,1.58;zoomy,1.6;y,30;decelerate,0.175;zoomx,1.09;zoomy,1.1;y,21;accelerate,0.06;zoomx,1.14;zoomy,1.1;y,22;sleep,0.04;diffusealpha,0.5;zoomx,1.66;zoomy,1.1;sleep,0.04;zoomx,1.14;zoomy,1.1;decelerate,0.175;zoomx,1.34;zoomy,1.1;diffusealpha,0)
	};
	
	--"PERFECT"!
	JudgmentMessageCommand=function(self,param)
		local this = self:GetChildren()
		local iTns = TNSframe[param.TapNoteScore]
		
		if getenv("ReverseGrade"..player) then
			iTns = TNSframeReversed[param.TapNoteScore]
		end
		
		--no player, no job
		if param.Player ~= player then return end
		if param.HoldNoteScore then return end
		
		this.judgm:visible(true);
		this.judgm:setstate(iTns);
		--this.combo:visible(true);
		--this.label:visible(true);
		
		this.judgm:stoptweening();
		this.judgm:queuecommand("Normal");
	end;

	ComboCommand=function(self,param)
		local this = self:GetChildren()
		local combo = param.Misses or param.Combo;
		
		--color misses o RG
		local ccolor
		if param.Misses then
			ccolor = color("1,0,0,1");
			--GradeReverse, combo misses no rojo
			--puedes cambiar userprefs por getenv
			if getenv("ReverseGrade"..player) then
				ccolor = color("1,1,1,1");
			end
		else
			ccolor = color("1,1,1,1");
			--GradeReverse, combo rojo
			if getenv("ReverseGrade"..player) then
				ccolor = color("1,0,0,1");
			end
		end;
		
		--visibilidad
		this.combo:visible(combo >= 4);
		this.combo:stoptweening();
		this.combo:settextf("%03i",combo);
		this.combo:diffuse(ccolor);
		this.combo:queuecommand("Normal");
		
		this.label:visible(combo >= 4);
		this.label:stoptweening();
		this.label:diffuse(ccolor);
		this.label:queuecommand("Normal");
	end;
}