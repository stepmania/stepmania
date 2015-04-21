--[[ 
	This file contains 4 examples showing off the various features of ActorFrameTextures. Drop this into any Screen's Decorations or 
	Overlay to see the examples.
--]]

-- Choose which example you'd like to see
local ExampleToShow = 1;

-- Example 1: An ActorFrameTexture draws 3 quads and then hides itself. All of the quads are the same size, but two of them extend
-- off the edge of the texture. A sprite draws the texture, which begins scrolling the texture after 2 seconds. 
local Example1_AlphaBuffer = true; -- Change this to see the effect of the AlphaBuffer. The white background is to demonstate this.

-- Example 2: An ActorFrameTexture draws a spinning ActorFrame containing 3 color-channging quads. The texture is drawn by a sprite.
local Example2_PreserveTexture = false; -- Change this to see the effect of PreserveTexture.

-- Example 3: This example shows how the Float property of ActorFrameTexture works. An ActorFrameTexture draws a white quad, 
-- and then draws 3 quads on top of it all using "BlendMode_Add", each with either R, G, or B. Change the diffuse to see different 
-- colors show up as a result of the texture having RGB values greater than 1.
local Example3_Diffuse = { 1,0.5,1,1 } -- Note that with Green of 50%, there is still a white rectangle where the Green square intersect the background white square.

-- Example 4: Use a pair of AFTs to draw an object with a fading trail behind it. This example is a bit odd, in that the length of the 
-- trail ends up depending heavily on frame rate. This will look fairly different with and without VSync on. Holding tab or ~ will also 
-- change the length.

--[[
	ActorFrameTexture Explained:
	
	ActorFrameTexture is like a regular ActorFrame except that it draws to a texture instead of drawing to the screen( the 
	way all other actors draw ). Other actor classes such as Sprite and ActorMultiVertex can draw the texture and maniplate 
	it in ways that you cannot normally manipulate an ActorFrame.
	
	Because it is not drawing to the screen, an ActorFrameTexture's postion, rotation, and zoom have no affect. Diffuse and 
	glow are applied to the children of an ActorFrameTexture the same way they are applied to the children of an ActorFrame.
	Additionally, an ActorFrameTexture can draw to it's texture at any time, by calling the Actor method Draw. For all other
	Actor classes, the Draw method will have no effect if called outside of the Screen's draw cycle.
	
	ActorFrameTextures draw only actors or parts of actors that are located within the range x = 0 to x = <width> and y = 0
	to y = <height>. If an actor is sitting at x = 0 inside the ActorFrameTexture, only the right half of the Actor will draw.
	The SetWidth and SetHeight methods are used to set the width and height. They are explained in more detail below.

	An ActorFrameTexture can be used to cache 'expensive to draw' ensembles of objects by drawing them once to the texture,
	hiding the AFT, and	then having a single sprite draw that texture instead.
	
	ActorFrameTexture methods:
	
	The follow methods have effect only if used BEFORE the 'Create' method.
	
	EnableAlphaBuffer( boolean ) 
	- The texture can have transparency if this is set true. If false it will render over a black background.
	- See example 1 below.
	
	EnableDepthBuffer( boolean )
	- If true the texture is created with a zbuffer. This allows masking to occur while rendering to the texture. Both
	- mask source and mask destination must drawn while rendering to the texture in order to have any effect. Regardless
	- of this setting, masks drawn before rendering to the texture has begun will not affect objects drawn while rendering 
	- to the texture, and masks drawn while rending to the texture will not affect objects drawn after rendering to the 
	- texture has finished.
	
	EnableFloat( boolean )
	- This allows the values ( RGB ) to go outside the range [0,1] when using Blend modes. See example 3.
	
	SetTextureName( string )
	- This allows the created texture to be called by name.
	- This is used in all of the examples below.
	
	SetHeight( float ) and SetWidth( float )
	- These methods belong to the Actor class, but the properties they set are used in a specific way by ActorFrameTexture.
	- These methods are used to set the size of the created texture. The actual texture will have its dimensions rounded up
	- to the next power of two, but only the portion within the chose height and width will be rendered to.
	
	Create()
	- This creates the texture with the properties set by the above methods. Calling any of those methods after calling 
	- create will have no effect.
	
	The following methods can be used any time.
	
	EnablePreserveTexture( boolean )
	- If this is false(default) the texture will be cleared before each time the AFT renders to it. If this is true, the AFT
	- will render on top of the existing texture without clearing it first. This can be changed dynamically after the texture 
	- has been created.
	- See example 2.
	
	GetTexture()
	- This returns the RageTextureRenderTarget object created with the Create method. You can use with the "SetTexture" methods
	- of other Actor classes including Sprite and ActorMultiVertex. This is the only way to access the texture if the texture 
	- was created without using SetTextureName.
	-
	- This RageTextureRenderTarget has two unique methods of it's own: BeginRenderingTo and FinishRenderingTo. When it's parent
	- ActorFrameTexture draws it will automatically call these methods, before it begins drawing and after it finishes respectively.
	- These methods allow you to use a DrawFunction on any ActorFrame's to render to an AFT's texture, and there are several powerful
	- techinques which take advantage of this.
]]

