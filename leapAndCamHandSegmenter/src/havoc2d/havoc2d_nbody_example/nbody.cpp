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


#include "nbody.h"


//=============================================================
void nbody::DrawMinMaxBox(float mx, float my, float Mx, float My)
{
	
	glBegin(GL_LINE_LOOP);
		glVertex2f(mx,my); 
		glVertex2f(Mx,my); 
		glVertex2f(Mx,My); 
		glVertex2f(mx,My);
	glEnd();
  
}


//=============================================================
void nbody::DrawMinMaxBoxFilled(float mx, float my, float Mx, float My)
{
	
	glBegin(GL_QUADS);
		glVertex2f(mx,my); 
		glVertex2f(Mx,my); 
		glVertex2f(Mx,My); 
		glVertex2f(mx,My);
	glEnd();
  
}


//=============================================================
// OVERLAPPING MIN/MAX BOX WILL BE MIN = max of mins, MAX = min of maxs
// IF MIN/MAX DOES NOT FORM A VALID MIN/MAX BOX (MAX<MIN), THEN A AND B DO NOT OVERLAP
// IF 0 IS RETURNED, (mx,my,Mx,My) IS INVALID
inline int nbody::MinMaxBoxOverlap(const float Amx, const float Amy, const float AMx, const float AMy,
                                   const float Bmx, const float Bmy, const float BMx, const float BMy,
                                   float *mx, float *my, float *Mx, float *My)
{
  *mx = (Amx>Bmx)?Amx:Bmx;
  *Mx = (AMx<BMx)?AMx:BMx;
  if (*Mx<*mx) return(0);
  *my = (Amy>Bmy)?Amy:Bmy;
  *My = (AMy<BMy)?AMy:BMy;
  if (*My<*my) return(0);
  return(1);
}


//=============================================================
void nbody::DrawMinMaxOverlaps(PolygonObject* *Polys, int NumPolys)
{
  int i;
  float Amx, Amy, AMx, AMy, Bmx, Bmy, BMx, BMy, mx, my, Mx, My;
  for (i=0; i<(NumPolys-1); i++)
  {
    Polys[i]->ComputeMinMaxBox(&Amx,&Amy,&AMx,&AMy);
    for (int j=i+1; j<NumPolys; j++)
    {
      Polys[j]->ComputeMinMaxBox(&Bmx,&Bmy,&BMx,&BMy);
      if (MinMaxBoxOverlap(Amx,Amy,AMx,AMy,Bmx,Bmy,BMx,BMy,&mx,&my,&Mx,&My))
        DrawMinMaxBox(mx,my,Mx,My);
    }
  }
}


//=============================================================
void nbody::DrawMinMaxBoxes(PolygonObject* *Polys, int NumPolys)
{
  float mx, my, Mx, My;
  for (int i=0; i<NumPolys; i++)
  {
    Polys[i]->ComputeMinMaxBox(&mx,&my,&Mx,&My);
    DrawMinMaxBox(mx,my,Mx,My);
  }
}


//=============================================================
void nbody::ComputeVoronoiOverlaps(VoronoiSites* vSites, PolygonObject* *Polys, int NumPolys,
                            VoronoiOverlap* &VorOverlaps, int &NumOverlaps, int ReadDepth)
{
  int i;

  if (VorOverlaps==NULL) VorOverlaps = new VoronoiOverlap[NumPolys*NumPolys];
  NumOverlaps=0;

  float Amx, Amy, AMx, AMy, Bmx, Bmy, BMx, BMy, mx, my, Mx, My;
  for (i=0; i<(NumPolys-1); i++)
  {
    Polys[i]->ComputeMinMaxBox(&Amx,&Amy,&AMx,&AMy);
    for (int j=i+1; j<NumPolys; j++)
    {
      Polys[j]->ComputeMinMaxBox(&Bmx,&Bmy,&BMx,&BMy);
      if (MinMaxBoxOverlap(Amx,Amy,AMx,AMy,Bmx,Bmy,BMx,BMy,&mx,&my,&Mx,&My))
      {
        VoronoiSites* tmp_vSites = new VoronoiSites();
        tmp_vSites->Add(vSites->Sites[i],vSites->Sites[i]->SiteID);
        tmp_vSites->Add(vSites->Sites[j],vSites->Sites[j]->SiteID);

        vdCompute(tmp_vSites, mx,my,Mx,My, MaxDist, DistError,FillGeomRatio, 
                  &(VorOverlaps[NumOverlaps].ResX), &(VorOverlaps[NumOverlaps].ResY),UseFastClear?2:1);
        if (ReadDepth)
          VorOverlaps[NumOverlaps].TexID = vdCreateTextureObjFromDepthBuffer(VorOverlaps[NumOverlaps].ResX,VorOverlaps[NumOverlaps].ResY,1);
        else
          VorOverlaps[NumOverlaps].TexID = vdCreateTextureObjFromColorBuffer(VorOverlaps[NumOverlaps].ResX,VorOverlaps[NumOverlaps].ResY);
        VorOverlaps[NumOverlaps].mx=mx;
        VorOverlaps[NumOverlaps].my=my;
        VorOverlaps[NumOverlaps].Mx=Mx;
        VorOverlaps[NumOverlaps].My=My;
        NumOverlaps++;

        tmp_vSites->NumSites=0;
        delete tmp_vSites;
      }
    }
  }
}


