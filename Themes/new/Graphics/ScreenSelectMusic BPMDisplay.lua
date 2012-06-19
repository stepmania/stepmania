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
  -- BPM Label
  LoadActor(THEME:GetPathB("_frame","3x3"),"rounded black",160,20);
  -- BPM Display Low
  Def.RollingNumbers {
    Name="BPMTextLow";
    File=THEME:GetPathF("Common","Normal");
    InitCommand=cmd(Load,"RollingNumbersBPMDisplay");
    OnCommand=cmd(horizalign,left;x,-64;maxwidth,92/0.75;);
    BeginCommand=cmd(playcommand,"Set");
    CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
    SetCommand=function(self);
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
  LoadFont("Common Normal") .. {
    Text="-";
  };
  -- BPM Display High
  Def.RollingNumbers {
    Name="BPMTextHigh";
    File=THEME:GetPathF("Common","Normal");
    InitCommand=cmd(Load,"RollingNumbersBPMDisplay");
    OnCommand=cmd(horizalign,left;x,8;maxwidth,92/0.75;);
    BeginCommand=cmd(playcommand,"Set");
    CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
    HideCommand=cmd(finishtweening;linear,0.2;diffusealpha,0);
    ShowCommand=cmd(finishtweening;decelerate,0.125;diffusealpha,1);
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