local Examples = { }

Examples[1] = Def.ActorFrame{
	Def.Quad{ InitCommand=cmd(FullScreen) }; -- a blank background.
	Def.ActorFrameTexture{
		InitCommand=function(self)
			self:SetTextureName( "Example 1" )
			self:SetWidth( 128 );
			self:SetHeight( 128 );
			self:EnableAlphaBuffer( Example1_AlphaBuffer ); 
			self:Create();

			-- The ActorFrameTexture only needs to draw once, so hide it after the first draw.
			self:Draw()
			self:hibernate(math.huge)
		end;
		Def.ActorFrame{
			Name = "Draw";
			-- three random quads, two of them strattling the edge of the texture.
			Def.Quad{ InitCommand=cmd(zoom,50;diffuse,1,0,0,0.5) };
			Def.Quad{ InitCommand=cmd(zoom,50;diffuse,0,1,0,0.5;x,64;y,64) };
			Def.Quad{ InitCommand=cmd(zoom,50;diffuse,0,0,1,0.5;x,120;y,100) };
		};
	};
	Def.Sprite{ 
		Texture="Example 1";
		InitCommand=cmd(Center;sleep,1;queuecommand,"Scroll");
		ScrollCommand=cmd(texcoordvelocity,1,0.5); -- Scroll the texture. Texture manipulation is one of the things that makes AFT special.
	};
}

Examples[2] = Def.ActorFrame{
	Def.Quad{ InitCommand=cmd(FullScreen;diffuse,0,0,0,1) }; -- a blank background.
	Def.ActorFrameTexture{
		InitCommand=function(self)
			self:SetTextureName( "Example 2" )
			self:SetWidth( 128 );
			self:SetHeight( 128 );
			self:EnableAlphaBuffer( true );
			self:Create();
			
			self:EnablePreserveTexture( Example2_PreserveTexture );

			-- No draw function this time, this will draw on every frame.
		end;
		Def.ActorFrame{
			Name = "Draw";
			InitCommand=cmd(spin;x,64;y,64); -- Something has to change to demonstrate PreserveTexture.
			Def.Quad{ InitCommand=cmd(zoom,50;rainbow;effectperiod,3;effectoffset,1;x,-40;y,-56) };
			Def.Quad{ InitCommand=cmd(zoom,50;rainbow;effectperiod,3;effectoffset,0) };
			Def.Quad{ InitCommand=cmd(zoom,50;rainbow;effectperiod,3;effectoffset,2;x,64;y,50) };
		};
	};
	Def.Sprite{ Texture="Example 2"; InitCommand=cmd(Center); };
}

