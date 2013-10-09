local c = {
	color("#33B5E5"),
	color("#0099CC"),
	color("#AA66CC"),
	color("#9933CC"),
	color("#99CC00"),
	color("#669900"),
	color("#FFBB33"),
	color("#FF8800"),
	color("#FF4444"),
	color("#CC0000"),
}
local t = Def.ActorFrame {};

for i=1,#c do 
	t[#t+1] = Def.Quad {
		InitCommand=cmd(zoomto,32,32;x,32*((i-1)%2);y,32*math.floor((i-1)/2));
		OnCommand=cmd(diffuse,c[i];shadowlength,1.5);
	};
end

t.OnCommand=cmd(x,96;y,128;shadowlength,1.5);

return t;