local t = ...;
assert( type(t) == "table" );

return t .. {
	InitCommand=NOTESKIN:GetMetricA('ReceptorArrow', 'InitCommand');
	MissCommand=NOTESKIN:GetMetricA('ReceptorArrow', 'MissCommand');
	NoneCommand=NOTESKIN:GetMetricA('ReceptorArrow', 'NoneCommand');
	HitMineCommand=NOTESKIN:GetMetricA('ReceptorArrow', 'HitMineCommand');
	W5Command=NOTESKIN:GetMetricA('ReceptorArrow', 'W5Command');
	W4Command=NOTESKIN:GetMetricA('ReceptorArrow', 'W4Command');
	W3Command=NOTESKIN:GetMetricA('ReceptorArrow', 'W3Command');
	W2Command=NOTESKIN:GetMetricA('ReceptorArrow', 'W2Command');
	W1Command=NOTESKIN:GetMetricA('ReceptorArrow', 'W1Command');
};

