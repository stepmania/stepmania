local fontPath = THEME:GetPathF( "Common", "normal" )
local songs = tableslice( tableshuffle( SONGMAN:GetAllSongs() ), 100 );
local spacing_y = 24;
local num_items_to_draw = math.ceil( 480 / spacing_y );
local num_padding_items = (num_items_to_draw/2)+2;
local seconds_per_item = 0.1
local begin_fading_out_seconds = (#songs + num_padding_items * 2) * seconds_per_item;

local t = LoadFallbackB();
t[#t+1] = Def.ActorFrame {
	InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;);
	BeginCommand=function(self)
		SCREENMAN:GetTopScreen():PostScreenMessage( "SM_BeginFadingOut", begin_fading_out_seconds );
	end;
	LoadActor("bg") .. {
		InitCommand=cmd();
	};
	LoadActor("char left") .. {
		InitCommand=cmd(x,-250+4;y,50+4;);
	};
	LoadActor("char right") .. {
		InitCommand=cmd(x,250+10;y,50+7;);
	};
};


local t2 = Def.ActorScroller {
	NumItemsToDraw = num_items_to_draw;
	SecondsPerItem = seconds_per_item;
	TransformFunction = function( self, offset, itemIndex, numItems )
		self:y( offset*spacing_y )
	end;
	BeginCommand = cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;scrollwithpadding,num_padding_items,num_padding_items);
}

for i=1,#songs do
	local song = songs[i];
	local c = SONGMAN:GetSongColor( song )
	local text = Def.BitmapText {
		_Level = 1;
		File = fontPath;
		InitCommand = cmd(settext,song:GetDisplayFullTitle();diffuse,c;strokecolor,color("#575100");shadowlength,0;);
	}
		
	table.insert( t2, Def.ActorFrame { text } )
end

t[#t+1] = t2;

return t;
