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

struct wp
{
    wp() {}
    wp(double _lat, double _lon) : lat(_lat), lon(_lon) {}
    bool eq(wp &w) { return lat == w.lat && lon == w.lon; }
    double lat, lon;
};

namespace computation_gc
{
    wp closest(wp &p, wp &p0, wp &p1);
    wp closest_seg(wp &p, wp &p0, wp &p1);
    double distance(wp &p0, wp &p1);
    bool intersect_circle(wp &p, double dist, wp &p0, wp &p1, wp &w);
    bool intersect(wp &p, double brg, wp &p0, wp &p1, wp &w);
}

namespace computation_mc
{
    wp closest(wp &p, wp &p0, wp &p1);
    wp closest_seg(wp &p, wp &p0, wp &p1);
    double distance(wp &p0, wp &p1);
    bool intersect_circle(wp &p, double dist, wp &p0, wp &p1, wp &w);
    bool intersect(wp &p, double brg, wp &p0, wp &p1, wp &w);
}
