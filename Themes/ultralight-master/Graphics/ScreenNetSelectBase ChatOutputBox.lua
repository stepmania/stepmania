local boxWidth = SCREEN_CENTER_X*0.9
local boxHeight = SCREEN_CENTER_Y
local boxColor = color("0.625,0,0,0.375")
return Def.Quad{ InitCommand=cmd(zoomto,boxWidth,boxHeight;diffuse,boxColor;diffusebottomedge,Brightness(boxColor,0.5)); };