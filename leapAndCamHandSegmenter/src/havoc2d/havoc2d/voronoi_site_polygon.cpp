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
// voronoi_site_polygon.cpp
//============================================================================

#include <stdio.h>
#include <math.h>
#include "ofMain.h"
#include "voronoi_site_polygon.h"

//----------------------------------------------------------------------------
// Utilities for dealing with non-convex polygons
//----------------------------------------------------------------------------
static void DrawNonconvexPoly(float *Pts, int NumPts) 
{
	
	printf ("Attempting to render in commented-out function DrawNonconvexPoly in voronoi_sites.cpp\n");
	
	/*
  int i,j,k;
  GLUtesselator *tobj = gluNewTess();
  double *pts = new double[NumPts*3];
  for (j=0,k=0; j<NumPts*2; j+=2,k+=3)
    { pts[k]=Pts[j]; pts[k+1]=Pts[j+1]; pts[k+2]=0; }
  gluTessNormal(tobj,0,0,1);
  gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD); 
  gluTessCallback(tobj,GLU_TESS_VERTEX,(GLvoid (CALLBACK *)(void))glVertex3dv);
  gluTessCallback(tobj,GLU_TESS_BEGIN,(GLvoid (CALLBACK *)(void))glBegin);
  gluTessCallback(tobj,GLU_TESS_END,glEnd);

  gluTessBeginPolygon(tobj,0);
    gluTessBeginContour(tobj);
      for (i=0; i<NumPts*3; i+=3)
        gluTessVertex(tobj,&pts[i],&pts[i]);
    gluTessEndContour(tobj);
  gluTessEndPolygon(tobj);

  gluDeleteTess(tobj);
  delete[] pts;
	 */
}


/*
void CALLBACK combineCallback(GLdouble coords[3], GLdouble *vertex_data[4],GLfloat weight[4], GLdouble **dataOut);
void CALLBACK combineCallback(GLdouble coords[3], GLdouble *vertex_data[4],GLfloat weight[4], GLdouble **dataOut)
{
	GLdouble *vertex;
	vertex = (GLdouble *) malloc(3 * sizeof(GLdouble));
	vertex[0] = coords[0];
	vertex[1] = coords[1];
	vertex[2] = coords[2];
	*dataOut = vertex;
}



	gluTessCallback(tobj, GLU_TESS_VERTEX, (void(CALLBACK*)())&vertexCallback);
	gluTessCallback(tobj, GLU_TESS_BEGIN,  (void(CALLBACK*)())&glBegin);
	gluTessCallback(tobj, GLU_TESS_END,  (void(CALLBACK*)())&glEnd);
	gluTessCallback(tobj, GLU_TESS_COMBINE,  (void(CALLBACK*)())&combineCallback);
	*/
	



static float CalcSignedArea(float *Pts, int NumPts)
{
  int i;
  float Area=0;
  int iLastPt=(NumPts-1)*2;
  for (i=0; i<iLastPt; i+=2)
    Area += (Pts[i]*Pts[i+3] - Pts[i+1]*Pts[i+2]);
  Area += (Pts[iLastPt]*Pts[1] - Pts[iLastPt+1]*Pts[0]);
  return (Area*0.5f);
}

static int OrderingIsCCW(float *Pts, int NumPts)  // 1=CCW, 0=CW
{
  return (CalcSignedArea(Pts,NumPts)>=0);
}

static int IsConvex(float *Pts, int NumPts)
{
  // FOR POLYGON TO BE CONVEX, EACH VERTEX MUST BE CONVEX!
  float nextDX, nextDY;
  int i,j, N2=NumPts*2;
  float prevDX=Pts[0]-Pts[N2-2], prevDY=Pts[1]-Pts[N2-1];
  for (i=0; i<N2; i+=2)
  {
    j=(i+2)%N2;
    nextDX=Pts[j]-Pts[i];
    nextDY=Pts[j+1]-Pts[i+1];
    if (prevDX*nextDY-prevDY*nextDX < 0) return(0);  // IF VERTEX i IS CONCAVE
    prevDX = nextDX;
    prevDY = nextDY;
  }
  return(1);  // IF NO CONCAVE VERTICES ENCOUNTERED!
}

//----------------------------------------------------------------------------
// VoronoiPolygonSite implementation
//----------------------------------------------------------------------------
VoronoiPolygonSite::VoronoiPolygonSite(float *pts, int numpts)
{
  Pts=pts;
  NumPts=numpts;
  
  // Here's our polygonizer.. use OF POLYGON INSTEAD //// GOLAN
	/*
	Poly.Init();
	Poly.Set_Winding_Rule(GLU_TESS_WINDING_ODD); 
	*/
	for (int j=0; j<MAX_TESSPOINTS; j++){
		tess_poly_input[j][0] = 0;
		tess_poly_input[j][1] = 0;
		tess_poly_input[j][2] = 0;
	}
	 
}



