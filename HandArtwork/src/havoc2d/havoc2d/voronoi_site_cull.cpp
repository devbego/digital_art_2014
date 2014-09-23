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
// voronoi_site_cull.cpp
//============================================================================

#include "voronoi_sites.h"

//----------------------------------------------------------------------------
// Culls the sites against Min/Max window based on a bound on the MaxDist.
// No need to call this routine, if you know the sites will always intersect
// the window (no reasonable bound on the MaxDist). This culling routine is
// conservative, but FAST, since it uses the min/max extents of the sites
// (axis-aligned bounding boxes) to cull against the window. The IsActive
// flags of the culled sites are set to 0. Flags are assumed to be set
// properly before entering this routine (only cull Active sites).
//----------------------------------------------------------------------------
void CullSites_MinMax(VoronoiSites *VS, float MaxDist, float MinX, float MinY, float MaxX, float MaxY)
{
  // FIRST ENLARGE THE Min/Max WINDOW BY MaxDist
  MinX-=MaxDist;
  MinY-=MaxDist;
  MaxX+=MaxDist;
  MaxY+=MaxDist;

  float mx,my,Mx,My;
  for (int i=0; i<VS->NumSites; i++)
    if (VS->Sites[i]->IsActive)      // ONLY CULL ACTIVE SITES
    {
      VS->Sites[i]->CalcMinMax(&mx,&my,&Mx,&My);
      if (Mx<MinX || mx>MaxX) VS->Sites[i]->IsActive=0;
      else if (My<MinY || my>MaxY) VS->Sites[i]->IsActive=0;
    }
}
