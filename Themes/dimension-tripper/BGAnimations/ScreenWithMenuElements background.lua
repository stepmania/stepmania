local bgcolor = color("1,0.8,0.115")
local lightcolor = color("1,0.935,0.25")

local t = Def.ActorFrame {}

t[#t+1] = Def.Quad {}
t[#t].InitCommand=cmd(FullScreen;diffuse,bgcolor)

t[#t+1] = Def.Quad {}
t[#t].InitCommand=cmd(zoomto,SCREEN_WIDTH,60;CenterX;diffuse,lightcolor;diffusealpha,0.5)
t[#t].OnCommand=cmd(y,224;linear,4;addy,420;sleep,0.5;queuecommand,"On")

t[#t+1] = Def.Quad {}
t[#t].InitCommand=cmd(diffuse,lightcolor;zoomto,SCREEN_WIDTH,224;xy,0,168;horizalign,left)

local j = Def.ActorFrame {}

for i=1,20 do
	j[#j+1] = Def.Quad {}
	j[#j].InitCommand=cmd(zoomto,math.random(5,25),60;x,math.random(SCREEN_WIDTH);vertalign,top;diffuse,lightcolor)
	j[#j].OnCommand=cmd(linear,math.random(0.75,1.25);x,math.random(SCREEN_WIDTH);queuecommand,"On")
end

t[#t+1] = j

return t