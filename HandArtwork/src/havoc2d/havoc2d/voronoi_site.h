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
// voronoi_site.h : container class for Voronoi site objects
//============================================================================

#ifndef _VORONOISITE_
#define _VORONOISITE_

class VoronoiSite
{
  public:
   
    //----------------------------------------------------------------------------
    // Flag indicating whether or not this site will be drawn or not (on by default)
    // This routine is used by the adaptive "growing regions" routines and by
    // the site-culling routines.
    //----------------------------------------------------------------------------
    int IsActive;
   
    //----------------------------------------------------------------------------
    // User-specified site ID. This is not used in the Voronoi computation and
    // has no relation to the Color/ID mapping used internally.
    //----------------------------------------------------------------------------
    int SiteID;

    //----------------------------------------------------------------------------
    // Draws the Voronoi site distance mesh. z values correspond to distance.
    // Only the core geometry calls are made. Proper state must be set externally.
    // For sites that are closed shapes (polygons, circles, etc), we can often
    // improve the efficiency by drawing only the outside portion of the distance
    // mesh (as defined by CCW winding order, right of the boundary is outside).
    //----------------------------------------------------------------------------
    virtual void DrawDistMesh(const float SiteRadius, const float SiteMaxAng, const int OutsideOnly=0) = 0;

    //----------------------------------------------------------------------------
    // Draws the 2D site geometry.
    // Only the core geometry calls are made. Proper state must be set externally.
    //----------------------------------------------------------------------------
    virtual void DrawSite() = 0;

    //----------------------------------------------------------------------------
    // Draws the filled site geometry (only applicable to sites with a distinct inside
    // and outside. e.g. polygons, spheres, etc. used for signed distance computation)
    //----------------------------------------------------------------------------
    virtual void DrawSiteFilled() { DrawSite(); };

    //----------------------------------------------------------------------------
    // Return the Min/Max extents of the site.
    //----------------------------------------------------------------------------
    virtual void CalcMinMax(float *MinX, float *MinY, float *MaxX, float *MaxY) = 0;
};

#endif
