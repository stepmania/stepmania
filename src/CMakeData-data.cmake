list(APPEND SM_DATA_LUA_SRC
            "LuaBinding.cpp"
            "LuaExpressionTransform.cpp"
            "LuaReference.cpp")

list(APPEND SM_DATA_LUA_HPP
            "LuaBinding.h"
            "LuaExpressionTransform.h"
            "LuaReference.h")

source_group("Data Structures\\\\Lua"
             FILES
             ${SM_DATA_LUA_SRC}
             ${SM_DATA_LUA_HPP})

list(APPEND SM_DATA_FONT_SRC
            "Font.cpp"
            "FontCharAliases.cpp"
            "FontCharmaps.cpp")

list(APPEND SM_DATA_FONT_HPP
            "Font.h"
            "FontCharAliases.h"
            "FontCharmaps.h")

source_group("Data Structures\\\\Fonts"
             FILES
             ${SM_DATA_FONT_SRC}
             ${SM_DATA_FONT_HPP})

list(APPEND SM_DATA_COURSE_SRC
            "Course.cpp"
            "CourseLoaderCRS.cpp"
            "CourseUtil.cpp"
            "CourseWriterCRS.cpp"
            "Trail.cpp"
            "TrailUtil.cpp")

list(APPEND SM_DATA_COURSE_HPP
            "Course.h"
            "CourseLoaderCRS.h"
            "CourseUtil.h"
            "CourseWriterCRS.h"
            "Trail.h"
            "TrailUtil.h")

source_group("Data Structures\\\\Courses and Trails"
             FILES
             ${SM_DATA_COURSE_SRC}
             ${SM_DATA_COURSE_HPP})

list(APPEND SM_DATA_NOTEDATA_SRC
            "NoteData.cpp"
            "NoteDataUtil.cpp"
            "NoteDataWithScoring.cpp")

list(APPEND SM_DATA_NOTEDATA_HPP
            "NoteData.h"
            "NoteDataUtil.h"
            "NoteDataWithScoring.h")

source_group("Data Structures\\\\Note Data"
             FILES
             ${SM_DATA_NOTEDATA_SRC}
             ${SM_DATA_NOTEDATA_HPP})

list(APPEND SM_DATA_NOTELOAD_SRC
            "NotesLoader.cpp"
            "NotesLoaderBMS.cpp"
            "NotesLoaderDWI.cpp"
            "NotesLoaderJson.cpp"
            "NotesLoaderKSF.cpp"
            "NotesLoaderSM.cpp"
            "NotesLoaderSMA.cpp"
            "NotesLoaderSSC.cpp")

list(APPEND SM_DATA_NOTELOAD_HPP
            "NotesLoader.h"
            "NotesLoaderBMS.h"
            "NotesLoaderDWI.h"
            "NotesLoaderJson.h"
            "NotesLoaderKSF.h"
            "NotesLoaderSM.h"
            "NotesLoaderSMA.h"
            "NotesLoaderSSC.h")

source_group("Data Structures\\\\Notes Loaders"
             FILES
             ${SM_DATA_NOTELOAD_SRC}
             ${SM_DATA_NOTELOAD_HPP})

list(APPEND SM_DATA_NOTEWRITE_SRC
            "NotesWriterDWI.cpp"
            "NotesWriterJson.cpp"
            "NotesWriterSM.cpp"
            "NotesWriterSSC.cpp")

list(APPEND SM_DATA_NOTEWRITE_HPP
            "NotesWriterDWI.h"
            "NotesWriterJson.h"
            "NotesWriterSM.h"
            "NotesWriterSSC.h")

source_group("Data Structures\\\\Notes Writers"
             FILES
             ${SM_DATA_NOTEWRITE_SRC}
             ${SM_DATA_NOTEWRITE_HPP})

list(APPEND SM_DATA_SCORE_SRC
            "ScoreKeeper.cpp"
            "ScoreKeeperNormal.cpp"
            "ScoreKeeperRave.cpp"
            "ScoreKeeperShared.cpp")

list(APPEND SM_DATA_SCORE_HPP
            "ScoreKeeper.h"
            "ScoreKeeperNormal.h"
            "ScoreKeeperRave.h"
            "ScoreKeeperShared.h")

source_group("Data Structures\\\\Score Keepers"
             FILES
             ${SM_DATA_SCORE_SRC}
             ${SM_DATA_SCORE_HPP})

list(APPEND SM_DATA_SONG_SRC
            "Song.cpp"
            "SongCacheIndex.cpp"
            "SongOptions.cpp"
            "SongPosition.cpp"
            "SongUtil.cpp")

list(APPEND SM_DATA_SONG_HPP
            "Song.h"
            "SongCacheIndex.h"
            "SongOptions.h"
            "SongPosition.h"
            "SongUtil.h")

source_group("Data Structures\\\\Songs"
             FILES
             ${SM_DATA_SONG_SRC}
             ${SM_DATA_SONG_HPP})

list(APPEND SM_DATA_STEPS_SRC
            "Steps.cpp"
            "StepsUtil.cpp"
            "Style.cpp"
            "StyleUtil.cpp")

