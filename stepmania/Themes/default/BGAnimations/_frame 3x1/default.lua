local File, Width = ...
local FullFile = THEME:GetPathB('','_frame files 3x1/'..File )
return Def.ActorFrame { children = {
	LoadActor( FullFile )() .. { InitCommand=cmd(setstate,0;pause;horizalign,right;x,-Width/2) };
	LoadActor( FullFile )() .. { InitCommand=cmd(setstate,1;pause;zoomtowidth,Width) };
	LoadActor( FullFile )() .. { InitCommand=cmd(setstate,2;pause;horizalign,left;x,Width/2) };
}}
