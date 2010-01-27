-- temp, delete me when default is finalized

-- ProductivityHelpers: A set of useful aliases for theming.
-- Latest version at http://kki.ajworld.net/wiki/ProductivityHelpers

--[[ Aliases ]]

-- Blend Modes
-- Aliases for blend modes.
bmNormal   = 'BlendMode_Normal';
bmAdd      = 'BlendMode_Add';
bmMultiply = 'BlendMode_WeightedMultiply';
bmInvert   = 'BlendMode_InvertDest';
bmNoEffect = 'BlendMode_NoEffect';

-- Health Declarations
-- Used primarily for lifebars.
hsMax    = 'HealthState_Hot';
hsAlive  = 'HealthState_Alive';
hsDanger = 'HealthState_Danger';
hsDead   = 'HealthState_Dead';

--[[ Actor commands ]]
function Actor:CenterX()
	self:x(SCREEN_CENTER_X);
end;

function Actor:CenterY()
	self:y(SCREEN_CENTER_Y);
end;

-- xy(actorX,actorY)
-- Sets the x and y of an actor in one command.
function Actor:xy(actorX,actorY)
	self:x(actorX);
	self:y(actorY);
end;

-- MaskSource()
-- Sets an actor up as the source for a mask.
function Actor:MaskSource()
	self:clearzbuffer(true);
	self:zwrite(true);
	self:blend('BlendMode_NoEffect');
end;

-- MaskDest()
-- Sets an actor up to be masked by anything with MaskSource().
function Actor:MaskDest()
	self:ztest(true);
end;

--[[ BitmapText commands ]]

-- PixelFont()
-- An alias that turns off texture filtering.
-- Named because it works best with pixel fonts.
function BitmapText:PixelFont()
	self:SetTextureFiltering(false);
end;

-- Stroke(color)
-- Sets the text's stroke color.
function BitmapText:Stroke(c)
	self:strokecolor( c );
end;

-- NoStroke()
-- Removes any stroke.
function BitmapText:NoStroke()
	self:strokecolor( color("0,0,0,0") );
end;

-- DiffuseAndStroke(diffuse,stroke)
-- Set diffuse and stroke at the same time.
function BitmapText:DiffuseAndStroke(diffuseC,strokeC)
	self:diffuse(diffuseC);
	self:strokecolor(strokeC);
end;
--[[ end BitmapText commands ]]

--[[ ----------------------------------------------------------------------- ]]

--[[ helper functions ]]
function tobool(v)
	if type(v) == "string" then
		local cmp = string.lower(v);
		if cmp == "true" or cmp == "t" then
			return true;
		elseif cmp == "false" or cmp == "f" then
			return false;
		end;
	elseif type(v) == "number" then
		if v == 0 then return false;
		else return true;
		end;
	end;
end;

-- does not assume multiplayer, that would be mpname, if it existed.
function pname(pn)
	return (pn == PLAYER_1) and "P1" or "P2";
end;
--[[ end helper functions ]]
-- this code is in the public domain.