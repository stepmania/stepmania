return Def.ActorFrame {
  -- Time Background
  Def.Quad {
    Name="SortBackground";
    InitCommand=cmd(zoomto,96,32;diffuse,ThemeColor.Secondary;shadowlength,2;shadowcolor,Color.Alpha(ColorDarkTone(ThemeColor.Primary),0.95));
  };
  -- Time Label
  LoadFont("Common Normal") .. {
    Name="SortLabel";
    Text="SORT";
    InitCommand=cmd(x,-32;y,-8;zoom,0.5);
  };
  -- Time Text
  LoadFont("Common Normal") .. {
    Name="SortText";
    OnCommand=cmd(horizalign,left;x,-46;y,6;zoom,0.75;maxwidth,92/0.75;);
    BeginCommand=cmd(playcommand,"Set");
    SortOrderChangedMessageCommand=cmd(playcommand,"Set";);
    SetCommand=function(self)
      local s = SortOrderToLocalizedString( GAMESTATE:GetSortOrder() );
      -- reset numbers
      self:settext(s);
    end;
  };
};