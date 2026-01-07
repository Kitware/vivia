// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#ifndef __vgFlags_h
#define __vgFlags_h

#define VG_DECLARE_FLAGS(_flags, _enum) typedef vgFlags<_enum> _flags;

#define VG_DECLARE_OPERATORS_FOR_FLAGS(_flags) \
  inline _flags operator|(_flags::EnumType f1, _flags::EnumType f2) \
    { return _flags(f1) | f2; } \
  inline _flags operator|(_flags::EnumType f1, _flags f2) \
    { return f2 | f1; } \
  inline vgIncompatibleFlag operator|(_flags::EnumType f1, int f2) \
    { return vgIncompatibleFlag(static_cast<int>(f1) | f2); }

//-----------------------------------------------------------------------------
class vgIncompatibleFlag
{
public:
  inline explicit vgIncompatibleFlag(int i) : i(i) {}
  inline operator int() const { return i; }
protected:
  int i;
};

//-----------------------------------------------------------------------------
template <typename Enum> class vgFlags
{
public:
  typedef Enum EnumType;

  inline vgFlags() : i(0) {}
  inline vgFlags(Enum flag) : i(flag) {}
  inline vgFlags(const vgFlags& other) : i(other.i) {}

#ifdef SHIBOKEN
  inline vgFlags(int value) : i(value) {}

  inline vgFlags& operator=(const QFlags<Enum>& other)
    { this->i = other; return *this; }

  inline operator QFlags<Enum>() const { return QFlag(this->i); }
#endif

  inline vgFlags& operator=(const vgFlags& other)
    { this->i = other.i; return *this; }

  inline vgFlags& operator&=(int mask)
    { this->i &= mask; return *this; }
  inline vgFlags& operator&=(unsigned int mask)
    { this->i &= mask; return *this; }

  inline vgFlags& operator|=(vgFlags f)
    { this->i |= f.i; return *this; }
  inline vgFlags& operator|=(Enum f)
    { this->i |= f; return *this; }

  inline vgFlags& operator^=(vgFlags f)
    { this->i ^= f.i; return *this; }
  inline vgFlags& operator^=(Enum f)
    { this->i ^= f; return *this; }

  inline operator int() const { return this->i; }

  inline vgFlags operator&(Enum f) const
    { vgFlags g; g.i = this->i & f; return g; }
  inline vgFlags operator&(int mask) const
    { vgFlags g; g.i = this->i & mask; return g; }
  inline vgFlags operator&(unsigned int mask) const
    { vgFlags g; g.i = this->i & mask; return g; }

  inline vgFlags operator|(vgFlags f) const
    { vgFlags g; g.i = this->i | f.i; return g; }
  inline vgFlags operator|(Enum f) const
    { vgFlags g; g.i = this->i | f; return g; }

  inline vgFlags operator^(vgFlags f) const
    { vgFlags g; g.i = this->i ^ f.i; return g; }
  inline vgFlags operator^(Enum f) const
    { vgFlags g; g.i = this->i ^ f; return g; }

  inline int operator~() const
    { return ~this->i; }

  inline bool operator!() const { return !i; }

  inline bool testFlag(Enum f) const
    { return (i & f) == f && (f != 0 || i == 0); }

protected:
  int i;
};

#endif
