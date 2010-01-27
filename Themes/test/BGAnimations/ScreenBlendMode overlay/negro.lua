-- basic MultiTexture with quads
function MakeTexture()
	-- [AJ] c is for children
	local c;

	-- [AJ] define Draw function
	local Draw = function(self)
		-- Render the quads to the texture.
		-- [AJ] this becomes Target.
		-- you must begin rendering to a target before drawing the
		-- components of it.
		c.Target:GetTexture():BeginRenderingTo();
		c.Shape1:Draw();	-- [AJ] draw shape 1
		c.Shape2:Draw();
		-- [AJ] you also have to finalize the render.
		c.Target:GetTexture():FinishRenderingTo();

		-- [AJ] render to the second target
		c.Target2:GetTexture():BeginRenderingTo();
		-- Unpremultiply the rendered shapes.
		c.mt:Draw();

		-- Knock out the text.
		c.nWo:Draw();
		c.Target2:GetTexture():FinishRenderingTo();
		-- [AJ] Target2 consists of:
		-- mt (MultiTexture using Target 1 as source)
		-- nWo (image layer)
		-- Target2 becomes the premultiplied Target 1 + anything else
		-- drawn to Target2. As you'll see below, Out uses Target2
		-- as the texture.

		-- Draw the result.
		c.Out:Draw();
	end

	local parts = {
		-- [AJ] render target 1
		Def.ActorFrameTexture { Name = "Target"; };
		-- [AJ] render target 2
		Def.ActorFrameTexture { Name = "Target2"; };
		-- [AJ] multitexture
		Def.ActorMultiTexture { Name = "mt"; };
		-- [AJ] output sprite
		Def.Sprite {
			Name = "Out";
			InitCommand=cmd(diffusealpha,0);
			OnCommand=cmd(stoptweening;accelerate,0.3;diffusealpha,1);
			SpinCommand=cmd(spin;effectmagnitude,0,60,0);
		};
		-- [AJ] text
		--[[
		LoadFont("_frutiger") .. {
			Name = "nWo";
			Text = "intrade";
		};
		]]

		LoadActor("_nWo")..{
			Name="nWo";
		};

		-- [AJ] the two Run commands are concurrent. That is to say, the Run
		-- command on the ActorFrame triggers the Run command on the Quad.

		Def.ActorFrame{
			Name="Shape1";
			OnCommand=cmd(stoptweening;playcommand,"Run");
			RunCommand=cmd(
				y,0;
				y,50;
				sleep,1;
				linear,2;addy,100;
				sleep,1.3;
				queuecommand,"Run"
			);

			Def.Quad{
				InitCommand=cmd(diffuse,color("#00CCFF");x,256-32;y,100;zoomto,128,128;fadetop,0.25;fadebottom,0.25;faderight,0.1);
				RunCommand=cmd(
					finishtweening;
					diffusealpha,0;
					sleep,1.25;
					linear,1;diffusealpha,1;
					linear,.25;diffusealpha,1;
					linear,.5;diffusealpha,0;
				);
			};
		};

		Def.ActorFrame{
			Name="Shape2";
			OnCommand=cmd(stoptweening;playcommand,"Run");
			RunCommand=cmd(
				y,0;
				y,50;
				sleep,1;
				linear,2;addy,100;
				sleep,1.3;
				queuecommand,"Run"
			);

			Def.Quad{
				InitCommand=cmd(diffuse,color("#FFCC00");x,256+32;y,100;zoomto,128,128;fadetop,0.25;fadebottom,0.25);
				RunCommand=cmd(
					finishtweening;
					diffusealpha,0;
					sleep,1.25;
					linear,1;diffusealpha,1;
					linear,.25;diffusealpha,1;
					linear,.5;diffusealpha,0;
				);
			};
		};
	};

	return Def.ActorFrame {
		children = parts;

		InitCommand = function(self)
			c = self:GetChildren();
			-- [AJ] sets draw function
			self:SetDrawFunction( Draw );

			-- Create the render target for the first pass.
			-- [AJ] uses render target 1
			c.Target:setsize(512, 512);
			c.Target:EnableAlphaBuffer(true);
			c.Target:EnableFloat(true);
			c.Target:Create();

			-- Create the render target for the second pass.
			-- [AJ] uses render target 2
			c.Target2:setsize(512, 512);
			c.Target2:EnableAlphaBuffer(true);
			c.Target2:Create();

			-- [AJ] modify text
			--c.nWo:shadowlength(0);
			c.nWo:x(256);
			c.nWo:y(236);
			c.nWo:blend("BlendMode_AlphaKnockOut");

			-- mt controls multitexture rendering for the unpremultiply pass.
			-- [AJ] unpremultiply pass is here, this is the crazy one
			local SourceTexture = c.Target:GetTexture();
			c.mt:SetSizeFromTexture( SourceTexture );
			c.mt:ClearTextures();
			c.mt:AddTexture( SourceTexture );
			c.mt:SetEffectMode( "EffectMode_HardMix" );
			--c.mt:SetTextureMode(0, 'TextureMode_Glow')
			c.mt:blend("BlendMode_CopySrc");
			c.mt:x( 0 );
			c.mt:y( 0 );
			c.mt:align(0,0);

			-- Out renders the final pass.
			c.Out:valign(0);
			c.Out:SetTexture( c.Target2:GetTexture() );
		end;
	};
end

local t = Def.ActorFrame{
	MakeTexture() .. {
		InitCommand=cmd(x,SCREEN_CENTER_X;y,SCREEN_CENTER_Y/2);
	};
};

return t;