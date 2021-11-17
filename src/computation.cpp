/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  autopilot route Plugin
 * Author:   Sean D'Epagnier
 *
 ***************************************************************************
 *   Copyright (C) 2018 by Sean D'Epagnier                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 */

#include <math.h>
#include "computation.h"

#ifndef M_PI
      #define M_PI        3.1415926535897931160E0      /* pi */
#endif

double deg2rad(double x) { return x * M_PI / 180; }
double rad2deg(double x) { return x * 180 / M_PI; }

// spherical computations
namespace computation_gc
{

struct vector
{
    vector() {}
    vector(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
    double norm() { return sqrt(x*x + y*y + z*z); }
    void normalize() { double n = norm(); x/=n, y/=n, z/=n; }
    double x, y, z;
};

vector cross(vector &a, vector &b) { return vector(a.y*b.z - a.z*b.y,
                                                   a.z*b.x - a.x*b.z,
                                                   a.x*b.y - a.y*b.x); }
double dot(vector &a, vector &b) { return a.x*b.x + a.y*b.y + a.z*b.z; }

struct quaternion
{
    quaternion(double angle, vector &v) {
        double n = v.norm(), fac;
        if(n == 0)
            fac = 0;
        else
            fac = sin(angle/2) / n;
        r = cos(angle/2), i = v.x*fac, j = v.y*fac, k = v.z*fac;
    }
    quaternion operator*(const quaternion &q) {
        return quaternion(r*q.r - i*q.i - j*q.j - k*q.k,
                          r*q.i + i*q.r + j*q.k - k*q.j,
                          r*q.j - i*q.k + j*q.r + k*q.i,
                          r*q.k + i*q.j - j*q.i + k*q.r);
    }

    quaternion conjugate() { return quaternion(r, -i, -j, -k); }

