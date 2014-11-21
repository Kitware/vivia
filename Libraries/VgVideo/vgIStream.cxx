/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include "vgIStream.h"

#ifdef Q_OS_WIN
  #include <cstdio>
  #include <iostream>
#endif
#include <fstream>

// This class exists to wrap a std::istream such that files larger than 2 GiB
// can be read. The trouble is on Windows, where std::streampos is a 32-bit
// integer (even on 64-bit Windows), which means we cannot directly seek to a
// position beyond 2 GiB using std::ifstream. The workaround, implemented by
// this class, is to create a std::istream using a std::filebuf wrapped around
// a FILE*, and use the C API that supports large files to seek the FILE*.

QTE_IMPLEMENT_D_FUNC(vgIStream)

//-----------------------------------------------------------------------------
class vgIStreamPrivate
{
public:
#ifdef Q_OS_WIN
  FILE* file;
  std::filebuf* buffer;
  std::istream* stream;
#else
  QScopedPointer<std::ifstream> stream;
#endif
};

//-----------------------------------------------------------------------------
vgIStream::vgIStream() : d_ptr(new vgIStreamPrivate)
{
#ifdef Q_OS_WIN
  QTE_D(vgIStream);

  d->file = 0;
  d->buffer = 0;
  d->stream = 0;
#endif
}

//-----------------------------------------------------------------------------
vgIStream::~vgIStream()
{
#ifdef Q_OS_WIN
  QTE_D(vgIStream);

  delete d->stream;
  delete d->buffer;
  fclose(d->file);
#endif
}

//-----------------------------------------------------------------------------
bool vgIStream::open(const QString& fileName)
{
  QTE_D(vgIStream);

#ifdef Q_OS_WIN
  if (fopen_s(&d->file, qPrintable(fileName), "rb"))
    {
    return false;
    }
  d->buffer = new std::filebuf(d->file);
  d->stream = new std::istream(d->buffer);
  return true;
#else
  d->stream.reset(new std::ifstream(qPrintable(fileName)));
  return d->stream->good();
#endif
}

//-----------------------------------------------------------------------------
bool vgIStream::seek(qint64 position)
{
  QTE_D(vgIStream);

#ifdef Q_OS_WIN

  // Don't seek if not necessary, to avoid flushing data that has already been
  // read into the buffer
  if (_ftelli64(d->file) == position)
    {
    return true;
    }

  if (_fseeki64(d->file, position, SEEK_SET) == 0)
    {
    d->buffer->pubsync();
    return true;
    }

  d->stream->setstate(std::ios::failbit);
  return false;

#else

  d->stream->seekg(position);
  return d->stream->good();

#endif
}

//-----------------------------------------------------------------------------
vgIStream::operator std::istream*()
{
  QTE_D(vgIStream);
#ifdef Q_OS_WIN
  return d->stream;
#else
  return d->stream.data();
#endif
}
