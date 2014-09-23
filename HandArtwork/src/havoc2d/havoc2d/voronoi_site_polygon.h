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
// voronoi_site_polygon.h
//   NOTE: this site can be treated as a connected polyline with distance
//   meshing on the inside and outside (outsideonly=0) or it can be treated
//   as one sided (outsideonly=1) where the distance mesh is only computed
//   for the "outside" as determined by the winding order: if you walk along
//   the boundary, the right-side is called the "outside". So normal CCW
//   ordering will produce the distance for the outside of the polygon and a
//   CW ordering will be for the actual inside. When outsideonly=0, we assume
//   that the polygons are simple (no self-intersections).
//============================================================================

#ifndef _VORONOIPOLYGONSITE_
#define _VORONOIPOLYGONSITE_

#include "voronoi_site.h"
#include "ofMain.h"

#define MAX_TESSPOINTS	256

class VoronoiPolygonSite : public VoronoiSite
{
  public:

    float *Pts;  // POINTERS TO LOCATION OF SITE COORDINATES
    int NumPts;

    VoronoiPolygonSite(float *pts, int numpts);
    void DrawDistMesh(const float SiteRadius, const float SiteMaxAng, const int OutsideOnly=0);
    void DrawSite(); 
    void DrawSiteFilled();
    void DrawSiteFilledWithTessPoly(float *Pts, int NumPts);
    void CalcMinMax(float *MinX, float *MinY, float *MaxX, float *MaxY);
    
    /* 
	 /// USE OF POLYGON INSTEAD /// GOLAN
	 Tess_Poly 	Poly;
	*/ 
	 GLdouble 	tess_poly_input [MAX_TESSPOINTS][3];
};

#endif
