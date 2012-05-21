return Def.ActorFrame {
  -- Time Background
  Def.Quad {
    Name="TimeBackground";
    InitCommand=cmd(zoomto,96,32;diffuse,ThemeColor.Secondary;shadowlength,2;shadowcolor,Color.Alpha(ColorDarkTone(ThemeColor.Primary),0.95));
  };
  -- Time Label
  LoadFont("Common Normal") .. {
    Name="TimeLabel";
    Text="TIME";
    InitCommand=cmd(x,-32;y,-8;zoom,0.5);
  };
  -- Time Text
  LoadFont("Common Normal") .. {
    Name="BPMText";
    OnCommand=cmd(horizalign,left;x,-46;y,6;zoom,0.75;maxwidth,92/0.75;);
    BeginCommand=cmd(playcommand,"Set");
    CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
    SetCommand=function(self)
      Song = GAMESTATE:GetCurrentSong();
      Time = Song:GetStepsSeconds() or 0;
      Minutes = math.floor( Time / 60 );
      Seconds = tonumber(Time) - tonumber(Minutes*60);
      -- reset numbers
      self:settextf("%02i:%02i",Minutes,Seconds);
    end;
  };
};