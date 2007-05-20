local t = Def.ActorFrame {
        LoadSongBackground() .. {
                Condition=not GAMESTATE:IsCourseMode();
                OnCommand=cmd(diffusealpha,1;sleep,.5;linear,.5;diffusealpha,0);
        };
};
local OnCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y+180;diffusealpha,.75;sleep,0.5;linear,0.5;diffusealpha,0);
local function AddChild( name )
        t[#t+1] = LoadActor( "ScreenStage overlay/" .. name .. ".png" ) .. {
                OnCommand=OnCommand;
                Condition=GAMESTATE:GetCurrentStage() == "Stage_" .. name;
        };
end
for i = 1, 6, 1 do
        AddChild( i );
end
AddChild( "Final" );
AddChild( "Extra1" );
AddChild( "Extra2" );
AddChild( "Event" );
AddChild( "Nonstop" );
AddChild( "Oni" );
AddChild( "Endless" );
return t;

