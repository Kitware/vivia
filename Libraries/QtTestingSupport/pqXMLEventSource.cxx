/*=========================================================================

   Program: ParaView
   Module:    pqXMLEventSource.cxx

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/

#include "pqXMLEventSource.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QtDebug>

///////////////////////////////////////////////////////////////////////////////
// pqXMLEventSource::pqImplementation

class pqXMLEventSource::pqImplementation
{
public:
  QDomDocument XML;
  QDomNodeList Events;
  int CurrentEvent;
};

///////////////////////////////////////////////////////////////////////////////
// pqXMLEventSource

pqXMLEventSource::pqXMLEventSource(QObject* p) :
  pqEventSource(p),
  Implementation(new pqImplementation())
{
}

pqXMLEventSource::~pqXMLEventSource()
{
  delete this->Implementation;
}

void pqXMLEventSource::setContent(const QString& xmlfilename)
{
  QFile xml(xmlfilename);
  if (!xml.open(QIODevice::ReadOnly))
    {
    qDebug() << "Failed to load" << xmlfilename;
    return;
    }

  QString error;
  int el, ec;
  if (!this->Implementation->XML.setContent(&xml, &error, &el, &ec))
    {
    qDebug() << "Failed to parse" << xmlfilename;
    qDebug() << "At" << qPrintable(QString("%1:%2:").arg(el).arg(ec))
             << qPrintable(error);
    return;
    }

  QDomElement elem = this->Implementation->XML.firstChildElement();
  if (elem.tagName() != "pqevents")
    {
    qCritical() << xmlfilename << "is not an XML test case document";
    return;
    }

  this->Implementation->Events = elem.childNodes();
  this->Implementation->CurrentEvent = 0;
}

int pqXMLEventSource::getNextEvent(
  QString& object,
  QString& command,
  QString& arguments)
{
  if (this->Implementation->Events.count() ==
      this->Implementation->CurrentEvent)
    {
    return DONE;
    }

  QDomElement elem = this->Implementation->Events.at(
    this->Implementation->CurrentEvent).toElement();

  object = elem.attribute("object");
  command = elem.attribute("command");
  arguments = elem.attribute("arguments");

  this->Implementation->CurrentEvent++;

  return SUCCESS;
}
