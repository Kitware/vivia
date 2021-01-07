set(PROJECT VISGUI)
set(INCLUDE_GUARD __vgExport_h)

set(CONTENT_HEADER
"// This file is part of ViViA, and is distributed under the"
"// OSI-approved BSD 3-Clause License. See top-level LICENSE file or"
"// https://github.com/Kitware/vivia/blob/master/LICENSE for details."
)

set(CONTENT_PRE_EXPORTS
"// Disable warning caused from static VTK and shared visgui"
"#ifdef _MSC_VER"
"#  pragma warning (disable:4275) /* non-DLL-interface base class used */"
"#endif"
)

add_export(qtTestingSupport QT_TESTINGSUPPORT)
add_export(qtVgCommon       QTVG_COMMON)
add_export(qtVgWidgets      QTVG_WIDGETS)
add_export(vgCommon         VG_COMMON)
add_export(vgVideo          VG_VIDEO)
add_export(vgVtkVideo       VG_VTKVIDEO)
add_export(vgDataFramework  VG_DATA_FRAMEWORK)
add_export(vgGuiFramework   VG_GUI_FRAMEWORK)
add_export(vspData          VSP_DATA)
add_export(vspSourceUtil    VSP_SOURCEUTIL)
add_export(vspUserInterface VSP_USERINTERFACE)
add_export(vtkVgCore        VTKVG_CORE)
add_export(vtkVgIO          VTKVG_IO)
add_export(vtkVgModelView   VTKVG_MODELVIEW)
add_export(vtkVgSceneGraph  VTKVG_SCENEGRAPH)
add_export(vtkVgVideo       VTKVG_VIDEO)
add_export(vtkVgQtUtil      VTKVGQT_UTIL)
add_export(vtkVgQtSceneUtil VTKVGQT_SCENEUTIL)
add_export(vtkVgQtWidgets   VTKVGQT_WIDGETS)
add_export(vtkVwCore        VTKVW_CORE)
add_export(vvIO             VV_IO)
add_export(vvVidtk          VV_VIDTK)
add_export(vvVtkWidgets     VV_VTKWIDGETS)
add_export(vvWidgets        VV_WIDGETS)
