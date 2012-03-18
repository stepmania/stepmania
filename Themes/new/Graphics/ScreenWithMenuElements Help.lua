local t = Def.ActorFrame {};
t[#t+1] = Def.ActorFrame {
  InitCommand=cmd(y,-56;visible,false);
  --
  Def.ActorFrame {
    InitCommand=cmd(x,32);
    LoadActor(THEME:GetPathB("","_frame 3x3"),"rounded black",32,12) .. {
      Name="Background";
    };
    LoadFont("Common Normal") .. {
      Text=string.upper("Info");
      InitCommand=cmd(zoom,0.5);
    };
    --
    LoadFont("Common Normal") .. {
      Text=string.upper("Let's get it on now, select and choose the best ones. Let's get it on now 5 4 3 2 1 ..");
      InitCommand=cmd(horizalign,left;x,32;zoom,0.5;shadowlength,1);
    };
    -- Fade
    Def.Quad {
      InitCommand=cmd(horizalign,left;x,32;zoomto,64,32);
      OnCommand=cmd(diffuse,ThemeColor.Secondary;faderight,1);
    };
  };
  -- 
};
return t;