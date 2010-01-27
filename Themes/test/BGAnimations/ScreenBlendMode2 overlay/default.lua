local modelNames = { "luna", "luna", "luna2" };

local t = Def.ActorFrame{
	Lighting=true;
	InitCommand=function(self)
		self:fov(45);
		--SetAmbientLightColor (default 1,1,1,1)
		--self:SetAmbientLightColor( color("0.2,0.2,0.2,1") );
		--SetDiffuseLightColor (default 1,1,1,1)
		self:SetDiffuseLightColor( color("1,0,1,1") );
		--SetSpecularLightColor (default 1,1,1,1)
		self:SetSpecularLightColor( color("0,1,0,1") );
		--SetLightDirection (WARNING: does not error out correctly yet) default 0,0,1
	end;
	Def.Model{
		Materials = modelNames[1]..".txt";
		Meshes = modelNames[1]..".txt";
		Bones = "Rest.bones.txt";
		-- was zoom,35 @ center, SCREEN_BOTTOM
		InitCommand=cmd(x,SCREEN_CENTER_X*0.75;y,SCREEN_BOTTOM*0.9;zoom,24;wag;effectmagnitude,0,25,0;rotationy,90;rotationx,-30;cullmode,'CullMode_None');
	};
	Def.Model{
		Materials = modelNames[2]..".txt";
		Meshes = modelNames[2]..".txt";
		Bones = "Rest.bones.txt";
		InitCommand=cmd(x,SCREEN_CENTER_X*1.25;y,SCREEN_BOTTOM*0.9;zoom,24;wag;effectmagnitude,0,-25,0;rotationy,-90;rotationx,-30;cullmode,'CullMode_None';diffusecolor,color("0,1,0.75,1"););
		OnCommand=cmd(queuecommand,"ShiftAlpha");
		ShiftAlphaCommand=function(self)
			self:linear(3);
			local newAlpha = scale(math.random(), 0, 1, 0.325, 0.75  );
			self:diffusealpha( newAlpha );
			self:sleep(math.random(0.2,5));
			self:queuecommand("ShiftAlpha");
		end;
	};

	Def.Model{
		Materials = modelNames[3]..".txt";
		Meshes = modelNames[3]..".txt";
		Bones = "Rest.bones.txt";
		-- was zoom,35 @ center, SCREEN_BOTTOM
		InitCommand=cmd(x,SCREEN_CENTER_X*0.25;y,SCREEN_BOTTOM*1.25;zoom,32;rotationy,157;rotationx,-30;cullmode,'CullMode_None');
	};

	--[[
	LoadFont("Common normal")..{
		InitCommand=cmd(x,SCREEN_LEFT+16;y,SCREEN_TOP+16;horizalign,left;);
		OnCommand=function(self)
			local chars = CHARMAN:GetAllCharacters();
			self:settext("I'm thiiiiiiiiiiiiiis big: ".. #chars);
		end;
	};
	--]]
};

return t;