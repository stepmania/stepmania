function Sprite:LoadFromSongBanner(song)
	if song then
		local Path = song:GetBannerPath()
		if not Path then
			Path = THEME:GetPathG("Common","fallback banner")
		end

		self:LoadBanner( Path )
	else
		self:LoadBanner( THEME:GetPathG("Common","fallback banner") )
	end
	return self
end

function Sprite:LoadFromSongBackground(song)
	local Path = song:GetBackgroundPath()
	if not Path then
		Path = THEME:GetPathG("Common","fallback background")
	end

	self:LoadBackground( Path )
	return self
end

function LoadSongBackground()
	return Def.Sprite {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y),
		BeginCommand=cmd(LoadFromSongBackground,GAMESTATE:GetCurrentSong();scale_or_crop_background)
	}
end

function Sprite:LoadFromCurrentSongBackground()
	local song = GAMESTATE:GetCurrentSong()
	if not song then
		local trail = GAMESTATE:GetCurrentTrail(GAMESTATE:GetMasterPlayerNumber())
		local e = trail:GetTrailEntries()
		if #e > 0 then
			song = e[1]:GetSong()
		end
	end

	if not song then return self end

	self:LoadFromSongBackground(song)
	return self
end

function Sprite:position( f )
	self:GetTexture():position( f )
	return self
end

function Sprite:loop( f )
	self:GetTexture():loop( f )
	return self
end

function Sprite:rate( f )
	self:GetTexture():rate( f )
	return self
end

function Sprite.LinearFrames(NumFrames, Seconds)
	local Frames = {}
	for i = 0,NumFrames-1 do
		Frames[#Frames+1] = {
			Frame = i,
			Delay = (1/NumFrames)*Seconds
		}
	end
	return Frames
end

-- command aliases:
function Sprite:cropto(w,h) self:CropTo(w,h) return self end

-- (c) 2005 Glenn Maynard
-- All rights reserved.
-- 
-- Permission is hereby granted, free of charge, to any person obtaining a
-- copy of this software and associated documentation files (the
-- "Software"), to deal in the Software without restriction, including
-- without limitation the rights to use, copy, modify, merge, publish,
-- distribute, and/or sell copies of the Software, and to permit persons to
-- whom the Software is furnished to do so, provided that the above
-- copyright notice(s) and this permission notice appear in all copies of
-- the Software and that both the above copyright notice(s) and this
-- permission notice appear in supporting documentation.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
-- OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
-- MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
-- THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
-- INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
-- OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
-- OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
-- OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
-- PERFORMANCE OF THIS SOFTWARE.
