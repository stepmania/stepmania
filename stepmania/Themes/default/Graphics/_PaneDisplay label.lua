local text = ...;

return Def.ActorFrame {
        OnCommand=cmd();
        OffFocusedCommand=cmd();
        OffUnfocusedCommand=cmd();

	children =
	{
		Def.BitmapText {
			Text=THEME:GetString("PaneDisplay", text);
			File="Common normal";
			OnCommand=cmd(shadowlength,0;horizalign,left;zoom,0.5);
		};
	};
};
