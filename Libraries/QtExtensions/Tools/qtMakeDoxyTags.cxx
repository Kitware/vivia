/*ckwg +5
 * Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
 * KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
 * Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
 */

#include <QDebug>
#include <QDomDocument>
#include <QFile>
#include <QStringList>
#include <QTextStream>

#include "../Core/qtCliArgs.h"
#include "../Dom/qtDom.h"
#include "../Dom/qtDomElement.h"
#include "qteVersion.h"

//-----------------------------------------------------------------------------
QDomNode parseInput(
  const QString& tag, const QString& filename, QDomDocument& doc)
{
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly))
    {
    qWarning().nospace()
      << "Error opening page " << filename << ": " << file.errorString();
    return QDomNode();
    }

  QDomDocument html;
  QString error;
  int el, ec;
  if (!html.setContent(&file, &error, &el, &ec))
    {
    qWarning().nospace()
      << qPrintable(filename) << ':' << el << ':' << ec
      << ": " << qPrintable(error);
    return QDomNode();
    }

  QList<QDomElement> title = qtDom::findElements(html, "h1");
  if (title.isEmpty())
    {
    qWarning() << "Could not find page title in" << filename;
    return QDomNode();
    }

  // TODO extract sections?
  // <docanchor file="${tag}">${anchor-name}</docanchor>

  qtDomElement node(doc, "compound");
  node.setAttribute("kind", "page");
  node.add(qtDomElement(doc, "name").addText(tag))
      .add(qtDomElement(doc, "title").addText(title[0].text()))
      .add(qtDomElement(doc, "filename").addText(tag));

  return node;
}

//-----------------------------------------------------------------------------
int main(int argc, char** argv)
{
  qtCliArgs args(argc, argv);

  // Set up options
  qtCliOptions options;
  options.add("o <file>", "Name of output tag file to create",
              qtCliOption::Required);
  options.add("d <dir>", "Location of Qt HTML documentation",
              qtCliOption::Required);
  args.addOptions(options);

  qtCliOptions nargs;
  nargs.add("tags", "List of tags (page names) to extract",
            qtCliOption::NamedList);
  args.addNamedArguments(nargs);

  // Parse arguments
  args.parseOrDie();
  const QString outname = args.value("o");
  const QString docdir = args.value("d");

  // Create tagfile root
  QDomDocument tags;
  static const char* const processing =
    "version='1.0' encoding='utf-8' standalone='yes' ";
  tags.appendChild(tags.createProcessingInstruction("xml", processing));
  QDomElement root = tags.createElement("tagfile");

  // Extract tags
  int errors = 0;
  foreach(const QString& tag, args.values("tags"))
    {
    qDebug() << "Extracting tag" << tag;
    QDomNode node = parseInput(tag, docdir + '/' + tag + ".html", tags);
    if (node.isNull())
      {
      ++errors;
      continue;
      }
    root.appendChild(node);
    }

  if (errors)
    {
    qWarning() << "Error(s) occurred; no output was written";
    return 1;
    }

  tags.appendChild(root);

  // Write output tag file
  QFile out(outname);
  if (!out.open(QIODevice::WriteOnly))
    {
    qWarning().nospace()
      << "Error opening output file " << outname << ": " << out.errorString();
    return 1;
    }

  QTextStream stream(&out);
  stream.setCodec("UTF-8");
  stream << tags.toString(2);

  return 0;
}
