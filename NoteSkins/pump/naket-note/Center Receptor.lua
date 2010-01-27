local t = ...;
t = Def.ActorFrame {
	children = {
		Def.Sprite {
			Texture=NOTESKIN:GetPath( '_center', 'receptor' );
			Frame0000=0;
			Delay0000=1;
			InitCommand=NOTESKIN:GetMetricA('ReceptorArrow', 'InitCommand');
			NoneCommand=NOTESKIN:GetMetricA('ReceptorArrow', 'NoneCommand');
			PressCommand=NOTESKIN:GetMetricA('ReceptorArrow', 'PressCommand');
		};

	}
}
return t;