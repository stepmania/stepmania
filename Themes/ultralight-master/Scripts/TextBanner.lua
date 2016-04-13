-- you know the drill: so important, needs own file, etc.
local titleMaxWidth = THEME:GetMetric("MusicWheelItem","NormalTextMaxWidth")
local subtitleMaxWidth = titleMaxWidth*1.25

function TextBannerSetFunc(self,param) 
	local Title=self:GetChild("Title")
	local Subtitle=self:GetChild("Subtitle")
	-- Artist is unused.

	Title:halign(0)
	Subtitle:halign(0)

	if Subtitle:GetText() ~= "" then
		Title:zoom(0.75)
		Title:y(-12)
		Title:maxwidth(titleMaxWidth*1.1)

		Subtitle:visible(true)
		Subtitle:maxwidth(subtitleMaxWidth)
		Subtitle:zoom(0.625)
		Subtitle:y(4)
	else
		Subtitle:visible(false)
		Title:zoom(0.825)
		Title:maxwidth(titleMaxWidth)
		Title:y(-6)
	end
end

function CourseDisplayTextBanner(self,param)
	local Title=self:GetChild("Title")
	local Subtitle=self:GetChild("Subtitle")
	local Artist=self:GetChild("Artist")

	Title:halign(0)
	Subtitle:halign(0)
	Artist:halign(1)

	if Subtitle:GetText() ~= "" then
		Title:zoom(0.625)
		Title:y(-14)
		Title:maxwidth(titleMaxWidth*1.1)

		Subtitle:visible(true)
		Subtitle:maxwidth(subtitleMaxWidth)
		Subtitle:zoom(0.5)
		Subtitle:y(-2)
	else
		Subtitle:visible(false)
		Title:zoom(0.825)
		Title:maxwidth(titleMaxWidth)
		Title:y(-10)
	end

	Artist:x(236)
	Artist:y(10)
	Artist:zoom(0.5)
	Subtitle:maxwidth(subtitleMaxWidth*0.8)
end