//=============================================================
void nbody::DrawVoronoiOverlaps(VoronoiOverlap *VorOverlaps, int NumOverlaps)
{
  for (int i=0; i<NumOverlaps; i++)
    vdDrawTextureObj(VorOverlaps[i].mx,VorOverlaps[i].my,VorOverlaps[i].Mx,VorOverlaps[i].My,
                     VorOverlaps[i].TexID,VorOverlaps[i].ResX,VorOverlaps[i].ResY);
}


//=============================================================
void nbody::DeleteVoronoiOverlaps(VoronoiOverlap* &VorOverlaps, int &NumOverlaps)
{
  for (int i=0; i<NumOverlaps; i++)
    glDeleteTextures(1,&(VorOverlaps[i].TexID));
  delete[] VorOverlaps;
  VorOverlaps=NULL;
  NumOverlaps=0;
}


//=============================================================
void nbody::Pixel2Eye(float px, float py, float *ex, float *ey)
{
  int VP[4];
  glGetIntegerv(GL_VIEWPORT,VP);
  int W=VP[2], H=VP[3];  
  *ex=(px/W)*2.0f-1.0f;
  *ey=(1.0f-py/H)*2.0f-1.0f;
}


//=============================================================
int nbody::SelectSite(float px, float py, PolygonObject* *Polys, int NumPolys)
{
  if (NumPolys<1) return(-1);

  float ex,ey;
  Pixel2Eye(px,py,&ex,&ey);

  int iSelected=0;
  float mx, my, Mx, My, cx, cy, dx, dy, Dist2, MinDist2;
  Polys[0]->ComputeMinMaxBox(&mx,&my,&Mx,&My);
  cx=(mx+Mx)*0.5f;   cy=(my+My)*0.5f;
  dx=cx-ex;          dy=cy-ey;
  MinDist2 = dx*dx+dy*dy;
  for (int i=1; i<NumPolys; i++)
  {
    Polys[i]->ComputeMinMaxBox(&mx,&my,&Mx,&My);
    cx=(mx+Mx)*0.5f;   cy=(my+My)*0.5f;
    dx=cx-ex;          dy=cy-ey;
    Dist2 = dx*dx+dy*dy;
    if (Dist2<MinDist2) { MinDist2=Dist2; iSelected=i; }
  }
  return(iSelected);
}


//=============================================================
void nbody::ErrorCheckGL()
{
  GLenum errCode;
  if ((errCode=glGetError()) != GL_NO_ERROR)
    fprintf(stderr,"OpenGL error: %s\n",gluErrorString(errCode));
}


//=============================================================
void nbody::myDisplay_Overlaps(void)
{
  clock_t StartTime;
  if (TimerOn) StartTime=clock();

  VoronoiOverlap *VorOverlaps=NULL;
  int NumOverlaps=0;
  ComputeVoronoiOverlaps(vSites,Polys,NUM_NBODIES,VorOverlaps,NumOverlaps,ShowDepth);
  DrawVoronoiOverlaps(VorOverlaps,NumOverlaps);
  DeleteVoronoiOverlaps(VorOverlaps,NumOverlaps);

  // DRAW VORONOI SITES
  if (DrawSitesOn)
  {
    glColor3f(0.75f,0.75f,0.75f);
    vdDrawSites(vSites, -1,-1,1,1, ResX,ResY);
  }

  // DRAW BOUNDING BOXES
  if (DrawMinMaxBoxesOn)
  {
    glColor3f(0,0,1);
    DrawMinMaxBoxes(Polys,NUM_NBODIES);
  }

  /// if (TimerOn) { glColor3f(1,1,1); Text(0,0,"%4.1f fps", (float)CLOCKS_PER_SEC/(clock()-StartTime)); }

  //glutSwapBuffers();
	//ErrorCheckGL();
}


