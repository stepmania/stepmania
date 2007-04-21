local time = THEME:GetMetric( "ScreenMusicScroll", "TimerSeconds" )
local num = time * 2
local fontPath = THEME:GetPathF( "ScreenMusicScroll", "titles" )
local children = {}
for i=1,num do
	local song = SONGMAN:GetRandomSong()
	local color = SONGMAN:GetSongColor( song )
	local t = Def.BitmapText {
		_Level = 1;
		File = fontPath;
		Text = song:GetDisplayFullTitle();
		OnCommand = cmd(zoom,0.7;diffuse,color);
	}
		
	table.insert( children, WrapInActorFrame({t}) )
end

return Def.ActorScroller {
	NumItemsToDraw = 6;
	SecondsPerItem = time / num;
	TransformFunction = function( self, offset, itemIndex, numItems )
		self:y( offset*80 )
	end;
	BeginCommand = cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y;scrollwithpadding,3,3);
	children = children;
}
