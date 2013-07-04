local File, Width = ...
assert( File );
assert( Width );

local FullFile = THEME:GetPathG('','_frame files 3x1/'..File )
local Frame = LoadActor( FullFile )

return Def.ActorFrame {
	Frame .. { 
		InitCommand=function(self)
			self:setstate(0);
			self:pause();
			self:horizalign(right);
			self:x(-Width / 2);
		end;
	};
	Frame .. { 
		InitCommand=function(self)
			self:setstate(1);
			self:pause();
			self:zoomtowidth(Width);
		end;
	};
	Frame .. { 
		InitCommand=function(self)
			self:setstate(2);
			self:pause();
			self:horizalign(left);
			self:x(Width / 2);
		end;
	};
};
