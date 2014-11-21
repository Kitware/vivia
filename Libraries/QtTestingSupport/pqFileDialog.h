/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#ifndef _pqFileDialog_h
#define _pqFileDialog_h

#include <QFileDialog>

#include <QApplication>
#include <QVariant>

class pqFileDialog : public QFileDialog
{
public:
  inline pqFileDialog(QWidget* parent, Qt::WindowFlags f);
  inline explicit pqFileDialog(QWidget* parent = 0,
                               const QString& caption = QString(),
                               const QString& directory = QString(),
                               const QString& filter = QString());

  inline void setOption(Option option, bool on = true);
  inline void setOptions(Options options);

  inline static QString getOpenFileName(
    QWidget* parent = 0,
    const QString& caption = QString(),
    const QString& dir = QString(),
    const QString& filter = QString(),
    QString* selectedFilter = 0,
    Options options = 0);

  inline static QString getSaveFileName(
    QWidget* parent = 0,
    const QString& caption = QString(),
    const QString& dir = QString(),
    const QString& filter = QString(),
    QString* selectedFilter = 0,
    Options options = 0);

  inline static QString getExistingDirectory(
    QWidget* parent = 0,
    const QString& caption = QString(),
    const QString& dir = QString(),
    Options options = ShowDirsOnly);

  inline static QStringList getOpenFileNames(
    QWidget* parent = 0,
    const QString& caption = QString(),
    const QString& dir = QString(),
    const QString& filter = QString(),
    QString* selectedFilter = 0,
    Options options = 0);

protected:
  inline void setNativeMode();
  inline static void setNativeMode(Options&);
};

//-----------------------------------------------------------------------------
void pqFileDialog::setNativeMode()
{
  QVariant t = qApp->property("testingActive");
  if (t.isValid() && t.toBool())
    {
    this->setOption(DontUseNativeDialog, true);
    }
}

//-----------------------------------------------------------------------------
void pqFileDialog::setNativeMode(QFileDialog::Options& options)
{
  QVariant t = qApp->property("testingActive");
  if (t.isValid() && t.toBool())
    {
    options |= DontUseNativeDialog;
    }
}

//-----------------------------------------------------------------------------
pqFileDialog::pqFileDialog(QWidget* parent, Qt::WindowFlags f)
  : QFileDialog(parent, f)
{
  this->setNativeMode();
}

//-----------------------------------------------------------------------------
pqFileDialog::pqFileDialog(QWidget* parent, const QString& caption,
                           const QString& directory, const QString& filter)
  : QFileDialog(parent, caption, directory, filter)
{
  this->setNativeMode();
}

//-----------------------------------------------------------------------------
void pqFileDialog::setOption(QFileDialog::Option option, bool on)
{
  if (option == DontUseNativeDialog)
    {
    QVariant t = qApp->property("testingActive");
    if (t.isValid() && t.toBool())
      {
      on = true;
      }
    }
  QFileDialog::setOption(option, on);
}

//-----------------------------------------------------------------------------
void pqFileDialog::setOptions(QFileDialog::Options options)
{
  setNativeMode(options);
  QFileDialog::setOptions(options);
}

//-----------------------------------------------------------------------------
QString pqFileDialog::getOpenFileName(
  QWidget* parent, const QString& caption, const QString& dir,
  const QString& filter, QString* selectedFilter, QFileDialog::Options options)
{
  setNativeMode(options);
  return QFileDialog::getOpenFileName(
           parent, caption, dir, filter, selectedFilter, options);
}

//-----------------------------------------------------------------------------
QString pqFileDialog::getSaveFileName(
  QWidget* parent, const QString& caption, const QString& dir,
  const QString& filter, QString* selectedFilter, QFileDialog::Options options)
{
  setNativeMode(options);
  return QFileDialog::getSaveFileName(
           parent, caption, dir, filter, selectedFilter, options);
}

//-----------------------------------------------------------------------------
QString pqFileDialog::getExistingDirectory(
  QWidget* parent, const QString& caption, const QString& dir,
  QFileDialog::Options options)
{
  setNativeMode(options);
  return QFileDialog::getExistingDirectory(parent, caption, dir, options);
}

//-----------------------------------------------------------------------------
QStringList pqFileDialog::getOpenFileNames(
  QWidget* parent, const QString& caption, const QString& dir,
  const QString& filter, QString* selectedFilter, QFileDialog::Options options)
{
  setNativeMode(options);
  return QFileDialog::getOpenFileNames(
           parent, caption, dir, filter, selectedFilter, options);
}

#endif
