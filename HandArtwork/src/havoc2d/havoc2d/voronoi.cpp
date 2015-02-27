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
// voronoi.cpp
//============================================================================

#include <stdio.h>
#include <math.h>
#include "ofMain.h"
#include "voronoi.h"
#include "voronoi_pixel_error.h"

//----------------------------------------------------------------------------
// Calculates the max triangle fan angle given a specified site max dist (radius)
// and the desired max dist error. Angle is returned in radians. Fan angle
// is chosen so that max error (=DistError) is at midpoint of far fan edge.
//----------------------------------------------------------------------------
static inline float MaxAngWithinDistError(float Radius, float DistError)
{
  return( 2.0f * acos( (Radius-DistError) / Radius ) );
}

//----------------------------------------------------------------------------
// Clears the depth and color of a portion of the screen using polygon
// drawing (avoids call to glClear) given starting pixel and width and height.
// Clears color to black and depth to far distance.
//----------------------------------------------------------------------------
void vdFastClear(int StartX, int StartY, int Width, int Height, int ClearStencil)
{
  if (ClearStencil)
  {
    glPushAttrib(GL_STENCIL_BUFFER_BIT); // SET ALL COVERED PIXELS' STENCIL VALS TO 1 (ASSUMED TO ALREADY BE CLEARED)
      int NumStencilBits;
      glGetIntegerv(GL_STENCIL_BITS,&NumStencilBits);
      glEnable(GL_STENCIL_TEST);
      glStencilFunc(GL_ALWAYS,0,(1<<NumStencilBits)-1);
      glStencilOp(GL_REPLACE,GL_REPLACE,GL_REPLACE);
  }
  glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
  glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(StartX,StartY,Width,Height);
    glDepthRange(0,1);
  glMatrixMode(GL_PROJECTION); 
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1,1,-1,1,-1,0);
  glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
      glColor3f(0,0,0);
      glRectf(-1,-1,1,1);
  glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  glMatrixMode(GL_PROJECTION); 
    glPopMatrix();
  glPopAttrib();   // GL_VIEWPORT_BIT
  glPopAttrib();   // GL_DEPTH_BUFFER_BIT
  if (ClearStencil)
    glPopAttrib(); // GL_STENCIL_BUFFER_BIT
}

