-- General guide:
-- ActorMultiVertex is an actor class that can have any number of vertices.
-- Use it for drawing anything that needs to have some arbitrary shape
-- This screen file contains a simple demo for each draw mode.
-- Each demo starts with a diagram of how its verts are connected.
-- Each of these actors has three commands as part of its demo.
-- The Reset command  sets the actor to some initial state.
-- The FirstMove command moves some verts around.
-- The SecondMove command changes the first and num to draw.

-- Testing this screen:
-- To add this screen to your theme so you can observe each of these actors, add these lines to your metrics.ini: (Remove the "-- " from the start of each line, of course)
-- [ScreenAMVTest]
-- Fallback="ScreenWithMenuElements"
-- Class="ScreenWithMenuElements"
-- CodeNames="left,right,reset"
-- Codeleft="Left"
-- Coderight="Right"
-- Codereset="Up"

-- Then set the InitialScreen value in the Common section to the name of this screen.  This will make your theme go to this screen when starting.
-- When on the screen, use left and right to change which demo is displayed, and up to send the Reset command to the currently displayed demo so you can watch its animation again.


local function quad_demo(x, y)
	-- Minimum verts: 4
	-- Verts per group: 4
	-- Vert diagram:
	-- 1 - 4   5 - 8
	-- |   |   |   |
	-- 2 - 3   6 - 7
	local verts= {
		{{0, 0, 0}, Color.Red},
		{{0, 20, 0}, Color.Blue},
		{{20, 20, 0}, Color.Green},
		{{20, 0, 0}, Color.Yellow},
		{{40, 0, 0}, Color.Orange},
		{{40, 20, 0}, Color.Purple},
		{{60, 20, 0}, Color.Black},
		{{60, 0, 0}, Color.White},
	}
	return Def.ActorMultiVertex{
		Name= "AMV_Quads",
		InitCommand=
			function(self)
				self:visible(false)
				self:xy(x, y)
				self:SetDrawState{Mode="DrawMode_Quads"}
			end,
		ResetCommand=
			function(self)
				self:SetDrawState{First= 1, Num= -1}
				verts[1][1][1]= 0
				verts[4][1][1]= 20
				verts[7][1][2]= 20
				verts[8][1][2]= 0
				self:SetVertices(verts)
				self:finishtweening()
				self:queuecommand("FirstMove")
				self:queuecommand("SecondMove")
			end,
		FirstMoveCommand=
			function(self)
				self:linear(1)
				verts[1][1][1]= verts[1][1][1]+10
				verts[4][1][1]= verts[4][1][1]-10
				verts[7][1][2]= verts[7][1][2]+10
				verts[8][1][2]= verts[8][1][2]-10
				self:SetVertices(verts)
			end,
		SecondMoveCommand=
			function(self)
				self:linear(1)
				self:SetDrawState{First= 3, Num= 4}
			end
	}
end

local function quadstrip_demo(x, y)
	-- Minimum verts: 4
	-- Verts per group: 2
	-- Vert diagram:
	-- 1 - 3 - 5 - 7
	-- |   |   |   |
	-- 2 - 4 - 6 - 8
	local verts= {
		{{0, 0, 0}, Color.Red},
		{{0, 20, 0}, Color.Blue},
		{{20, 0, 0}, Color.Green},
		{{20, 20, 0}, Color.Yellow},
		{{40, 0, 0}, Color.Orange},
		{{40, 20, 0}, Color.Purple},
		{{60, 0, 0}, Color.Black},
		{{60, 20, 0}, Color.White},
	}
	return Def.ActorMultiVertex{
		Name= "AMV_QuadStrip",
		InitCommand=
			function(self)
				self:visible(false)
				self:xy(x, y)
				self:SetDrawState{Mode="DrawMode_QuadStrip"}
			end,
		ResetCommand=
			function(self)
				self:SetDrawState{First= 1, Num= -1}
				verts[1][1][1]= 0
				verts[4][1][1]= 20
				verts[7][1][2]= 0
				verts[8][1][2]= 20
				self:SetVertices(verts)
				self:finishtweening()
				self:queuecommand("FirstMove")
				self:queuecommand("SecondMove")
			end,
		FirstMoveCommand=
			function(self)
				self:linear(1)
				verts[1][1][1]= verts[1][1][1]+10
				verts[4][1][1]= verts[4][1][1]-10
				verts[7][1][2]= verts[7][1][2]+10
				verts[8][1][2]= verts[8][1][2]-10
				self:SetVertices(verts)
			end,
		SecondMoveCommand=
			function(self)
				self:linear(1)
				self:SetDrawState{First= 3, Num= 4}
			end
	}
