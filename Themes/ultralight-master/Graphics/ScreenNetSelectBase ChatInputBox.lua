local boxWidth = SCREEN_CENTER_X*0.9
local boxHeight = SCREEN_HEIGHT*0.1
local boxColor = color("0,0,0.625,0.375")
return Def.Quad{ InitCommand=cmd(zoomto,boxWidth,boxHeight;diffuse,boxColor;diffusebottomedge,Brightness(boxColor,0.5)); };