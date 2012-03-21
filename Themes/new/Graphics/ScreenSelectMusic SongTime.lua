return Def.ActorFrame {
  CurrentSongChangedMessageCommand=function(self)
    self:aux(0);
    self:playcommand( GAMESTATE:GetCurrentSong():GetTimingData():HasBPMChanges() and "MultipleBPM" or "SingleBPM" );
  end;
  -- BPM Background
  Def.Quad {
    Name="BPMBackground";
    InitCommand=cmd(zoomto,96,32;diffuse,ThemeColor.Secondary;shadowlength,2;shadowcolor,Color.Alpha(ColorDarkTone(ThemeColor.Primary),0.95));
  };
  -- BPM Label
  LoadFont("Common Normal") .. {
    Name="BPMLabel";
    Text="Time";
    InitCommand=cmd(x,-32;y,-8;zoom,0.5);
  };
};