void VoronoiPolygonSite::DrawDistMesh(const float SiteRadius, const float SiteMaxAng, const int OutsideOnly)
{
  int k, k_1, k_2, k_3, k_4, k_5;  // INDICES INTO Pts

  if (NumPts<2) return;
  int NumPts2 = NumPts*2;
  int NewNumPts2 = NumPts2;

  // EDGE REGION QUADS (EDGE DEFINED WITH ENDPTS: (k,k+1) and (k+2,k+3))
  // LAST PT MAY LOOP AROUND TO FIRST PT (IF CLOSED POLYLINE)
  float dx, dy, l, Scale;
  NewNumPts2=(NumPts+1)*2;          // ONE EXTRA EDGE IN A CLOSED LOOP
  for (k=0; k<(NewNumPts2-2); k+=2)
  {
    k_1=k+1; 
    k_2=(k+2) % NumPts2;
    k_3=(k+3) % NumPts2;

    dx = (Pts[k_2]) - (Pts[k]);
    dy = (Pts[k_3]) - (Pts[k_1]);
    l = (float)sqrt(dx*dx + dy*dy);
    Scale = SiteRadius/l;
    dx *= Scale;
    dy *= Scale;

    glBegin(GL_QUAD_STRIP);    
      if (!OutsideOnly)
      {
        glVertex3f(Pts[k_2]-dy,Pts[k_3]+dx,SiteRadius);
        glVertex3f(Pts[k]-dy,Pts[k_1]+dx,SiteRadius);
      }
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

  // DO INTERNAL "IN-BETWEEN SEGMENTS" TRIANGLE FANS
  // FANS DRAWN BETWEEN CURRENT EDGE (k,k+1)<=>(k+2,k+3) AND NEXT EDGE 
  // (k+2,k+3)<=>(k+4,k+5). PTS MAY LOOP BACK TO FIRST PT (IF CLOSED POLYLINE)
  NewNumPts2=(NumPts+2)*2;           // TWO EXTRA FANS FOR A CLOSED LOOP
  for (k=0; k<(NewNumPts2-4); k+=2)    
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
    if (!OutsideOnly || TurnDir_CrossZ>0)  // IF OutsideOnly, ONLY LEFT TURN FANS
    {
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
            glVertex3f(Pts[k_2]+cos(Ang)*SiteRadius, Pts[k_3]+sin(Ang)*SiteRadius, SiteRadius);
          glVertex3f(Pts[k_2]+EndDX*SiteRadius,Pts[k_3]+EndDY*SiteRadius,SiteRadius);
        glEnd();
      }
    }
            
    Adx=Bdx;
    Ady=Bdy;
    Al=Bl;
  }
}

void VoronoiPolygonSite::DrawSite()
{
  glBegin(GL_LINE_STRIP);
    for (int k=0; k<NumPts*2; k+=2)
      glVertex2fv(&(Pts[k]));
    glVertex2fv(Pts);
  glEnd();
}




void VoronoiPolygonSite::DrawSiteFilledWithTessPoly(float *Pts, int NumPts){

	int j = 0;
	int np2 = (NumPts*2);
	if (np2 > (MAX_TESSPOINTS*2)){ np2 = (MAX_TESSPOINTS*2); }
	for (int k=0; k<np2; k+=2){
		tess_poly_input[j][0] = Pts[k];
		tess_poly_input[j][1] = Pts[k+1];
		j++;
	}

	/* USE OF POLYGON INSTEAD
	Poly.Init();
	Poly.Set_Winding_Rule(GLU_TESS_WINDING_ODD);//NONZERO);
	Poly.Begin_Polygon(); 
	Poly.Begin_Contour();
	Poly.Render_Contour(tess_poly_input, NumPts); 
	Poly.End_Contour();
	Poly.End_Polygon();
	Poly.End();
	 */
}



void VoronoiPolygonSite::DrawSiteFilled()
{
	
  // DETERMINE WINDING ORDER (ONLY DRAW CAP ON CCW, "OUTSIDE" IS REALLY OUTSIDE)
  if (OrderingIsCCW(Pts,NumPts))
  {
    // DETERMINE IF POLYGON IS CONVEX OR NONCONVEX
    if (IsConvex(Pts,NumPts))
    {
      glBegin(GL_POLYGON);
        for (int k=0; k<(NumPts*2); k+=2)
          glVertex2fv(&(Pts[k]));
      glEnd();
    }
    else  // NONCONVEX POLYGONS MUST BE TRIANGULATED (OPENGL DOES NOT HANDLE NONCONVEX)
    {
      //DrawNonconvexPoly(Pts,NumPts);
      DrawSiteFilledWithTessPoly(Pts, NumPts);
    }
  } else {
     DrawSiteFilledWithTessPoly(Pts, NumPts);
  }

}



void VoronoiPolygonSite::CalcMinMax(float *MinX, float *MinY, float *MaxX, float *MaxY)
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
