local t = Def.ActorFrame {};
local function UpdateTriangleFrame(self)
  local curBPS = GAMESTATE:GetSongBPS() and GAMESTATE:GetSongBPS() or 1;
  local c = self:GetChildren();
  local auxRate = 3;
  --
  c.TriangleFrame:aux( math.mod( c.TriangleFrame:getaux() + ( curBPS * auxRate ), 360) );
  c.TriangleFrame:rotationz( c.TriangleFrame:GetRotationZ() + ( curBPS * 0.75 ) );
  c.TriangleFrame:zoom( 1.125 - ((c.TriangleFrame:getaux() / 360) * 0.35) );
  -- c.TriangleFrame:x( math.cos( math.rad(c.TriangleFrame:getaux()) ) * 8 );
  -- c.TriangleFrame:y(math.sin( math.rad(c.TriangleFrame:getaux()) ) * 8);
end;
t[#t+1] = Def.ActorFrame {
  -- Base
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,top;
      zoomto,SCREEN_WIDTH,48;
      diffuse,ThemeColor.Secondary);
  };
  -- Inner Shadow
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,bottom;
      y,48;
      zoomto,SCREEN_WIDTH,5;
      diffuse,Color('Black');diffusealpha,0.25;
      fadetop,1);
  };
  -- Inner Hard Line
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,bottom;
      y,48;
      zoomto,SCREEN_WIDTH,1;
      diffuse,Color('Black');diffusealpha,0.125;);
  };
  -- Outer White Fade
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,top;
      y,48;
      zoomto,SCREEN_WIDTH,8;
      diffuse,Color('White');diffusealpha,0.125;
      fadebottom,1);
  };
  -- Outer White Line
  Def.Quad {
    InitCommand=cmd(horizalign,left;vertalign,top;
      y,48;
      zoomto,SCREEN_WIDTH,1;
      diffuse,Color('White');diffusealpha,0.125;);
  };
}
t[#t+1] = Def.ActorFrame { Name="TriangleFrame";
  InitCommand=cmd(x,24;y,32);
  BeginCommand=function(self)
    self:SetUpdateFunction( UpdateTriangleFrame );
  end;
  -- Triangle Beaterator
  Def.ActorFrame { 
    Name="TriangleFrame";
--~     OnCommand=cmd(thump,2;effectclock,'beatnooffset';effectmagnitude,1,1.135,1);
    --
    LoadActor(THEME:GetPathG("_primitive","Triangle")) .. {
      Name="Triangle";
      OnCommand=cmd(diffuse,ThemeColor.Primary;zoom,0.75);
    };
  };
};
t[#t+1] = Def.ActorFrame {
  -- Text
  LoadFont("Common Normal") .. {
    Text=string.upper(THEME:GetString( Var "LoadingScreen","HeaderText"));
    InitCommand=cmd(horizalign,left;
      x,16;y,16;
      diffuse,ThemeColor.Primary;
      zoom,0.875;
    );
  };
  -- //
--[[   LoadFont("Common Normal") .. {
    Text="//";
    InitCommand=cmd(horizalign,left;
      x,16;y,32;
      diffuse,color("0.7,0.7,0.7,1");
      zoom,0.5;
    );
  }; ]]
  -- Textem
  LoadFont("Common Normal") .. {
    Text=string.upper("SELECT AND CHOOSE THE BEST ONE");
    InitCommand=cmd(horizalign,left;
      x,34;y,32;
      diffuse,color("0.7,0.7,0.7,1");
      zoom,0.5;
      skewx,-0.125;
    );
  };
};

return t