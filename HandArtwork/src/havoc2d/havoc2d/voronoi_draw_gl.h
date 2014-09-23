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
// voronoi_draw_gl.h :
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

#ifndef _VORONOI_DRAW_GL_
#define _VORONOI_DRAW_GL_

#include "ofMain.h"

GLuint vdCreateTextureObjFromColorBuffer(GLubyte *Color, int Width, int Height);
GLuint vdCreateTextureObjFromColorBuffer(int Width, int Height);

GLuint vdCreateTextureObjFromDepthBuffer(GLfloat *Depth, int Width, int Height);
GLuint vdCreateTextureObjFromDepthBuffer(int Width, int Height, int NormalizeOn=0);

void vdDrawTextureObj(float MinX, float MinY, float MaxX, float MaxY, GLuint TexID, int ResX, int ResY);

#endif
