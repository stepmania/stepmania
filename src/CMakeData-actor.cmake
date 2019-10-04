list(APPEND SMDATA_ACTOR_BASE_SRC
            "Actor.cpp"
            "ActorFrame.cpp"
            "ActorFrameTexture.cpp"
            "ActorMultiTexture.cpp"
            "ActorMultiVertex.cpp"
            "ActorProxy.cpp"
            "ActorScroller.cpp"
            "ActorSound.cpp"
            "ActorUtil.cpp"
            "AutoActor.cpp"
            "BitmapText.cpp"
            "DynamicActorScroller.cpp"
            "Model.cpp"
            "ModelManager.cpp"
            "ModelTypes.cpp"
            "Quad.cpp"
            "RollingNumbers.cpp"
            "Sprite.cpp"
            "Tween.cpp")
list(APPEND SMDATA_ACTOR_BASE_HPP
            "Actor.h"
            "ActorFrame.h"
            "ActorFrameTexture.h"
            "ActorMultiTexture.h"
            "ActorMultiVertex.h"
            "ActorProxy.h"
            "ActorScroller.h"
            "ActorSound.h"
            "ActorUtil.h"
            "AutoActor.h"
            "BitmapText.h"
            "DynamicActorScroller.h"
            "Model.h"
            "ModelManager.h"
            "ModelTypes.h"
            "Quad.h"
            "RollingNumbers.h"
            "Sprite.h"
            "Tween.h")

source_group("Actors\\\\Base"
             FILES
             ${SMDATA_ACTOR_BASE_SRC}
             ${SMDATA_ACTOR_BASE_HPP})

list(APPEND SMDATA_ACTOR_GAMEPLAY_SRC
            "ActiveAttackList.cpp"
            "ArrowEffects.cpp"
            "AttackDisplay.cpp"
            "Background.cpp"
            "BeginnerHelper.cpp"
            "CombinedLifeMeterTug.cpp"
            "DancingCharacters.cpp"
            "Foreground.cpp"
            "GhostArrowRow.cpp"
            "HoldJudgment.cpp"
            "Inventory.cpp"
            "LifeMeter.cpp"
            "LifeMeterBar.cpp"
            "LifeMeterBattery.cpp"
            "LifeMeterTime.cpp"
            "LyricDisplay.cpp"
            "NoteDisplay.cpp"
            "NoteField.cpp"
            "PercentageDisplay.cpp"
            "Player.cpp"
            "ReceptorArrow.cpp"
            "ReceptorArrowRow.cpp"
            "ScoreDisplay.cpp"
            "ScoreDisplayAliveTime.cpp"
            "ScoreDisplayBattle.cpp"
            "ScoreDisplayCalories.cpp"
            "ScoreDisplayLifeTime.cpp"
            "ScoreDisplayNormal.cpp"
            "ScoreDisplayOni.cpp"
            "ScoreDisplayPercentage.cpp"
            "ScoreDisplayRave.cpp")

list(APPEND SMDATA_ACTOR_GAMEPLAY_HPP
            "ActiveAttackList.h"
            "ArrowEffects.h"
            "AttackDisplay.h"
            "Background.h"
            "BeginnerHelper.h"
            "CombinedLifeMeter.h"
            "CombinedLifeMeterTug.h"
            "DancingCharacters.h"
            "Foreground.h"
            "GhostArrowRow.h"
            "HoldJudgment.h"
            "Inventory.h"
            "LifeMeter.h"
            "LifeMeterBar.h"
            "LifeMeterBattery.h"
            "LifeMeterTime.h"
            "LyricDisplay.h"
            "NoteDisplay.h"
            "NoteField.h"
            "PercentageDisplay.h"
            "Player.h"
            "ReceptorArrow.h"
            "ReceptorArrowRow.h"
            "ScoreDisplay.h"
            "ScoreDisplayAliveTime.h"
            "ScoreDisplayBattle.h"
            "ScoreDisplayCalories.h"
            "ScoreDisplayLifeTime.h"
            "ScoreDisplayNormal.h"
            "ScoreDisplayOni.h"
            "ScoreDisplayPercentage.h"
            "ScoreDisplayRave.h")
source_group("Actors\\\\Gameplay"
             FILES
             ${SMDATA_ACTOR_GAMEPLAY_SRC}
             ${SMDATA_ACTOR_GAMEPLAY_HPP})

