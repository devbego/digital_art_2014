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
// polygon_object.h
//============================================================================

#define MAX_NBODY_POLYLINE_LENGTH	5

#ifndef MIN
#define MIN(x,y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef ABS
#define ABS(x) (((x) < 0) ? -(x) : (x))
#endif


class PolygonObject
{
  public:

    float *oPts;  // OBJ-SPACE CTRL PT COORDS
    float *wPts;  // WORLD-SPACE CTRL PT COORDS
    int NumPts, NumPtsAlloced;

    float OrigX, OrigY, Angle;
    float wmx, wmy, wMx, wMy;   // WORLD MIN/MAX BOUNDING BOX

    int WorldPtsUpdated, WorldMinMaxUpdated;
    float dOrigX, dOrigY, dAngle;
  
    PolygonObject(int numpts, float origx, float origy, float angle);
    PolygonObject(float mx, float my, float Mx, float My,      // OBJ-SPACE MIN/MAX
                  float wmx, float wmy, float wMx, float wMy,  // WORLD-SPACE MIN/MAX
                  int numpts, float TransSpeed, float RotSpeed);
    ~PolygonObject();
    void AddPt(float x, float y);
    void SetWithRandomPoints(float mx, float my, float Mx, float My);
    void SetBoundary (float *xpts, float *ypts, int npts);
    void outsideUpdate();
    
    void UpdateWorldPts();
    void ComputeMinMaxBox(float *mx, float *my, float *Mx, float *My);
    void Animate(float wmx, float wmy, float wMx, float wMy);
};
