// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

// templates for static function pointer
template <typename LT, typename MF QTMAP_EXTRA_TEMPLATE>
void map(const QList<LT>& list, MF func QTMAP_EXTRA_ARGS)
{
  Q_FOREACH(const LT& item, list)
  (*func)(item QTMAP_EXTRA_INVOKE);
}

template <typename KT, typename LT, typename MF QTMAP_EXTRA_TEMPLATE>
void map(QHash<KT, LT> const& list, MF func QTMAP_EXTRA_ARGS)
{
  Q_FOREACH(const LT& item, list)
  (*func)(item QTMAP_EXTRA_INVOKE);
}

template <typename KT, typename LT, typename MF QTMAP_EXTRA_TEMPLATE>
void map(QMap<KT, LT> const& list, MF func QTMAP_EXTRA_ARGS)
{
  Q_FOREACH(const LT& item, list)
  (*func)(item QTMAP_EXTRA_INVOKE);
}

template <typename LT, typename MF QTMAP_EXTRA_TEMPLATE>
void map(const QSet<LT>& list, MF func QTMAP_EXTRA_ARGS)
{
  Q_FOREACH(const LT& item, list)
  (*func)(item QTMAP_EXTRA_INVOKE);
}

template <typename KT, typename VT, typename MF QTMAP_EXTRA_TEMPLATE>
void mapWithKey(QHash<KT, VT> const& list, MF func QTMAP_EXTRA_ARGS)
{
  typedef QHash<KT, VT> LT;
  QTE_FOREACH_ITER (typename LT::const_iterator, iter, list)
    (*func)(iter.key(), iter.value() QTMAP_EXTRA_INVOKE);
}

template <typename KT, typename VT, typename MF QTMAP_EXTRA_TEMPLATE>
void mapWithKey(QMap<KT, VT> const& list, MF func QTMAP_EXTRA_ARGS)
{
  typedef QMap<KT, VT> LT;
  QTE_FOREACH_ITER (typename LT::const_iterator, iter, list)
    (*func)(iter.key(), iter.value() QTMAP_EXTRA_INVOKE);
}

// templates for bound (member) function pointer; pointer to object
template <typename LT, typename MO, typename MF QTMAP_EXTRA_TEMPLATE>
void mapBound(const QList<LT>& list, MO* mapObject,
              MF func QTMAP_EXTRA_ARGS)
{
  Q_FOREACH(const LT& item, list)
  (*mapObject.*func)(item QTMAP_EXTRA_INVOKE);
}

template < typename KT, typename LT, typename MO,
         typename MF QTMAP_EXTRA_TEMPLATE >
void mapBound(QHash<KT, LT> const& list, MO* mapObject,
              MF func QTMAP_EXTRA_ARGS)
{
  Q_FOREACH(const LT& item, list)
  (*mapObject.*func)(item QTMAP_EXTRA_INVOKE);
}

template < typename KT, typename LT, typename MO,
         typename MF QTMAP_EXTRA_TEMPLATE >
void mapBound(QMap<KT, LT> const& list, MO* mapObject,
              MF func QTMAP_EXTRA_ARGS)
{
  Q_FOREACH(const LT& item, list)
  (*mapObject.*func)(item QTMAP_EXTRA_INVOKE);
}

template <typename LT, typename MO, typename MF QTMAP_EXTRA_TEMPLATE>
void mapBound(const QSet<LT>& list, MO* mapObject,
              MF func QTMAP_EXTRA_ARGS)
{
  Q_FOREACH(const LT& item, list)
  (*mapObject.*func)(item QTMAP_EXTRA_INVOKE);
}

template < typename KT, typename VT, typename MO,
         typename MF QTMAP_EXTRA_TEMPLATE >
void mapBoundWithKey(QHash<KT, VT> const& list, MO* mapObject,
                     MF func QTMAP_EXTRA_ARGS)
{
  typedef QHash<KT, VT> LT;
  QTE_FOREACH_ITER (typename LT::const_iterator, iter, list)
    (*mapObject.*func)(iter.key(), iter.value() QTMAP_EXTRA_INVOKE);
}

template < typename KT, typename VT, typename MO,
         typename MF QTMAP_EXTRA_TEMPLATE >
void mapBoundWithKey(QMap<KT, VT> const& list, MO* mapObject,
                     MF func QTMAP_EXTRA_ARGS)
{
  typedef QMap<KT, VT> LT;
  QTE_FOREACH_ITER (typename LT::const_iterator, iter, list)
    (*mapObject.*func)(iter.key(), iter.value() QTMAP_EXTRA_INVOKE);
}

// templates for bound (member) function pointer; reference to object
template <typename LT, typename MO, typename MF QTMAP_EXTRA_TEMPLATE>
void mapBound(const QList<LT>& list, MO& mapObject,
              MF func QTMAP_EXTRA_ARGS)
{
  Q_FOREACH(const LT& item, list)
  (mapObject.*func)(item QTMAP_EXTRA_INVOKE);
}

template < typename KT, typename LT, typename MO,
         typename MF QTMAP_EXTRA_TEMPLATE >
void mapBound(QHash<KT, LT> const& list, MO& mapObject,
              MF func QTMAP_EXTRA_ARGS)
{
  Q_FOREACH(const LT& item, list)
  (mapObject.*func)(item QTMAP_EXTRA_INVOKE);
}

template < typename KT, typename LT, typename MO,
         typename MF QTMAP_EXTRA_TEMPLATE >
void mapBound(QMap<KT, LT> const& list, MO& mapObject,
              MF func QTMAP_EXTRA_ARGS)
{
  Q_FOREACH(const LT& item, list)
  (mapObject.*func)(item QTMAP_EXTRA_INVOKE);
}

template <typename LT, typename MO, typename MF QTMAP_EXTRA_TEMPLATE>
void mapBound(const QSet<LT>& list, MO& mapObject,
              MF func QTMAP_EXTRA_ARGS)
{
  Q_FOREACH(const LT& item, list)
  (mapObject.*func)(item QTMAP_EXTRA_INVOKE);
}

template < typename KT, typename VT, typename MO,
         typename MF QTMAP_EXTRA_TEMPLATE >
void mapBoundWithKey(QHash<KT, VT> const& list, MO& mapObject,
                     MF func QTMAP_EXTRA_ARGS)
{
  typedef QHash<KT, VT> LT;
  QTE_FOREACH_ITER (typename LT::const_iterator, iter, list)
    (mapObject.*func)(iter.key(), iter.value() QTMAP_EXTRA_INVOKE);
}

template < typename KT, typename VT, typename MO,
         typename MF QTMAP_EXTRA_TEMPLATE >
void mapBoundWithKey(QMap<KT, VT> const& list, MO& mapObject,
                     MF func QTMAP_EXTRA_ARGS)
{
  typedef QMap<KT, VT> LT;
  QTE_FOREACH_ITER (typename LT::const_iterator, iter, list)
    (mapObject.*func)(iter.key(), iter.value() QTMAP_EXTRA_INVOKE);
}
