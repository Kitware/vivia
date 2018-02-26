project(qtExtensions)

###############################################################################

# BEGIN action manager compiler

include(qteVersion.cmake)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/qteVersion.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/qteVersion.h @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(qte-amc Tools/qtActionManagerCompiler.cxx)
target_link_libraries(qte-amc ${QT_LIBRARIES})

get_property(QTE_AMC_EXECUTABLE TARGET qte-amc PROPERTY LOCATION)
set(QTE_AMC_EXECUTABLE "${QTE_AMC_EXECUTABLE}"
    CACHE INTERNAL "Location of the qte-amc executable" FORCE
)

# END action manager compiler

###############################################################################

# BEGIN qtExtensions library sources

set(qtExtensionsSources
    # Core
    Core/qtCliArgs.cxx
    Core/qtCliOption.cxx
    Core/qtCliOptions.cxx
    Core/qtDebug.cxx
    Core/qtOnce.cxx
    Core/qtScopedValueChange.cxx
    Core/qtStlUtil.cxx
    Core/qtTest.cxx
    Core/qtThread.cxx
    Core/qtUtil.cxx
    # Util
    Util/qtAbstractAnimation.cxx
    Util/qtAbstractSetting.cxx
    Util/qtActionFactory.cxx
    Util/qtActionManager.cxx
    Util/qtActionPriorityList.cxx
    Util/qtColorScheme.cxx
    Util/qtColorUtil.cxx
    Util/qtDockController.cxx
    Util/qtGradient.cxx
    Util/qtJson.cxx
    Util/qtPrioritizedMenuProxy.cxx
    Util/qtPrioritizedToolBarProxy.cxx
    Util/qtScopedSettingGroup.cxx
    Util/qtSettings.cxx
    Util/qtStatusForwarder.cxx
    Util/qtStatusManager.cxx
    Util/qtStatusNotifier.cxx
    Util/qtStatusSource.cxx
    Util/qtUiState.cxx
    # IO
    IO/qtKstReader.cxx
    IO/qtKstParser.cxx
    IO/qtKstSeparator.cxx
    IO/qtTemporaryFile.cxx
    # Widgets
    Widgets/qtColorButton.cxx
    Widgets/qtDoubleSlider.cxx
    Widgets/qtDrawer.cxx
    Widgets/qtDrawerWidget.cxx
    Widgets/qtExpander.cxx
    Widgets/qtGradientWidget.cxx
    Widgets/qtOverlayWidget.cxx
    Widgets/qtSqueezedLabel.cxx
    Widgets/qtThrobber.cxx
    # ItemViews
    ItemViews/qtAbstractListDelegate.cxx
    ItemViews/qtColorButtonItemWidget.cxx
    ItemViews/qtComboBoxDelegate.cxx
    ItemViews/qtDoubleSpinBoxDelegate.cxx
    ItemViews/qtListDelegate.cxx
    ItemViews/qtRichTextDelegate.cxx
    ItemViews/qtSpinBoxDelegate.cxx
    # Dialogs
    Dialogs/qtActionManagerDialog.cxx
    Dialogs/qtConfirmationDialog.cxx
    Dialogs/qtDoubleInputDialog.cxx
    # Dom
    Dom/qtDom.cxx
    Dom/qtDomElement.cxx
    # Sax
    Sax/qtSaxNodes.cxx
    Sax/qtSaxTraversal.cxx
    Sax/qtSaxWriter.cxx
)

