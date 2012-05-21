local t = Def.ActorFrame {};
-- Background
t[#t+1] = Def.ActorFrame { Name="Background";
  Def.Quad {
    InitCommand=cmd(zoomto,320+8,96;diffuse,ThemeColor.Secondary);
  };
};
-- Radar Text
local function RadarLabel( rc )
  local t = Def.ActorFrame {};
  --
  t[#t+1] = LoadFont("Common Normal") .. { 
    Text=string.upper(THEME:GetString("RadarCategoryShort",ToEnumShortString(rc))); 
    InitCommand=cmd(shadowlength,1;zoom,0.675;horizalign,right); 
  };
  --
  return t;
end;
--
local function RadarItem( pn, rc ) 
  local t = Def.ActorFrame {};
  --
  local function GetRadarValue( _rc, _pn )
    if GAMESTATE:GetCurrentSteps( _pn ) then
      local Steps = GAMESTATE:GetCurrentSteps( _pn );
      local RadarValues = Steps:GetRadarValues( _pn );
      local ThisValue = RadarValues:GetValue( _rc );
      --
      return tonumber(ThisValue);
    else
      return 0;
    end;
  end;
  --
  t[#t+1] = LoadFont("Common Normal") .. { 
    Text=string.format("%04i",GetRadarValue(rc,pn)); 
    InitCommand=cmd(shadowlength,1;x,128;zoom,0.675;diffuse,PlayerColor(pn);playcommand,"Set");
    SetCommand=function(self,params)
      if params.Player == pn then
        self:settextf("%04i",GetRadarValue(rc,pn));
      end;
    end;
  };
  --
  t.CurrentSongChangedMessageCommand=function(self) self:playcommand("Set"); end;
  t.CurrentCourseChangedMessageCommand=function(self) self:playcommand("Set"); end;
  t.CurrentTrailP1ChangedMessageCommand=function(self) self:playcommand("Set",{ Player = PLAYER_1}); end;
  t.CurrentTrailP2ChangedMessageCommand=function(self) self:playcommand("Set",{ Player = PLAYER_2}); end;
  t.CurrentStepsP1ChangedMessageCommand=function(self) self:playcommand("Set",{ Player = PLAYER_1}); end;
  t.CurrentStepsP2ChangedMessageCommand=function(self) self:playcommand("Set",{ Player = PLAYER_2}); end;
  --
  return t;
end;
--
local function RadarPercent( pn, rc ) 
  local t = Def.ActorFrame {};
  --
  local function GetRadarValue( _rc, _pn )
    if GAMESTATE:GetCurrentSteps( _pn ) then
      local Steps = GAMESTATE:GetCurrentSteps( _pn );
      local RadarValues = Steps:GetRadarValues( _pn );
      local ThisValue = RadarValues:GetValue( _rc );
      --
      return tonumber(ThisValue);
    else
      return 0;
    end;
  end;
  --
  t[#t+1] = LoadFont("Common Normal") .. { 
    Text=string.format("%03i%%",math.floor(GetRadarValue(rc,pn)*100)); 
    InitCommand=cmd(shadowlength,1;x,128;zoom,0.675;diffuse,PlayerColor(pn);playcommand,"Set");
    SetCommand=function(self,params)
      if params.Player == pn then
        self:settextf("%03i%%",math.floor(GetRadarValue(rc,pn)*100));
      end;
    end;
  };
  --
  t.CurrentSongChangedMessageCommand=function(self) self:playcommand("Set"); end;
  t.CurrentCourseChangedMessageCommand=function(self) self:playcommand("Set"); end;
  t.CurrentTrailP1ChangedMessageCommand=function(self) self:playcommand("Set",{ Player = PLAYER_1}); end;
  t.CurrentTrailP2ChangedMessageCommand=function(self) self:playcommand("Set",{ Player = PLAYER_2}); end;
  t.CurrentStepsP1ChangedMessageCommand=function(self) self:playcommand("Set",{ Player = PLAYER_1}); end;
  t.CurrentStepsP2ChangedMessageCommand=function(self) self:playcommand("Set",{ Player = PLAYER_2}); end;
  --
  return t;
end;
-- 
for index,rc in ipairs(RadarCategory) do
  if index > 5 then 
    local VisualIndex = index-6
    local VertOffset = ( VisualIndex % 4 ) * 16; 
    local HorizOffset = math.floor( (VisualIndex)/4 ) * 156;
    t[#t+1] = Def.ActorFrame {
      Name="RadarCategoryRow" .. tonumber(index-5);
      InitCommand=cmd(x,-96+HorizOffset;y,-10+VertOffset);
      --
      RadarLabel(rc);
      RadarItem(PLAYER_1, rc) .. { InitCommand=cmd(x,-96-8); };
      RadarItem(PLAYER_2, rc) .. { InitCommand=cmd(x,-64+4); };
    };
  end
end;
--
for index,rc in ipairs(RadarCategory) do
  if index < 6 then 
    local HorizOffset = (index-1) * 64;
    t[#t+1] = Def.ActorFrame {
      Name="RadarCategoryRow" .. tonumber(index-5);
      InitCommand=cmd(x,-148+HorizOffset;y,-33);
      --
      RadarLabel(rc);
      RadarPercent(PLAYER_1, rc) .. { InitCommand=cmd(x,-104;y,-7); };
      RadarPercent(PLAYER_2, rc) .. { InitCommand=cmd(x,-104;y,7); };
    };
  end
end;
--
--[[ for index,rc in ipairs(RadarCategory) do
  if index > 5 then
    t[#t+1] = LoadFont("Common Normal") .. { Text=ToEnumShortString(rc); InitCommand=cmd(zoom,0.5;shadowlength,1;y,(index-5)*10); };
  end
end; ]]
return t