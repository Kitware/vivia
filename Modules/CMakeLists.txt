vg_install_files_with_prefix(
  visgui/__init__.py
  visgui/core/__init__.py
  visgui/core/QueryService.py
  visgui/core/VideoPlayer.py
  visgui/testing/__init__.py
  visgui/testing/VtkTest.py
  visgui/web/__init__.py
  visgui/web/ParaViewServer.py
  TARGET PythonModules
  DESTINATION lib/${PYTHON_SHORT}/site-packages
)
