// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// Forward declarations
class QComboBox;
class QSlider;
class QWidget;

class vtkVgBaseImageSource;

class vpQtSceneUtils
{
public:
  static void addContextLevels(vtkVgBaseImageSource* contextSource,
                               QComboBox* comboBox);

  static QWidget* createContextSlider(QSlider* slider, QWidget* parent = 0);
};
