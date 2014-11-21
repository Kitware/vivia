project(VisGUI-Web)

###############################################################################

# BEGIN user configurable build options

vg_option(VISGUI_ENABLE_WEB ON
  "Enable VisGUI Web"
  "VISGUI_ENABLE_PYTHON"
)
if(NOT VISGUI_ENABLE_WEB)
  # If the web GUI's are disabled, we are only here to declare our options, so
  # leave now
  return()
endif()

# END user configurable build options

###############################################################################

#BEGIN web helper functions

set(VGWEB_RUN_SERVER_SCRIPT_TEMPLATE
  "${CMAKE_CURRENT_SOURCE_DIR}/vgweb.sh.in" CACHE INTERNAL ""
)

# Function to create and install a script to run an application's web server
function(vg_web_server NAME)
  set(_configure_file_script
    "${visGUI_SOURCE_DIR}/CMake/configure-file-script.cmake"
  )
  set(_out "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}-server")
  add_custom_command(
    OUTPUT "${_out}"
    DEPENDS "${VGWEB_RUN_SERVER_SCRIPT_TEMPLATE}" "${_configure_file_script}"
    COMMAND "${CMAKE_COMMAND}"
      "-DINPUT_FILE=${VGWEB_RUN_SERVER_SCRIPT_TEMPLATE}"
      "-DOUTPUT_FILE=${_out}"
      "-DLIB_SUFFIX=${LIB_SUFFIX}"
      "-DPYTHON_SHORT=${PYTHON_SHORT}"
      "-DAPPLICATION=${NAME}"
      "-DSERVER_PYTHON_SCRIPT=${NAME}.py"
      "-DCONFIGURE_ARGS=ESCAPE_QUOTES @ONLY"
      -P "${_configure_file_script}"
  )
  add_custom_target(${NAME}-server ALL DEPENDS "${_out}")
  install(PROGRAMS "${_out}" DESTINATION bin COMPONENT Web)
endfunction()

#END web helper functions

###############################################################################

#BEGIN build and install rules

# Install ParaView third-party redistributables
vg_install_files_with_prefix(
  ext/js-core/autobahn.js
  ext/js-core/autobahn.min.js
  ext/js-core/gl-matrix.js
  ext/js-core/gl-matrix-min.js
  ext/js-core/hammer.js
  ext/js-core/hammer.min.js
  ext/js-core/jquery-1.8.2.min.js
  ext/js-core/jquery.hammer.js
  ext/js-core/jquery.hammer.min.js
  ext/widgets/jquery-ui/jquery-ui-1.10.0.min.js
  SOURCE "${PARAVIEW_WWW_DIR}"
  TARGET webThirdPartyJsContentFiles
  DESTINATION share/VisGUI/www/content/js
  STRIP_PATH
)

vg_install_files_with_prefix(
  ext/widgets/jquery-ui/jquery-ui-1.10.0.css
  SOURCE "${PARAVIEW_WWW_DIR}"
  TARGET webThirdPartyCssContentFiles
  DESTINATION share/VisGUI/www/content/css
  STRIP_PATH
)

# Install ParaView redistributables
vg_install_files_with_prefix(
  css/paraview.ui.pipeline.css
  css/paraview.ui.toolbar.css
  css/paraview.ui.toolbar.vcr.css
  css/paraview.ui.toolbar.viewport.css
  js/paraview-all.js
  js/paraview-all.min.js
  js/paraview.ui.pipeline.js
  js/paraview.ui.toolbar.vcr.js
  js/paraview.ui.toolbar.viewport.js
  SOURCE "${PARAVIEW_WWW_DIR}/lib"
  TARGET webParaViewContentFiles
  DESTINATION share/VisGUI/www/content
)

# Install our own content files
vg_install_files_with_prefix(
  content/index.html
  content/css/bootstrap.css
  content/css/bootstrap.min.css
  content/css/bootstrap-responsive.css
  content/css/bootstrap-responsive.min.css
  content/css/visgui.css
  content/js/bootstrap.js
  content/js/bootstrap.min.js
  content/js/bootstrap-contextmenu.js
  content/js/visgui.js
  content/js/OpenLayers.js
  content/images/icons/loading.gif
  content/images/icons/marker-blue.png
  content/images/icons/marker-gold.png
  content/images/icons/playback-play.png
  content/images/icons/playback-stop.png
  content/images/icons/show-trails.png
  content/images/icons/okay.png
  content/images/icons/cancel.png
  TARGET webContentFiles
  DESTINATION share/VisGUI/www
)

add_dependencies(webContentFiles
  webThirdPartyJsContentFiles
  webThirdPartyCssContentFiles
  webParaViewContentFiles
)

# END build and install rules
