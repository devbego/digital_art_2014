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
// voronoi_queries.cpp
//============================================================================

#include <stdio.h>
#include "ofMain.h"
#include "voronoi_sites.h"
#include <math.h>
#include <float.h>
#include "voronoi_queries.h"
//----------------------------------------------------------------------------
// Read back the ID buffer (color buffer) as a 1D array of unsigned char or
// as translated integer IDs (original values). If IDs array is set to NULL,
// memory is allocated automatically. 
// It is assumed that the Voronoi diagram has been correctly computed into
// the current readbuffer (usually GL_BACK). The user must specify the width
// and height (in pixels) of the Voronoi diagram computed.
// Integer IDs version is much slower since each pixel color value has to be 
// translated back into the user ID; however, it may be useful when many
// query accesses to the buffer are required and the overhead of repeated
// color/ID conversions will overwhelm the constant overhead of this routine.
// NOTE: be sure to set glReadBuffer to the buffer containing the Voronoi
// diagram (usually GL_BACK).
//----------------------------------------------------------------------------
void vdReadbackIDs(unsigned char* &IDs, int W, int H)
{
  if (IDs==NULL) IDs = new unsigned char[W*H*3];
  glReadPixels(0,0,W,H, GL_RGB, GL_UNSIGNED_BYTE, IDs);
}

void vdReadbackIDs(int* &IDs, int W, int H, VoronoiSites *VS)
{
  if (IDs==NULL) IDs = new int[W*H];

  unsigned char *tIDs = NULL;
  vdReadbackIDs(tIDs,W,H);

  unsigned char *ucIDs = tIDs;
  for (int i=0; i<(W*H); i++, ucIDs+=3)
    IDs[i] = VS->GetSiteID(ucIDs);

  delete[] tIDs;
}

//----------------------------------------------------------------------------
// Read back the ID (or color) at a particular pixel location (Px,Py). As in the
// whole buffer version, there are 2 versions: one that returns the actual
// RGB unsigned char color and one that translate the color into the site
// ID.
//----------------------------------------------------------------------------
void vdReadbackID(unsigned char *ID, int Px, int Py)
{
  glReadPixels(Px,Py,1,1, GL_RGB, GL_UNSIGNED_BYTE, ID);
}

void vdReadbackID(int *ID, int Px, int Py, VoronoiSites *VS)
{
  unsigned char ucID[3];
  vdReadbackID(ucID,Px,Py);
  (*ID) = VS->GetSiteID(ucID);
}

//----------------------------------------------------------------------------
// Read back the distance buffer into a 1D array of floats from the current
// glReadBuffer (usually GL_BACK). The user must provide the MaxDist (or
// MaxMaxDist if more general routine is being used) of the depth values so
// that the properly scaled distance values are returned. The scaling is done
// by the hardware using the scale/bias routines. Again, if Dists is NULL,
// automatic allocation occurs. WARNING: on some hardware, the depth scaling
// is very expensive, so setting MaxDist to 1 will avoid this scaling. Values
// are returned in [0,MaxDist]. Depth values were produced using an orthographic
// projection, so the values are linear and can be easily scaled to any desired range.
// YIKES! Depth values get clamped to [0,1] after scale and bias!
//----------------------------------------------------------------------------
void vdReadbackDists(float* &Dists, int W, int H, float MaxDist)
{
  if (Dists==NULL) Dists = new float[W*H];
  if (MaxDist!=1) { glPushAttrib(GL_PIXEL_MODE_BIT); glPixelTransferf(GL_DEPTH_SCALE,MaxDist); }
  glReadPixels(0,0,W,H, GL_DEPTH_COMPONENT,GL_FLOAT,Dists);
  if (MaxDist!=1) glPopAttrib();
}

//----------------------------------------------------------------------------
// Same as above, but depth values are fixed-point unsigned ints. The scale
// factor to multiply depth value to get actual distance value is returned.
//----------------------------------------------------------------------------
float vdReadbackDists(unsigned int* &Dists, int W, int H, float MaxDist)
{
  if (Dists==NULL) Dists = new unsigned int[W*H];
  glReadPixels(0,0,W,H, GL_DEPTH_COMPONENT,GL_UNSIGNED_INT,Dists);
  float DepthScale = (1.0/0xffffffff)*MaxDist;
  return(DepthScale);
}

