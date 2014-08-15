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
// voronoi_site_polyline.cpp
//============================================================================

#include <math.h>
#include "ofMain.h"
#include "voronoi_site_polyline.h"

VoronoiPolylineSite::VoronoiPolylineSite(float *pts, int numpts)
{
  Pts=pts;
  NumPts=numpts;
  
  dist_mesh_color[0] = 0.01;
  dist_mesh_color[1] = 0.01;
  dist_mesh_color[2] = 0.01;
  dist_mesh_color[3] = 1.00;
}

void VoronoiPolylineSite::DrawDistMesh(const float SiteRadius, const float SiteMaxAng, const int OutsideOnly)
{
  
  // GOLAN added color:
  glColor3fv(dist_mesh_color);
  
  
  int k, k_1, k_2, k_3, k_4, k_5;  // INDICES INTO Pts

  if (NumPts<2) return;
  int NumPts2 = NumPts*2;

  // EDGE REGION QUADS (EDGE DEFINED WITH ENDPTS: (k,k+1) and (k+2,k+3))
  // LAST PT MAY LOOP AROUND TO FIRST PT (IF CLOSED POLYLINE)
  float dx, dy, l, Scale;
  for (k=0; k<(NumPts2-2); k+=2)
  {
    k_1=k+1; 
    k_2=(k+2) % NumPts2;
    k_3=(k+3) % NumPts2;

    dx = Pts[k_2] - Pts[k];
    dy = Pts[k_3] - Pts[k_1];
    l = (float)sqrt(dx*dx + dy*dy);
    Scale = SiteRadius/l;
    dx *= Scale;
    dy *= Scale;

    glBegin(GL_QUAD_STRIP);
      glVertex3f(Pts[k_2]-dy,Pts[k_3]+dx,SiteRadius);
      glVertex3f(Pts[k]-dy,Pts[k_1]+dx,SiteRadius);
      glVertex3f(Pts[k_2],Pts[k_3],0);
      glVertex3f(Pts[k],Pts[k_1],0);
      glVertex3f(Pts[k_2]+dy,Pts[k_3]-dx,SiteRadius);
      glVertex3f(Pts[k]+dy,Pts[k_1]-dx,SiteRadius);
    glEnd();
  }

  // VERTEX REGION TRIANGLE FANS
  int NumSides;
  float AngIncr, Ang, FanAng;
  float Adx,Ady,Bdx,Bdy,Al,Bl;
  float TurnDir_CrossZ,StartDotEnd,StartDX,StartDY,EndDX,EndDY;
  Adx = Pts[2] - Pts[0];             // FIRST LINE SEGMENT A
  Ady = Pts[3] - Pts[1];
  Al = (float)sqrt(Adx*Adx + Ady*Ady);
  Adx /= Al;
  Ady /= Al;

  // BEGINNING ENDCAP TRIANGLE FAN (DRAW ONLY IF POLYLINE IS NOT CLOSED)
  NumSides = 3.1415927f / SiteMaxAng;
  AngIncr = 3.1415927f / (float)NumSides;
  Ang = (float)atan2(Adx,-Ady) + AngIncr;
  glBegin(GL_TRIANGLE_FAN);
    glVertex3f(Pts[0],Pts[1],0);
    glVertex3f(Pts[0]-Ady*SiteRadius,Pts[1]+Adx*SiteRadius,SiteRadius);
    for (k=1; k<NumSides; k++, Ang+=AngIncr)
      glVertex3f(Pts[0]+cos(Ang)*SiteRadius,Pts[1]+sin(Ang)*SiteRadius,SiteRadius);
    glVertex3f(Pts[0]+Ady*SiteRadius,Pts[1]-Adx*SiteRadius,SiteRadius);
  glEnd();

  // DO INTERNAL "IN-BETWEEN SEGMENTS" TRIANGLE FANS
  // FANS DRAWN BETWEEN CURRENT EDGE (k,k+1)<=>(k+2,k+3) AND NEXT EDGE 
  // (k+2,k+3)<=>(k+4,k+5). PTS MAY LOOP BACK TO FIRST PT (IF CLOSED POLYLINE)
  for (k=0; k<(NumPts2-4); k+=2)    
  {
    k_2=(k+2) % NumPts2; 
    k_3=(k+3) % NumPts2;
    k_4=(k+4) % NumPts2; 
    k_5=(k+5) % NumPts2;

    Bdx = Pts[k_4] - Pts[k_2];       // NEXT LINE SEGMENT B
    Bdy = Pts[k_5] - Pts[k_3];
    Bl = (float)sqrt(Bdx*Bdx + Bdy*Bdy);
    Bdx /= Bl;
    Bdy /= Bl;

    // WHICH WAY DOES THE POLYLINE TURN FROM A TO B, 
    // COMPUTE START AND END DISPLACEMENT RAYS FOR TRIANGLE FAN
    TurnDir_CrossZ = Adx*Bdy - Ady*Bdx;
    if (TurnDir_CrossZ>0)  // TURN IS LEFT
      { StartDX=Ady; StartDY=-Adx; EndDX=Bdy; EndDY=-Bdx; }
    else                   // TURN IS RIGHT
      { StartDX=-Bdy; StartDY=Bdx; EndDX=-Ady; EndDY=Adx; }

    // DRAW THE FAN FROM THE START RAY TO THE END RAY
    StartDotEnd = StartDX*EndDX + StartDY*EndDY;
    if (StartDotEnd<-1) StartDotEnd=-1; else if (StartDotEnd>1) StartDotEnd=1;
    if (StartDotEnd<1)     // IF ==1, NO TURN IN SEGMENT!
    {
      FanAng = (float)acos( StartDotEnd );
      NumSides = FanAng / SiteMaxAng;
      AngIncr = FanAng / (float)NumSides;
      Ang = (float)atan2(StartDY,StartDX) + AngIncr;
      glBegin(GL_TRIANGLE_FAN);
        glVertex3f(Pts[k_2],Pts[k_3],0);
        glVertex3f(Pts[k_2]+StartDX*SiteRadius,Pts[k_3]+StartDY*SiteRadius,SiteRadius);
        for (int i=1; i<NumSides; i++, Ang+=AngIncr)
          glVertex3f(Pts[k_2]+cos(Ang)*SiteRadius,Pts[k_3]+sin(Ang)*SiteRadius,SiteRadius);
        glVertex3f(Pts[k_2]+EndDX*SiteRadius,Pts[k_3]+EndDY*SiteRadius,SiteRadius);
      glEnd();
    }
        
    Adx=Bdx;
    Ady=Bdy;
    Al=Bl;
  }

  // ENDING ENDCAP TRIANGLE FAN (DRAW ONLY IF POLYLINE IS NOT CLOSED)
  int iX=NumPts2-2, iY=NumPts2-1;
  NumSides = 3.1415927f / SiteMaxAng;
  AngIncr = 3.1415927f / (float)NumSides;
  Ang = (float)atan2(-Adx,Ady) + AngIncr;
  glBegin(GL_TRIANGLE_FAN);
    glVertex3f(Pts[iX],Pts[iY],0);
    glVertex3f(Pts[iX]+Ady*SiteRadius,Pts[iY]-Adx*SiteRadius,SiteRadius);
    for (k=1; k<NumSides; k++, Ang+=AngIncr)
      glVertex3f(Pts[iX]+cos(Ang)*SiteRadius,Pts[iY]+sin(Ang)*SiteRadius,SiteRadius);
    glVertex3f(Pts[iX]-Ady*SiteRadius,Pts[iY]+Adx*SiteRadius,SiteRadius);
  glEnd();
}

void VoronoiPolylineSite::DrawSite()
{
  glBegin(GL_LINE_STRIP);
    for (int k=0; k<NumPts*2; k+=2)
      glVertex2fv(&(Pts[k]));
  glEnd();
}

void VoronoiPolylineSite::CalcMinMax(float *MinX, float *MinY, float *MaxX, float *MaxY)
{
  if (NumPts<1) return;
  *MinX=*MaxX=Pts[0];
  *MinY=*MaxY=Pts[1];
  for (int k=2; k<NumPts*2; k+=2)
  {
    if (Pts[k]<*MinX)   *MinX=Pts[k];   else if (Pts[k]>*MaxX)   *MaxX=Pts[k];
    if (Pts[k+1]<*MinY) *MinY=Pts[k+1]; else if (Pts[k+1]>*MaxY) *MaxY=Pts[k+1];
  }
}
