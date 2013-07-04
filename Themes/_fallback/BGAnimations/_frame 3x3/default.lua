local File, Width, Height = ...
assert( File );
assert( Width );
assert( Height );

local FullFile = THEME:GetPathB('','_frame files 3x3/'..File )
local Frame = LoadActor( FullFile )
return Def.ActorFrame {
	Frame .. { 
		InitCommand=function(self)
			self:setstate(0);
			self:pause();
			self:horizalign(right);
			self:vertalign(bottom);
			self:x(-Width / 2);
			self:y(-Height / 2);
		end;
	};
	Frame .. { 
		InitCommand=function(self)
			self:setstate(1);
			self:pause();
			self:zoomtowidth(Width);
			self:vertalign(bottom);
			self:zoomtowidth(Width);
			self:y(-Height / 2);
		end;
	};
	Frame .. { 
		InitCommand=function(self)
			self:setstate(2);
			self:pause();
			self:horizalign(left);
			self:vertalign(bottom);
			self:x(Width / 2);
			self:y(-Height / 2);
		end;
	};
	Frame .. { 
		InitCommand=function(self)
			self:setstate(3);
			self:pause();
			self:horizalign(right);
			self:x(-Width / 2);
			self:zoomtoheight(Height);
		end;
	};
	Frame .. { 
		InitCommand=function(self)
			self:setstate(4);
			self:pause();
			self:zoomtowidth(Width);
			self:zoomtoheight(Height);
		end;
	};
	Frame .. { 
		InitCommand=function(self)
			self:setstate(5);
			self:pause();
			self:horizalign(left);
			self:x(Width / 2);
			self:zoomtoheight(Height);
		end;
	};
	Frame .. { 
		InitCommand=function(self)
			self:setstate(6);
			self:pause();
			self:horizalign(right);
			self:vertalign(top);
			self:x(-Width / 2);
			self:y(Height / 2);
		end;
	};
	Frame .. { 
		InitCommand=function(self)
			self:setstate(7);
			self:pause();
			self:zoomtowidth(Width);
			self:vertalign(top);
			self:zoomtowidth(Width);
			self:y(Height / 2);
		end;
	};
	Frame .. { 
		InitCommand=function(self)
			self:setstate(8);
			self:pause();
			self:horizalign(left);
			self:vertalign(top);
			self:x(Width / 2);
			self:y(Height / 2);
		end;
	};
};