//=============================================================
void nbody::myDisplay(void)
{
	
	int i;

	// COMPUTE WORLD COORDS OF SITES
	for (i=0; i<NUM_NBODIES; i++){
		Polys[i]->UpdateWorldPts();
	}
	
	// COMPUTE VORONOI DIAGRAM
	//glColor3f(1,1,1);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  	if (ComputeVoronoiOn){
		vdCompute (vSites, -1,-1,1,1, MaxDist, FillGeomRatio, ResX,ResY, UseFastClear?2:1);
	}
	
/*
VoronoiOverlap *VorOverlaps=NULL;
int NumOverlaps=0;
ComputeVoronoiOverlaps(vSites,Polys,NUM_NBODIES,VorOverlaps,NumOverlaps,ShowDepth);
DrawVoronoiOverlaps(VorOverlaps,NumOverlaps);
DeleteVoronoiOverlaps(VorOverlaps,NumOverlaps);
*/
   
	/*
	// THE POLYS THEMSELVES
	if (DrawSitesOn){
	   glColor3f(1.0f,1.0f,0.0f);
	   vdDrawSites(vSites, -1,-1,1,1, ResX,ResY);
   }
   
   
	// BOUNDING BOXES
   if (DrawMinMaxBoxesOn){
	   glColor3f(0,0,1);
	   glPushMatrix();
	   glScalef(ResX*0.5, ResY*0.5, 1.0);
	   glTranslatef(1, 1, 0.0);
	   DrawMinMaxBoxes(Polys,NUM_NBODIES);
	   glPopMatrix();
   }
   
   
   // OVERLAPS
   if (DrawOverlappingRegionsOn){
	   glColor3f(1,0,0);
	   glPushMatrix();
	   glScalef(ResX*0.5, ResY*0.5, 1.0);
	 	glTranslatef(1, 1, 0.0);
	   DrawMinMaxOverlaps(Polys,NUM_NBODIES);
	   glPopMatrix();
   }
 */

}


//=============================================================
void nbody::myIdle()
{
	for (int i=0; i<NUM_NBODIES; i++){
		((VoronoiPolylineSite *)(vSites->Sites[i]))->dist_mesh_color[0] = siteColors[i]->r;
		((VoronoiPolylineSite *)(vSites->Sites[i]))->dist_mesh_color[1] = siteColors[i]->g;
		((VoronoiPolylineSite *)(vSites->Sites[i]))->dist_mesh_color[2] = siteColors[i]->b;
	}
	
}

void nbody::myReshape(int w, int h)
{
  //glViewport(0, 0, w, h);
  ResX=w;
  ResY=h;

  // COMPUTE DistError BASED ON CURRENT RESOLUTION OF WINDOW SO FULL-SCREEN AND OVERLAP
  // VORONOI REGIONS ARE THE SAME RESOLUTION
  DistError = vdCalcDistError(-1,-1,1,1,ResX,ResY,FillGeomRatio);
}


//=============================================================
void nbody::myMouseFunc(int button, int state, int x, int y)
{
	
	LeftButtonDown=!LeftButtonDown;
    NewX=x; NewY=y;
    iSelectedSite = SelectSite(x,y,Polys,NUM_NBODIES);
	
	/*
  CtrlPressed = glutGetModifiers() & GLUT_ACTIVE_CTRL;

  if (button==GLUT_LEFT_BUTTON)
  {
    LeftButtonDown=!LeftButtonDown;
    NewX=x; NewY=y;
//  vdReadbackID(&iSelectedSite,x,(glutGet((GLenum)GLUT_WINDOW_HEIGHT)-1)-y,vSites);
    iSelectedSite = SelectSite(x,y,Polys,NUM_NBODIES);
  }
  else if (button==GLUT_RIGHT_BUTTON && !CtrlPressed)
  {
    RightButtonDown=!RightButtonDown;
  }
  else if (button==GLUT_RIGHT_BUTTON && CtrlPressed)
  {
    RightButtonDown=!RightButtonDown;
  }
	 */
}


