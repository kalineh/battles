//
// math extension helpers for linalg.h
//

#ifndef LINALGEX_H_INCLUDED
#define LINALGEX_H_INCLUDED

#include "linalg.h"

#define LINALG_PI 3.1415926535897932384f
#define LINALG_TWOPI 6.28318530717958647692f
#define LINALG_HALFPI 1.57079632679489661923132f

#ifdef __cplusplus
namespace linalg {
#endif

static inline v4
v4rgba(real r, real g, real b, real a)
{
	return v4new(r, g, b, a);
}

static inline v4
v4rgb1(real r, real g, real b)
{
	return v4new(r, g, b, 1.0f);
}

static inline v3
v3moveto(v3 src, v3 dst, float d)
{
	v3 ofs = dst - src;
	real len = v3lensafe(ofs);
	v3 dir = v3div(ofs, len);
	real move = LINALG_MIN(d, len);
	v3 result = v3add(src, v3mul(dir, move));
	return result;
}

static inline v2
v2fromangle(float angle)
{
	return v2new(LINALG_COS(angle), LINALG_SIN(angle));
}

static inline real
v2toangle(v2 v)
{
	real a = LINALG_ATAN2(v.y, v.x);
	if (a < 0)
		a += LINALG_TWOPI;
	return a;
}

static inline float
anglediff(real a, real b)
{
	return LINALG_PI - LINALG_ABS(LINALG_ABS(a - b) - LINALG_PI);
}

static inline real
angleto(real src, real dst, float d)
{
	real diff = anglediff(dst, src);
	if (diff < 0)
		diff = LINALG_MAX(diff, -d);
	if (diff > 0)
		diff = LINALG_MIN(diff, d);
	return src + diff;
}

static inline bool
rectcontains(v2 p, v2 bl, v2 tr)
{
	if (p.x > tr.x) return false;
	if (p.y > tr.y) return false;
	if (p.x < bl.x) return false;
	if (p.y < bl.y) return false;
	return true;
}

static inline bool
rectoverlap(v2 abl, v2 atr, v2 bbl, v2 btr)
{
	if (abl.x > btr.x) return false;
	if (abl.y > btr.y) return false;
	if (atr.x < bbl.x) return false;
	if (atr.y < bbl.y) return false;
	return true;
}

#ifdef __cplusplus
} /* namespace linalg */
#endif

#endif /* LINALGEX_H_INCLUDED */