//----------------------------------------------------------------------------
// Same as above, except that the color buffer is also read back. Pixels
// whose color is WHITE (255,255,255) has negative distances. 
// This assumes that the Voronoi diagram was computed using SignedDist=1.
//----------------------------------------------------------------------------
void vdReadbackSignedDists_COLOR(float* &Dists, int W, int H, float MaxDist)
{
  vdReadbackDists(Dists,W,H,MaxDist);  // READ BACK UNSIGNED DEPTH
  unsigned char *Signs = NULL;          // READ BACK COLOR BUFFER
  vdReadbackIDs(Signs,W,H);
  for (int i=0,k=0; i<(W*H); i++,k+=3)
    Dists[i] *= ((Signs[k]==255 && Signs[k+1]==255 && Signs[k+2]==255) ? -1 : 1);
  delete[] Signs;
}

//----------------------------------------------------------------------------
// Signed distance, but stencil value > 0 indicates negative distance.
// This assumes that the Voronoi diagram was computed using SignedDist=2.
//----------------------------------------------------------------------------
void vdReadbackSignedDists_STENCIL(float* &Dists, int W, int H, float MaxDist)
{
  vdReadbackDists(Dists,W,H,MaxDist);  // READ BACK UNSIGNED DEPTH
  unsigned char *Signs = new unsigned char[W*H];
  glReadPixels(0,0,W,H,GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,Signs);
  for (int i=0; i<(W*H); i++)
    Dists[i] *= ((Signs[i]>0) ? -1 : 1);
  delete[] Signs;
}

//----------------------------------------------------------------------------
// Read back the distance value at a particular pixel location (Px,Py).
// The user must specify the MaxDist so the depth can be properly scaled.
//----------------------------------------------------------------------------
void vdReadbackDist(float *Dist, int Px, int Py, float MaxDist)
{
  if (MaxDist!=1) { glPushAttrib(GL_PIXEL_MODE_BIT); glPixelTransferf(GL_DEPTH_SCALE,MaxDist); }
  glReadPixels(Px,Py,1,1, GL_DEPTH_COMPONENT,GL_FLOAT,Dist);
  if (MaxDist!=1) glPopAttrib();
}

//----------------------------------------------------------------------------
// Same as above, except that the pixel color is also read. If the color is
// WHITE then, the sign of the distance value is negative.
//----------------------------------------------------------------------------
void vdReadbackSignedDist_COLOR(float *Dist, int Px, int Py, float MaxDist)
{
  vdReadbackDist(Dist,Px,Py,MaxDist);
  unsigned char Sign[3];
  vdReadbackID(Sign,Px,Py);
  (*Dist) *= ((Sign[0]==255 && Sign[1]==255 && Sign[2]==255) ? -1 : 1);
}

//----------------------------------------------------------------------------
// Signed distance, but stencil value > 0 indicates negative distance.
//----------------------------------------------------------------------------
void vdReadbackSignedDist_STENCIL(float *Dist, int Px, int Py, float MaxDist)
{
  vdReadbackDist(Dist,Px,Py,MaxDist);
  unsigned char Sign;
  glReadPixels(Px,Py,1,1,GL_STENCIL_INDEX,GL_UNSIGNED_BYTE,&Sign);
  if (Sign>0) (*Dist) = -(*Dist);
}

//----------------------------------------------------------------------------
// The following routines allow a user to specify an arbitrary (floating point)
// location in the Min/Max box and query for the NEAREST ID or distance value.
// The versions provided are the same as the pixel-level queries above.
// NOTE: the (wx,wy) point is clamped to lie in the Min/Max box.
// Returns whether or not the world pt (wx,wy) was in the Min/Max box. If
// 0 is returned then the values in (px,py) are not valid.
//----------------------------------------------------------------------------
static inline int IsInMinMax(float wx, float wy, float MinX, float MinY, float MaxX, float MaxY)
{
  return ( wx>=MinX && wx<=MaxX && wy>=MinY && wy<=MaxY );
}

int vdNearestPixel(float wx, float wy,                              // WORLD POSITION IN MIN/MAX BOX
                   float MinX, float MinY, float MaxX, float MaxY,  // MIN/MAX BOX
                   int W, int H,                                    // PIXEL DIMENSION OF VOR DIAG
                   int &px, int &py)                                // RETURNED NEAREST PIXEL LOCATION
{
  if (!IsInMinMax(wx,wy,MinX,MinY,MaxX,MaxY)) return(0);
  float s = (wx-MinX) / (MaxX-MinX);
  float t = (wy-MinY) / (MaxY-MinY);
  px = (int)(s*W);
  py = (int)(t*H);
  if (px<0) px=0; else if (px>=W) px=W-1;
  if (py<0) py=0; else if (py>=H) py=H-1;
  return(1);
}                