end

local function fan_demo(x, y)
	-- Minimum verts: 3
	-- Verts per group: 1
	-- Vert diagram:
	--     2 - 3
	--     | / |
	-- 8 - 1 - 4
	-- | / | \ |
	-- 7 - 6 - 5
	local verts= {
		{{0, 0, 0}, Color.Red},
		{{0, -20, 0}, Color.Blue},
		{{20, -20, 0}, Color.Green},
		{{20, 0, 0}, Color.Yellow},
		{{20, 20, 0}, Color.Orange},
		{{0, 20, 0}, Color.Purple},
		{{-20, 20, 0}, Color.Black},
		{{-20, 0, 0}, Color.White},
	}
	return Def.ActorMultiVertex{
		Name= "AMV_Fan",
		InitCommand=
			function(self)
				self:visible(false)
				self:xy(x, y)
				self:SetDrawState{Mode="DrawMode_Fan"}
			end,
		ResetCommand=
			function(self)
				self:SetDrawState{First= 1, Num= -1}
				verts[2][1][2]= -20
				verts[4][1][1]= 20
				verts[6][1][2]= 20
				verts[8][1][1]= -20
				self:SetVertices(verts)
				self:finishtweening()
				self:queuecommand("FirstMove")
				self:queuecommand("SecondMove")
			end,
		FirstMoveCommand=
			function(self)
				self:linear(1)
				verts[2][1][2]= verts[2][1][2]-10
				verts[4][1][1]= verts[4][1][1]+10
				verts[6][1][2]= verts[6][1][2]+10
				verts[8][1][1]= verts[8][1][1]-10
				self:SetVertices(verts)
			end,
		SecondMoveCommand=
			function(self)
				self:linear(1)
				self:SetDrawState{First= 3, Num= 6}
			end
	}
end

local function strip_demo(x, y)
	-- Minimum verts: 3
	-- Verts per group: 1
	-- Suspiciously similar to quad strip, 
	-- Vert diagram:
	-- 1 - 3 - 5 - 7
	-- | / | / | / |
	-- 2 - 4 - 6 - 8
	local verts= {
		{{0, 0, 0}, Color.Red},
		{{0, 20, 0}, Color.Blue},
		{{20, 0, 0}, Color.Green},
		{{20, 20, 0}, Color.Yellow},
		{{40, 0, 0}, Color.Orange},
		{{40, 20, 0}, Color.Purple},
		{{60, 0, 0}, Color.Black},
		{{60, 20, 0}, Color.White},
	}
	return Def.ActorMultiVertex{
		Name= "AMV_Strip",
		InitCommand=
			function(self)
				self:visible(false)
				self:xy(x, y)
				self:SetDrawState{Mode="DrawMode_Strip"}
			end,
		ResetCommand=
			function(self)
				self:SetDrawState{First= 1, Num= -1}
				verts[1][1][1]= 0
				verts[4][1][1]= 20
				verts[7][1][2]= 0
				verts[8][1][2]= 20
				self:SetVertices(verts)
				self:finishtweening()
				self:queuecommand("FirstMove")
				self:queuecommand("SecondMove")
			end,
		FirstMoveCommand=
			function(self)
				self:linear(1)
				verts[1][1][1]= verts[1][1][1]+10
				verts[4][1][1]= verts[4][1][1]-10
				verts[7][1][2]= verts[7][1][2]+10
				verts[8][1][2]= verts[8][1][2]-10
				self:SetVertices(verts)
			end,
		SecondMoveCommand=
			function(self)
				self:linear(1)
				self:SetDrawState{First= 3, Num= 4}
			end
	}