set(qtExtensionsMocHeaders
    # Util
    Util/qtAbstractAnimation.h
    Util/qtActionManager.h
    Util/qtDockControllerPrivate.h
    Util/qtStatusForwarder.h
    Util/qtStatusManager.h
    Util/qtStatusNotifier.h
    Util/qtStatusSourcePrivate.h
    # Widgets
    Widgets/qtColorButton.h
    Widgets/qtDoubleSlider.h
    Widgets/qtDrawer.h
    Widgets/qtDrawerWidget.h
    Widgets/qtExpander.h
    Widgets/qtGradientWidget.h
    Widgets/qtOverlayWidget.h
    Widgets/qtSqueezedLabel.h
    Widgets/qtThrobber.h
    # ItemViews
    ItemViews/qtAbstractListDelegate.h
    ItemViews/qtColorButtonItemWidget.h
    ItemViews/qtComboBoxDelegate.h
    ItemViews/qtDoubleSpinBoxDelegate.h
    ItemViews/qtListDelegate.h
    ItemViews/qtRichTextDelegate.h
    ItemViews/qtSpinBoxDelegate.h
    # Dialogs
    Dialogs/qtActionManagerDialog.h
    Dialogs/qtConfirmationDialog.h
    Dialogs/qtDoubleInputDialog.h
)

set(qtExtensionsUiFiles
    Dialogs/qtActionManagerDialog.ui
    Dialogs/qtConfirmationDialog.ui
)

set(qtExtensionsInstallHeaders
    # Core
    Core/qtCliArgs.h
    Core/qtCliOption.h
    Core/qtCliOptions.h
    Core/qtDebug.h
    Core/qtDebugArea.h
    Core/qtDebugHelper.h
    Core/qtDebugImpl.h
    Core/qtGlobal.h
    Core/qtMath.h
    Core/qtOnce.h
    Core/qtScopedValueChange.h
    Core/qtStlUtil.h
    Core/qtTest.h
    Core/qtThread.h
    Core/qtTransferablePointerArray.h
    Core/qtTransferablePointer.h
    Core/qtUtil.h
    # Util
    Util/qtAbstractAnimation.h
    Util/qtAbstractSetting.h
    Util/qtActionFactory.h
    Util/qtActionManager.h
    Util/qtColorScheme.h
    Util/qtColorUtil.h
    Util/qtDockController.h
    Util/qtGradient.h
    Util/qtJson.h
    Util/qtMap.h
    Util/qtMapImpl.h
    Util/qtPrioritizedMenuProxy.h
    Util/qtPrioritizedToolBarProxy.h
    Util/qtRand.h
    Util/qtScopedSettingGroup.h
    Util/qtSettings.h
    Util/qtSettingsImpl.h
    Util/qtStatusForwarder.h
    Util/qtStatusManager.h
    Util/qtStatusNotifier.h
    Util/qtStatusSource.h
    Util/qtUiState.h
    Util/qtUtilNamespace.h
    # IO
    IO/qtKstReader.h
    IO/qtStringStream.h
    IO/qtTemporaryFile.h
    # Widgets
    Widgets/qtColorButton.h
    Widgets/qtDoubleSlider.h
    Widgets/qtDrawer.h
    Widgets/qtDrawerWidget.h
    Widgets/qtExpander.h
    Widgets/qtGradientWidget.h
    Widgets/qtOverlayWidget.h
    Widgets/qtSqueezedLabel.h
    Widgets/qtThrobber.h
    # ItemViews
    ItemViews/qtAbstractListDelegate.h
    ItemViews/qtColorButtonItemWidget.h
    ItemViews/qtComboBoxDelegate.h
    ItemViews/qtDoubleSpinBoxDelegate.h
    ItemViews/qtFlagListDelegate.h
    ItemViews/qtListDelegate.h
    ItemViews/qtNumericTreeWidgetItem.h
    ItemViews/qtRichTextDelegate.h
    ItemViews/qtSpinBoxDelegate.h
    # Dialogs
    Dialogs/qtActionManagerDialog.h
    Dialogs/qtConfirmationDialog.h
    Dialogs/qtDoubleInputDialog.h
    # Dom
    Dom/qtDom.h
    Dom/qtDomElement.h
    # Sax
    Sax/qtSax.h
    Sax/qtSaxNamespace.h
    Sax/qtSaxNodes.h
    Sax/qtSaxTraversal.h
    Sax/qtSaxWriter.h
)

# END qtExtensions library build sources

