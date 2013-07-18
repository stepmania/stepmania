--[[--------------------------------------------------------------------------
Shape Drawing module by Daisuke Master
------------------------------------------------------------------------------
-= What's this? =-
Those are functions that can draw some basic shapes

-= How to use =-
Add them as layers

<code>
Def.ActorFrame {
	Draw.RoundBox(410,310,20,20,color(#000000));
	Draw.RoundBox(400,300,10,10);
	Draw.Box(256,64,color("#ffcc66"))..{
		InitCommand=cmd(y,-300);
	};
}
</code>

You'll need graphics included under Graphics/_shapes for the roundboxes and
ovals (a 1000x1000 circle, 500x500 quarter circle and 1000x850 triangle)

The neatest thing about those things is that those are overloaded:

ºBoxesº (pretty much same as quads but without the need of using initcommand
every fucking time to resize them, color them and agh!...)
Draw.Box(width) <-- this will make a blank square (it says blank not black)
Draw.Box(width,color) <-- this will make a colored square
Draw.Box(width,height) <------------ Rectangles!
Draw.Box(width,height,color) <--/

ºTrianglesº (BROKEN)
Draw.Triangle(base) <-- this will make an equilateral blank trine
Draw.Triangle(base,color) <-- colored equilateral trine
Draw.Triangle(base,height)
Draw.Triangle(base,height,color)
Draw.Triangle(base,height,point)
Draw.Triangle(base,height,point,color)
--planned/impossible
Draw.Triangle(base,point,color) <-- sorry, I can't figure how to do this
									(maybe if I use negative values...)
Draw.Triangle(base,skew) <-- rect/irregular trines
Draw.Triangle(base,skew,color)
--the point should be in 0..1 range

ºRoundBoxesº (cute rounded boxes, better than sharp edged boxes)
Draw.RoundBox(width,height)
Draw.RoundBox(width,height,color)
Draw.RoundBox(width,height,radius)
Draw.RoundBox(width,height,radius,color)
Draw.RoundBox(width,height,horiz_radius,vert_radius,color)

ºOvals/Circlesº (yes, figures)
Draw.Oval(radius)
Draw.Oval(radius,color)
Draw.Oval(radius1,radius2)
Draw.Oval(radius1,radius2,color)

ºLinesº (not yet)
how it's (supposed to be) drawn:
S = start
E = end
dash,slash,backslash,bar = line body

Draw.Line(4,0,0,4)
	E
   /
  /
 /
S

Draw.Line(0,0,4,0)
S---E

Draw.Line(0,0,4,4)
S
 \
  \
   \
	E

Draw.Line(0,0,0,4)
S
|
|
|
E

lines are pending...
--]]--------------------------------------------------------------------------

Draw = {
	Box = function(w,...)
		assert(w)
		local h,c = ...
		if not h and not c then
			h = w
			c = color("#ffffff")
		elseif not c then
			if type(h) == "number" then
				c = color("#ffffff")
			else
				c = h
				h = w
			end
		end
		--option
		--if not h then h = w end
		--if not c then c = color("#ffffff") end
		return Def.Quad {
			InitCommand=function(self)
				self:basezoomx(w);
				self:basezoomy(h);
				self:diffuse(c);
			end;
		};
	end;
	Triangle = function(b,h,p,c)
		--I'M BROKEN
		assert(b)
		assert(h)
		assert(p)
		assert(c)
		--local h,p,c = ...
		
		return LoadActor(THEME:GetPathG("","_Figures/triangle.png"))..{
			InitCommand=function(self)
				local base = self:GetWidth()
				local height = self:GetHeight()
				local xzoom = scale(b,0,base,0,1)
				local yzoom = scale(h,0,height,0,1)
				local rot = scale(p,0,1,0,360)
				self:basezoomx(xzoom);
				self:basezoomx(yzoom);
				self:baserotationz(rot);
				self:diffuse(c);
			end;
		};
	end;
	--I had to take a small glance into optical's roundbox code to do this...
	RoundBox = function(w,h,...)
		assert(w)
		assert(h)
		--grab color and radii from arguments
		local hr,vr,c = ...
		if not hr and not vr and not c then
			hr = 10
			vr = 10
			c = color("#ffffff")
		elseif not vr and not c then
			if type(hr) == "number" then
				vr = hr
				c = color("#ffffff")
			else
				c = hr
				hr = 10
				vr = 10
			end
		elseif not c then
			if type(vr) == "number" then
				c = color("#ffffff")
			else
				c = vr
				vr = hr
			end
		end
		--horizontal radius can't overflow the half of the width and can't be
		--below zero, same with vertical radius
		hr = clamp(hr,0,w/2)
		vr = clamp(vr,0,h/2)
		
		local corner = LoadActor(THEME:GetPathG("","_Figures/corner.png"))..{
			InitCommand=function(self)
				local width = self:GetWidth()
				local height = self:GetHeight()
				local xpixels = scale(hr,0,width,0,1)
				local ypixels = scale(vr,0,height,0,1)
				self:basezoomx(xpixels)
				self:basezoomy(ypixels)
			end;
		};
		
		return Def.ActorFrame {
			InitCommand=function(self)
				--this's pretty a shitload of operations...
				self:GetChild("Top"):y(-h/2+vr/2);
				self:GetChild("Bottom"):y(h/2-vr/2);
				--also here
				self:GetChild("TopLeft"):xy(-w/2+hr/2,-h/2+vr/2);
				self:GetChild("TopRight"):xy(w/2-hr/2,-h/2+vr/2);
				self:GetChild("BottomLeft"):xy(-w/2+hr/2,h/2-vr/2);
				self:GetChild("BottomRight"):xy(w/2-hr/2,h/2-vr/2);
				--coloree :3
				self:runcommandsonleaves(cmd(diffuse,c))
			end;
			--lol sound effects
			--wrrr whrr wrrr
			Draw.Box(w-hr-hr,vr,c)..{Name="Top"};
			Draw.Box(w,h-vr-vr,c)..{Name="Middle"};
			Draw.Box(w-hr-hr,vr,c)..{Name="Bottom"};
			--corners
			corner..{ Name="TopLeft"; };
			corner..{ Name="TopRight"; BaseRotationY=180; };
			corner..{ Name="BottomLeft"; BaseRotationX=180; };
			corner..{ Name="BottomRight"; BaseRotationX=180; BaseRotationY=180; };
		}
	end;
	Oval = function(w,...)
		assert(w)
		local h,c = ...
		if not h and not c then
			h = w
			c = color("#ffffff")
		elseif not c then
			color("#ffffff")
			if type(h) == "number" then
				c = color("#ffffff")
			else
				c = h
				h = w
			end
		end
		return LoadActor(THEME:GetPathG("","_Figures/circle.png"))..{
			InitCommand=function(self)
				--self:SetTextureFiltering(false);
				local width = self:GetWidth()
				local height = self:GetHeight()
				local xzoom = scale(w,0,width,0,1)
				local yzoom = scale(h,0,height,0,1)
				self:diffuse(c);
				self:basezoomx(xzoom);
				self:basezoomy(yzoom);
			end;
		};
	end;
	Line = function(x1,x2,y1,y2,c)
		assert(x1)
		assert(x2)
		assert(y1)
		assert(y2)
		
		return Def.Quad {
			InitCommand=function(self)
				self:horizalign(left);
				self:xy(x1,y1);
				self:zoomto(1,1);
				self:rotationz(0);
				self:diffuse(c);
			end;
		};
	end
}