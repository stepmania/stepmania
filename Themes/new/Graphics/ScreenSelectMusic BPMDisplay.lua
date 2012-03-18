local switchTime = 2;
local bThen, bNow, bMultipleBPMs;
local iCurrent = 0;
local Song, BPMs;
local updateTimer = function(self, fDeltaTime)
  local c = self:GetChildren();
  -- use aux as a timer internally.
  self:aux( self:getaux() + fDeltaTime );
  -- debug 
  -- c.BPMText:settextf("d:%0.16f (a:%0.2f)",fDeltaTime,self:getaux());
  bNow = (math.floor( self:getaux() % switchTime ) == 0) and true or false;
  local bChanged = (bThen == bNow);
  -- Toggle current index
  if bChanged then
    iCurrent = 1 - iCurrent;
    if ( BPMs[1] == BPMs[2] ) then
      bMultipleBPMs = false;
      c.BPMText:targetnumber(clamp(BPMs[1],0,9999));
    else
      bMultipleBPMs = true;
      c.BPMText:targetnumber(clamp(BPMs[1+iCurrent],0,9999));
    end;
  end;
  --
  --
  bThen = (math.floor( self:getaux() % 2 ) ~= 0) and true or false;
end;
return Def.ActorFrame {
  BeginCommand=function(self)
    self:SetUpdateFunction( updateTimer );
  end;
  CurrentSongChangedMessageCommand=function(self)
    self:aux(0);
    self:playcommand( GAMESTATE:GetCurrentSong():GetTimingData():HasBPMChanges() and "MultipleBPM" or "SingleBPM" );
  end;
  -- BPM Background
  Def.Quad {
    Name="BPMBackground";
    InitCommand=cmd(zoomto,96,32;diffuse,ThemeColor.Secondary;shadowlength,2;shadowcolor,Color.Alpha(ColorDarkTone(ThemeColor.Primary),0.95));
  };
  -- BPM Multiple Warning 
  Def.Quad {
    Name="BPMFlag";
    InitCommand=cmd(x,-32;y,4;basezoomx,24;basezoomy,4;fadeleft,0.2;faderight,0.2;diffuse,ThemeColor.Primary;thump,1;effectclock,'beatnooffset');
    SingleBPMCommand=cmd(finishtweening;decelerate,0.1;zoom,0;zoomx,8;diffusealpha,0);
    MultipleBPMCommand=cmd(finishtweening;smooth,0.05;zoom,1;diffusealpha,1;);
  };
  -- BPM Label
  LoadFont("Common Normal") .. {
    Name="BPMLabel";
    Text="BPM";
    InitCommand=cmd(x,-32;y,-8;zoom,0.5);
  };
  -- BPM Display
  Def.RollingNumbers {
    Name="BPMText";
    File=THEME:GetPathF("Common","Normal");
    InitCommand=cmd(Load,"RollingNumbersBPMDisplay");
    OnCommand=cmd(horizalign,left;x,-16);
    BeginCommand=cmd(playcommand,"Set");
    CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
    SetCommand=function(self)
      Song = GAMESTATE:GetCurrentSong();
      BPMs = Song:GetDisplayBpms() or {0,0};
      -- reset numbers
      self:targetnumber( clamp(BPMs[1],0,9999) );
      -- (force once)
      if (self:getaux() == 0) then
        self:settext(clamp(BPMs[1],0,9999));
        self:aux( 1 );
      end;
    end;
  };
};