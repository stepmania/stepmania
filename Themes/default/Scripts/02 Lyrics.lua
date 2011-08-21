function Actor:LyricCommand(side)
	self:settext(Var "LyricText")

	self:stoptweening()
	self:shadowlengthx(0)
	self:shadowlengthy(0)
	self:strokecolor(Color("Outline"))
	self:zoomx(clamp(SCREEN_WIDTH/(self:GetZoomedWidth()+1), 0, 1))

	local Color = Var "LyricColor"
	local Factor = 1
	if side == "Back" then
		Factor = 0.5
	elseif side == "Front" then
		Factor = 0.9
	end
	self:diffuse({
		Color[1] * Factor,
		Color[2] * Factor,
		Color[3] * Factor,
		Color[4] * Factor
	})

	if side == "Front" then
		self:cropright(1)
	else
		self:cropleft(0)
	end

	self:diffusealpha(0)
	self:linear(0.2)
	self:diffusealpha(0.75)
	self:linear(Var "LyricDuration" * 0.75)
	if side == "Front" then
		self:cropright(0)
	else
		self:cropleft(1)
	end
	self:sleep(Var "LyricDuration" * 0.25)
	self:linear(0.2)
	self:diffusealpha(0)
end

-- (c) 2006 Glenn Maynard
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

