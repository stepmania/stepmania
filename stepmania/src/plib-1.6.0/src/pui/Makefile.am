if BUILD_PUI

lib_LIBRARIES = libplibpu.a

include_HEADERS = pu.h

libplibpu_a_SOURCES = \
        pu.cxx            puBox.cxx       puButton.cxx puButtonBox.cxx    \
        puArrowButton.cxx puDialogBox.cxx puFrame.cxx  puGroup.cxx        \
        puInput.cxx       puInterface.cxx puLocal.h    puMenuBar.cxx      \
        puObject.cxx      puOneShot.cxx   puPopup.cxx  puPopupMenu.cxx    \
        puFilePicker.cxx  puListBox.cxx   puSlider.cxx puText.cxx         \
        puValue.cxx       puFont.cxx      puBiSlider.cxx  puTriSlider.cxx \
        puDial.cxx        puVerticalMenu.cxx  puLargeInput.cxx            \
        puFileSelector.cxx puComboBox.cxx puSelectBox.cxx puRange.cxx	  \
        puSpinBox.cxx     puScrollBar.cxx

INCLUDES = -I$(top_srcdir)/src/sg -I$(top_srcdir)/src/fnt
INCLUDES += -I$(top_srcdir)/src/util

endif

EXTRA_DIST = pui.dsp


