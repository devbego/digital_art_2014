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
// voronoi_site_line.cpp
//============================================================================

#include <math.h>
#include "ofMain.h"
#include "voronoi_site_line.h"

VoronoiLineSite::VoronoiLineSite(float *ptr2ax, float *ptr2ay, float *ptr2bx, float *ptr2by)
{
  ax=ptr2ax;
  ay=ptr2ay;
  bx=ptr2bx;
  by=ptr2by;
}

void VoronoiLineSite::DrawDistMesh(const float SiteRadius, const float SiteMaxAng, const int OutsideOnly)
{
  float Ax=(*ax), Ay=(*ay), Bx=(*bx), By=(*by);
  float dx=Bx-Ax, dy=By-Ay;
  float l = (float)sqrt(dx*dx + dy*dy);
  float Scale = SiteRadius/l;
  dx*=Scale;
  dy*=Scale;

  int NumSides = 3.1415927f / SiteMaxAng;
  float AngIncr = 3.1415927f / (float)NumSides;
  float Ang;
  int i;

  glBegin(GL_QUAD_STRIP);
    glVertex3f(Bx-dy,By+dx,SiteRadius);
    glVertex3f(Ax-dy,Ay+dx,SiteRadius);
    glVertex3f(Bx,By,0);
    glVertex3f(Ax,Ay,0);
    glVertex3f(Bx+dy,By-dx,SiteRadius);
    glVertex3f(Ax+dy,Ay-dx,SiteRadius);
  glEnd();

  glBegin(GL_TRIANGLE_FAN);
    glVertex3f(Bx,By,0);
    glVertex3f(Bx+dy,By-dx,SiteRadius);
    Ang = (float)atan2(-dx,dy) + AngIncr;
    for (i=1; i<NumSides; i++, Ang+=AngIncr)
      glVertex3f(Bx+cos(Ang)*SiteRadius, By+sin(Ang)*SiteRadius, SiteRadius);
    glVertex3f(Bx-dy,By+dx,SiteRadius);
  glEnd();

  glBegin(GL_TRIANGLE_FAN);
    glVertex3f(Ax,Ay,0);
    glVertex3f(Ax-dy,Ay+dx,SiteRadius);
    Ang = (float)atan2(dx,-dy) + AngIncr;
    for (i=1; i<NumSides; i++, Ang+=AngIncr)
      glVertex3f(Ax+cos(Ang)*SiteRadius, Ay+sin(Ang)*SiteRadius, SiteRadius);
    glVertex3f(Ax+dy,Ay-dx,SiteRadius);
  glEnd();
}

void VoronoiLineSite::DrawSite()
{
  glBegin(GL_LINES);
    glVertex2f((*ax),(*ay));
    glVertex2f((*bx),(*by));
  glEnd();
}

void VoronoiLineSite::CalcMinMax(float *MinX, float *MinY, float *MaxX, float *MaxY)
{
  if (*ax<*bx) { *MinX=*ax; *MaxX=*bx; } else { *MinX=*bx; *MaxX=*ax; }
  if (*ay<*by) { *MinY=*ay; *MaxY=*by; } else { *MinY=*by; *MaxY=*ay; }
}
