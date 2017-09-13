local option_data= {}

for i, name in ipairs{
	"AllowExtraStage", "AllowHoldForOptions",
	"AllowMultipleHighScoreWithSameName", "AllowMultipleInstances",
	"AllowMultipleToasties", "AllowMultitexture", "AllowSongDeletion",
	"AllowUnacceleratedRenderer", "AnisotropicFiltering",
	"ArcadeOptionsNavigation", "AutoJoin", "AutoMapOnJoyChange",
	"AutogenGroupCourses", "AutogenSteps", "AxisFix", "BackUpAllSongSaves",
	"BlinkGameplayButtonLightsOnNote", "BreakComboToGetItem", "CelShadeModels",
	"Center1Player", "ComboContinuesBetweenSongs", "CustomSongsEnable",
	"DancePointsForOni", "DelayedBack", "DelayedCreditsReconcile",
	"Disqualification", "EasterEggs", "EditorShowBGChangesPlay",
	"EnableAttackSounds", "EnableMineHitSound", "EnableScoreboard",
	"EventMode", "FailOffForFirstStageEasy", "FailOffInBeginner", "FastLoad",
	"FastLoadAdditionalSongs", "FastNoteRendering", "ForceLogFlush",
	"ForceMipMaps", "FullscreenIsBorderlessWindow", "HarshHotLifePenalty",
	"HiddenSongs", "HideIncompleteCourses",  "LockCourseDifficulties",
	"LogCheckpoints", "LogFPS", "LogSkips", "LogToDisk",
	"MemoryCardProfiles", "MemoryCards", "MenuTimer", "MercifulBeginner",
	"MercifulDrain", "MercifulSuperMeter", "Minimum1FullSongInCourses",
	"MoveRandomToEnd", "MuteActions", "NeverBoostAppPriority",
	"OnlyDedicatedMenuButtons", "OnlyPreferredDifficulties", "PAL",
	"PalettedBannerCache", "PercentageScoring", "PreCacheAllWheelSorts",
	"PreferredSortUsesGroups", "ProfileCourseEdits", "ProfileDataCompress",
	"ProfileStepEdits", "ProgressiveLifebar", "ProgressiveNonstopLifebar",
	"ProgressiveStageLifebar", "PseudoLocalize", "QuirksMode", "ShowBanners",
	"ShowBeginnerHelper", "ShowCaution", "ShowDancingCharacters", "ShowDanger",
	"ShowInstructions", "ShowLoadingWindow", "ShowLogOutput", "ShowLyrics",
	"ShowLyrics", "ShowMasks", "ShowMouseCursor", "ShowNativeLanguage",
	"ShowPPS", "ShowStats", "ShowThemeErrors", "SignProfileData",
	"SmoothLines", "SongBackgrounds", "StretchBackgrounds",
	"SubSortByNumSteps", "ThreadedInput", "ThreadedMovieDecode",
	"ThreeKeyNavigation", "TrilinearFiltering", "UpdateCheckEnable",
	"UseUnlockSystem", "Vsync", "WarnOnNoProfile", "Windowed",
} do
	option_data[name]= {broad_type= "bool"}
end
for i, info in ipairs{
	{"AllowW1", AllowW1}, {"AttractSoundFrequency", AttractSoundFrequency},
	{"BackgroundFitMode", BackgroundFitMode}, {"BannerCache", BannerCacheMode},
	{"CoinMode", CoinMode}, {"GetRankingName", GetRankingName},
	{"HighResolutionTextures", HighResolutionTextures},
	{"MinTNSToHideNotes", TapNoteScore},
	{"MusicWheelUsesSections", MusicWheelUsesSections}, {"Premium", Premium},
	{"RandomBackgroundMode", RandomBackgroundMode},
	{"ShowSongOptions", Maybe},
} do
	option_data[info[1]]= {broad_type= "choice", choices= info[2]}
end
for i, name in ipairs{
	"BGBrightness", "FrameLimitPercent", "LifeDifficultyScale",
	"MinPercentageForMachineCourseHighScore",
	"MinPercentageForMachineSongHighScore", "NumBackgrounds",
	"SoundVolume", "SoundVolumeAttract", "TimingWindowScale",
} do
	option_data[name]= {broad_type= "percent"}
end
for i, name in ipairs{
	"ConstantUpdateDeltaSeconds", "DebounceCoinInputTime",
	"GlobalOffsetSeconds", "InputDebounceTime", "LightsAheadSeconds",
	"LightsFalloffSeconds", "MaxInputLatencySeconds", "PadStickSeconds",
	"VisualDelaySeconds",
} do
	option_data[name]= {broad_type= "millisecond"}
end
for i, name in ipairs{
	"CustomSongsMaxSeconds", "LongVerSongSeconds", "MarathonVerSongSeconds",
} do
	option_data[name]= {broad_type= "time"}
end
for i, name in ipairs{
	"CenterImageAddHeight", "CenterImageAddWidth", "CenterImageTranslateX",
	"CenterImageTranslateY", "CoinsPerCredit", "CustomSongsMaxCount",
	"EditClearPromptThreshold", "MaxHighScoresPerListForMachine",
	"MaxHighScoresPerListForPlayer", "MaxRegenComboAfterMiss",
	"MaxSongsInEditCourse", "MusicWheelSwitchSpeed", "RegenComboAfterMiss",
	"SongsPerPlay",
} do
	option_data[name]= {broad_type= "number"}
end
for i, name in ipairs{
	"CustomSongsLoadTimeout", "CustomSongsMaxMegabytes",
	"DefaultRecordLength", "EditRecordModeLeadIn",
	"NetworkStartOffset",
} do
	option_data[name]= {broad_type= "small_number"}
end

local judge_names= {
	"CheckpointHit", "CheckpointMiss", "Held", "HitMine", "LetGo", "Miss",
	"MissedHold", "W1", "W2", "W3", "W4", "W5",
}

for i, meter_name in ipairs{"Super", "Tug"} do
	for i, jname in ipairs(judge_names) do
		local name= meter_name .. "MeterPercentChange" .. jname
		option_data[name]= {broad_type= "percent"}
	end
end

for i, jname in ipairs(judge_names) do
	local name= "TimeMeterSecondsChange" .. jname
	option_data[name]= {broad_type= "millisecond"}
end

for i, tname in ipairs{
	"Add", "Jump", "SecondsAttack", "SecondsCheckpoint", "SecondsHold",
	"SecondsMine", "SecondsRoll", "SecondsW1", "SecondsW2", "SecondsW3",
	"SecondsW4", "SecondsW5",
} do
	local name= "TimingWindow"..tname
	option_data[name]= {broad_type= "millisecond"}
end

return option_data
