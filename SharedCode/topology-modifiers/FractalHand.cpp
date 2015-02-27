#include "FractalHand.h"

const IndexSet pinkyIndices  = IndexSet() /IndexRange(21,41);
const IndexSet ringIndices   = IndexSet() /IndexRange(42,62);
const IndexSet middleIndices = IndexSet() /IndexRange(63,83);
const IndexSet indexIndices  = IndexSet() /IndexRange(84,104);
const IndexSet thumbIndices  = IndexSet() /IndexRange(0,20);

const IndexSet allFingerIndices = IndexSet()
/IndexRange(21,41)
/IndexRange(42,62)
/IndexRange(63,83)
/IndexRange(84,104)
/IndexRange(0,20);

const IndexSet miniHandIndices = IndexSet()
/IndexRange(0,104)
/IndexRange(114,150);

const IndexSet fingertipIndices = IndexSet()
/IndexRange(12,20);

string FractalHand::getName() const {
    return "FractalHand";
}

void FractalHand::initialize(){
    FractalHand::initializeGui();
    this->gui->addIntSlider("sourceBlendOpIndex", 0, 8, &sourceBlendOpIndex);
    this->gui->addIntSlider("destBlendOpIndex", 0, 7, &destBlendOpIndex);
    this->gui->autoSizeToFitWidgets();
    
	final.load("models/mesh_FractalHand_final.ply");
	for (int i = 0; i < final.getNumVertices(); i++) {
        // This is important
		ofVec2f aMeshVertex;
		aMeshVertex.x =  final.getVertex(i).y;
		aMeshVertex.y =  768 - final.getVertex(i).x;
		final.addTexCoord( aMeshVertex );
	}
    
    fingerMeshes.resize(5);
    fingertipMeshes.resize(5);
    miniHandMeshes.resize(5);
    
	// Cache the topology subdivision.
    butterflySubdivider.topology_start(final);
    butterflySubdivider.topology_subdivide_boundary(2);
    refinedMesh = butterflySubdivider.topology_end();
    
	bUseButterfly = true;
}

void FractalHand::update(const ofMesh& mesh) {
    ofMesh handMesh = mesh;
    
    // make copies of the fingers, so we can stitch little hands onto them later
    fingerMeshes[0] = copySubmesh(handMesh, pinkyIndices);
    fingerMeshes[1] = copySubmesh(handMesh, ringIndices);
    fingerMeshes[2] = copySubmesh(handMesh, middleIndices);
    fingerMeshes[3] = copySubmesh(handMesh, indexIndices);
    fingerMeshes[4] = copySubmesh(handMesh, thumbIndices);
    
    for(int i = 0; i < 5; i++) {
        // make a copy of the hand to attach to finger
        miniHandMeshes[i] = copySubmesh(handMesh, miniHandIndices);
        
        // get width of finger
        int fingertipBottomEdgeIndex = 12;
        int fingertipTopEdgeIndex    = 14;
        
        ofVec3f fingertipBottomEdgeVert = fingerMeshes[i].getVertex(fingertipBottomEdgeIndex);
        ofVec3f fingertipTopEdgeVert    = fingerMeshes[i].getVertex(fingertipTopEdgeIndex);
        
        float fingertipWidth = fingertipBottomEdgeVert.distance(fingertipTopEdgeVert);
        
        // get width of wrist
        int wristBottomEdgeIndex = 107;
        int wristTopEdgeIndex    = 105;
        
        ofVec3f wristBottomEdgeVert = miniHandMeshes[i].getVertex(wristBottomEdgeIndex);
        ofVec3f wristTopEdgeVert    = miniHandMeshes[i].getVertex(wristTopEdgeIndex);
        
        float wristWidth = wristBottomEdgeVert.distance(wristTopEdgeVert);
        
        // use ratio of finger width to wrist width to scale hand
        float widthRatio = fingertipWidth / wristWidth;
        
        for(int v = 0; v < miniHandMeshes[i].getVertices().size(); v++) {
            ofVec3f newPosition = miniHandMeshes[i].getVertex(v);
            newPosition = newPosition * widthRatio;
            miniHandMeshes[i].setVertex(v, newPosition);
        }
        
        // save fingertip on separate mesh
        fingertipMeshes[i] = copySubmesh(fingerMeshes[i], fingertipIndices);
        
        vector<ofFloatColor> colors(9);
        colors[0] = ofFloatColor(1,1);
        colors[1] = ofFloatColor(1,1);
        colors[2] = ofFloatColor(1,1);
        colors[3] = ofFloatColor(1,0);
        colors[4] = ofFloatColor(1,0);
        colors[5] = ofFloatColor(1,0);
        colors[6] = ofFloatColor(1,0);
        colors[7] = ofFloatColor(1,0);
        colors[8] = ofFloatColor(1,0);
        fingertipMeshes[i].addColors(colors);
        
        // remove fingertip from finger
        fingerMeshes[i] = removeSubmesh(fingerMeshes[i], fingertipIndices);
        
        // attach minihand to finger
        wristBottomEdgeVert = miniHandMeshes[i].getVertex(wristBottomEdgeIndex);
        wristTopEdgeVert    = miniHandMeshes[i].getVertex(wristTopEdgeIndex);
        
        orientMesh(
            miniHandMeshes[i],
            ofVec2f(wristTopEdgeVert.x,        wristTopEdgeVert.y),
            ofVec2f(wristBottomEdgeVert.x,     wristBottomEdgeVert.y),
            ofVec2f(fingertipBottomEdgeVert.x, fingertipBottomEdgeVert.y),
            ofVec2f(fingertipTopEdgeVert.x,    fingertipTopEdgeVert.y)
        );
        
        // stretch out fingertip slightly
        fingertipMeshes[i].setVertex(3, miniHandMeshes[i].getVertex(108));
        fingertipMeshes[i].setVertex(5, miniHandMeshes[i].getVertex(136));
    }
    
    // get rid of the fingers on the main hand mesh
    handMesh = removeSubmesh(handMesh, allFingerIndices);
    
    final = handMesh;
    
    if(saveFinalMesh) final.save("models/mesh_FractalHand_final.ply");
}

ofMesh& FractalHand::getModifiedMesh() {
    return final;
}

void FractalHand::drawBlends() {
	
}

void FractalHand::draw (const ofTexture& texture) {
	
    //butterflySubdivider.fixMesh (final, refinedMesh);
    //refinedMesh.drawFaces();
    
    texture.bind();
    
    //glEnable(GL_BLEND);
    
    int sfact[] = {
		GL_ZERO,
		GL_ONE,
		GL_DST_COLOR,
		GL_ONE_MINUS_DST_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA,
		GL_SRC_ALPHA_SATURATE
	};
    
	int dfact[] = {
		GL_ZERO,
		GL_ONE,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA
	};
    
    //glBlendFunc(sfact[sourceBlendOpIndex], dfact[destBlendOpIndex]);
    
    for(int i = 0; i < 5; i++) {
        miniHandMeshes[i].drawFaces();
        fingerMeshes[i].drawFaces();
        fingertipMeshes[i].drawFaces();
    }
    
    final.drawFaces();
		
	texture.unbind();
}

void FractalHand::saveMeshes(){
    
}