int vdReadbackID(unsigned char *ID, 
                 float wx, float wy,
                 float MinX, float MinY, float MaxX, float MaxY,
                 int W, int H)
{
  if (!IsInMinMax(wx,wy,MinX,MinY,MaxX,MaxY)) return(0);
  int px, py;
  vdNearestPixel(wx,wy,MinX,MinY,MaxX,MaxY,W,H, px,py);
  vdReadbackID(ID,px,py);
  return(1);
}

int vdReadbackID(int *ID, VoronoiSites *VS,
                 float wx, float wy,
                 float MinX, float MinY, float MaxX, float MaxY,
                 int W, int H)
{
  if (!IsInMinMax(wx,wy,MinX,MinY,MaxX,MaxY)) return(0);
  int px, py;
  vdNearestPixel(wx,wy,MinX,MinY,MaxX,MaxY,W,H, px,py);
  vdReadbackID(ID,px,py,VS);
  return(1);
}

int vdReadbackDist(float *Dist, float MaxDist,
                   float wx, float wy,
                   float MinX, float MinY, float MaxX, float MaxY,
                   int W, int H)
{
  if (!IsInMinMax(wx,wy,MinX,MinY,MaxX,MaxY)) return(0);
  int px, py;
  vdNearestPixel(wx,wy,MinX,MinY,MaxX,MaxY,W,H, px,py);
  vdReadbackDist(Dist,px,py,MaxDist);
  return(1);
}

int vdReadbackSignedDist_COLOR(float *Dist, float MaxDist,
                               float wx, float wy,
                               float MinX, float MinY, float MaxX, float MaxY,
                               int W, int H)
{
  if (!IsInMinMax(wx,wy,MinX,MinY,MaxX,MaxY)) return(0);
  int px, py;
  vdNearestPixel(wx,wy,MinX,MinY,MaxX,MaxY,W,H, px,py);
  vdReadbackSignedDist_COLOR(Dist,px,py,MaxDist);
  return(1);
}

int vdReadbackSignedDist_STENCIL(float *Dist, float MaxDist,
                                float wx, float wy,
                                float MinX, float MinY, float MaxX, float MaxY,
                                int W, int H)
{
  if (!IsInMinMax(wx,wy,MinX,MinY,MaxX,MaxY)) return(0);
  int px, py;
  vdNearestPixel(wx,wy,MinX,MinY,MaxX,MaxY,W,H, px,py);
  vdReadbackSignedDist_STENCIL(Dist,px,py,MaxDist);
  return(1);
}

//----------------------------------------------------------------------------
// GRADIENT COMPUTATION ROUTINES : Given a min/max box, a depth image, and a
// point in the min/max box within the range [1,H-1),[1,W-1) in pixel space
// (this is to avoid boundary conditions), the gradient is computed 
// (remember the gradient is a vector that points in the direction of maximum 
// rate of change whose magnitude is the rate of change in that direction). 
// Returns 1 if successful, and 0 is point is outside of the valid range.
// Assumes that Dists buffer is already properly "signed".
//----------------------------------------------------------------------------
int vdComputeGradient(float *Dists, int W, int H,
                      float wx, float wy, 
                      float MinX, float MinY, float MaxX, float MaxY,
                      float *gx, float *gy)
{
  // FIRST TRANSFORM WORLD POINT INTO PIXEL SPACE
  float px = ((wx-MinX)/(MaxX-MinX))*W;
  float py = ((wy-MinY)/(MaxY-MinY))*H;

  // COMPUTE LOWER-LEFT NEAREST PIXEL
  float fPx = px-0.5f;
  float fPy = py-0.5f;
  int Px = (int)fPx;
  int Py = (int)fPy;
  Px -= (fPx-Px==0)?1:0;
  Py -= (fPy-Py==0)?1:0;

  // TEST IF LOWER-LEFT PIXEL IS IN VALID AREA OF IMAGE (ROOM FOR 1 "BELOW" AND 2 "ABOVE")
  if (Px<1 || Py<1 || Px>W-3 || Py>H-3) return(0);

  // COMPUTE CORRESPONDING DEPTH VALUE INDEX
  int iDists = Py*W+Px;

  // COMPUTE BILINEAR WEIGHTS IN PRINCIPAL DIRECTIONS
  float tx=fPx-Px, itx=1.0f-tx;
  float ty=fPy-Py, ity=1.0f-ty;

  // USE SYMMETRIC ONE-STEP (UNIT SUPPORT) DIFFERENCE IN EACH PRINCIPAL DIRECTION TO COMPUTE GRADIENT
  // BILINEARLY RECONSTRUCT FOR STEP POINTS AND COMPUTE DIFFERENCES (B IS VALUE FOR STEP IN POS DIR
  // AND A IS VALUE FOR STEP IN NEG DIR)
  float A,B;

  // X DIRECTION
  B = ity * (itx*Dists[iDists+1]   + tx*Dists[iDists+2]) +
       ty * (itx*Dists[iDists+W+1] + tx*Dists[iDists+W+2]);
  A = ity * (itx*Dists[iDists-1]   + tx*Dists[iDists]) +
       ty * (itx*Dists[iDists+W-1] + tx*Dists[iDists+W]);
  *gx = (B-A)*0.5f;

  // Y DIRECTION
  B = ity * (itx*Dists[iDists+W]   + tx*Dists[iDists+W+1]) +
       ty * (itx*Dists[iDists+2*W] + tx*Dists[iDists+2*W+1]);
  A = ity * (itx*Dists[iDists-W]   + tx*Dists[iDists-W+1]) +
       ty * (itx*Dists[iDists]     + tx*Dists[iDists+1]);
  *gy = (B-A)*0.5f;

  return(1);
}