//----------------------------------------------------------------------------
// The main routine that computes the 2D voronoi diagram for the given set of sites.
// The user specifies a min/max bounding box window in world (voronoi site)
// space, the min and max distances from the sites that should be included
// in the Voronoi computation (MinDist is usually 0 and MaxDist should cover the entire
// bounding box (except under special situations where a bounded distance
// function is appropriate; this will give major speedups)). MinDist and
// MaxDist are depth "windows" in the range [0,MaxMaxDist].
// The dimensions (width/height) of the Voronoi diagram are returned and the
// Voronoi diagram image resides in the lower-left corner of the current
// draw buffer (usually GL_BACK). User can specify whether or not to auto
// clear the color and depth buffers. Some apps may not want to clear the
// buffers (incremental update, etc). NOTE: the depth range is enlarged 
// slightly to allow some overlap between adjacent ranges (MeshDistError).
// MeshDistError is the error tolerance for mesh approximation of distance
// function (independent from sampling resolution distance error).
// Total DistError = PixelDistError + MeshDistError. PixelDistError is determined
// by the specified resolution, so MeshDistError = DistError - PixelDistError
// The user only provides the resolution and a FillGeomRatio value in (0,1)
// (see comment below). This value allows the meshing error to change within
// the pixel resolution given. This routine returns the total DistError bound:
//   PixelDistError = DistError * (1-FillGeomRatio)
//   MeshDistError  = DistError * FillGeomRatio
//   DistError = PixelDistError + MeshDistError
//----------------------------------------------------------------------------
float vdCompute(VoronoiSites *VS, 
                float MinX, float MinY, float MaxX, float MaxY,
                float MinDist, float MaxDist, float MaxMaxDist,
                float FillGeomRatio,
                int Width, int Height,
                int ClearBuffersOn, int OutsideOnly, int SignedDistOn)
{
  if (SignedDistOn) OutsideOnly=0; 

	int WW = (int) ofGetWidth(); /// (int)glutGet((GLenum)GLUT_WINDOW_WIDTH);
    int WH = (int) ofGetHeight(); /// (int)glutGet((GLenum)GLUT_WINDOW_HEIGHT);
  if (Width>WW || Height>WH)
    printf("Voronoi error: window not large enough, need %dx%d\n",Width,Height);

  // COMPUTE ERRORS
  float PixelDistError = vdCalcPixelDistError(MinX,MinY,MaxX,MaxY,Width,Height);
  float DistError = PixelDistError / (1-FillGeomRatio);
  if (PixelDistError>=DistError) 
    { printf("Error bounds not possible given current res. Need %dx%d or reduce FillGeomRatio!\n",Width,Height); return(DistError); }
  float MeshDistError = DistError - PixelDistError;

  if (ClearBuffersOn==1)
  {
    if (SignedDistOn){ 
    	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }  else {
    	// GOLAN CRITICAL:
    	// we comment this out because we do it in the helloApp instead!!!
    	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
  }
  else if (ClearBuffersOn==2)
  {
    vdFastClear(0,0,Width,Height,SignedDistOn);
  }
  
  glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
  glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0,0,Width,Height);
    glDepthRange(MinDist/MaxMaxDist,MaxDist/MaxMaxDist);  // MaxDist>MaxMaxDist WILL BE CLAMPED TO [0,1]
  glMatrixMode(GL_PROJECTION); 
    glPushMatrix();
    glLoadIdentity();
    glOrtho(MinX,MaxX,MinY,MaxY,-MinDist,-MaxDist);  // NEGATE NEAR/FAR SO DIST-MESH Z-VALUES CAN BE POSITIVE
  glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

  float Radius = MaxDist;
  float MaxAng = MaxAngWithinDistError(Radius,MeshDistError);

  VS->DrawDistMeshes(Radius,MaxAng,1,OutsideOnly);

  // IF DESIRED, ENCODE THE NEGATIVE SIGN OF THE DIST FOR THE INSIDE OF THE POLYGON AS THE COLOR WHITE
  if (SignedDistOn==1)
  {
    glPushAttrib(GL_CURRENT_BIT);       // SET COLOR TO WHITE
      glColor3ub(255,255,255);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);  // DISABLE WRITING TO THE DEPTH BUFFER
      glDisable(GL_DEPTH_TEST);
    VS->DrawSitesFilled(0);
    glPopAttrib(); // GL_DEPTH_BUFFER_BIT
    glPopAttrib(); // GL_CURRENT_BIT

  }
  // USE STENCIL BUFFER TO STORE SIGN OF DISTANCE. DRAW THE FILLED VORONOI SITE AS A 1
  // IN THE STENCIL BUFFER (NEGATIVE DISTANCE IS INSIDE OF OBJECT)
  else if (SignedDistOn==2)
  {
    glPushAttrib(GL_COLOR_BUFFER_BIT);   // DISABLE WRITING TO THE COLOR BUFFER
      glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
    glPushAttrib(GL_DEPTH_BUFFER_BIT);   // DISABLE WRITING TO THE DEPTH BUFFER
      glDisable(GL_DEPTH_TEST);
    glPushAttrib(GL_STENCIL_BUFFER_BIT); // SET ALL COVERED PIXELS' STENCIL VALS TO 1 (ASSUMED TO ALREADY BE CLEARED)
      glEnable(GL_STENCIL_TEST);
      glStencilFunc(GL_ALWAYS,1,1);
      glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
    VS->DrawSitesFilled(0);
    glPopAttrib(); // GL_STENCIL_BUFFER_BIT
    glPopAttrib(); // GL_DEPTH_BUFFER_BIT
    glPopAttrib(); // GL_COLOR_BUFFER_BIT
  }
  // JUST DRAW THE INSIDE "CAP" AT ZERO DEPTH FOR OUTSIDEONLY MESHES
  else if (OutsideOnly)
  {
    VS->DrawSitesFilled(1);
  }

  glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  glMatrixMode(GL_PROJECTION); 
    glPopMatrix();
  glPopAttrib(); // GL_VIEWPORT_BIT
  glPopAttrib(); // GL_DEPTH_BUFFER_BIT

  return(DistError);
}

