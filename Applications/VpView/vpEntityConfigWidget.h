// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

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
