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
// voronoi_site_bezier.cpp
//============================================================================

#include <stdio.h>
#include <math.h>
#include "ofMain.h"
#include "voronoi_site_bezier.h"
#include "voronoi_site_polyline.h"

//----------------------------------------------------------------------------
// CREATES AN ARRAY OF 2D PTS ALONG THE GIVEN CUBIC BEZIER SEGMENT
// Creates an array of 2D pts along the given cubic bezier curve (4 ctrl pts).
// Pts define a polyline with NumSegs number of line segments.
//----------------------------------------------------------------------------
static void CalcBezierCurvePolyLine(float* &Pts, int &NumPts,
                                    float Ax, float Ay,
                                    float Bx, float By,
                                    float Cx, float Cy,
                                    float Dx, float Dy,
                                    int NumSegs)
{
  NumPts = NumSegs+1;
  if (Pts==NULL) Pts = new float[NumPts*2];  // 2 FLOATS PER POINT

  int k=0; 

  Pts[k]=Ax; k++;
  Pts[k]=Ay; k++;

  float Uincr=1.0f/(float)NumSegs, U=Uincr;
	float U2,U3,T,T2,T3;
	float b0,B1,B2,B3; // "B0" is defined as a modem speed.
  for (int i=1;  i<NumSegs;  i++, U+=Uincr)
  {
    U2=U*U; U3=U2*U; T=1-U; T2=T*T; T3=T2*T; b0=T3; B1=3*U*T2; B2=3*U2*T; B3=U3;
    Pts[k] = b0*Ax + B1*Bx + B2*Cx + B3*Dx; k++;
    Pts[k] = b0*Ay + B1*By + B2*Cy + B3*Dy; k++;
  }

  Pts[k]=Dx; k++;
  Pts[k]=Dy; k++;
}

VoronoiBezierSite::VoronoiBezierSite(float *ptr2ax, float *ptr2ay, float *ptr2bx, float *ptr2by,
                                     float *ptr2cx, float *ptr2cy, float *ptr2dx, float *ptr2dy,
                                     int numsegs)
{
  A[0]=ptr2ax; A[1]=ptr2ay;
  B[0]=ptr2bx; B[1]=ptr2by;
  C[0]=ptr2cx; C[1]=ptr2cy;
  D[0]=ptr2dx; D[1]=ptr2dy;
  NumSegs=numsegs;
}

void VoronoiBezierSite::DrawDistMesh(const float SiteRadius, const float SiteMaxAng, const int OutsideOnly)
{
  float *Pts=NULL;
  int NumPts;
  CalcBezierCurvePolyLine(Pts,NumPts,
                          *(A[0]),*(A[1]),
                          *(B[0]),*(B[1]),
                          *(C[0]),*(C[1]),
                          *(D[0]),*(D[1]),
                          NumSegs);
  VoronoiPolylineSite *t = new VoronoiPolylineSite(Pts,NumPts);
  t->DrawDistMesh(SiteRadius,SiteMaxAng);
  delete t;
  delete[] Pts;
}

void VoronoiBezierSite::DrawSite()
{
  float *Pts=NULL;
  int NumPts;
  CalcBezierCurvePolyLine(Pts,NumPts,
                          *(A[0]),*(A[1]),
                          *(B[0]),*(B[1]),
                          *(C[0]),*(C[1]),
                          *(D[0]),*(D[1]),
                          NumSegs);
  VoronoiPolylineSite *t = new VoronoiPolylineSite(Pts,NumPts);
  t->DrawSite();
  delete t;
  delete[] Pts;
}

void VoronoiBezierSite::CalcMinMax(float *MinX, float *MinY, float *MaxX, float *MaxY)
{
//#define USE_CTRLPT_MINMAX 
#ifdef USE_CTRLPT_MINMAX
  // USE MIN/MAX EXTENTS OF CONTROL POINTS (CONVEX HULL PROPERTY), MAY BE EXCESSIVELY CONSERVATIVE
  *MinX=*MaxX=*(A[0]);
  *MinY=*MaxY=*(A[1]);
  if (*(B[0])<*MinX) *MinX=*(B[0]); else if (*(B[0])>*MaxX) *MaxX=*(B[0]);
  if (*(B[1])<*MinY) *MinY=*(B[1]); else if (*(B[1])>*MaxY) *MaxY=*(B[1]);
  if (*(C[0])<*MinX) *MinX=*(C[0]); else if (*(C[0])>*MaxX) *MaxX=*(C[0]);
  if (*(C[1])<*MinY) *MinY=*(C[1]); else if (*(C[1])>*MaxY) *MaxY=*(C[1]);
  if (*(D[0])<*MinX) *MinX=*(D[0]); else if (*(D[0])>*MaxX) *MaxX=*(D[0]);
  if (*(D[1])<*MinY) *MinY=*(D[1]); else if (*(D[1])>*MaxY) *MaxY=*(D[1]);
#else
  // TESSELLATE INTO A POLYLINE AND USE ITS MIN/MAX EXTENTS
  float *Pts=NULL;
  int NumPts;
  CalcBezierCurvePolyLine(Pts,NumPts,
                          *(A[0]),*(A[1]),
                          *(B[0]),*(B[1]),
                          *(C[0]),*(C[1]),
                          *(D[0]),*(D[1]),
                          NumSegs);
  *MinX=*MaxX=Pts[0];
  *MinY=*MaxY=Pts[1];
  for (int k=2; k<NumPts*2; k+=2)
  {
    if (Pts[k]<*MinX)   *MinX=Pts[k];   else if (Pts[k]>*MaxX)   *MaxX=Pts[k];
    if (Pts[k+1]<*MinY) *MinY=Pts[k+1]; else if (Pts[k+1]>*MaxY) *MaxY=Pts[k+1];
  }
  delete[] Pts;
#endif
}
