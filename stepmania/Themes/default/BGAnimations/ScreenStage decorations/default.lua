local t = Def.ActorFrame {};
t[#t+1] = LoadActor("horizon") .. {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
};
t[#t+1] = LoadActor("sun") .. {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
};
local InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
local OnCommand=cmd(cropbottom,1;linear,0.5;cropbottom,0;sleep,10;);
local function AddChild( i )
	t[#t+1] = LoadActor( i ) .. {
		InitCommand=InitCommand;
		OnCommand=OnCommand;
		Condition=GAMESTATE:GetCurrentStage() == i;
	};
end

for i in ivalues(Stage) do
	AddChild( i );
end
return t;
