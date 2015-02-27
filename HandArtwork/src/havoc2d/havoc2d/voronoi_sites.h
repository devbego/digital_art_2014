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
// voronoi_sites.h : dynamic array of Voronoi site objects
//============================================================================

#ifndef _VORONOISITES_
#define _VORONOISITES_

#include "voronoi_site.h"

class VoronoiSites
{
  private:

    int NumSitesAlloced;
    int DestroySites;  // FLAG INDICATING WHETHER SITES SHOULD BE FREEed IN DESTRUCTOR CALL

  public:

    VoronoiSite* *Sites;
    int NumSites;


    VoronoiSites();
    ~VoronoiSites();
    void SetDestroySitesFlag(int destroysites);  // USE THIS WHEN YOU WANT TO DELETE VoronoiSites, BUT NOT THE SITES

    void Add(VoronoiSite* NewSite, int ID);
    void Add(VoronoiSite* NewSite);

    int GetSiteID(unsigned char *ColorID);

    void SetAllIsActiveFlags(int IsActive);
    void SetSiteIsActiveFlag(unsigned char *ColorID, int IsActive);

    void DrawDistMeshes(float Radius, float MaxAng, const int UseColorIDs=1, const int OutsideOnly=0);
    void DrawSites(const int UseColorIDs=0);
    void DrawSitesFilled(const int UseColorIDs=0);
};

#endif
