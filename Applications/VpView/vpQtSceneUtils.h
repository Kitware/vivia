/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

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
