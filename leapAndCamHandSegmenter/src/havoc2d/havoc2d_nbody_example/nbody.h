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

#ifndef _NBODY_H_
#define _NBODY_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ofMain.h"

#include "voronoi.h"
#include "voronoi_pixel_error.h"
#include "voronoi_sites.h"
#include "voronoi_site_polygon.h"
#include "voronoi_site_polyline.h"

#include "voronoi_draw_gl.h"
#include "polygon_object.h"


struct GolanColor
{
	float r;
	float g;
	float b;
};

struct VoronoiOverlap
{
  GLuint TexID;
  float mx, my, Mx, My;
  int ResX, ResY;
};


#define NB_RAND() (float)(rand()/(float)RAND_MAX)             // NORMALIZED RAND FUNCTION [0,1]
#define NUM_NBODIES 5

class nbody
{

public:


	nbody(int inw, int inh);

	int ResX;
	int ResY;
	float FillGeomRatio;  // IN (0,1), HIGHER VALUES INCREASE FILL REQUIREMENTS AND REDUCE GEOMETRY
	VoronoiSites *vSites;
	float DistError;
	float MaxDist; // sqrt(8.0f);
	//float MaxDist=0.1f;
	
	char *handyString;
	GolanColor	**siteColors; 
	void	set_region_color(int i);

	int FullScreenModeOn;
	int AnimationOn;
	int DrawSitesOn;
	int DrawMinMaxBoxesOn;
	int DrawOverlappingRegionsOn;
	int ComputeVoronoiOn;
	int UseFastClear;
	int TimerOn;
	int ShowDepth;

	// NAVIGATION
	int LeftButtonDown;        // MOUSE STUFF
	int RightButtonDown;
	int CtrlPressed;
	int OldX,OldY,NewX,NewY;
	int iSelectedSite;
	
	inline int MinMaxBoxOverlap(const float Amx, const float Amy, const float AMx, const float AMy,
                                   const float Bmx, const float Bmy, const float BMx, const float BMy,
                                   float *mx, float *my, float *Mx, float *My);


	PolygonObject *Polys[NUM_NBODIES];


	void DrawMinMaxBox(float mx, float my, float Mx, float My);
	void DrawMinMaxBoxFilled(float mx, float my, float Mx, float My);
	void DrawMinMaxOverlaps(PolygonObject* *Polys, int NumPolys);
	void DrawMinMaxBoxes(PolygonObject* *Polys, int NumPolys);
	void ComputeVoronoiOverlaps(VoronoiSites* vSites, PolygonObject* *Polys, int NumPolys,
	                            VoronoiOverlap* &VorOverlaps, int &NumOverlaps, int ReadDepth);
	void DrawVoronoiOverlaps(VoronoiOverlap *VorOverlaps, int NumOverlaps);
	void DeleteVoronoiOverlaps(VoronoiOverlap* &VorOverlaps, int &NumOverlaps);
	void Pixel2Eye(float px, float py, float *ex, float *ey);
	int  SelectSite(float px, float py, PolygonObject* *Polys, int NumPolys);
	void ErrorCheckGL();
	void myDisplay_Overlaps(void);
	void myDisplay(void);
	void myIdle();
	void myReshape(int w, int h);
	void myMouseFunc(int button, int state, int x, int y);
	void KeyInputHandler(unsigned char Key, int x, int y);


};

#endif // _NBODY_H_