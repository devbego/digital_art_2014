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
// voronoi_draw_gl.cpp :
//   Routines used for drawing the computed Voronoi diagram.
//   The VD is normally computed in the bottom-left of the framebuffer (GL_BACK)
//   from a viewplane window defined by a Min/Max box at the specified resolution
//   ResX x ResY. These routines allow drawing of the Voronoi image at the
//   location of the Min/Max box (texture-mapped quad).
//
//   How to use:
//     < DRAW THE VORONOI DIAGRAM >
//     GLuint TexID = vdCreateTextureObjFromColorBuffer(ResX,ResY);
//     < DRAW USER SCENE >
//     vdDrawTextureObj(MinX,MinY,MaxX,MaxY,TexID,ResX,ResY);
//     glDeleteTextures(1,&TexID);
//
//============================================================================

#include <stdio.h>
/*#include <memory.h> // dis-included by golan */
#include <math.h>
#include "ofMain.h"
#include "voronoi_draw_gl.h"

//----------------------------------------------------------------------------
// CALCULATE MAX TEX-COORDS (FOR NON-SQUARE, NON POWER-OF-2 TEXTURES)
// FIND SMALLEST SQUARE POWER-OF-2 THAT CONTAINS ResX x ResY IMAGE
//----------------------------------------------------------------------------
static int CalcTexRes(int ResX, int ResY)
{
  #define LOG2(x) (log((float)x)/log(2.0f))
  int MaxRes = (ResX>ResY)?ResX:ResY;
  float flog2MaxRes = LOG2(MaxRes);
  int ilog2MaxRes = (int)flog2MaxRes;
  float Frac = flog2MaxRes - (float)ilog2MaxRes;
  if (Frac>0) ilog2MaxRes++;
  int TexRes = 1<<ilog2MaxRes;
  return(TexRes);
}

//----------------------------------------------------------------------------
// Routine used just for debugging and depth range verification
//----------------------------------------------------------------------------
static void PrintMinMax(GLfloat *Depth, int W, int H)
{
  float Min,Max;
  Min=Max=Depth[0];
  for (int i=0; i<(W*H); i++)
    if (Depth[i]<Min) Min=Depth[i]; else if (Depth[i]>Max) Max=Depth[i];
  printf("[%g,%g]\n",Min,Max);
}

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
GLuint vdCreateTextureObjFromColorBuffer(GLubyte *Color, int Width, int Height)
{
  // BE SURE THAT GIVEN BUFFER IS SQUARE, POWER-OF-2 IMAGE
  int TexRes = CalcTexRes(Width,Height);
  GLubyte *NewColor=Color;
  if (TexRes!=Width || TexRes!=Height)
  {
    NewColor = new GLubyte[TexRes*TexRes*3];
    GLubyte *Src=Color, *Dst=NewColor;
    int SrcWidth=Width*3, DstWidth=TexRes*3;
    for (int i=0; i<Height; i++, Src+=SrcWidth, Dst+=DstWidth)
      memcpy(Dst,Src,SrcWidth);
  }

  GLuint TexID;
  glGenTextures(1,&TexID);             
  glBindTexture(GL_TEXTURE_2D,TexID);  
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
  glTexImage2D(GL_TEXTURE_2D,0,3,TexRes,TexRes,0,GL_RGB,GL_UNSIGNED_BYTE,NewColor);

  if (TexRes!=Width || TexRes!=Height) delete[] NewColor;
  return( TexID );
}

GLuint vdCreateTextureObjFromColorBuffer(int Width, int Height)
{
  int TexRes = CalcTexRes(Width,Height);
  GLuint TexID;
  glGenTextures(1,&TexID);             
  glBindTexture(GL_TEXTURE_2D,TexID);  
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
  glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB,0,0,TexRes,TexRes,0);  // GRABS FROM CURRENT glReadBuffer
  return( TexID );
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
GLuint vdCreateTextureObjFromDepthBuffer(GLfloat *Depth, int Width, int Height)
{
  // BE SURE THAT GIVEN BUFFER IS SQUARE, POWER-OF-2 IMAGE
  int TexRes = CalcTexRes(Width,Height);
  GLfloat *NewDepth=Depth;
  if (TexRes!=Width || TexRes!=Height)
  {
    NewDepth = new GLfloat[TexRes*TexRes];
    GLfloat *Src=Depth, *Dst=NewDepth;
    for (int i=0; i<Height; i++, Src+=Width, Dst+=TexRes)
      memcpy(Dst,Src,Width*4);
  }

  GLuint TexID;
  glGenTextures(1,&TexID);             
  glBindTexture(GL_TEXTURE_2D,TexID);  
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
  glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,TexRes,TexRes,0,GL_LUMINANCE,GL_FLOAT,NewDepth);  // GRABS FROM CURRENT glReadBuffer

  if (TexRes!=Width || TexRes!=Height) delete[] NewDepth;
  return( TexID );
}

GLuint vdCreateTextureObjFromDepthBuffer(int Width, int Height, int NormalizeOn)
{
  int TexRes = CalcTexRes(Width,Height);

  GLfloat *Depth = new GLfloat[TexRes*TexRes];
  glReadPixels(0,0,TexRes,TexRes, GL_DEPTH_COMPONENT, GL_FLOAT, Depth);

  // NORMALIZE DEPTH VALUES (IN PLACE), FIND MAX, DIVIDE BY MAX
  if (NormalizeOn)
  {
    int i,j,k;
    float Max=0;
    for (i=0; i<Height; i++) { k=i*TexRes; for (j=0; j<Width; j++, k++) if (Depth[k]>Max) Max=Depth[k]; }
    for (i=0; i<Height; i++) { k=i*TexRes; for (j=0; j<Width; j++, k++) Depth[k]/=Max; }
  }

  GLuint TexID = vdCreateTextureObjFromDepthBuffer(Depth,TexRes,TexRes);
  delete[] Depth;

  return( TexID );
}

//----------------------------------------------------------------------------
// DRAW THE RECTANGULAR SLICE WITH THE VORONOI DIAGRAM TEXTURE-MAPPED ONTO IT
// Handles textures that are non-square and not a power of 2.
//----------------------------------------------------------------------------
void vdDrawTextureObj(float MinX, float MinY, float MaxX, float MaxY, GLuint TexID, int ResX, int ResY)
{
  int TexRes = CalcTexRes(ResX,ResY);
  float MaxS = (float)ResX / TexRes;
  float MaxT = (float)ResY / TexRes;
  
  glPushAttrib(GL_POLYGON_BIT);
  glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,TexID);  
    glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
   
    glBegin(GL_QUADS);
      glTexCoord2f(0,0);       glVertex2f(MinX,MinY);
      glTexCoord2f(MaxS,0);    glVertex2f(MaxX,MinY);
      glTexCoord2f(MaxS,MaxT); glVertex2f(MaxX,MaxY);
      glTexCoord2f(0,MaxT);    glVertex2f(MinX,MaxY);
    glEnd();

    glDisable(GL_TEXTURE_2D);
  glPopAttrib();
}
