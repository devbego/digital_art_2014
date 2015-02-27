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
// voronoi_sites.cpp : dynamic array of Voronoi site objects
//============================================================================

#include <stdio.h>
#include "ofMain.h"
#include "voronoi_sites.h"
#include "voronoi_color_ids.h"

VoronoiSites::VoronoiSites()
{
  Sites = new VoronoiSite*[1];
  NumSites = 0;
  NumSitesAlloced = 1;
  DestroySites=1;
}

VoronoiSites::~VoronoiSites()
{
  if (DestroySites)
    for (int i=0; i<NumSites; i++)
      delete Sites[i];
  delete[] Sites;
}

void VoronoiSites::SetDestroySitesFlag(int destroysites)
{
  DestroySites=destroysites;
}

void VoronoiSites::Add(VoronoiSite* NewSite, int ID)
{
  if (NumSites==NumSitesAlloced)
  {
    NumSitesAlloced*=2;
    VoronoiSite* *NewSites = new VoronoiSite*[NumSitesAlloced];
    for (int i=0; i<NumSites; i++) NewSites[i]=Sites[i];
    delete[] Sites;
    Sites=NewSites;
  }
  Sites[NumSites] = NewSite;
  Sites[NumSites]->IsActive=1;  // ACTIVE BY DEFAULT
  Sites[NumSites]->SiteID=ID;
  NumSites++;
}

void VoronoiSites::Add(VoronoiSite* NewSite)
{
  Add(NewSite,NumSites); // AUTOMATICALLY GENERATES A UNIQUE ID
}

int VoronoiSites::GetSiteID(unsigned char *ColorID)
{
  return( Sites[RGB2INDEX(ColorID)]->SiteID );
}

void VoronoiSites::SetAllIsActiveFlags(int IsActive)
{
  for (int i=0; i<NumSites; i++)
    Sites[i]->IsActive=IsActive;
}

void VoronoiSites::SetSiteIsActiveFlag(unsigned char *ColorID, int IsActive)
{
  Sites[RGB2INDEX(ColorID)]->IsActive = IsActive;
}

void VoronoiSites::DrawDistMeshes(float Radius, float MaxAng, const int UseColorIDs, const int OutsideOnly)
{
  if (UseColorIDs)
  {
    unsigned char ID[3];
    for (int i=0; i<NumSites; i++)
      if (Sites[i]->IsActive)
      {
        //INDEX2RGB(i,ID);
        //glColor3ubv(ID);
        Sites[i]->DrawDistMesh(Radius,MaxAng,OutsideOnly);
      }
  }
  else
  {
    for (int i=0; i<NumSites; i++)
      if (Sites[i]->IsActive)
        Sites[i]->DrawDistMesh(Radius,MaxAng,OutsideOnly);
  }
}

void VoronoiSites::DrawSites(const int UseColorIDs)
{
  if (UseColorIDs)
  {
    unsigned char ID[3];
    for (int i=0; i<NumSites; i++)
      if (Sites[i]->IsActive)
      {
        INDEX2RGB(i,ID);
        glColor3ubv(ID);
        Sites[i]->DrawSite();
      }
  }
  else
  {
    for (int i=0; i<NumSites; i++)
      if (Sites[i]->IsActive)
        Sites[i]->DrawSite();
  }
}

void VoronoiSites::DrawSitesFilled(const int UseColorIDs)
{
  if (UseColorIDs)
  {
    unsigned char ID[3];
    for (int i=0; i<NumSites; i++)
      if (Sites[i]->IsActive)
      {
        INDEX2RGB(i,ID);
        glColor3ubv(ID);
        Sites[i]->DrawSiteFilled();
      }
  }
  else
  {
    for (int i=0; i<NumSites; i++)
    if (Sites[i]->IsActive)
      Sites[i]->DrawSiteFilled();
  }
}