//----------------------------------------------------------------------------
// Same as the above core routine, but here we specify the total distance error
// tolerance and a FillGeomRatio (in (0,1) usually just 0.5f). Higher values
// increase the fill requirements and decrease geometry.
// GIVEN GLOBAL DistError and FillGeomRatio:
//   PixelDistError = DistError * (1-FillGeomRatio)
//   MeshDistError  = DistError * FillGeomRatio
//   DistError = PixelDistError + MeshDistError
// A point in Min/Max window cannot be farther than PixelDistError from
// a pixel sample point. The resolution used is returned (Width x Height).
//----------------------------------------------------------------------------
void vdCompute(VoronoiSites *VS, 
               float MinX, float MinY, float MaxX, float MaxY,
               float MinDist, float MaxDist, float MaxMaxDist,
               float DistError, float FillGeomRatio,
               int *Width, int *Height,
               int ClearBuffersOn, int OutsideOnly, int SignedDistOn)
{
  vdCalcResolution(MinX,MinY,MaxX,MaxY,DistError,FillGeomRatio,Width,Height);
  vdCompute(VS,MinX,MinY,MaxX,MaxY,MinDist,MaxDist,MaxMaxDist,
            FillGeomRatio,*Width,*Height,ClearBuffersOn,OutsideOnly,SignedDistOn);
}

//----------------------------------------------------------------------------
// Various convenience routines
//----------------------------------------------------------------------------
float vdCompute(VoronoiSites *VS, 
                float MinX, float MinY, float MaxX, float MaxY,
                float MaxDist,
                float FillGeomRatio,
                int Width, int Height,
                int ClearBuffersOn, int OutsideOnly, int SignedDistOn)
{
  return( vdCompute(VS,MinX,MinY,MaxX,MaxY,0,MaxDist,MaxDist,
                    FillGeomRatio,Width,Height,ClearBuffersOn,OutsideOnly,SignedDistOn) );
}               

void vdCompute(VoronoiSites *VS, 
               float MinX, float MinY, float MaxX, float MaxY,
               float MaxDist,
               float DistError, float FillGeomRatio,
               int *Width, int *Height,
               int ClearBuffersOn, int OutsideOnly, int SignedDistOn)
{
  vdCompute(VS,MinX,MinY,MaxX,MaxY,0,MaxDist,MaxDist,
            DistError,FillGeomRatio,Width,Height,ClearBuffersOn,OutsideOnly,SignedDistOn);
}               

//----------------------------------------------------------------------------
// Draw the active 2D Voronoi sites. Uses the current color, point size, and
// line width. Requires same parameters as needed to compute the VD.
//----------------------------------------------------------------------------
void vdDrawSites(VoronoiSites *VS, 
                 float MinX, float MinY, float MaxX, float MaxY,
                 int Width, int Height)
{
	
	int WW = (int) ofGetWidth(); /// (int)glutGet((GLenum)GLUT_WINDOW_WIDTH);
    int WH = (int) ofGetHeight(); /// (int)glutGet((GLenum)GLUT_WINDOW_HEIGHT);

  if (Width>WW || Height>WH)
    printf("Voronoi error: window not large enough, need %dx%d\n",Width,Height);
  
  glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0,0,Width,Height);
  glPushAttrib(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
  glMatrixMode(GL_PROJECTION); 
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(MinX,MaxX,MinY,MaxY);
  glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

  VS->DrawSites(0);
 // VS->DrawSitesFilled(0);

  glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  glMatrixMode(GL_PROJECTION); 
    glPopMatrix();
  glPopAttrib(); // GL_DEPTH_BUFFER_BIT
  glPopAttrib(); // GL_VIEWPORT_BIT
}

void vdDrawSites(VoronoiSites *VS, 
                 float MinX, float MinY, float MaxX, float MaxY,
                 float DistError, float FillGeomRatio)
{
  int Width, Height;
  vdCalcResolution(MinX,MinY,MaxX,MaxY,DistError,FillGeomRatio,&Width,&Height);
  vdDrawSites(VS,MinX,MinY,MaxX,MaxY,Width,Height);
}
