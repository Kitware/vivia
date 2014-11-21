#ckwg +4
# Copyright 2013 by Kitware, Inc. All Rights Reserved. Please refer to
# KITWARE_LICENSE.TXT for licensing information, or contact General Counsel,
# Kitware, Inc., 28 Corporate Drive, Clifton Park, NY 12065.

from vgDataFrameworkPython import *
from vvIOPython import *

#==============================================================================
class QueryService:
  #----------------------------------------------------------------------------
  def __init__(self):
    self._completed = False
    self._node = None
    self._message = ''

  #----------------------------------------------------------------------------
  def ExecuteQuery(self, service, plan, workingSetSize):
    if type(plan) is str or type(plan) is unicode:
      reader = vvReader()
      header = vvHeader()

      if not reader.setInput(plan):
        self.SetStatus('Failed to parse query plan: KST syntax error')
        return False

      if not reader.readHeader(header):
        self.SetStatus('Failed to parse query plan: error reading header')
        return False

      if not header.isValid():
        self.SetStatus('Failed to parse query plan: invalid header')
        return False

      if not header.type == vvHeader.QueryPlan:
        self.SetStatus('Failed to parse query plan: header mismatch')
        return False

      plan = vvQueryInstance()
      if not reader.readQueryPlan(plan):
        self.SetStatus('Failed to parse query plan')
        return False

    self._node = vdfQuerySessionNode(service)
    self._node.resultSetComplete.connect(self.Complete)
    self._node.error.connect(self.SetStatus)

    sn = self._node.statusNotifier()
    sn.statusMessageAvailable.connect(self.SetStatus)

    if not self._node.canExecute:
      self.SetStatus('Failed to create query session')
      return False

    return self._node.execute(plan, workingSetSize)

  #----------------------------------------------------------------------------
  def Refine(self):
    self._completed = False
    return False if self._node is None else self._node.refine()

  #----------------------------------------------------------------------------
  def ShutDown(self):
    self._node = None

  #----------------------------------------------------------------------------
  def IsExecuting(self):
    return self._node is not None and self._node.isBusy()

  #----------------------------------------------------------------------------
  def IsCompleted(self):
    return self._completed

  #----------------------------------------------------------------------------
  def GetResultCount(self):
    return 0 if self._node is None else self._node.resultCount()

  #----------------------------------------------------------------------------
  def GetResultsAndFeedbackRequests(self, *args):
    if self._node is None:
      return {'results': {}, 'feedbackIds': []}

    results = {}
    order = vdfAbstractQueryResultSetNode.SortByRank
    result_ids = self._node.results(order, *args)
    feedback_ids = self._node.feedbackRequests()

    for iid in result_ids:
      results[iid] = vvJson.serialize(self._node.result(iid))

    for iid in feedback_ids:
      if iid not in result_ids:
        results[iid] = vvJson.serialize(self._node.result(iid))

    return {
      'results': results,
      'resultIds': result_ids,
      'feedbackIds': feedback_ids
    }

  #----------------------------------------------------------------------------
  def GetSerializedResult(self, iid):
    if self._node is None:
      return None
    return vvJson.serialize(self._node.result(iid))

  #----------------------------------------------------------------------------
  def SetResultFeedback(self, iid, score):
    if self._node is None:
      return False
    return self._node.setResultFeedback(iid, vvIqr.Classification(score))

  #----------------------------------------------------------------------------
  def GetStatus(self):
    return self._message

  #----------------------------------------------------------------------------
  def SetStatus(self, *args):
    self._message = args[-1]

  #----------------------------------------------------------------------------
  def Complete(self, feedbackRequests):
    self._completed = feedbackRequests
