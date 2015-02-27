/*************************************************************************\
>
> Copyright 2001 The University of North Carolina at Chapel Hill.
> All Rights Reserved.
>
> Permission to use, copy, modify OR distribute this software and its
> documentation for educational, research and non-profit purposes, without
> fee, and without a written agreement is hereby granted, provided that the
> above copyright notice and the following three paragraphs appear in all
> copies.
>
> IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL BE
> LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
> CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE
> USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY
> OF NORTH CAROLINA HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
> DAMAGES.
>
> THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY
> WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
> MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE
> PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
> NORTH CAROLINA HAS NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT,
> UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
>
> The authors may be contacted via:
>
> US Mail: K. Hoff, A. Zaferakis, M. Lin, D. Manocha
> Department of Computer Science
> Sitterson Hall, CB #3175
> University of N. Carolina
> Chapel Hill, NC 27599-3175
>
> Phone: (919) 962-1749
>
> EMail: 
> geom@cs.unc.edu 	<mailto:geom@cs.unc.edu>
> hoff@cs.unc.edu 	<mailto:hoff@cs.unc.edu>
> andrewz@cs.unc.edu 	<mailto:andrewz@cs.unc.edu>
> lin@cs.unc.edu 		<mailto:lin@cs.unc.edu>
> dm@cs.unc.edu 		<mailto:dm@cs.unc.edu>
>
\**************************************************************************/

//============================================================================
// voronoi.hpp
//     ClearBuffers=0: No clearing of buffers before drawing VD
//     ClearBuffers=1: Use regular glClear
//     ClearBuffers=2: Use FAST clear
//   In addition, we have an option for drawing the polygon to cap off the 
//   distance values to zero inside of the object.
//   Here are the options specified by outsideonly:
//     OutsideOnly=0: inside/outside, no capping
//     OutsideOnly=1: outside (defined by ordering, CCW, right-side), with capping
//                    routine automatically the ordering and whether or
//                    not it is convex or nonconvex. USER MUST ONLY SUPPLY
//                    SIMPLE POLYGONS (no self-intersections) WITH THIS OPTION!
//   In addition, you have the option of having SIGNED DISTANCES (negative
//   for inside the polygon and positive outside). The negative sign is encoded
//   in the color buffer as WHITE (255,255,255). Normally, the color for any pixel
//   would correspond to the ID of the site. When signeddist=1, the polygon
//   cap is drawn even when the inside distances are computed (outsideonly=0)
//   but writing to the depth buffer is disabled and the color is WHITE.
//   When the user queries a depth value, the sign is determined by looking
//   up the color of the corresponding pixel. This can only be done for
//   simple (non self-intersecting) polygons with a CCW winding order.
//   You also have the option of encoding the sign as a single bit in the
//   stencil buffer (instead of the color white in the color buffer). A set
//   stencil bit means negative. This is activated by setting SignedDist==2.
//     SignedDist=1: color (255,255,255) is negative
//     SignedDist=2: set stencil bit is negative
//============================================================================

#ifndef _VORONOI_
#define _VORONOI_

#include "voronoi_sites.h"

void vdFastClear(int StartX, int StartY, int Width, int Height, int ClearStencil=0);

float vdCompute(VoronoiSites *VS, 
                float MinX, float MinY, float MaxX, float MaxY,
                float MinDist, float MaxDist, float MaxMaxDist,
                float FillGeomRatio,
                int Width, int Height,
                int ClearBuffersOn=1, int OutsideOnly=0, int SignedDistOn=0);

void vdCompute(VoronoiSites *VS, 
               float MinX, float MinY, float MaxX, float MaxY,
               float MinDist, float MaxDist, float MaxMaxDist,
               float DistError, float FillGeomRatio,
               int *Width, int *Height,
               int ClearBuffersOn=1, int OutsideOnly=0, int SignedDistOn=0);

float vdCompute(VoronoiSites *VS, 
                float MinX, float MinY, float MaxX, float MaxY,
                float MaxDist,
                float FillGeomRatio,
                int Width, int Height,
                int ClearBuffersOn=1, int OutsideOnly=0, int SignedDistOn=0);

void vdCompute(VoronoiSites *VS, 
               float MinX, float MinY, float MaxX, float MaxY,
               float MaxDist,
               float DistError, float FillGeomRatio,
               int *Width, int *Height,
               int ClearBuffersOn=1, int OutsideOnly=0, int SignedDistOn=0);

void vdDrawSites(VoronoiSites *VS, 
                 float MinX, float MinY, float MaxX, float MaxY,
                 int Width, int Height);

void vdDrawSites(VoronoiSites *VS, 
                 float MinX, float MinY, float MaxX, float MaxY,
                 float DistError, float FillGeomRatio);

#endif
