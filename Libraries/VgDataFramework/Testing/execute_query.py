#!/bin/env python

#ckwg +5
# Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
# KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
# Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.
#

import json
import os
import sys

from PySide.QtCore import *
from PySide.QtGui import *

from vgDataFrameworkPython import *
from vvIOPython import *

#==============================================================================
def usage(exit_code=0):
  pname = os.path.basename(__file__)
  sys.stderr.write("Usage: %s <query service> <query plan>\n" % pname)
  sys.exit(exit_code)

#==============================================================================
def die_bad_query_plan(path):
    sys.stderr.write("Failed to load query plan from '%s'\n" % path)
    sys.exit(2)

#==============================================================================
def read_query_plan(path):
  reader = vvReader()
  reader.open(path) or die_bad_query_plan(path)

  head = vvHeader()
  reader.readHeader(head) or die_bad_query_plan(path)
  if not head.type == vvHeader.QueryPlan:
    sys.stderr.write("'%s' does not contain a query plan\n" % path)
    sys.exit(2)

  plan = vvQueryInstance()
  reader.readQueryPlan(plan) or die_bad_query_plan(path)

  return plan

#==============================================================================
class Application(QCoreApplication):
  #----------------------------------------------------------------------------
  def __init__(self, args):
    QCoreApplication.__init__(self, args)
    self.m_node = None

  #----------------------------------------------------------------------------
  def createSessionNode(self, service):
    self.m_node = vdfQuerySessionNode(service)

    self.m_node.resultSetComplete.connect(self.printResults)

    return self.m_node

  #----------------------------------------------------------------------------
  def printResults(self):
    print self.m_node.resultCount()
    results = self.m_node.results(vdfAbstractQueryResultSetNode.SortByRelevancy)
    for r in results:
      print json.dumps(vvJson.serialize(self.m_node.result(r)), indent=2)
    self.exit()

#==============================================================================
def main(args):

  if len(args) < 3:
    usage(exit_code=1)

  app = Application(args)

  plan = read_query_plan(args[2])

  node = app.createSessionNode(QUrl(args[1]))
  if not node.canExecute():
    sys.stderr.write("Node cannot execute query\n")
    sys.exit(2)

  node.execute(plan, 3000)
  app.exec_()

#==============================================================================
if __name__ == '__main__':
  main(sys.argv)
