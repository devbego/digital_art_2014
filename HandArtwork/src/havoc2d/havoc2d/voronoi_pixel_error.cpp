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
// voronoi_pixel_error.cpp : routines for computing relationships between
//   pixel sampling error and distance error (unrelated to distance meshing error).
//============================================================================

#include <stdio.h> // temp
#include <math.h>

//----------------------------------------------------------------------------
// Calculates the pixel resolution error (farthest a point in the window can
// be from a pixel sample for a given min/max window and pixel sampling resolution)
// Allows for non-uniform sampling where the aspect ratio between the window
// and the pixel dimensions are not the same (not necessarily).
//   WorlddimX / worldDimY  !=  PixelDimX / PixelDimY
//----------------------------------------------------------------------------
float vdCalcPixelDistError(float MinX, float MinY, float MaxX, float MaxY,
                           int PixelDimX, int PixelDimY)
{
  float PixelWidth = (MaxX-MinX) / PixelDimX;
  float PixelHeight = (MaxY-MinY) / PixelDimY;
  float DiagonalLength = (float)sqrt(PixelWidth*PixelWidth + PixelHeight*PixelHeight);
  return( DiagonalLength * 0.5f );
}

//----------------------------------------------------------------------------
// Compute actual distance error based on desired resolution in a min/max window
// and a specified FillGeomRatio.
//----------------------------------------------------------------------------
float vdCalcDistError(float MinX, float MinY, float MaxX, float MaxY,
                      int PixelDimX, int PixelDimY, float FillGeomRatio)
{
  float PixelDistError = vdCalcPixelDistError(MinX,MinY,MaxX,MaxY,PixelDimX,PixelDimY);
  float DistError = PixelDistError / (1-FillGeomRatio);
  return(DistError);
}

//----------------------------------------------------------------------------
// Calculate pixel dimensions of the Voronoi diagram for the specified
// min/max box and distance error. Pixel sampling density
// is chosen so that no point in the min/max box is farther than DistError
// away from a sample point (DistError becomes half diagonal length of a
// pixel "square").
//----------------------------------------------------------------------------
void vdCalcResolution(float MinX, float MinY, float MaxX, float MaxY,
                      float PixelDistError, int *Width, int *Height)
{
  float PixelWidth = 1.41421356f*PixelDistError; // 2*(PixelDistError/sqrt(2))
  float NumPixelsW = (MaxX-MinX) / PixelWidth;
  float NumPixelsH = (MaxY-MinY) / PixelWidth;
  int iNumPixelsW = (int)NumPixelsW;
  int iNumPixelsH = (int)NumPixelsH;
  *Width  = iNumPixelsW + (NumPixelsW-iNumPixelsW > 0);
  *Height = iNumPixelsH + (NumPixelsH-iNumPixelsH > 0);
}

//----------------------------------------------------------------------------
// Calculate pixel dimensions given a total DistError and a FillGeomRatio
//----------------------------------------------------------------------------
void vdCalcResolution(float MinX, float MinY, float MaxX, float MaxY,
                      float DistError, float FillGeomRatio,
                      int *Width, int *Height)
{
  float PixelDistError = DistError * (1-FillGeomRatio);
  vdCalcResolution(MinX,MinY,MaxX,MaxY,PixelDistError,Width,Height);
}