//----------------------------------------------------------------------------
// GRADIENT COMPUTATION ROUTINES : Given a pixel coordinate in [1,W-2], [1,H-2],
// returns the gradient
//----------------------------------------------------------------------------
int vdComputeGradient(float *Dists, int W, int H,
                      int px, int py, 
                      float *gx, float *gy)
{
  // TEST IF POINT IS IN VALID AREA OF IMAGE (MIN BIASED)
  if (px<1 || px>=W-1 || py<1 || py>=H-1) return(0);

  // COMPUTE LOWER-LEFT NEAREST DEPTH VALUE
  int iDists = py*W+px;

  // COMPUTE GRADIENT
  *gx = (Dists[iDists+1] - Dists[iDists-1]) * 0.5f;
  *gy = (Dists[iDists+W] - Dists[iDists-W]) * 0.5f;

  return(1);
}

static float MaxValue(unsigned int *Dists, float DepthScale, int W, int H)
{
  unsigned int Max=Dists[0];
  for (int i=1; i<(W*H); i++)
    if (Dists[i]>Max) Max=Dists[i];
  return(DepthScale*Max);
}

int vdComputeGradient(unsigned int *Dists, float DepthScale, int W, int H,
                      int px, int py, 
                      float *gx, float *gy)
{
  // TEST IF POINT IS IN VALID AREA OF IMAGE (MIN BIASED)
  if (px<1 || px>W-2 || py<1 || py>H-2) return(0);

  // COMPUTE LOWER-LEFT NEAREST DEPTH VALUE
  int iDists = py*W+px;

  // COMPUTE GRADIENT
  DepthScale*=0.5f;
  *gx = (DepthScale*Dists[iDists+1] - DepthScale*Dists[iDists-1]);
  *gy = (DepthScale*Dists[iDists+W] - DepthScale*Dists[iDists-W]);
if (isnan((double)(*gx))) { printf("gx: %f\n",*gx); *gx=0; }
if (isnan((double)(*gy))) { printf("gy: %f\n",*gy); *gy=0; }

  return(1);
}

//----------------------------------------------------------------------------
// Enlarge given min/max box (with specified pixel dimensions) by NumPixelsOnSide
// on each side. This is useful for operations that require a minimum size voronoi 
// diagram (gradient computation and other image operations with difficult to 
// handle boundary conditions).
//----------------------------------------------------------------------------
void vdEnlargeMinMaxBox(float *mx, float *my, float *Mx, float *My, int *W, int *H, 
                        float DistError, float FillGeomRatio, int NumPixelsOnSide)
{
  float PixelDistError = DistError * (1-FillGeomRatio);
  float PixelWidth = 1.41421356f*PixelDistError; // 2*(PixelDistError/sqrt(2))
  (*W)+=NumPixelsOnSide*2;
  (*H)+=NumPixelsOnSide*2;
  float dX = PixelWidth*(*W);
  float dY = PixelWidth*(*H);
  float Xoffset = (dX-((*Mx)-(*mx)))*0.5f;
  float Yoffset = (dY-((*My)-(*my)))*0.5f;
  (*mx)-=Xoffset;
  (*Mx)+=Xoffset;
  (*my)-=Yoffset;
  (*My)+=Yoffset;
}

