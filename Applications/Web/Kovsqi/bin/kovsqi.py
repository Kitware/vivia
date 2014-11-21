if __name__ == "__main__":
  from PySide.QtCore import QCoreApplication
  QCoreApplication.setApplicationName("KOVSQI")

  from visgui.web import ParaViewServer
  ParaViewServer.run()
