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
// voronoi_site_pt.cpp
//============================================================================

#include <math.h>
#include "ofMain.h"
#include "voronoi_site_pt.h"

VoronoiPointSite::VoronoiPointSite(float *ptr2x, float *ptr2y)
{
  x=ptr2x;
  y=ptr2y;
}

void VoronoiPointSite::DrawDistMesh(const float SiteRadius, const float SiteMaxAng, const int OutsideOnly)
{
  float X=(*x), Y=(*y);
  int NumSides = (2*3.1415927f) / SiteMaxAng;
  float AngIncr = (2*3.1415927f) / (float)NumSides;
  float Ang = AngIncr;
  glBegin(GL_TRIANGLE_FAN);
    glVertex3f(X,Y,0);
    glVertex3f(X+SiteRadius,Y,SiteRadius);
    for (int i=1; i<NumSides; i++, Ang+=AngIncr)
      glVertex3f(X+cos(Ang)*SiteRadius,Y+sin(Ang)*SiteRadius,SiteRadius);
    glVertex3f(X+SiteRadius,Y,SiteRadius);
  glEnd();
}

void VoronoiPointSite::DrawSite()
{
  glBegin(GL_POINTS);
    glVertex2f((*x),(*y));
  glEnd();
}

void VoronoiPointSite::CalcMinMax(float *MinX, float *MinY, float *MaxX, float *MaxY)
{
  *MinX=*MaxX=*x;
  *MinY=*MaxY=*y;
}

