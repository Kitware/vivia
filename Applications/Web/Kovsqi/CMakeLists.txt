project(kovsqi)

vg_install_files_with_prefix(
  bin/kovsqi.py
  content/index.html
  content/config.json
  TARGET ${PROJECT_NAME}-webContentFiles
  DESTINATION share/VisGUI/www/apps/${PROJECT_NAME}
)

vg_web_server(${PROJECT_NAME})