end

local function triangles_demo(x, y)
	-- Minimum verts: 3
	-- Verts per group: 3
	-- Vert diagram:
	--   2     5     8
	--  / \   / \   / \
	-- 1 - 3 4 - 6 7 - 9
	local verts= {
		{{0, 0, 0}, Color.Red},
		{{10, -20, 0}, Color.Blue},
		{{20, 0, 0}, Color.Green},
		{{30, 0, 0}, Color.Yellow},
		{{40, -20, 0}, Color.Orange},
		{{50, 0, 0}, Color.Purple},
		{{60, 20, 0}, Color.Black},
		{{70, 0, 0}, Color.White},
		{{80, 20, 0}, Color.Orange},
	}
	return Def.ActorMultiVertex{
		Name= "AMV_Triangles",
		InitCommand=
			function(self)
				self:visible(false)
				self:xy(x, y)
				self:SetDrawState{Mode="DrawMode_Triangles"}
			end,
		ResetCommand=
			function(self)
				verts[2][1][2]= -20
				verts[4][1][2]= 0
				verts[6][1][2]= 0
				verts[8][1][2]= -20
				self:SetDrawState{First= 1, Num= -1}
				self:SetVertices(verts)
				self:finishtweening()
				self:queuecommand("FirstMove")
				self:queuecommand("SecondMove")
			end,
		FirstMoveCommand=
			function(self)
				self:linear(1)
				verts[2][1][2]= verts[2][1][2]-20
				verts[4][1][2]= verts[4][1][2]+20
				verts[6][1][2]= verts[6][1][2]+20
				verts[8][1][2]= verts[8][1][2]-20
				self:SetVertices(verts)
			end,
		SecondMoveCommand=
			function(self)
				self:linear(1)
				self:SetDrawState{First= 3, Num= 6}
			end
	}
end

local function linestrip_demo(x, y)
	-- Minimum verts: 2
	-- Verts per group: 1
	-- Vert diagram:
	-- 1 - 2 - 3
	--         |
	-- 8       4
	-- |       |
	-- 7 - 6 - 5
	local verts= {
		{{-40, -40, 0}, Color.Red},
		{{0, -40, 0}, Color.Blue},
		{{40, -40, 0}, Color.Green},
		{{40, 0, 0}, Color.Yellow},
		{{40, 40, 0}, Color.Orange},
		{{0, 40, 0}, Color.Purple},
		{{-40, 40, 0}, Color.Black},
		{{-40, 0, 0}, Color.White},
	}
	return Def.ActorMultiVertex{
		Name= "AMV_LineStrip",
		InitCommand=
			function(self)
				self:visible(false)
				self:xy(x, y)
				self:SetDrawState{Mode="DrawMode_LineStrip"}
			end,
		ResetCommand=
			function(self)
				self:SetLineWidth(1)
				self:SetDrawState{First= 1, Num= -1}
				verts[1][1][2]= -40
				verts[4][1][1]= 40
				verts[6][1][2]= 40
				verts[8][1][1]= -40
				self:SetVertices(verts)
				self:finishtweening()
				self:queuecommand("FirstMove")
				self:queuecommand("SecondMove")
			end,
		FirstMoveCommand=
			function(self)
				self:linear(1)
				verts[1][1][2]= verts[1][1][2]+20
				verts[4][1][1]= verts[4][1][1]+20
				verts[6][1][2]= verts[6][1][2]-10
				verts[8][1][1]= verts[8][1][1]+10
				self:SetLineWidth(20)
				self:SetVertices(verts)
			end,
		SecondMoveCommand=
			function(self)
				self:linear(1)
				self:SetDrawState{First= 3, Num= 4}
			end
	}
end