###############################################################################

# BEGIN qtExtensions library build rules

qt4_wrap_ui(qtExtensionsUiSources ${qtExtensionsUiFiles})
qt4_wrap_cpp(qtExtensionsMocSources ${qtExtensionsMocHeaders})

source_group("Generated" FILES
  ${qtExtensionsMocSources}
  ${qtExtensionsUiSources}
)

add_library(${PROJECT_NAME}
  ${qtExtensionsSources} ${qtExtensionsMocSources} ${qtExtensionsUiSources}
)

if(WIN32)
  set(ONCE_THREAD_LIBRARY)
else()
  set(ONCE_THREAD_LIBRARY -lpthread)
endif()

target_link_libraries(${PROJECT_NAME}
  ${QT_LIBRARIES}
  ${ONCE_THREAD_LIBRARY}
)

if(MSVC)
  set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS /Zc:wchar_t-)
endif()

install_library_targets(${PROJECT_NAME})
install_headers(${qtExtensionsInstallHeaders} TARGET ${PROJECT_NAME}
                DESTINATION include/QtE)

# END qtExtensions library build rules

###############################################################################

# BEGIN qtExtensions library python wrapping rules

set(qtExtensionsWrapObjects
    # Core
    qtCliOptions
    #qtDebug - not wrapped for now; need to figure out how to wrap areas
    #qtTest - not wrapped for now; needs special handling to pass suites
    qtThread qtBareThread
    # Util
    qtAbstractAnimation
    qtActionFactory
    qtActionManager
    qtColorScheme
    qtDockController
    qtPrioritizedMenuProxy
    qtPrioritizedToolBarProxy
    qtScopedSettingGroup
    qtStatusForwarder
    qtStatusManager
    qtStatusNotifier
    qtStatusSource
    # IO
    qtTemporaryFile
    # Widgets
    qtColorButton
    qtDrawerWidget
    qtExpander
    qtGradientWidget
    qtOverlayWidget
    qtThrobber
    # ItemViews
    qtAbstractListDelegate
    qtColorButtonItemWidget
    qtComboBoxDelegate
    qtDoubleSpinBoxDelegate
    #qtFlagListDelegate - not wrapped for now; template class, maybe impossible
    qtListDelegate
    qtNumericTreeWidgetItem
    qtRichTextDelegate
    qtSpinBoxDelegate
    # Dialogs
    qtActionManagerDialog
    # Dom
    qtDomElement
)

vg_wrap_library(${PROJECT_NAME}
  TYPESYSTEM typesystem.xml
  OBJECTS ${qtExtensionsWrapObjects}
  HEADERS ${qtExtensionsInstallHeaders}
  DEPENDS PySide:Gui
)

# END qtExtensions library python wrapping rules

###############################################################################

# BEGIN doxytag helper

add_executable(qte-makedoxytag Tools/qtMakeDoxyTags.cxx)
target_link_libraries(qte-makedoxytag ${PROJECT_NAME} ${QT_LIBRARIES})

# List of extra tags (pages) to extract
set(qt4_extra_tags
  implicit-sharing
)

# Create dependency list
set(qt4_extra_tag_files)
foreach(tag ${qt4_extra_tags})
  list(APPEND qt4_extra_tag_files "${QT_DOC_DIR}/html/${tag}.html")
endforeach()

# Add target to generate extra tags file
add_custom_command(
  OUTPUT "${QT4_EXTRA_TAG_FILE}"
  COMMAND qte-makedoxytag -o "${QT4_EXTRA_TAG_FILE}" -d "${QT_DOC_DIR}/html"
                          ${qt4_extra_tags}
  DEPENDS qte-makedoxytag ${qt4_extra_tag_files}
)
add_custom_target(doxygen-qt4-extra-tag DEPENDS "${QT4_EXTRA_TAG_FILE}")

# END doxytag helper

###############################################################################

vg_add_documentation(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR} Qt)

vg_add_test_subdirectory()
