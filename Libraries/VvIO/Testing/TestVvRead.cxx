/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <QDebug>
#include <QUrl>

#include <qtCliArgs.h>

#include <vvQueryResult.h>

#include "../vvEventSetInfo.h"
#include "../vvHeader.h"
#include "../vvReader.h"

#define die(_msg) do { \
  qWarning() << "error parsing input file" << fileName << '-' << _msg; \
  return 1; \
  } while (0)

#define test_or_die(_expr, _msg) \
  if (!(_expr)) die(_msg)

//-----------------------------------------------------------------------------
int testRead(const QString& fileName)
{
  vvReader reader;
  vvHeader header;

  if (!reader.open(QUrl::fromLocalFile(fileName)))
    {
    qWarning() << "unable to open input file" << fileName;
    return 1;
    }

  while (!reader.atEnd())
    {
    test_or_die(reader.readHeader(header), "unable to read header");

    switch (header.type)
      {
      case vvHeader::EventSetInfo:
        {
        vvEventSetInfo info;
        test_or_die(reader.readEventSetInfo(info),
                    "unable to read event set info");
        qDebug() << "read an event set info";
        } break;
      case vvHeader::QueryResults:
        {
        QList<vvQueryResult> results;
        test_or_die(reader.readQueryResults(results),
                    "unable to read query results");
        qDebug() << "read" << results.count() << "results";
        } break;
      default:
        die("data format not supported");
      }
    }

  return 0;
}

//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  qtCliArgs args(argc, argv);

  qtCliOptions nargs;
  nargs.add("file", "KST file(s) to read", qtCliOption::NamedList);

  args.addNamedArguments(nargs);

  args.parseOrDie();

  int result = 0;
  foreach (const QString& fileName, args.values("file"))
    {
    result |= testRead(fileName);
    }

  return result;
}