list(APPEND SM_DATA_STEPS_HPP
            "Steps.h"
            "StepsUtil.h"
            "Style.h"
            "StyleUtil.h")

source_group("Data Structures\\\\Steps and Styles"
             FILES
             ${SM_DATA_STEPS_SRC}
             ${SM_DATA_STEPS_HPP})

list(APPEND SM_DATA_REST_SRC
            "AdjustSync.cpp"
            "Attack.cpp"
            "AutoKeysounds.cpp"
            "BackgroundUtil.cpp"
            "ImageCache.cpp"
            "Character.cpp"
            "CodeDetector.cpp"
            "CodeSet.cpp"
            "CubicSpline.cpp"
            "Command.cpp"
            "CommonMetrics.cpp"
            "ControllerStateDisplay.cpp"
            "CreateZip.cpp"
            "CryptHelpers.cpp"
            "DateTime.cpp"
            "Difficulty.cpp"
            "DisplaySpec.cpp"
            "EnumHelper.cpp"
            "FileDownload.cpp"
            "Game.cpp"
            "GameCommand.cpp"
            "GameConstantsAndTypes.cpp"
            "GameInput.cpp"
            "GameplayAssist.cpp"
            "GamePreferences.cpp"
            "Grade.cpp"
            "HighScore.cpp"
            "Inventory.cpp"
            "JsonUtil.cpp"
            "LocalizedString.cpp"
            "LyricsLoader.cpp"
            "ModsGroup.cpp"
            "NoteTypes.cpp"
            "OptionRowHandler.cpp"
            "PlayerAI.cpp"
            "PlayerNumber.cpp"
            "PlayerOptions.cpp"
            "PlayerStageStats.cpp"
            "PlayerState.cpp"
            "Preference.cpp"
            "Profile.cpp"
            "RadarValues.cpp"
            "RandomSample.cpp"
            "SampleHistory.cpp"
            "ScreenDimensions.cpp"
            "SoundEffectControl.cpp"
            "StageStats.cpp"
            "TimingData.cpp"
            "TimingSegments.cpp"
            "TitleSubstitution.cpp")

list(APPEND SM_DATA_REST_HPP
            "AdjustSync.h"
            "Attack.h"
            "AutoKeysounds.h"
            "BackgroundUtil.h"
            "ImageCache.h"
            "Character.h"
            "CodeDetector.h"
            "CodeSet.h"
            "Command.h"
            "CommonMetrics.h"
            "ControllerStateDisplay.h"
            "CreateZip.h"
            "CryptHelpers.h"
            "CubicSpline.h"
            "DateTime.h"
            "DisplaySpec.h"
            "Difficulty.h"
            "EnumHelper.h"
            "FileDownload.h"
            "Game.h"
            "GameCommand.h"
            "GameConstantsAndTypes.h"
            "GameInput.h"
            "GameplayAssist.h"
            "GamePreferences.h"
            "Grade.h"
            "HighScore.h"
            "InputEventPlus.h"
            "Inventory.h"
            "JsonUtil.h"
            "LocalizedString.h"
            "LyricsLoader.h"
            "ModsGroup.h"
            "NoteTypes.h"
            "OptionRowHandler.h"
            "PlayerAI.h"
            "PlayerNumber.h"
            "PlayerOptions.h"
            "PlayerStageStats.h"
            "PlayerState.h"
            "Preference.h"
            "Profile.h"
            "RadarValues.h"
            "RandomSample.h"
            "SampleHistory.h"
            "ScreenDimensions.h"
            "SoundEffectControl.h"
            "SubscriptionManager.h"
            "StageStats.h"
            "ThemeMetric.h"
            "TimingData.h"
            "TimingSegments.h"
            "TitleSubstitution.h")

if(WITH_NETWORKING)
  list(APPEND SM_DATA_REST_SRC "RoomWheel.cpp")
  list(APPEND SM_DATA_REST_HPP "RoomWheel.h")
endif()

source_group("Data Structures\\\\Misc Objects"
             FILES
             ${SM_DATA_REST_SRC}
             ${SM_DATA_REST_HPP})

list(APPEND SMDATA_ALL_DATA_SRC
            ${SM_DATA_COURSE_SRC}
            ${SM_DATA_FONT_SRC}
            ${SM_DATA_LUA_SRC}
            ${SM_DATA_NOTEDATA_SRC}
            ${SM_DATA_NOTELOAD_SRC}
            ${SM_DATA_NOTEWRITE_SRC}
            ${SM_DATA_SCORE_SRC}
            ${SM_DATA_SONG_SRC}
            ${SM_DATA_STEPS_SRC}
            ${SM_DATA_REST_SRC})

list(APPEND SMDATA_ALL_DATA_HPP
            ${SM_DATA_COURSE_HPP}
            ${SM_DATA_FONT_HPP}
            ${SM_DATA_LUA_HPP}
            ${SM_DATA_NOTEDATA_HPP}
            ${SM_DATA_NOTELOAD_HPP}
            ${SM_DATA_NOTEWRITE_HPP}
            ${SM_DATA_SCORE_HPP}
            ${SM_DATA_SONG_HPP}
            ${SM_DATA_STEPS_HPP}
            ${SM_DATA_REST_HPP})
