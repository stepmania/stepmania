--
-- Load a resizable 1-d frame.  p contains an odd number of parts.
-- Even parts are resized; odd parts are not (unless the requested
-- width is too small).  The frame is centered on the midway part
-- of p; parts to the left and right are anchored on the inside.
--
-- A 3-part frame; the center third is dynamic, expanding outwards
-- in both directions:
-- p = { 1/3, 1/3, 1/3 };
--         --oo--
--     --oooooooooo--
--
-- A 2-part frame.  The left half is static, the right side dynamic;
-- the left part stays in place as the right half expands to the right,
-- with the frame centered on the boundary:
--    <<--
--    <<-----------
-- p = { 1/2, 0, 0, 1/2, 0 };
--
-- A 5-part frame, with an anchored center and static caps on each
-- end, expanding outwards:
--         <<--oo-->>
-- <<----------oo---------->>
-- p = { 1/5, 1/5, 1/10, 0, 1/10, 1/5, 1/5 };
--

local p, a = ...
assert( (#p%2) == 1 );

local t = Def.ActorFrame {
	children = { }
};
local Widths = { }

local TextureXPos = 0
for i = 1,#p do
	t.children[i] = a .. {
		InitCommand = function(self)
			if i < math.ceil(#p/2) then self:horizalign("HorizAlign_Right");
			elseif i == math.ceil(#p/2) then self:horizalign("HorizAlign_Center");
			else self:horizalign("HorizAlign_Left");
			end

			Widths[i] = self:GetWidth();
		end;

		SetPartSizeCommand = function(self, params)
			self:x( params.XPos[i] );
			self:zoomtowidth( params.Zooms[i] );
		end;
	};

	local Width = p[i];
	t.children[i].Frames =
	{
		{ -- state 0
			{ x=TextureXPos, y=0 },		-- top-left
			{ x=(TextureXPos+Width), y=1 }	-- bottom-right
		}
	};

	TextureXPos = TextureXPos + Width;
end

t.SetPartSizeCommand = function(self, params)
	self:zoomx( params.FrameZoom );
end

t.SetSizeCommand = function(self, params)
	local Width = params.Width;
	assert( Width );

	if params.tween then
		params.tween(self);
		self:RunCommandsOnChildren( params.tween );
	end;

	local UnscaledPartWidth = 0;
	local ScaledPartWidth = 0;
	for i = 1,#p do
		if(i%2)==0 then
			ScaledPartWidth = ScaledPartWidth + Widths[i];
		else
			UnscaledPartWidth = UnscaledPartWidth + Widths[i];
		end
	end

	local WidthToScale = Width - UnscaledPartWidth;
	local CurrentScaledPartWidth = math.max( WidthToScale, 0 );

	params.Zooms = {};
	params.XPos = {};
	local pos = 0;
	for i = math.ceil(#p/2),1,-1 do
		local ThisScaledPartWidth = Widths[i];
		if (i % 2) == 0 then
			ThisScaledPartWidth = scale( CurrentScaledPartWidth, 0, ScaledPartWidth, 0, Widths[i] );
		end

		params.XPos[i] = pos;

		if i == math.ceil(#p/2) then
			pos = pos - ThisScaledPartWidth/2;
		else
			pos = pos - ThisScaledPartWidth;
		end

		params.Zooms[i] = ThisScaledPartWidth;
	end
	pos = 0;
	for i = math.ceil(#p/2),#p do
		local ThisScaledPartWidth = Widths[i];
		if (i % 2) == 0 then
			ThisScaledPartWidth = scale( CurrentScaledPartWidth, 0, ScaledPartWidth, 0, Widths[i] );
		end

		params.XPos[i] = pos;

		if i == math.ceil(#p/2) then
			pos = pos + ThisScaledPartWidth/2;
		else
			pos = pos + ThisScaledPartWidth;
		end
	
		params.Zooms[i] = ThisScaledPartWidth;
	end

	params.FrameZoom = math.min(Width / UnscaledPartWidth, 1);

	self:playcommand("SetPartSize", params);
end

return t;

--
-- (c) 2007 Glenn Maynard
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
--

