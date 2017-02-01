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

static inline v2
v2moveto(v2 src, v2 dst, float d)
{
	v2 ofs = dst - src;
	real len = v2lensafe(ofs);
	v2 dir = v2unitsafe(ofs);
	real move = LINALG_MIN(d, len);
	v2 result = v2add(src, v2mul(dir, move));
	return result;
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
	if (a > LINALG_PI) a -= LINALG_TWOPI;
	if (b > LINALG_PI) b -= LINALG_TWOPI;
	real d = b - a;
	if (d < -LINALG_PI) d += LINALG_TWOPI;
	if (d > +LINALG_PI) d -= LINALG_TWOPI;
	return d;
}

static inline real
anglewrap0TWOPI(real f)
{
	while (f < 0)
		f += LINALG_TWOPI;
	while (f >= LINALG_TWOPI)
		f -= LINALG_TWOPI;
	return f;
}

static inline real
angleto(real src, real dst, real d)
{
	src = anglewrap0TWOPI(src);
	dst = anglewrap0TWOPI(dst);

	real diff = anglediff(src, dst);
	real move = diff < 0 ? LINALG_MAX(diff, -d) : LINALG_MIN(diff, d);

	return src + move;
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

static inline bool
circleoverlap(v2 ap, float ar, v2 bp, float br)
{
	real radsq = (ar + br) * (ar + br);
	real lensq = v2lensq(bp - ap);
	return lensq < radsq;
}

static inline v3
v3desaturatergb(v3 rgb)
{
	float g = rgb.x * 0.299f + rgb.y * 0.587f + rgb.z * 0.144f;
	return v3new(g, g, g);
}

static inline v4
v4desaturatergb(v4 rgba)
{
	float g = rgba.x * 0.299f + rgba.y * 0.587f + rgba.z * 0.144f;
	return v4new(g, g, g, rgba.w);
}

static inline v2
v2rotate(v2 v, float a)
{
	float c = cosf(a);
	float s = sinf(a);
	return v2new(c * v.x - s * v.y, s * v.x + c * v.y);
}

#ifdef __cplusplus
} /* namespace linalg */
#endif

#endif /* LINALGEX_H_INCLUDED */
