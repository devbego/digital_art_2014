#include "SwellingFingers.h"

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

const IndexSet knuckeIndices = IndexSet() /IndexRange(3,11);

string SwellingFingers::getName() const {
    return "SwellingFingers";
}

void SwellingFingers::initialize(){
    SwellingFingers::initializeGui();
    this->gui->addSlider("exponentialSwellAmt", 0.0, 0.1, &exponentialSwellAmt);
    this->gui->autoSizeToFitWidgets();
    
    exponentialSwellAmt = 0.0;
    
    fingerMeshes.resize(5);
    
	final.load("models/mesh_SwellingFingers_final.ply");
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

void SwellingFingers::update(const ofMesh& mesh) {
    ofMesh handMesh = mesh;

    // make a copy of the removal region, to be used for blending
    //blendMesh = copySubmesh(handMesh, ringBaseIndices);

    // remove the triangles for the remaining indices
    //handMesh = removeSubmesh(handMesh, ringIndices);
    
    fingerMeshes[0] = copySubmesh(handMesh, pinkyIndices);
    fingerMeshes[1] = copySubmesh(handMesh, ringIndices);
    fingerMeshes[2] = copySubmesh(handMesh, middleIndices);
    fingerMeshes[3] = copySubmesh(handMesh, indexIndices);
    fingerMeshes[4] = copySubmesh(handMesh, thumbIndices);
    
    handMesh = removeSubmesh(handMesh, allFingerIndices);
    
    float baseSwellAmount[] = {
        0, 1, 2, 3, 4,
        5, 5, 5, 5, 5,
        4, 3, 2, 1, 0,
    };
    
    for(int i = 0; i < 15; i++) {
        baseSwellAmount[i] = (baseSwellAmount[i] * baseSwellAmount[i]) * exponentialSwellAmt;
        if(baseSwellAmount[i] < 0) baseSwellAmount[i] = 0;
    }
    
    int indicesNeedNormals[]     = {
        0,3,6,9,12,
        15,18,19,20,17,
        14,11,8,5,2
    };
    int normalsDirectionSource[] = {
        1,4,7,10,13,
        16,16,16,16,16,
        13,10,7,4,1
    };
    int normalsToCalculateCount = 15;
    
    for(int i = 0; i < normalsToCalculateCount; i++) {
        for(int f = 0; f < 5; f++) {
            ofVec3f source = fingerMeshes[f].getVertex(normalsDirectionSource[i]);
            ofVec3f dir    = fingerMeshes[f].getVertex(indicesNeedNormals[i]);
            
            ofVec3f normal = dir.operator-(source);
            normal.operator*=(baseSwellAmount[i]);
            
            fingerMeshes[f].setVertex(indicesNeedNormals[i], dir.operator+(normal));
        }
    }
    
    final = handMesh;
    for(int f = 0; f < 5; f++) {
        final.append(fingerMeshes[f]);
    }
    
    if(saveFinalMesh) final.save("models/mesh_SwellingFingers_final.ply");
}

ofMesh& SwellingFingers::getModifiedMesh() {
    return final;
}

void SwellingFingers::drawBlends() {
	
}

void SwellingFingers::draw (const ofTexture& texture) {
	
	texture.bind();
	if (bUseButterfly){
		
		butterflySubdivider.fixMesh (final, refinedMesh);
		refinedMesh.drawFaces();
		
        final.drawFaces();
        
        for(int f = 0; f < 5; f++) {
            fingerMeshes[f].drawFaces();
        }
        
	} else {
		final.drawFaces();     // was drawBase();
		blendMesh.drawFaces(); // was drawBlends();
	}
	texture.unbind();
}

void SwellingFingers::saveMeshes(){

}
