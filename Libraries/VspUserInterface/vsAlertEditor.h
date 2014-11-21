/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef __vsAlertEditor_h
#define __vsAlertEditor_h

#include <QDialog>

#include <qtGlobal.h>

class vvReader;

struct vsAlert;

class vsAlertEditorPrivate;

class vsAlertEditor : public QDialog
{
  Q_OBJECT

public:
  vsAlertEditor(QWidget* parent = 0);
  ~vsAlertEditor();

  bool loadAlert(const QString& fileName);
  void setAlert(const vsAlert&);

  vsAlert alert() const;

  void selectName();

signals:
  void changesApplied();

public slots:
  virtual void accept();
  virtual void reject();

  void apply();
  void reset();

  void saveAlert();

protected slots:
  void browseForQuery();

  void nameChanged();
  void queryFileChanged(QString);

  void setModified();

protected:
  QTE_DECLARE_PRIVATE_RPTR(vsAlertEditor)

  bool readQuery(vvReader& reader, const QString& fileName);
  bool abort(const QString& fileName, const QString& message);
  bool abort(const QString& fileName, const char* message);

  void validate();
  void setQueryInfo(QString error = QString());

private:
  QTE_DECLARE_PRIVATE(vsAlertEditor)
  Q_DISABLE_COPY(vsAlertEditor)
};

#endif
