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
// spherical computations

namespace computation
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


double deg2rad(double x) { return x * M_PI / 180; }
double rad2deg(double x) { return x * 180 / M_PI; }
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

}
