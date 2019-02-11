list(APPEND SMDATA_SCREEN_GAMEPLAY_SRC
            "ScreenGameplay.cpp"
            "ScreenGameplayLesson.cpp"
            "ScreenGameplayNormal.cpp"
            "ScreenGameplayShared.cpp"
            "ScreenGameplaySyncMachine.cpp")

list(APPEND SMDATA_SCREEN_GAMEPLAY_HPP
            "ScreenGameplay.h"
            "ScreenGameplayNormal.h"
            "ScreenGameplayShared.h"
            "ScreenGameplaySyncMachine.h")

source_group("Screens\\\\Gameplay"
             FILES
             ${SMDATA_SCREEN_GAMEPLAY_SRC}
             ${SMDATA_SCREEN_GAMEPLAY_HPP})

list(APPEND SMDATA_SCREEN_OPTION_SRC
            "ScreenOptions.cpp"
            "ScreenOptionsCourseOverview.cpp"
            "ScreenOptionsEditCourse.cpp"
            "ScreenOptionsEditProfile.cpp"
            "ScreenOptionsExportPackage.cpp"
            "ScreenOptionsManageCourses.cpp"
            "ScreenOptionsManageEditSteps.cpp"
            "ScreenOptionsManageProfiles.cpp"
            "ScreenOptionsMaster.cpp"
            "ScreenOptionsMasterPrefs.cpp"
            "ScreenOptionsMemoryCard.cpp"
            "ScreenOptionsToggleSongs.cpp")
list(APPEND SMDATA_SCREEN_OPTION_HPP
            "ScreenOptions.h"
            "ScreenOptionsCourseOverview.h"
            "ScreenOptionsEditCourse.h"
            "ScreenOptionsEditProfile.h"
            "ScreenOptionsExportPackage.h"
            "ScreenOptionsManageCourses.h"
            "ScreenOptionsManageEditSteps.h"
            "ScreenOptionsManageProfiles.h"
            "ScreenOptionsMaster.h"
            "ScreenOptionsMasterPrefs.h"
            "ScreenOptionsMemoryCard.h"
            "ScreenOptionsToggleSongs.h")

source_group("Screens\\\\Options"
             FILES
             ${SMDATA_SCREEN_OPTION_SRC}
             ${SMDATA_SCREEN_OPTION_HPP})

list(APPEND SMDATA_SCREEN_REST_SRC
            "Screen.cpp"
            "ScreenAttract.cpp"
            "ScreenBookkeeping.cpp"
            "ScreenContinue.cpp"
            "ScreenDebugOverlay.cpp"
            "ScreenDemonstration.cpp"
            "ScreenEdit.cpp"
            "ScreenEditMenu.cpp"
            "ScreenEnding.cpp"
            "ScreenEvaluation.cpp"
            "ScreenExit.cpp"
            "ScreenHighScores.cpp"
            "ScreenHowToPlay.cpp"
            "ScreenInstallOverlay.cpp"
            "ScreenInstructions.cpp"
            "ScreenJukebox.cpp"
            "ScreenMapControllers.cpp"
            "ScreenMessage.cpp"
            "ScreenMiniMenu.cpp"
            "ScreenNameEntry.cpp"
            "ScreenNameEntryTraditional.cpp"
            "ScreenPackages.cpp"
            "ScreenPlayerOptions.cpp"
            "ScreenProfileLoad.cpp"
            "ScreenProfileSave.cpp"
            "ScreenPrompt.cpp"
            "ScreenRanking.cpp"
            "ScreenReloadSongs.cpp"
            "ScreenSandbox.cpp"
            "ScreenSaveSync.cpp"
            "ScreenSelect.cpp"
            "ScreenSelectCharacter.cpp"
            "ScreenSelectLanguage.cpp"
            "ScreenSelectMaster.cpp"
            "ScreenSelectMusic.cpp"
            "ScreenSelectProfile.cpp"
            "ScreenServiceAction.cpp"
            "ScreenSetTime.cpp"
            "ScreenSongOptions.cpp"
            "ScreenSplash.cpp"
            "ScreenStatsOverlay.cpp"
            "ScreenSyncOverlay.cpp"
            "ScreenSystemLayer.cpp"
            "ScreenTestInput.cpp"
            "ScreenTestLights.cpp"
            "ScreenTestSound.cpp"
            "ScreenTextEntry.cpp"
            "ScreenTitleMenu.cpp"
            "ScreenUnlockBrowse.cpp"
            "ScreenUnlockCelebrate.cpp"
            "ScreenUnlockStatus.cpp"
            "ScreenWithMenuElements.cpp")
