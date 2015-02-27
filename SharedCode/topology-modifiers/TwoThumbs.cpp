#include "TwoThumbs.h"

const IndexSet thumbIndices = IndexSet()
/IndexRange(0,20)
/86/116/121/126/131/136
/IndexRange(144,150);

const IndexSet blendIndices = IndexSet()
/112/115/120/125/130/135/ 142/84/85/86/ 136/131/126/121/116/113/ 143;

const IndexSet replacementEdgeIndices = IndexSet()
/24/25/26 /21/22/23 /132/137/138/ 133/ 127/128/ 122/123/ 117/118/ 114/115/ 111/112;

const IndexSet gapReplacementIndices = IndexSet()
/112/115/118/123/128/
133/138/23/22/
21/132/127/122/117/114/111/
137;

string TwoThumbs::getName() const {
    return "TwoThumbs";
}

void TwoThumbs::initialize() {
    edgeStretch = 1.0;
    
    TwoThumbs::initializeGui();
    this->gui->addSlider("Edge Stretch", 0.0, 5.0, &edgeStretch);
    this->gui->autoSizeToFitWidgets();
    
	final.load("models/mesh_TwoThumbs_final.ply");
	for (int i = 0; i < final.getNumVertices(); i++) {
        // This is important
		ofVec2f aMeshVertex;
		aMeshVertex.x =  final.getVertex(i).y;
		aMeshVertex.y =  768 - final.getVertex(i).x;
		final.addTexCoord( aMeshVertex );
	}
    
	// Cache the topology subdivision.
    butterflySubdivider.topology_start(final);
    butterflySubdivider.topology_subdivide_boundary(2);
    refinedMesh = butterflySubdivider.topology_end();
    
	bUseButterfly = true;
}

void TwoThumbs::update(const ofMesh& mesh) {
    ofMesh handMesh = mesh;
    
    thumbMesh = copySubmesh(handMesh, thumbIndices);
    
    ofVec3f flipVertex = thumbMesh.getVertex(21); // closest point to index finger
    ofVec3f bottomVertex = thumbMesh.getVertex(22); // closest point to wrist
    
// fip vertically on flipVertex
    for (int i = 0; i < thumbMesh.getNumVertices(); i++) {
        ofVec3f v = thumbMesh.getVertex(i);
        
        ofVec3f flippedVertex = ofVec3f(v.x, flipVertex.y-(v.y-flipVertex.y)-150, v.z);
        thumbMesh.setVertex(i, flippedVertex);
	}
    
    handMesh.append(thumbMesh);
    
//fix mesh so it doesn't cactus
    //find ratio between length of thumb side and unthumbed side
    ofVec3f actualThumbA = handMesh.getVertex(86);
    ofVec3f actualThumbB = handMesh.getVertex(116);
    float actualThumbBaseLength = actualThumbB.distance(actualThumbA);
    
    ofVec3f extraThumbA = handMesh.getVertex(21);
    ofVec3f extraThumbB = handMesh.getVertex(114);
    float extraThumbBaseLength = extraThumbB.distance(extraThumbA);
    
    float lengthRatio = actualThumbBaseLength / extraThumbBaseLength;
    
// stretch the unthumbed side to the same length as thumbed side
    int stretchIndices[] = {21,132,127,122,117,114,111,108,105};
    int numStretchedIndices = 9;
    
    int stretchFromIndex = 21;
    ofVec3f stretchFromVertex = handMesh.getVertex(stretchFromIndex);
    
    for(int i = 0; i < numStretchedIndices; i++) {
        ofVec3f currentVertex = handMesh.getVertex(stretchIndices[i]);
        
        ofVec3f d = currentVertex - stretchFromVertex;
        d = d * lengthRatio;
        d = d * edgeStretch;
        d = d + stretchFromVertex;
        
        handMesh.setVertex(stretchIndices[i], d);
    }
    
// attach thumb
    
    int toStitchLeft[] = {21, 132,127,122,117,114};
    int toStitchRight[] = {172,177,176,175,174,173};
    int toStitchCount = 6;
    stitchIndices.clear();
    for(int i = 0; i < toStitchCount; i++) {
        stitchIndices.push_back(pair<ofIndexType, ofIndexType>(toStitchLeft[i], toStitchRight[i]));
    }
    handMesh = stitch(handMesh, stitchIndices, STITCH_FROM_LEFT);
    
//save mesh (for butterfly smoothing)
    if(saveFinalMesh) handMesh.save("models/mesh_TwoThumbs_final.ply");
    
//blending (original)
    
    extraThumbBlendMesh = copySubmesh(handMesh, blendIndices);
    
    int toBeMovedIndices[] = {3,5,7,9,11,13,15,0,1,2,14,12,10,8,6,4,16};
    int moveToIndices[]    = {112,115,118,123,128,133,138,23,22,21,132,127,122,117,114,111,137};
    int moveVertsCount = 17;
    
    for(int i = 0; i < moveVertsCount; i++) {
        extraThumbBlendMesh.setVertex(toBeMovedIndices[i],
                                      handMesh.getVertex(moveToIndices[i]));
    }
    
    int opacity[] = {0,0.5,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0.5};
    int changeOpacityCount = 17;
    
    vector<ofFloatColor> colors(changeOpacityCount);
    for(int i = 0; i < changeOpacityCount; i++) {
        colors[i] = ofFloatColor(1, opacity[i]);
    }
    extraThumbBlendMesh.addColors(colors);
    
//finalize
    final = handMesh;
}

ofMesh& TwoThumbs::getModifiedMesh() {
    return final;
}

void TwoThumbs::drawBlends() {
	
}

void TwoThumbs::draw (const ofTexture& texture) {
	
	texture.bind();
	if (bUseButterfly){
		
		butterflySubdivider.fixMesh (final, refinedMesh);
		refinedMesh.drawFaces();
		
        glDisable(GL_CULL_FACE);
        
        final.drawFaces();
        
        extraThumbBlendMesh.drawFaces();
        
	} else {
		final.drawFaces();     // was drawBase();
		extraThumbBlendMesh.drawFaces(); // was drawBlends();
	}
	texture.unbind();
}

void TwoThumbs::saveMeshes(){
    
}
