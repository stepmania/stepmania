local t = Def.ActorFrame {};

-- Sort order
t[#t+1] = Def.ActorFrame {
    InitCommand=cmd(x,SCREEN_RIGHT-290;y,SCREEN_TOP+49;);
    OffCommand=cmd(linear,0.3;diffusealpha,0;);
	LoadActor(THEME:GetPathG("", "_sortFrame"))  .. {
	    InitCommand=cmd(diffusealpha,0.9;zoom,1.5);
		OnCommand=function(self)
			self:diffuse(ColorMidTone(ScreenColor(SCREENMAN:GetTopScreen():GetName())));
		end
	};

    LoadFont("Common Condensed") .. {
            InitCommand=cmd(zoom,1;diffuse,color("#FFFFFF");diffusealpha,0.85;horizalign,left;addx,-115;);
            OnCommand=cmd(queuecommand,"Set");
            ChangedLanguageDisplayMessageCommand=cmd(queuecommand,"Set");
            SetCommand=function(self)
                self:settext("SORT:");
                self:queuecommand("Refresh");
            end;
    };

    LoadFont("Common Condensed") .. {
          InitCommand=cmd(zoom,1;maxwidth,SCREEN_WIDTH;addx,115;diffuse,color("#FFFFFF");uppercase,true;horizalign,right;maxwidth,157;);
          OnCommand=cmd(queuecommand,"Set");
          SortOrderChangedMessageCommand=cmd(queuecommand,"Set");
          ChangedLanguageDisplayMessageCommand=cmd(queuecommand,"Set");
          SetCommand=function(self)
               local sortorder = GAMESTATE:GetSortOrder();
               if sortorder then
					self:finishtweening();
					self:smooth(0.4);
					self:diffusealpha(0);
                    self:settext(SortOrderToLocalizedString(sortorder));
                    self:queuecommand("Refresh"):stoptweening():diffusealpha(0):smooth(0.3):diffusealpha(1)
				else
					self:settext("");
					self:queuecommand("Refresh");
               end
          end;
    };
};

if not GAMESTATE:IsCourseMode() then
-- CD title
t[#t+1] = Def.ActorFrame {
	Def.Sprite {
		name="CDTitle";
		InitCommand=cmd(x,SCREEN_CENTER_X-40;y,SCREEN_CENTER_Y-140;zoom,0.75;);
		OnCommand=cmd(zoomy,0.3;diffusealpha,0;decelerate,0.3;zoomy,0.75;diffusealpha,1);
		OffCommand=cmd(decelerate,0.3;zoomy,0.3;diffusealpha,0;);
		CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
		SetCommand=function(self)
			local song = GAMESTATE:GetCurrentSong();
			if song then
			if song:HasCDTitle() then
				self:Load(song:GetCDTitlePath())
				self:visible(true)
			else
				self:Load(nil)
				self:visible(false)
			end
			else
				self:Load(nil)
				self:visible(false)
			end
		end;
	};
};
end;

return t