//=============================================================
void nbody::KeyInputHandler(unsigned char Key, int x, int y)
{
  switch(Key)
  {
    case '`': DrawSitesOn=!DrawSitesOn; break;
    case '\\': AnimationOn=!AnimationOn; 
               break;
    case ' ': FullScreenModeOn=!FullScreenModeOn;
    case 'r': DrawOverlappingRegionsOn=!DrawOverlappingRegionsOn; break;
    case 'v': ComputeVoronoiOn=!ComputeVoronoiOn; break;
    case 'c': UseFastClear=!UseFastClear; break;
    case 't': TimerOn=!TimerOn; break;
    case 'b': DrawMinMaxBoxesOn=!DrawMinMaxBoxesOn; break;
    case 'd': ShowDepth=!ShowDepth; break;;
  };
  //glutPostRedisplay();
}

//=============================================================
void nbody::set_region_color(int i){
	
	if ((i>=0) && (i<NUM_NBODIES)){
		/// GOLAN siteColors[i]->pickColorNewColor();
	}
	
}

//=============================================================

nbody::nbody (int inw, int inh){

	ResX = inw;//352;
	ResY = inh;//256;
	
	FillGeomRatio = 0.65f;  // slower if smaller -- IN (0,1)
	vSites=NULL;
	MaxDist = 0.19; //faster if smaller 2.83f; // sqrt(8.0f);
	//float MaxDist=0.1f;

	FullScreenModeOn=1;
	AnimationOn=1;
	DrawSitesOn=0;
	DrawMinMaxBoxesOn=0;
	DrawOverlappingRegionsOn=0;
	ComputeVoronoiOn=1;
	UseFastClear=0;
	TimerOn=0;
	ShowDepth=1;

	// NAVIGATION
	LeftButtonDown=0;        // MOUSE STUFF
	RightButtonDown=0;
	CtrlPressed=0;


  // CREATE LIST OF POLYGON OBJECTS
  for (int i=0; i<NUM_NBODIES; i++){
    Polys[i] = new PolygonObject(0,0,1,1,
    											0,0,1,1, 
    											MAX_NBODY_POLYLINE_LENGTH, 0.0f,0.0f);
    
  }

  // CREATE LIST OF VORONOI SITES (LINK SITES TO ANIMATED POINTS)
  vSites = new VoronoiSites();
  VoronoiPolylineSite *PolySite;
  
  
  // create colors for different voronoi regions
  printf("-----------------------------------\n");
  handyString = new char[64];
  siteColors = new GolanColor*[NUM_NBODIES];
  for (int i=0; i<NUM_NBODIES; i++){
	  float gi = (float)i/(float)NUM_NBODIES;
	  siteColors[i] = new GolanColor();
	  siteColors[i]->r = ofRandom(0, 1);
	  siteColors[i]->g = ofRandom(0, 1);
	  siteColors[i]->b = ofRandom(0, 1);
	  /*
  		sprintf(handyString, "terrain/color_%d.xml", i);
  		siteColors[i] = new dynamicColor(handyString, gi,gi,gi);
  		siteColors[i]->loadColorFromXML();
	   */
  }
  printf("-----------------------------------\n");
	


  for (int i=0; i<NUM_NBODIES; i++)
  {
    PolySite = new VoronoiPolylineSite(Polys[i]->wPts,Polys[i]->NumPts);
    PolySite->dist_mesh_color[0] = siteColors[i]->r;
    PolySite->dist_mesh_color[1] = siteColors[i]->g;
    PolySite->dist_mesh_color[2] = siteColors[i]->b;
  
    vSites->Add(PolySite);
  }
  
  myReshape(ResX, ResY);




	/*
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(ResX,ResY);
  glutInitWindowPosition(180,100);
  glutCreateWindow("Kenny's Voronoi-based Collisions");

  glutKeyboardFunc(KeyInputHandler);
  glutReshapeFunc(myReshape);
  glutDisplayFunc(myDisplay);
  glutIdleFunc(myIdle);
  glutMouseFunc(myMouseFunc);

  glDisable(GL_DEPTH_TEST);

  // DEFAULT STATE FOR VORONOI SYSTEM
  glShadeModel(GL_FLAT);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DITHER);
  glPixelStorei(GL_PACK_ALIGNMENT,1); 
  glPixelStorei(GL_UNPACK_ALIGNMENT,1);

  glutMainLoop();
  */
}

