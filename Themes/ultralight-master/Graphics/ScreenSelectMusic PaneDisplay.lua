local Player = ...
assert(Player,"Must pass in a player, dingus");

local paneCategoryValues = {
	{ Category = 'RadarCategory_TapsAndHolds', Text = THEME:GetString("RadarCategory","Taps"), Color = color("1,1,1,1") },
	{ Category = 'RadarCategory_Jumps', Text = THEME:GetString("RadarCategory","Jumps"), Color = color("1,1,1,1") },
	{ Category = 'RadarCategory_Holds', Text = THEME:GetString("RadarCategory","Holds"), Color = color("1,1,1,1") },
	{ Category = 'RadarCategory_Mines', Text = THEME:GetString("RadarCategory","Mines"), Color = color("1,1,1,1") },
	{ Category = 'RadarCategory_Hands', Text = THEME:GetString("RadarCategory","Hands"), Color = color("1,1,1,1") },
	{ Category = 'RadarCategory_Rolls', Text = THEME:GetString("RadarCategory","Rolls"), Color = color("1,1,1,1") },
};
-- todo: sm-ssc supports Lifts in PaneDisplay; add them?

local rb = Def.ActorFrame{
	Name="PaneDisplay"..Player;
	BeginCommand=function(self)
		self:visible(GAMESTATE:IsHumanPlayer(Player));
	end;
	PlayerJoinedMessageCommand=function(self,param)
		if param.Player == Player then
			self:visible(true);
		end;
	end;
	PlayerUnjoinedMessageCommand=function(self,param)
		if param.Player == Player then
			self:visible(false);
		end;
	end;
};

local yOffset = 16;	-- vertical offset between rows
local fontZoom = 0.5;			-- font zooming
local rv; -- for storing the RadarValues.

-- was _handelgothic 20px
local labelFont  = "_mods small";
local numberFont = "_mods small";

-- pre-runthrough setup of some very important junk:
local Selection; -- either song or course.
local bIsCourseMode = GAMESTATE:IsCourseMode();
local StepsOrTrail;

for idx, cat in pairs(paneCategoryValues) do
	local paneCategory = cat.Category;
	rb[#rb+1] = Def.ActorFrame{
		-- label
		LoadFont("Common normal")..{
			Text=cat.Text;
			InitCommand=cmd(x,-2;y,((idx-1)*yOffset)-1;halign,1;shadowlength,1;zoom,fontZoom;strokecolor,color("0,0,0,0"));
		};

		LoadFont("Common numbers")..{
			InitCommand=cmd(x,2;y,((idx-1)*yOffset)+2.25;halign,0;shadowlength,1;;zoom,fontZoom;strokecolor,color("0,0,0,0"));
			BeginCommand=cmd(playcommand,"Set");
			SetCommand=function(self)
				local value = 0;

				if bIsCourseMode then
					Selection = GAMESTATE:GetCurrentCourse();
					StepsOrTrail = GAMESTATE:GetCurrentTrail(Player);
				else
					Selection = GAMESTATE:GetCurrentSong();
					StepsOrTrail = GAMESTATE:GetCurrentSteps(Player);
				end;

				if not Selection then value = 0;
				else
					-- we have a selection.
					-- Make sure there's something to grab values from.
					if not StepsOrTrail then value = 0;
					else
						rv = StepsOrTrail:GetRadarValues(Player);
						value = rv:GetValue(paneCategory);
					end;
				end;
				value = value < 0 and "?" or value
				self:settext( value );
			end;
			-- generic song/course changes
			CurrentSongChangedMessageCommand=cmd(playcommand,"Set");
			CurrentCourseChangedMessageCommand=cmd(playcommand,"Set");
			-- player based changes TODO: add P3 and P4
			CurrentStepsP1ChangedMessageCommand=function(self)
				if Player == PLAYER_1 then self:playcommand("Set"); end;
			end;
			CurrentTrailP1ChangedMessageCommand=function(self)
				if Player == PLAYER_1 then self:playcommand("Set"); end;
			end;
			CurrentStepsP2ChangedMessageCommand=function(self)
				if Player == PLAYER_2 then self:playcommand("Set"); end;
			end;
			CurrentTrailP2ChangedMessageCommand=function(self)
				if Player == PLAYER_2 then self:playcommand("Set"); end;
			end;
		};
	};
end;

return rb;
