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
// voronoi_queries.h
//============================================================================

#ifndef _VORONOI_QUERIES_
#define _VORONOI_QUERIES_

#include "voronoi_sites.h"

// READBACK OF ENTIRE ID OR DISTANCE BUFFER
void vdReadbackIDs(unsigned char* &IDs, int W, int H);
void vdReadbackIDs(int* &IDs, int W, int H, VoronoiSites *VS);

void vdReadbackID(unsigned char *ID, int Px, int Py);
void vdReadbackID(int *ID, int Px, int Py, VoronoiSites *VS);

void vdReadbackDists(float* &Dists, int W, int H, float MaxDist);
float vdReadbackDists(unsigned int* &Dists, int W, int H, float MaxDist);

void vdReadbackSignedDists_COLOR(float* &Dists, int W, int H, float MaxDist);
void vdReadbackSignedDists_STENCIL(float* &Dists, int W, int H, float MaxDist);

// READBACK OF A SINGLE DISTANCE VALUE AT A PIXEL LOCATION
void vdReadbackDist(float *Dist, int Px, int Py, float MaxDist);
void vdReadbackSignedDist_COLOR(float *Dist, int Px, int Py, float MaxDist);
void vdReadbackSignedDist_STENCIL(float *Dist, int Px, int Py, float MaxDist);

// READBACK OF A SINGLE ID OR DISTANCE VALUE FOR A POINT IN THE MIN/MAX BOX
int vdNearestPixel(float wx, float wy,                              // WORLD POSITION IN MIN/MAX BOX
                   float MinX, float MinY, float MaxX, float MaxY,  // MIN/MAX BOX
                   int W, int H,                                    // PIXEL DIMENSION OF VOR DIAG
                   int &px, int &py);                               // RETURNED NEAREST PIXEL LOCATION

int vdReadbackID(unsigned char *ID, 
                 float wx, float wy,
                 float MinX, float MinY, float MaxX, float MaxY,
                 int W, int H);
int vdReadbackID(int *ID, VoronoiSites *VS,
                 float wx, float wy,
                 float MinX, float MinY, float MaxX, float MaxY,
                 int W, int H);

int vdReadbackDist(float *Dist, float MaxDist,
                   float wx, float wy,
                   float MinX, float MinY, float MaxX, float MaxY,
                   int W, int H);

int vdReadbackSignedDist_COLOR(float *Dist, float MaxDist,
                               float wx, float wy,
                               float MinX, float MinY, float MaxX, float MaxY,
                               int W, int H);
int vdReadbackSignedDist_STENCIL(float *Dist, float MaxDist,
                                float wx, float wy,
                                float MinX, float MinY, float MaxX, float MaxY,
                                int W, int H);

// GRADIENT COMPUTATION
int vdComputeGradient(float *Dists, int W, int H,
                      float wx, float wy, 
                      float MinX, float MinY, float MaxX, float MaxY,
                      float *gx, float *gy);

int vdComputeGradient(float *Dists, int W, int H,
                      int px, int py, 
                      float *gx, float *gy);
int vdComputeGradient(unsigned int *Dists, float DepthScale, int W, int H,
                      int px, int py, 
                      float *gx, float *gy);

// ROUTINE TO ENLARGE MIN/MAX BOX BY SOME NUMBER OF PIXELS ON EACH SIDE
void vdEnlargeMinMaxBox(float *mx, float *my, float *Mx, float *My, int *W, int *H, 
                        float DistError, float FillGeomRatio, int NumPixelsOnSide);

#endif
