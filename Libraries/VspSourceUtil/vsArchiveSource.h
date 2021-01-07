// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vsArchiveSource_h
#define __vsArchiveSource_h

#include <QUrl>

#include <vgExport.h>

#include <vsDataSource.h>

class vsArchiveSourcePrivate;

//-----------------------------------------------------------------------------
class VSP_SOURCEUTIL_EXPORT vsArchiveSourceHelper
{
public:
  virtual ~vsArchiveSourceHelper();

  void start();

  vsDataSource::Status status() const;
  QString text() const;
  QString toolTip() const;

protected:
  QTE_DECLARE_PRIVATE_PTR(vsArchiveSource)

  void finishConstruction();
  void wait();

  virtual void emitStatusChanged(vsDataSource::Status) = 0;
  virtual void suicide() = 0;

private:
  QTE_DECLARE_PRIVATE(vsArchiveSource)
  QTE_DISABLE_COPY(vsArchiveSourceHelper)

  template <typename T> friend class vsArchiveSource;
  vsArchiveSourceHelper(vsArchiveSourcePrivate*);
};

//-----------------------------------------------------------------------------
template <typename T>
class vsArchiveSource : public T, protected vsArchiveSourceHelper
{
public:
  ~vsArchiveSource() { this->wait(); }

  virtual void start() QTE_OVERRIDE;

  virtual vsDataSource::Status status() const QTE_OVERRIDE
    { return vsArchiveSourceHelper::status(); }

  virtual QString text() const QTE_OVERRIDE
    { return vsArchiveSourceHelper::text(); }

  virtual QString toolTip() const QTE_OVERRIDE
    { return vsArchiveSourceHelper::toolTip(); }

protected:
#if defined(__GNUC__) && (__GNUC__ == 4) && (__GNUC_MINOR__ < 7)
  // Work around bug in g++ 4.x < 4.7 where 'using <base>::d_ptr' appears to do
  // nothing
  vsArchiveSourcePrivate* const& d_ptr;

  vsArchiveSource(vsArchiveSourcePrivate* q) :
    vsArchiveSourceHelper(q), d_ptr(vsArchiveSourceHelper::d_ptr)
    { this->finishConstruction(); }
#else
  using vsArchiveSourceHelper::d_ptr;

  vsArchiveSource(vsArchiveSourcePrivate* q) : vsArchiveSourceHelper(q)
    { this->finishConstruction(); }
#endif

  virtual void emitStatusChanged(vsDataSource::Status status) QTE_OVERRIDE
    { emit this->statusChanged(status); }

  virtual void suicide() QTE_OVERRIDE { T::suicide(); }

private:
  QTE_DECLARE_PRIVATE(vsArchiveSource)
  QTE_DISABLE_COPY(vsArchiveSource)
};

//-----------------------------------------------------------------------------
template <typename T>
void vsArchiveSource<T>::start()
{
  T::start();
  vsArchiveSourceHelper::start();
}

//-----------------------------------------------------------------------------
template <typename T>
vsArchiveSourcePrivate* vsArchiveSource<T>::d_func()
{
  return vsArchiveSourceHelper::d_func();
}

//-----------------------------------------------------------------------------
template <typename T>
const vsArchiveSourcePrivate* vsArchiveSource<T>::d_func() const
{
  return vsArchiveSourceHelper::d_func();
}

#endif
