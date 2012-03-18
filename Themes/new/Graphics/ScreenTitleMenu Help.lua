return Def.ActorFrame {
--[[   Def.ActorFrame {
    InitCommand=cmd(y,-4);
    -- 1 1
    Def.ActorFrame {
      -- Icon
      LoadFont("Common Normal") .. {
        Text="&MENULEFT;&MENURIGHT;";
        InitCommand=cmd(x,32;y,-64;
          diffuse,ThemeColor.Primary;
          zoom,0.675;
        );
      };
      -- Text
      LoadFont("Common Normal") .. {
        Text="LeftHelpDisplayMenuItem1";
        InitCommand=cmd(horizalign,left;
          x,64;y,-64;
          diffuse,ThemeColor.Primary;
          zoom,0.675;
        );
      };
    };
    -- 2 1
    Def.ActorFrame {
      -- Icon
      LoadFont("Common Normal") .. {
        Text="&START;";
        InitCommand=cmd(x,32;y,-40;
          diffuse,ThemeColor.Primary;
          zoom,0.675;
        );
      };
      -- Text
      LoadFont("Common Normal") .. {
        Text="LeftHelpDisplayMenuItem2";
        InitCommand=cmd(horizalign,left;
          x,64;y,-40;
          diffuse,ThemeColor.Primary;
          zoom,0.675;
        );
      };
    };
  }; ]]
};