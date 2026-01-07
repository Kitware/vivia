// This file is part of ViViA, and is distributed under the
// OSI-approved BSD 3-Clause License. See top-level LICENSE file or
// https://github.com/Kitware/vivia/blob/master/LICENSE for details.

class qtTest;

struct vvDescriptor;
struct vgGeocodedPoly;
struct vvQueryResult;
struct vvTrack;

class vvQueryInstance;

extern void testTrack1(qtTest&, const vvTrack&);
extern void testTrack2(qtTest&, const vvTrack&);
extern void testTrack3(qtTest&, const vvTrack&);
extern void testDescriptor1(qtTest&, const vvDescriptor&);
extern void testDescriptor2(qtTest&, const vvDescriptor&);
extern void testDescriptor3(qtTest&, const vvDescriptor&);
extern void testGeoPoly1(qtTest&, const vgGeocodedPoly&);
extern void testGeoPoly2(qtTest&, const vgGeocodedPoly&);
extern void testGeoPoly3(qtTest&, const vgGeocodedPoly&);
extern void testQueryPlan1(qtTest&, const vvQueryInstance&);
extern void testQueryPlan2(qtTest&, const vvQueryInstance&);
extern void testQueryPlan3(qtTest&, const vvQueryInstance&);
extern void testQueryResult1(qtTest&, const vvQueryResult&, unsigned int);
extern void testQueryResult2(qtTest&, const vvQueryResult&, unsigned int);
extern void testEventSetInfo(qtTest&, const vvEventSetInfo&);
