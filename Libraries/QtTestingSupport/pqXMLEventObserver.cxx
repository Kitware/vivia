/*=========================================================================

   Program: ParaView
   Module:    pqXMLEventObserver.cxx

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

#include "pqXMLEventObserver.h"

#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>
#include <QTextStream>

///////////////////////////////////////////////////////////////////////////////
// pqXMLEventObserver::pqImplementation

class pqXMLEventObserver::pqImplementation
{
public:
  void initialize();

  QDomDocument XML;
  QDomNode RootNode;
};

void pqXMLEventObserver::pqImplementation::initialize()
{
  static const char* header = "version='1.0' encoding='UTF-8'";

  // Note: the Qt documentation says not to use createProcessingInstruction to
  // create the XML declaration, but there does not seem to be any other way to
  // do so (short of writing the declaration as a preformatted string literal
  // and using QDomDocument::setContent, which seems to defeat the purpose).

  this->XML.clear();
  this->XML.appendChild(this->XML.createProcessingInstruction("xml", header));
  this->XML.appendChild(this->XML.createElement("pqevents"));
  this->RootNode = this->XML.firstChildElement();
}


///////////////////////////////////////////////////////////////////////////////
// pqXMLEventObserver

pqXMLEventObserver::pqXMLEventObserver(QObject* p)
  : pqEventObserver(p), Implementation(new pqXMLEventObserver::pqImplementation)
{
}

pqXMLEventObserver::~pqXMLEventObserver()
{
  this->setStream(0);
}

void pqXMLEventObserver::setStream(QTextStream* stream)
{
  if (this->Stream)
    {
    *this->Stream << this->Implementation->XML;
    }
  pqEventObserver::setStream(stream);
  if (this->Stream)
    {
    this->Implementation->initialize();
    }
}

void pqXMLEventObserver::onRecordEvent(
  const QString& Widget,
  const QString& Command,
  const QString& Arguments)
{
  if (this->Stream)
    {
    QDomElement elem = this->Implementation->XML.createElement("pqevent");
    elem.setAttribute("object", Widget);
    elem.setAttribute("command", Command);
    elem.setAttribute("arguments", Arguments);
    this->Implementation->RootNode.appendChild(elem);
    }
}
