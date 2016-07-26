Practice mode and Edit Course mode are temporarilty removed.  They didn't work right anyway.

## Removed screens
* ScreenOptionsEdit
* ScreenPracticeMenu
* ScreenPractice
* ScreenEditCourseModsMenu
* ScreenEditCourseMods

## Removed metrics sections
* EditMenu
* ScreenOptionsEdit
* ScreenPracticeMenu
* PracticeMenu
* ScreenPractice
* ScreenEditCourseModsMenu
* CourseModsMenu
* ScreenEditCourseMods

## Changed metrics sections
* ScreenEditMenu


## Removed language entries:
Everything in the EditMenuAction section.
Everything in the EditMenu section.
Everything in the EditMenuRow section.
Everything in the ScreenEditMenu section.

## New language entries:
See ScreenEditMenu section in _fallback.

## These language entries were in the wrong place and had bad names anyway.
No, seriously, the name of the translation string should not be the string that needs to be translated and I want to punch whover wrote all this bullshit.

"ScreenEditMenu::Profile name cannot be blank." moved to "ScreenOptionsManageProfiles::profile_name_blank".
"ScreenEditMenu::The name you chose conflicts with another profile. Please use a different name." moved to "ScreenOptionsManageProfiles::profile_name_conflicts".


## bug Kyzentun to finish this explanation


## TODO:
Handle read-only song directory error.
Handle trying to delete steps produced by autogen.
Handle steps made by autogen in general.