Examples[3] = Def.ActorFrame{
	Def.Quad{ InitCommand=cmd(FullScreen;diffuse,0,0,0,1) }; -- a blank background.
	Def.ActorFrameTexture{
		InitCommand=function(self)
			self:SetTextureName( "Example 3" )
			self:SetWidth( 128 );
			self:SetHeight( 128 );
			self:EnableAlphaBuffer( true );
			self:EnableFloat( true );
			self:Create();
			
			self:Draw()
			self:hibernate(math.huge)
		end;
		Def.ActorFrame{
			Name = "Draw";
			Def.Quad{ InitCommand=cmd(zoom,80;diffuse,1,1,1,1;x,64;y,64) };
			Def.Quad{ InitCommand=cmd(zoom,80;diffuse,1,0,0,1;x,14;y,80;blend,"BlendMode_Add") };
			Def.Quad{ InitCommand=cmd(zoom,80;diffuse,0,1,0,1;x,74;y,20;blend,"BlendMode_Add") };
			Def.Quad{ InitCommand=cmd(zoom,80;diffuse,0,0,1,1;x,128;y,100;blend,"BlendMode_Add") };
		};
	};
	Def.Sprite{ Texture = "Example 3"; InitCommand=cmd(Center;diffuse,Example3_Diffuse); };
}

--[[
	We have 2 AFTs, and a sprite to draw the final output.
	On each frame, the following occurs:
		The first AFT draws the 'final output' sprite to it's texture, showing the final output of the previous frame.
		The second AFT draws a sprite containing the first AFT's texture, but at reduced alpha, and then draw's it's own image.
		The final output sprite draws the second AFT's texture to the screen.
		
	The image drawn in a frame will be drawn again and again of subsequant frames with continuously decreasing opacity, showing
	a trail of where it had been.
	
	We cannot use a single AFT, having it draw a sprite containing it's own texture at lower alpha because the texture is cleared
	as soon as the texture begins rendering, before it can be drawn. If PreserveTexture is true, we cannot have it fade.
--]]

Examples[4] = Def.ActorFrame{
	Def.Quad{ InitCommand=cmd(FullScreen;diffuse,0,0,0,1) }; -- a blank background.
	Def.ActorFrameTexture{
		Name = "Memory";
		InitCommand=function(self)
			self:SetTextureName( "Memory" )
			self:SetWidth( 256 );
			self:SetHeight( 256 );
			self:EnableAlphaBuffer( true );
			self:Create();
		end;
		-- Cannot call the second AFT's texture by name as it has not been created when this Sprite loads.
		Def.Sprite{ Name = "Sprite"; InitCommand=cmd(x,128;y,128) };
	};
	Def.ActorFrameTexture{
		InitCommand=function(self)
			self:SetTextureName( "Output" )
			self:SetWidth( 256 );
			self:SetHeight( 256 );
			self:EnableAlphaBuffer( true );
			self:Create();
			-- Set the first AFT's child's texture to this AFT's texture, now that it has been created.
			self:GetParent():GetChild("Memory"):GetChild("Sprite"):SetTexture( self:GetTexture() );
		end;
		-- A sprite to draw the 'trail' with.
		Def.Sprite{	Texture = "Memory"; InitCommand=cmd(x,128;y,128;diffuse,1,1,1,.995); };
		Def.ActorFrame{ -- eliptical motion.
			InitCommand=cmd(x,128;y,128;bob;effectmagnitude,96,0,0;effectoffset,0.5);
			Def.ActorFrame{
				InitCommand=cmd(bob;effectmagnitude,0,64,0);
				-- A pixel ghost.
				Def.Quad{ InitCommand=cmd(zoomto,32,40); };
				Def.Quad{ InitCommand=cmd(zoomto,8,8;x,-8;y,-8;diffuse,0,0,0,1); };
				Def.Quad{ InitCommand=cmd(zoomto,8,8;x,8;y,-8;diffuse,0,0,0,1); };
				Def.Quad{ InitCommand=cmd(zoomto,24,8;x,0;y,2;diffuse,0,0,0,1); };
				Def.Quad{ InitCommand=cmd(zoomto,8,16;x,-12;y,24); };
				Def.Quad{ InitCommand=cmd(zoomto,8,16;x,12;y,24); };
				Def.Quad{ InitCommand=cmd(zoomto,8,16;x,0;y,24); };
			};
		};
	};
	Def.Sprite{ Name = "Ghosting"; Texture = "Output"; InitCommand=cmd(Center); };
}

return Examples[ ExampleToShow ]