list(APPEND SMDATA_ACTOR_MENU_SRC
            "BPMDisplay.cpp"
            "ComboGraph.cpp"
            "ControllerStateDisplay.cpp"
            "CourseContentsList.cpp"
            "DifficultyList.cpp"
            "DualScrollBar.cpp"
            "EditMenu.cpp"
            "FadingBanner.cpp"
            "GradeDisplay.cpp"
            "GraphDisplay.cpp"
            "GrooveRadar.cpp"
            "HelpDisplay.cpp"
            "MemoryCardDisplay.cpp"
            "MenuTimer.cpp"
            "ModIcon.cpp"
            "ModIconRow.cpp"
            "MusicWheel.cpp"
            "MusicWheelItem.cpp"
            "OptionRow.cpp"
            "OptionsCursor.cpp"
            "OptionsList.cpp"
            "PaneDisplay.cpp"
            "ScrollBar.cpp"
            "SnapDisplay.cpp"
            "TextBanner.cpp"
            "WheelBase.cpp"
            "WheelItemBase.cpp"
            "WheelNotifyIcon.cpp"
            "WorkoutGraph.cpp")
list(APPEND SMDATA_ACTOR_MENU_HPP
            "BPMDisplay.h"
            "ComboGraph.h"
            "ControllerStateDisplay.h"
            "CourseContentsList.h"
            "DifficultyList.h"
            "DualScrollBar.h"
            "EditMenu.h"
            "FadingBanner.h"
            "GradeDisplay.h"
            "GraphDisplay.h"
            "GrooveRadar.h"
            "HelpDisplay.h"
            "MemoryCardDisplay.h"
            "MenuTimer.h"
            "ModIcon.h"
            "ModIconRow.h"
            "MusicWheel.h"
            "MusicWheelItem.h"
            "OptionRow.h"
            "OptionsCursor.h"
            "OptionsList.h"
            "PaneDisplay.h"
            "ScrollBar.h"
            "SnapDisplay.h"
            "TextBanner.h"
            "WheelBase.h"
            "WheelItemBase.h"
            "WheelNotifyIcon.h"
            "WorkoutGraph.h")

if(WITH_NETWORKING)
  list(APPEND SMDATA_ACTOR_MENU_SRC "RoomInfoDisplay.cpp")
  list(APPEND SMDATA_ACTOR_MENU_HPP "RoomInfoDisplay.h")
endif()

source_group("Actors\\\\Menus"
             FILES
             ${SMDATA_ACTOR_MENU_SRC}
             ${SMDATA_ACTOR_MENU_HPP})

list(APPEND SMDATA_ACTOR_GAMEPLAY_MENU_SRC
            "Banner.cpp"
            "BGAnimation.cpp"
            "BGAnimationLayer.cpp"
            "DifficultyIcon.cpp"
            "MeterDisplay.cpp"
            "StepsDisplay.cpp"
            "StreamDisplay.cpp"
            "Transition.cpp")

list(APPEND SMDATA_ACTOR_GAMEPLAY_MENU_HPP
            "Banner.h"
            "BGAnimation.h"
            "BGAnimationLayer.h"
            "DifficultyIcon.h"
            "MeterDisplay.h"
            "StepsDisplay.h"
            "StreamDisplay.h"
            "Transition.h")

source_group("Actors\\\\Gameplay and Menus"
             FILES
             ${SMDATA_ACTOR_GAMEPLAY_MENU_SRC}
             ${SMDATA_ACTOR_GAMEPLAY_MENU_HPP})

list(APPEND SMDATA_ALL_ACTORS_SRC
            ${SMDATA_ACTOR_BASE_SRC}
            ${SMDATA_ACTOR_GAMEPLAY_SRC}
            ${SMDATA_ACTOR_MENU_SRC}
            ${SMDATA_ACTOR_GAMEPLAY_MENU_SRC})
list(APPEND SMDATA_ALL_ACTORS_HPP
            ${SMDATA_ACTOR_BASE_HPP}
            ${SMDATA_ACTOR_GAMEPLAY_HPP}
            ${SMDATA_ACTOR_MENU_HPP}
            ${SMDATA_ACTOR_GAMEPLAY_MENU_HPP})