    vector rotate(vector &v) {
        quaternion w(0, v.x, v.y, v.z);
        quaternion r = *this*w*conjugate();
        return vector(r.i, r.j, r.k);
    }

private:
    quaternion(double _r, double _i, double _j, double _k) : r(_r), i(_i), j(_j), k(_k) {}
    double r, i, j, k;
};


const double earth_radius_meters       = 6378137.0;
double m2rad(double x) { return x/earth_radius_meters; }

vector ll2v(wp &p)
{
    double lat = deg2rad(p.lat), lon = deg2rad(p.lon);
    return vector(cos(lat)*sin(lon), cos(lat)*cos(lon), sin(lat));
}

wp v2ll(vector a)
{
    return wp(rad2deg(asin(a.z)), rad2deg(atan2(a.x, a.y)));
}

// find closest vector to c on great circle defined by v0 and v1
vector closest_vector_n(vector &c, vector &v0, vector &v1, vector &n)
{
    n = cross(v0, v1);
    n.normalize();
    vector m = cross(n, c);
    m.normalize();
    return cross(m, n);
}

vector closest_vector(vector &c, vector &v0, vector &v1)
{
    vector n;
    return closest_vector_n(c, v0, v1, n);
}

// find position closest to p, on great circle defined by p0 and p1
wp closest(wp &p, wp &p0, wp &p1)
{
    vector v = ll2v(p), v0 = ll2v(p0), v1 = ll2v(p1);
    return v2ll(closest_vector(v, v0, v1));
}

// find position closest to p, on great circle segment defined by p0 and p1
wp closest_seg(wp &p, wp &p0, wp &p1)
{
    vector c = ll2v(p), v0 = ll2v(p0), v1 = ll2v(p1);
    vector v = closest_vector(c, v0, v1);
    double d0 = dot(v0, v1), d1 = dot(v, v0), d2 = dot(v, v1);
    if(d1 > d0 && d2 > d0)
        return v2ll(v);
    if(d1 > d2)
        return p0;
    return p1;
}

// find distance in radians between two positions
double distance(wp &p0, wp &p1)
{
    vector v0 = ll2v(p0), v1 = ll2v(p1);
    return acos(dot(v0, v1));
}

// set w to the intersection of (position p, radius r) on great circle
// segment defined by p0 and p1 if circle intersects segment twice,
// return position closest to p1
bool intersect_circle(wp &p, double dist, wp &p0, wp &p1, wp &w)
{
    double r = m2rad(dist);
    vector c = ll2v(p), v0 = ll2v(p0), v1 = ll2v(p1), n;
    vector v = closest_vector_n(c, v0, v1, n);

    double a = dot(c, v), d = cos(r);
    if(a < d)
        return false; // spherical circle doesn't intersect great circle

    // spherical law of cosines, b is distance from closest position
    // to where spherical circle intersects p0<->p1
    double b = acos(d/a);
    quaternion q(b, n);
    vector w0 = q.rotate(v), w1 = q.conjugate().rotate(v);

    // ensure great circle intersections fall between p0 and p1
    d = dot(v0, v1);
    bool bp0 = dot(w0, v0) > d && dot(w0, v1) > d;
    bool bp1 = dot(w1, v0) > d && dot(w1, v1) > d;
    // if both valid, return position closest to p1
    if(bp0 && (!bp1 || dot(w0, v1) > dot(w1, v1))) {
        w = v2ll(w0);
        return true;
    } else if(bp1) {
        w = v2ll(w1);
        return true;
    }
    return false;
}

// set w to the intersection of great circle with (position p, brg)
// on great circle p0 and p1
bool intersect(wp &p, double brg, wp &p0, wp &p1, wp &w)
{
    vector north(0, 0, 1), c = ll2v(p);
    quaternion q(brg, c);
    vector b = q.rotate(north);
    vector m = cross(c, b);
    m.normalize(); // m is plane of p at brg

    vector v0 = ll2v(p0), v1 = ll2v(p1);
    vector n = cross(v0, v1);
    n.normalize(); // n is plane of p0 and p1

    vector i = cross(n, m); // intersection is w
    w = v2ll(i);

    // ensure w falls on the segment between p0 and p1
    double d = dot(v0, v1);
    return dot(i, v0) > d && dot(i, v1) > d;
}

}

// mercator calculations
namespace computation_mc
{

#include "georef.h"

wp closest_a(wp &p, wp &p0, wp &p1, double &a)
{
    double p0x, p0y, p1x, p1y;
    toSM(p0.lat, p0.lon, p.lat, p.lon, &p0x, &p0y);
    toSM(p1.lat, p1.lon, p.lat, p.lon, &p1x, &p1y);

/*
dx = (p1x - p0x)
dy = (p1y - p0y)

x = p0x + a*dx
y = p0y + a*dy
x = b*dy
y = b*(p0x - p1x)

b*dy = p0x + a*dx
b*-dx = p0y + a*dy

(p0x + a*dx)*-dx = (p0y + a*dy)*dy

a = -(p0y*dy + p0x*dx) / (dx*dx + dy*dy)

*/
    double dx = p1x - p0x, dy = p1y - p0y;
    a = -(p0y*dy + p0x*dx) / (dx*dx + dy*dy);
    double x = p0x + a*dx, y = p0y + a*dy;

    double lat, lon;
    fromSM(x, y, p.lat, p.lon, &lat, &lon);
    return wp(lat, lon);
}
    
// find position closest to p, on line defined by p0 and p1
wp closest(wp &p, wp &p0, wp &p1)
{
    double a;
    return closest_a(p, p0, p1, a);
}

// find position closest to p, on segment defined by p0 and p1
wp closest_seg(wp &p, wp &p0, wp &p1)
{
    double a;
    wp c = closest_a(p, p0, p1, a);
    if(a < 0)
        return p0;
    if(a > 1)
        return p1;
    return c;
}

// find distance between two positions
double distance(wp &p0, wp &p1)
{
    double dist;
    APR_DistanceBearingMercator(p0.lat, p0.lon, p1.lat, p1.lon, 0, &dist);
    return dist;
}

// set w to the intersection of (position p, radius r) on great circle
// segment defined by p0 and p1 if circle intersects segment twice,
// return position closest to p1
bool intersect_circle(wp &p, double dist, wp &p0, wp &p1, wp &w)
{
    double p0x, p0y, p1x, p1y;
    toSM(p0.lat, p0.lon, p.lat, p.lon, &p0x, &p0y);
    toSM(p1.lat, p1.lon, p.lat, p.lon, &p1x, &p1y);

    // circle cannot be projected perfectly in mercator...
    // this works for small scale anyway, use north
    double dx, dy;
    toSM(p.lat + dist/1852.0/60.0, p.lon, p.lat, p.lon, &dx, &dy);
    double r = sqrt(dx*dx + dy*dy);

    /*
x^2 + y^2 = r^2
x = p0x + a * (p1x - p0x)
y = p0y + a * (p1y - p0y)

(p0x + a * (p1x - p0x))^2 + (p0y + a * (p1y - p0y))^2 = r^2

p0x^2 + a*2*p0x*(p1x-p0x) + a^2*(p1x-p0x)^2 +
p0y^2 + a*2*p0y*(p1y-p0y) + a^2*(p1y-p0y)^2 = r^2

a^2*((p1x-p0x)^2 + (p1y-p0y)^2) +
a*(2*p0x*(p1x-p0x) +  2*p0y*(p1y-p0y)) +
p0x^2 + p0y^2 - r^2 = 0

A = ((p1x-p0x)^2 + (p1y-p0y)^2)
B = (2*p0x*(p1x-p0x) +  2*p0y*(p1y-p0y))
C = p0x^2 + p0y^2 - r^2

a = (-B +- sqrt(B^2 - 4*A*C)) / (2*A)
    */

    double A = (p1x-p0x)*(p1x-p0x) + (p1y-p0y)*(p1y-p0y);
    double B = 2*p0x*(p1x-p0x) +  2*p0y*(p1y-p0y);
    double C = p0x*p0x + p0y*p0y - r*r;

    double det = B*B - 4*A*C;

    if(det < 0)
        return false; // circle does not intersect line

    double a0 = (-B + sqrt(det)) / (2*A);
    double a1 = (-B - sqrt(det)) / (2*A);
    double a;

    if(a0 < 0 || a0 > 1) {// a0 invalid
        if(a1 < 0 || a1 > 1) // a1 invalid
            return false;
        a = a1;
    } else {
        if(a1 <0 || a1 > 1) // a1 invalid
            a = a0;
        else if(a1 > a0) // choose closest to p1
            a = a1;
        else
            a = a0;
    }

    double x = p0x + a * (p1x - p0x);
    double y = p0y + a * (p1y - p0y);
    fromSM(x, y, p.lat, p.lon, &w.lat, &w.lon);
    return true;
}

// set w to the intersection of of segment with position p, brg
// on segment p0 and p1
bool intersect(wp &p, double brg, wp &p0, wp &p1, wp &w)
{
    double p0x, p0y, p1x, p1y;
    toSM(p0.lat, p0.lon, p.lat, p.lon, &p0x, &p0y);
    toSM(p1.lat, p1.lon, p.lat, p.lon, &p1x, &p1y);
/*
  x = t*cos(brg)
  y = t*sin(brg)
  x = p0x + a * (p1x - p0x)
  y = p0y + a * (p1y - p0y)

  t*cos(brg) = p0x + a * (p1x - p0x)
  t*sin(brg) = p0y + a * (p1y - p0y)

  a = (p0y*cos(brg) - p0x*sin(brg)) / ((p1x - p0x)*sin(brg) - (p1y - p0y)*cos(brg))
*/
    double sb = sin(deg2rad(brg)), cb = cos(deg2rad(brg));
    double a = (p0y*cb - p0x*sb) / ((p1x - p0x)*sb - (p1y - p0y)*cb);
    double x = p0x + a * (p1x - p0x);
    double y = p0y + a * (p1y - p0y);

    fromSM(x, y, p.lat, p.lon, &w.lat, &w.lon);
    return a >= 0 && a <= 1;
}

}
