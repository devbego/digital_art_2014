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
// polygon_object.cpp
//============================================================================

#include <stdlib.h>
#include <math.h>
#include "polygon_object.h"

#define RAND() (float)(rand()/(float)RAND_MAX)             // NORMALIZED RAND FUNCTION [0,1]
#define RAND2() (float)(rand()*(2.0f/(float)RAND_MAX)-1.0) // NORMALIZED RAND FUNCTION [-1,1]

PolygonObject::PolygonObject(int numpts, float origx, float origy, float angle)
{
  NumPts=0;
  NumPtsAlloced=numpts;
  oPts = new float[NumPtsAlloced*2];
  wPts = new float[NumPtsAlloced*2];
  OrigX=origx; OrigY=origy;
  Angle=angle;
  dOrigX = 0;
  dOrigY = 0;
  dAngle = 0;
  WorldPtsUpdated=0;
  WorldMinMaxUpdated=0;
}

PolygonObject::PolygonObject(float mx, float my, float Mx, float My,      // OBJ-SPACE MIN/MAX
                             float wmx, float wmy, float wMx, float wMy,  // WORLD-SPACE MIN/MAX
                             int numpts, float TransSpeed, float RotSpeed)
{
  NumPts=NumPtsAlloced=numpts;
  oPts = new float[NumPtsAlloced*2];
  wPts = new float[NumPtsAlloced*2];

  SetWithRandomPoints(mx, my, Mx, My);

  OrigX = wmx + (wMx-wmx)*RAND();
  OrigY = wmy + (wMy-wmy)*RAND();
  Angle = RAND()*360.0f;

  dOrigX = RAND2()*TransSpeed;
  dOrigY = RAND2()*TransSpeed;
  dAngle = RAND2()*RotSpeed;

  WorldPtsUpdated=0;
  WorldMinMaxUpdated=0;
}

PolygonObject::~PolygonObject()
{
  delete[] oPts;
  delete[] wPts;
}

void PolygonObject::AddPt(float x, float y)
{
  int i=NumPts*2;
  oPts[i]=x;
  oPts[i+1]=y;
  NumPts++;
}



void PolygonObject::SetBoundary(float *xpts, float *ypts, int npts){
	/*
	NumPts = 0;
	int np = MAX(0, MIN(npts, NumPtsAlloced));
	for (int i=0; i<np; i++){
		oPts[i*2]	= xpts[i];
 		oPts[i*2+1]	= ypts[i];
	}
	NumPts = np;
	WorldPtsUpdated=0;
  	WorldMinMaxUpdated=0;
  	*/
  	
  	float xo = xpts[0];
  	float yo = ypts[0];
  	
  	NumPts = 0;
	int np = MAX(0, MIN(npts, NumPtsAlloced));
	for (int i=0; i<np; i++){
		oPts[i*2]	= xpts[i] - xo;
 		oPts[i*2+1]	= ypts[i] - yo;
	}
	NumPts = np;
	WorldPtsUpdated=0;
  	WorldMinMaxUpdated=0;
  	
  	
  	OrigX = xo;
  	OrigY = yo;
  	Angle = 0;

  	dOrigX = 0;
  	dOrigY = 0;
  	dAngle = 0;
  	
  	
  	for (int i=0; i<NumPts*2; i+=2){
		wPts[i]   = oPts[i] + OrigX;
		wPts[i+1] = oPts[i+1] + OrigY;
	}
	WorldPtsUpdated=1;
	


	wmx=wMx=wPts[0];
	wmy=wMy=wPts[1];
	for (int i=2; i<NumPts*2; i+=2)
	{
		if (wPts[i]<wmx) wmx=wPts[i]; else if (wPts[i]>wMx) wMx=wPts[i];
		if (wPts[i+1]<wmy) wmy=wPts[i+1]; else if (wPts[i+1]>wMy) wMy=wPts[i+1];
	}
	WorldMinMaxUpdated=1;
  
  	
}


void PolygonObject::outsideUpdate(){
	for (int i=0; i<NumPts*2; i+=2){
		wPts[i]   = oPts[i] + OrigX;
		wPts[i+1] = oPts[i+1] + OrigY;
	}
	WorldPtsUpdated=1;
	


	wmx=wMx=wPts[0];
	wmy=wMy=wPts[1];
	for (int i=2; i<NumPts*2; i+=2)
	{
		if (wPts[i]<wmx) wmx=wPts[i]; else if (wPts[i]>wMx) wMx=wPts[i];
		if (wPts[i+1]<wmy) wmy=wPts[i+1]; else if (wPts[i+1]>wMy) wMy=wPts[i+1];
	}
	WorldMinMaxUpdated=1;
}



void PolygonObject::SetWithRandomPoints(float mx, float my, float Mx, float My){
	// assumes NumPtsAlloced has been set
	float dx=Mx-mx, dy=My-my;
	for (int i=0; i<NumPtsAlloced*2; i+=2)
	{
		oPts[i]   = mx + dx*RAND();
		oPts[i+1] = my + dy*RAND();
	}
	
  WorldPtsUpdated=0;
  WorldMinMaxUpdated=0;
}





void PolygonObject::UpdateWorldPts()
{
  if (WorldPtsUpdated) return;
  float rAng = Angle*0.01745329f;  // DEG 2 RAD
  float ca=(float)cos(rAng), sa=(float)sin(rAng);
  for (int i=0; i<NumPts*2; i+=2)
  {
    wPts[i]   = ca*oPts[i] - sa*oPts[i+1] + OrigX;
    wPts[i+1] = sa*oPts[i] + ca*oPts[i+1] + OrigY;
  }
  WorldPtsUpdated=1;
  WorldMinMaxUpdated=0;
}

// ASSUMES WORLD POINTS HAVE BEEN UPDATED (call to ComputeWorldCoords)
void PolygonObject::ComputeMinMaxBox(float *mx, float *my, float *Mx, float *My)
{
  if (NumPts<1) return;
  UpdateWorldPts();
  if (!WorldMinMaxUpdated)
  {
    wmx=wMx=wPts[0];
    wmy=wMy=wPts[1];
    for (int i=2; i<NumPts*2; i+=2)
    {
      if (wPts[i]<wmx) wmx=wPts[i]; else if (wPts[i]>wMx) wMx=wPts[i];
      if (wPts[i+1]<wmy) wmy=wPts[i+1]; else if (wPts[i+1]>wMy) wMy=wPts[i+1];
    }
    WorldMinMaxUpdated=1;
  }
  *mx=wmx; *my=wmy; *Mx=wMx; *My=wMy;
}

void PolygonObject::Animate(float wmx, float wmy, float wMx, float wMy)
{
  OrigX+=dOrigX; if (OrigX>wMx || OrigX<wmx) { dOrigX=-dOrigX; OrigX+=dOrigX; };
  OrigY+=dOrigY; if (OrigY>wMy || OrigY<wmy) { dOrigY=-dOrigY; OrigY+=dOrigY; };
  Angle+=dAngle; if (Angle>360) Angle-=360; else if (Angle<-360) Angle+=360;
  WorldPtsUpdated=0;
  WorldMinMaxUpdated=0;
}