list(APPEND SMDATA_SCREEN_REST_HPP
            "Screen.h"
            "ScreenAttract.h"
            "ScreenBookkeeping.h"
            "ScreenContinue.h"
            "ScreenDebugOverlay.h"
            "ScreenDemonstration.h"
            "ScreenEdit.h"
            "ScreenEditMenu.h"
            "ScreenEnding.h"
            "ScreenEvaluation.h"
            "ScreenExit.h"
            "ScreenHighScores.h"
            "ScreenHowToPlay.h"
            "ScreenInstallOverlay.h"
            "ScreenInstructions.h"
            "ScreenJukebox.h"
            "ScreenMapControllers.h"
            "ScreenMessage.h"
            "ScreenMiniMenu.h"
            "ScreenNameEntry.h"
            "ScreenNameEntryTraditional.h"
            "ScreenPackages.h"
            "ScreenPlayerOptions.h"
            "ScreenProfileLoad.h"
            "ScreenProfileSave.h"
            "ScreenPrompt.h"
            "ScreenRanking.h"
            "ScreenReloadSongs.h"
            "ScreenSandbox.h"
            "ScreenSaveSync.h"
            "ScreenSelect.h"
            "ScreenSelectCharacter.h"
            "ScreenSelectLanguage.h"
            "ScreenSelectMaster.h"
            "ScreenSelectMusic.h"
            "ScreenSelectProfile.h"
            "ScreenServiceAction.h"
            "ScreenSetTime.h"
            "ScreenSongOptions.h"
            "ScreenSplash.h"
            "ScreenStatsOverlay.h"
            "ScreenSyncOverlay.h"
            "ScreenSystemLayer.h"
            "ScreenTestInput.h"
            "ScreenTestLights.h"
            "ScreenTestSound.h"
            "ScreenTextEntry.h"
            "ScreenTitleMenu.h"
            "ScreenUnlockBrowse.h"
            "ScreenUnlockCelebrate.h"
            "ScreenUnlockStatus.h"
            "ScreenWithMenuElements.h")

source_group("Screens\\\\Others"
             FILES
             ${SMDATA_SCREEN_REST_SRC}
             ${SMDATA_SCREEN_REST_HPP})

list(APPEND SMDATA_SCREEN_NET_SRC
            "ScreenNetEvaluation.cpp"
            "ScreenNetRoom.cpp"
            "ScreenNetSelectBase.cpp"
            "ScreenNetSelectMusic.cpp"
            "ScreenNetworkOptions.cpp")

list(APPEND SMDATA_SCREEN_NET_HPP
            "ScreenNetEvaluation.h"
            "ScreenNetRoom.h"
            "ScreenNetSelectBase.h"
            "ScreenNetSelectMusic.h"
            "ScreenNetworkOptions.h")

if(WITH_NETWORKING)
  list(APPEND SMDATA_SCREEN_NET_SRC "ScreenSMOnlineLogin.cpp")
  list(APPEND SMDATA_SCREEN_NET_HPP "ScreenSMOnlineLogin.h")
endif()

source_group("Screens\\\\Network"
             FILES
             ${SMDATA_SCREEN_NET_SRC}
             ${SMDATA_SCREEN_NET_HPP})

list(APPEND SMDATA_ALL_SCREENS_SRC
            ${SMDATA_SCREEN_GAMEPLAY_SRC}
            ${SMDATA_SCREEN_OPTION_SRC}
            ${SMDATA_SCREEN_NET_SRC}
            ${SMDATA_SCREEN_REST_SRC})

list(APPEND SMDATA_ALL_SCREENS_HPP
            ${SMDATA_SCREEN_GAMEPLAY_HPP}
            ${SMDATA_SCREEN_OPTION_HPP}
            ${SMDATA_SCREEN_NET_HPP}
            ${SMDATA_SCREEN_REST_HPP})
