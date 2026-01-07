// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

#include <qtTest.h>

#include "../vgAttributeSet.h"

enum { NumGroups = 2, NumAttrsPerGroup = 3 };

const char* Groups[NumGroups] = { "GroupA", "GroupB" };
const char* Attrs[][NumAttrsPerGroup] =
{
  { "a1", "a2", "a3" },
  { "b1", "b2", "b3" }
};

unsigned int Masks[][NumAttrsPerGroup] =
{
  { 1, 2, 4 },
  { 4, 8, 16 }
};

unsigned int GroupMasks[NumGroups] =
{
  Masks[0][0] | Masks[0][1] | Masks[0][2],
  Masks[1][0] | Masks[1][1] | Masks[1][2]
};

//-----------------------------------------------------------------------------
int testAll(qtTest& testObject)
{
  vgAttributeSet as;
  TEST(as.GetGroups().empty());
  TEST(as.GetEnabledGroups().empty());
  TEST(as.GetAttributes("foo").empty());
  TEST(as.GetGroupMask("foo") == 0);
  TEST(as.GetMask("foo", "bar") == 0);

  for (int i = 0; i < NumGroups; ++i)
    {
    if (i == 0)
      {
      as.SetGroupEnabled(Groups[i], false);
      }
    for (int j = 0; j < NumAttrsPerGroup; ++j)
      {
      as.SetMask(Groups[i], Attrs[i][j], Masks[i][j]);
      TEST(as.GetMask(Groups[i], Attrs[i][j]) == Masks[i][j]);
      }
    }

  for (int i = 0; i < NumGroups; ++i)
    {
    TEST(as.GetGroupMask(Groups[i]) == GroupMasks[i]);
    std::vector<vgAttribute> attrs = as.GetAttributes(Groups[i]);
    TEST(attrs.size() == NumAttrsPerGroup);
    TEST(as.IsGroupEnabled(Groups[i]) == (i != 0));

    for (int j = 0; j < NumAttrsPerGroup; ++j)
      {
      TEST(as.GetMask(Groups[i], Attrs[i][j]) == Masks[i][j]);
      TEST(attrs[j].Name == Attrs[i][j]);
      TEST(attrs[j].Mask == Masks[i][j]);
      }
    }

    {
    std::vector<std::string> groups = as.GetGroups();
    TEST(groups.size() == NumGroups);
    for (size_t i = 0, size = groups.size(); i < size; ++i)
      {
      TEST(groups[i] == Groups[i]);
      }
    }

    {
    std::vector<std::string> groups = as.GetEnabledGroups();
    TEST(groups.size() == NumGroups - 1);
    for (size_t i = 0, size = groups.size(); i < size; ++i)
      {
      TEST(groups[i] == Groups[i + 1]);
      }
    }

  as.Clear();
  TEST(as.GetGroups().empty());
  TEST(as.GetEnabledGroups().empty());

  return 0;
}

//-----------------------------------------------------------------------------
int main(int argc, const char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

  qtTest testObject;

  testObject.runSuite("All Tests", testAll);

  return testObject.result();
}
