/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vpEntityConfigWidget_h
#define __vpEntityConfigWidget_h

#include <QWidget>

class vpEntityConfig;
class vgEntityType;

class vpEntityConfigWidget : public QWidget
{
  Q_OBJECT

public:
  vpEntityConfigWidget(QWidget* parent = 0);
  virtual ~vpEntityConfigWidget();

  void Initialize(vpEntityConfig* config);

  void Update();

signals:
  void EntityTypeChanged(int type);

protected slots:
  void RenderStyleChanged(int index);
  void HasSecondaryColorChanged(int state);

  void PrimaryColorClicked();
  void SecondaryColorClicked();

  void ShowAllTypes(int state);

protected:
  void AddType(const vgEntityType& type);

  inline int GetWidgetRow(QObject* obj);
  inline const vgEntityType& GetType(int row);

  QColor GetPrimaryColor(const vgEntityType& type);
  QColor GetSecondaryColor(const vgEntityType& type);

  void UpdateRow(int row);
  void UpdateEntityType(int row, const vgEntityType& type);

private:
  class vgInternal;
  vgInternal* Internal;
};

#endif