local function symmetric_quadstrip_demo(x, y)
	-- Minimum verts: 6
	-- Verts per group: 3
	-- Vert diagram:
	-- 1 - 2 - 3
	-- |   |   |
	-- 4 - 5 - 6
	-- |   |   |
	-- 7 - 8 - 9
	local verts= {
		{{0, 0, 0}, Color.Red},
		{{20, 0, 0}, Color.Blue},
		{{40, 0, 0}, Color.Green},
		{{-10, 20, 0}, Color.Yellow},
		{{20, 20, 0}, Color.Orange},
		{{50, 20, 0}, Color.Purple},
		{{0, 40, 0}, Color.Black},
		{{20, 40, 0}, Color.White},
		{{40, 40, 0}, Color.Orange},
	}
	return Def.ActorMultiVertex{
		Name= "AMV_SymmetricQuadStrip",
		InitCommand=
			function(self)
				self:visible(false)
				self:xy(x, y)
				self:SetDrawState{Mode="DrawMode_SymmetricQuadStrip"}
			end,
		ResetCommand=
			function(self)
				self:SetDrawState{First= 1, Num= -1}
				verts[2][1][2]= 0
				verts[5][1][1]= 20
				verts[7][1][1]= 00
				verts[7][1][2]= 40
				verts[9][1][1]= 40
				verts[9][1][2]= 40
				self:SetVertices(verts)
				self:finishtweening()
				self:queuecommand("FirstMove")
				self:queuecommand("SecondMove")
			end,
		FirstMoveCommand=
			function(self)
				self:linear(1)
				verts[2][1][2]= verts[2][1][2]-20
				verts[5][1][1]= verts[5][1][1]-20
				verts[7][1][1]= verts[7][1][1]-10
				verts[7][1][2]= verts[7][1][2]+10
				verts[9][1][1]= verts[9][1][1]+10
				verts[9][1][2]= verts[9][1][2]+10
				self:SetVertices(verts)
			end,
		SecondMoveCommand=
			function(self)
				self:linear(1)
				self:SetDrawState{First= 4, Num= 6}
			end
	}
end

local children= {}
local prev_child_index= 1
local current_child_index= 1
local name_disp= false

local args= {
	quad_demo(SCREEN_CENTER_X, SCREEN_CENTER_Y),
	quadstrip_demo(SCREEN_CENTER_X, SCREEN_CENTER_Y),
	fan_demo(SCREEN_CENTER_X, SCREEN_CENTER_Y),
	strip_demo(SCREEN_CENTER_X, SCREEN_CENTER_Y),
	triangles_demo(SCREEN_CENTER_X, SCREEN_CENTER_Y),
	linestrip_demo(SCREEN_CENTER_X, SCREEN_CENTER_Y),
	symmetric_quadstrip_demo(SCREEN_CENTER_X, SCREEN_CENTER_Y),
	LoadFont("Common Normal") .. {
		Name= "NameDisp",
		InitCommand=
			function(self)
				self:xy(SCREEN_CENTER_X, 24)
				self:diffuse(Color.White)
			end
	},
	InitCommand=
		function(self)
			for n, c in pairs(self:GetChildren()) do
				if n:find("AMV") then
					Trace("Adding " .. n .. " to children list for manipulation.")
					children[#children+1]= c
				end
			end
			name_disp= self:GetChild("NameDisp")
			children[current_child_index]:visible(true)
			children[current_child_index]:queuecommand("Reset")
			name_disp:settext(children[current_child_index]:GetName())
		end,
	CodeMessageCommand=
		function(self, param)
			if param.Name == "left" then
				current_child_index= current_child_index - 1
				if current_child_index < 1 then
					current_child_index= #children
				end
			elseif param.Name == "right" then
				current_child_index= current_child_index + 1
				if current_child_index > #children then
					current_child_index= 1
				end
			end
			children[prev_child_index]:visible(false)
			children[current_child_index]:visible(true)
			children[current_child_index]:queuecommand("Reset")
			name_disp:settext(children[current_child_index]:GetName())
			prev_child_index= current_child_index
		end
}
return Def.ActorFrame(args)
