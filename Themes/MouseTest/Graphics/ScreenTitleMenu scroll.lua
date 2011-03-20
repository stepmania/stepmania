local gc = Var("GameCommand");
local index = gc:GetIndex();
local name  = gc:GetName();
local text  = gc:GetText();

local itemColors = {
	HSV(128, 0.6, 0.85),	-- play
	HSV(192, 0.6, 0.85),	-- options
	HSV(160, 0.6, 0.85),	-- customization
	HSV( 64, 0.6, 0.85),	-- edit
	HSV(  0, 0.6, 0.85),	-- exit
};

local t = Def.ActorFrame{
	--[[
	Def.Actor{
		Name="TitleMenuController";
		GainFocusCommand=function(self)
			MESSAGEMAN:Broadcast("TitleChange",{NewColor=itemColors[index+1]});
		end;
	};
	]]
	LoadFont("Common Normal")..{
		Text=Screen.String(text);
		InitCommand=cmd(diffuse,color("1,1,1,1"));
		DisabledCommand=cmd(diffuse,color("0.5,0.5,0.5,0.85"));
		--GainFocusCommand=cmd(stoptweening;linear,0.25;diffuse,itemColors[index+1]);
		GainFocusCommand=cmd(stoptweening;linear,0.25;diffuse,color("1,0.2,0.2,1"));
		LoseFocusCommand=cmd(stoptweening;linear,0.25;diffuse,color("1,1,1,1"));
	};
	--[[ begin triangle ]]
	Def.Quad{
		InitCommand=cmd(x,-SCREEN_WIDTH*0.105;zoomto,8,24;zwrite,true;blend,'BlendMode_NoEffect');
	};
	Def.Quad{
		InitCommand=cmd(x,-SCREEN_WIDTH*0.1;zoomto,12,12;rotationz,45;diffuselowerleft,color("0,0,0,0");ztest,true);
		GainFocusCommand=cmd(stoptweening;visible,true);
		LoseFocusCommand=cmd(stoptweening;visible,false);
	};
	--[[ end triangle ]]
};

return t;