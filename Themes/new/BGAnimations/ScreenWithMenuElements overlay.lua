local t = Def.ActorFrame {};
local function GetTime(self)
  local c = self:GetChildren();
  local tTime = { Hour = nil, Minute = nil, Second = nil, Append = nil};
  
  if Hour() then tTime.Hour = Hour() else tTime.Hour = 0 end;
  if Minute() then tTime.Minute = Minute() else tTime.Minute = 0 end;
  if Second() then tTime.Second = Second() else tTime.Second = 0 end;
  
  if( Hour() < 12 ) then 
    tTime.Append = "AM" 
  else 
    tTime.Append = "PM" 
  end;
  
  if( Hour() == 0 ) then
    tTime.Hour = 12;
  end;
  
  c.Time:settextf("%02i:%02i:%02i %s",tTime.Hour,tTime.Minute,tTime.Second,tTime.Append);
end;

t[#t+1] = Def.ActorFrame {
  LoadActor(THEME:GetPathB("","_frame 3x3"),"rounded black",108,16) .. {
    Name="Background";
  };
  LoadFont("Common Normal") ..  {
    Text="Test";
    Name="Time";
    InitCommand=cmd(zoom,0.75);
  };
  --
  InitCommand=cmd(x,SCREEN_RIGHT-96;y,24;visible,false);
  BeginCommand=function(self)
    self:SetUpdateFunction( GetTime );
    self:SetUpdateRate( 1/15 );
  end;
};
return t;