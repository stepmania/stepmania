local File, Width = ...
local FullFile = THEME:GetPathB('','_frame files 3x1/'..File )
local Frame = LoadActor( FullFile )

return WrapInActorFrame {
	Frame .. { InitCommand=cmd(setstate,0;pause;horizalign,right;x,-Width/2) };
	Frame .. { InitCommand=cmd(setstate,1;pause;zoomtowidth,Width) };
	Frame .. { InitCommand=cmd(setstate,2;pause;horizalign,left;x,Width/2) };
};
