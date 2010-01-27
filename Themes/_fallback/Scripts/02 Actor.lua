-- Convenience aliases:
left = "HorizAlign_Left";
center = "HorizAlign_Center";
right = "HorizAlign_Right";
top = "VertAlign_Top";
middle = "VertAlign_Middle";
bottom = "VertAlign_Bottom";

function Actor:ease(t, fEase)
	-- Optimizations:
	-- fEase = -100 is equivalent to TweenType_Accelerate.
	if fEase == -100 then
		self:accelerate(t);
		return;
	end

	-- fEase = 0 is equivalent to TweenType_Linear.
	if fEase == 0 then
		self:linear(t);
		return;
	end

	-- fEase = +100 is equivalent to TweenType_Decelerate.
	if fEase == 100 then
		self:decelerate(t);
		return;
	end

	self:tween( t, "TweenType_Bezier",
		{
			0,
			scale(fEase, -100, 100, 0/3, 2/3),
			scale(fEase, -100, 100, 1/3, 3/3),
			1
		}
	);
end
-- Notes On Beziers --
-- They can be 1D ( Quadratic ) or 2D ( Bezier )
-- 1D:
--	XA XB YC YD
-- 2D:
--  XA XB XC XD YA YB YC YD
-- In 1D Quads, XA XB are beginning time and size, YC YD are ending time and size
-- In 2D Quads, X
local BounceBeginBezier =
{
	0, 0,
	0.42, -0.42,
	2/3, 0.3,
	1, 1
}
function Actor:bouncebegin(t)
	self:tween( t, "TweenType_Bezier", BounceBeginBezier );
end

local BounceEndBezier =
{
	0,0,
	1/3, 0.7,
	0.58, 1.42,
	1, 1
}
function Actor:bounceend(t)
	self:tween( t, "TweenType_Bezier", BounceEndBezier );
end

local SmoothBezier =
{
	0, 0, 1, 1
}
function Actor:smooth(t)
	self:tween( t, "TweenType_Bezier", SmoothBezier );
end

-- Hide if b is true, but don't unhide if b is false.
function Actor:hide_if(b)
	if b then
		self:visible(false)
	end
end

function Actor:player(p)
	self:visible( GAMESTATE:IsHumanPlayer(p) )
end

function ActorFrame:propagatecommand(...)
	self:propagate(1);
	self:playcommand(...);
	self:propagate(0);
end

-- Shortcut for alignment.
--   cmd(align,0.5,0.5)  -- align center
--   cmd(align,0.0,0.0)  -- align top-left
--   cmd(align,0.5,0.0)  -- align top-center
function Actor:align(h, v)
	self:halign( h );
	self:valign( v );
end

function Actor:FullScreen()
	self:stretchto( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT );
end

--[[ Typical background sizes:
320x240 - DDR 1st-Extreme, most NVLM_ZK songs
640x480 - most simfiles in distribution today are this big.
768x480 - 16:10 aspect ratio backgrounds
854x480 - pump it up pro

This function will stretch any background up to 640x480]]
function Actor:scale_or_crop_background()
	--local bgAspectRatio = self:GetWidth() / self:GetHeight();
	if self:GetWidth() <= 640 and self:GetHeight() <= 480 then
		self:stretchto( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT );
	else
		self:scaletocover( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT );
	end
end
-- Testing
function Actor:scale_or_crop_alternative()
	if self:GetWidth() and self:GetHeight() then
		local fRatio = self:GetWidth() / self:GetHeight();
		if fRatio == 4/3 then
			self:stretchto( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT );
		else
			self:scaletocover( 0,0,SCREEN_WIDTH,SCREEN_HEIGHT );
		end
	end
end

function Actor:Center()
    self:x(SCREEN_CENTER_X);
    self:y(SCREEN_CENTER_Y);
end;

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

