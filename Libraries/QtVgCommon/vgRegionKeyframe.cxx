// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include "vgRegionKeyframe.h"

#include <QApplication>
#include <QDebug>
#include <QDomDocument>
#include <QDomNode>
#include <QFile>
#include <QMessageBox>

//-----------------------------------------------------------------------------
QList<vgRegionKeyframe> vgRegionKeyframe::readFromFile(QString fileName)
{
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
    QString msg = "Unable to open file \"%1\" for reading: %2";
    QMessageBox::critical(qApp->activeWindow(), "Error reading file",
                          msg.arg(fileName).arg(file.errorString()));
    return QList<vgRegionKeyframe>();
    }

  QList<vgRegionKeyframe> loadedKeyframes;

  QByteArray data = file.readAll();
  if (data.startsWith("<?xml"))
    {
    QDomDocument doc;
    int el, ec;
    QString et;
    if (!doc.setContent(data, &et, &el, &ec))
      {
      QString msg =
        "The file \"%1\" does not appear to be a valid XML document"
        "\n\n%2:%3: %4";
      msg = msg.arg(fileName).arg(el).arg(ec).arg(et);
      QMessageBox::critical(qApp->activeWindow(), "Error reading file", msg);
      return QList<vgRegionKeyframe>();
      }
    int readErrors = 0;

    // Get the block
    QDomNodeList blocks = doc.elementsByTagName("queryRegionElements");
    if (blocks.isEmpty())
      {
      QMessageBox::warning(qApp->activeWindow(), "Error reading file",
                           "No query region was found");
      return QList<vgRegionKeyframe>();
      }

    // Get the region elements
    QDomNodeList elements = blocks.item(0).childNodes();
    for (uint i = 0; i < elements.length(); ++i)
      {
      // Check if this element is the correct tag
      QDomElement e = elements.item(i).toElement();
      if (e.tagName() != "regionElement")
        {
        continue;
        }

      // Get image region
      QRect rect;
      if (!(e.hasAttribute("x") && e.hasAttribute("y") &&
            e.hasAttribute("width") && e.hasAttribute("height")))
        {
        qDebug() << "At block 0, element" << i
                 << "- missing one or more attributes"
                    " describing the image region";
        ++readErrors;
        continue;
        }
      rect.setTop(e.attribute("y").toInt());
      rect.setLeft(e.attribute("x").toInt());
      rect.setWidth(e.attribute("width").toInt());
      rect.setHeight(e.attribute("height").toInt());

      // Get frame number
      if (!e.hasAttribute("frameNumber"))
        {
        qDebug() << "At block 0, element" << i
                 << "- missing 'frameNumber' attribute";
        ++readErrors;
        continue;
        }
      unsigned int frameNumber = e.attribute("frameNumber").toUInt();

      // Add keyframe
      vgRegionKeyframe kf;
      kf.Region = rect;
      kf.Time = vgTimeStamp::fromFrameNumber(frameNumber);
      loadedKeyframes.append(kf);
      }

    if (readErrors)
      {
      QString msg = (readErrors > 1
                     ? "%1 keyframes were not valid and have been discarded."
                     : "%1 keyframe was not valid and has been discarded.");
      QMessageBox::warning(qApp->activeWindow(), "Errors occurred",
                           msg.arg(readErrors));
      }
    }
  else
    {
    // \TODO KST-based file format not implemented yet!
    QMessageBox::warning(qApp->activeWindow(), "Not implemented",
                         "Reading of .vvk files is not implemented yet");
    }

  return loadedKeyframes;
}
