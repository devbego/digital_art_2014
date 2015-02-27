#include "ExtraKnuckles.h"

const IndexSet pinkyIndices  = IndexSet() /IndexRange(21,41);
const IndexSet ringIndices   = IndexSet() /IndexRange(42,62);
const IndexSet middleIndices = IndexSet() /IndexRange(63,83);
const IndexSet indexIndices  = IndexSet() /IndexRange(84,104);

const IndexSet baseIndices = IndexSet() /IndexRange(0,5);
const IndexSet knuckleIndices = IndexSet() /IndexRange(3,11);
const IndexSet tipIndices = IndexSet() /IndexRange(9,20);

const IndexSet lowerGapIndices = IndexSet() /IndexRange(0,5);
const IndexSet upperGapIndices = IndexSet() /IndexRange(9,14);

const IndexSet blendIndices = IndexSet() /IndexRange(0,5);

const IndexSet allFingerIndices = IndexSet()
/IndexRange(21,41)
/IndexRange(42,62)
/IndexRange(63,83)
/IndexRange(84,104);

string ExtraKnuckles::getName() const {
    return "ExtraKnuckles";
}

void ExtraKnuckles::initialize(){
    ExtraKnuckles::initializeGui();
    this->gui->autoSizeToFitWidgets();
    
	final.load("models/mesh_ExtraKnuckles_final.ply");
	for (int i = 0; i < final.getNumVertices(); i++) {
        // This is important
		ofVec2f aMeshVertex;
		aMeshVertex.x =  final.getVertex(i).y;
		aMeshVertex.y =  768 - final.getVertex(i).x;
		final.addTexCoord( aMeshVertex );
	}
    
    fingerMeshes.resize(4);
    
    fingerBaseMeshes.resize(4);
    knuckleMeshes.resize(4);
    fingertipMeshes.resize(4);
    
    lowerGapMeshes.resize(4);
    upperGapMeshes.resize(4);
    
    blendMeshes.resize(4);
    
	// Cache the topology subdivision.
    butterflySubdivider.topology_start(final);
    butterflySubdivider.topology_subdivide_boundary(2);
    refinedMesh = butterflySubdivider.topology_end();
    
	bUseButterfly = true;
}

void ExtraKnuckles::update(const ofMesh& mesh) {
    ofMesh handMesh = mesh;
    
    // copy each finger into its own mesh
    fingerMeshes[0] = copySubmesh(handMesh, pinkyIndices);
    fingerMeshes[1] = copySubmesh(handMesh, ringIndices);
    fingerMeshes[2] = copySubmesh(handMesh, middleIndices);
    fingerMeshes[3] = copySubmesh(handMesh, indexIndices);
    
    // get rid of the fingers on the main mesh - we'll put them back on later
    handMesh = removeSubmesh(handMesh, allFingerIndices);
    
    for(int i = 0; i < 4; i++) {
        // split the fingers up into the pieces we need
        fingerBaseMeshes[i] = copySubmesh(fingerMeshes[i], baseIndices);
        knuckleMeshes   [i] = copySubmesh(fingerMeshes[i], knuckleIndices);
        fingertipMeshes [i] = copySubmesh(fingerMeshes[i], tipIndices);
        
        // make some gaps for the finger extension meshes
        ofVec3f knuckleMoveUpAmt = fingerMeshes[i].getVertex(4) - fingerMeshes[i].getVertex(1);
        for(int v = 0; v < knuckleMeshes[i].getVertices().size(); v++) {
            knuckleMeshes[i].setVertex(v, knuckleMeshes[i].getVertex(v) + knuckleMoveUpAmt);
        }
        
        ofVec3f fingertipMoveUpAmt = fingerMeshes[i].getVertex(10) - fingerMeshes[i].getVertex(7);
        for(int v = 0; v < fingertipMeshes[i].getVertices().size(); v++) {
            fingertipMeshes[i].setVertex(v, fingertipMeshes[i].getVertex(v) + knuckleMoveUpAmt);
            fingertipMeshes[i].setVertex(v, fingertipMeshes[i].getVertex(v) + fingertipMoveUpAmt);
        }
        
        // create the gap-filling meshes
        lowerGapMeshes[i] = copySubmesh(fingerMeshes[i], lowerGapIndices);
        upperGapMeshes[i] = copySubmesh(fingerMeshes[i], upperGapIndices);
        
        // position the gap-filling meshes so that they fill the gaps
        lowerGapMeshes[i].setVertex(0, fingerBaseMeshes[i].getVertex(3));
        lowerGapMeshes[i].setVertex(1, fingerBaseMeshes[i].getVertex(4));
        lowerGapMeshes[i].setVertex(2, fingerBaseMeshes[i].getVertex(5));
        
        lowerGapMeshes[i].setVertex(3, knuckleMeshes[i].getVertex(0));
        lowerGapMeshes[i].setVertex(4, knuckleMeshes[i].getVertex(1));
        lowerGapMeshes[i].setVertex(5, knuckleMeshes[i].getVertex(2));
        
        upperGapMeshes[i].setVertex(0, knuckleMeshes[i].getVertex(6));
        upperGapMeshes[i].setVertex(1, knuckleMeshes[i].getVertex(7));
        upperGapMeshes[i].setVertex(2, knuckleMeshes[i].getVertex(8));
        
        upperGapMeshes[i].setVertex(3, fingertipMeshes[i].getVertex(0));
        upperGapMeshes[i].setVertex(4, fingertipMeshes[i].getVertex(1));
        upperGapMeshes[i].setVertex(5, fingertipMeshes[i].getVertex(2));
        
        // fix the texture coordinates to align the meshes so we see the extra knuckle
        // UGHHH WHO WRITES CODE LIKE THIS ?????
        upperGapMeshes[i].setTexCoord(3, fingertipMeshes[i].getTexCoord(0));
        upperGapMeshes[i].setTexCoord(4, fingertipMeshes[i].getTexCoord(1));
        upperGapMeshes[i].setTexCoord(5, fingertipMeshes[i].getTexCoord(2));
        
        upperGapMeshes[i].setTexCoord(0, knuckleMeshes[i].getTexCoord(3));
        upperGapMeshes[i].setTexCoord(1, knuckleMeshes[i].getTexCoord(4));
        upperGapMeshes[i].setTexCoord(2, knuckleMeshes[i].getTexCoord(5));
        
        knuckleMeshes[i].setTexCoord(6, knuckleMeshes[i].getTexCoord(3));
        knuckleMeshes[i].setTexCoord(7, knuckleMeshes[i].getTexCoord(4));
        knuckleMeshes[i].setTexCoord(8, knuckleMeshes[i].getTexCoord(5));
        
        knuckleMeshes[i].setTexCoord(3, knuckleMeshes[i].getTexCoord(0));
        knuckleMeshes[i].setTexCoord(4, knuckleMeshes[i].getTexCoord(1));
        knuckleMeshes[i].setTexCoord(5, knuckleMeshes[i].getTexCoord(2));
        
        knuckleMeshes[i].setTexCoord(0, lowerGapMeshes[i].getTexCoord(0));
        knuckleMeshes[i].setTexCoord(1, lowerGapMeshes[i].getTexCoord(1));
        knuckleMeshes[i].setTexCoord(2, lowerGapMeshes[i].getTexCoord(2));
        
        lowerGapMeshes[i].setTexCoord(3, knuckleMeshes[i].getTexCoord(6));
        lowerGapMeshes[i].setTexCoord(4, knuckleMeshes[i].getTexCoord(7));
        lowerGapMeshes[i].setTexCoord(5, knuckleMeshes[i].getTexCoord(8));
        
        lowerGapMeshes[i].setTexCoord(0, knuckleMeshes[i].getTexCoord(3));
        lowerGapMeshes[i].setTexCoord(1, knuckleMeshes[i].getTexCoord(4));
        lowerGapMeshes[i].setTexCoord(2, knuckleMeshes[i].getTexCoord(5));
        
        // create blend meshes
        blendMeshes[i] = copySubmesh(knuckleMeshes[i], blendIndices);
        
        blendMeshes[i].setTexCoord(0, lowerGapMeshes[i].getTexCoord(3));
        blendMeshes[i].setTexCoord(1, lowerGapMeshes[i].getTexCoord(4));
        blendMeshes[i].setTexCoord(2, lowerGapMeshes[i].getTexCoord(5));
        
        blendMeshes[i].setTexCoord(3, upperGapMeshes[i].getTexCoord(3));
        blendMeshes[i].setTexCoord(4, upperGapMeshes[i].getTexCoord(4));
        blendMeshes[i].setTexCoord(5, upperGapMeshes[i].getTexCoord(5));
        
        vector<ofFloatColor> colors(6);
        colors[0] = ofFloatColor(1,1);
        colors[1] = ofFloatColor(1,1);
        colors[2] = ofFloatColor(1,1);
        colors[3] = ofFloatColor(1,0);
        colors[4] = ofFloatColor(1,0);
        colors[5] = ofFloatColor(1,0);
        blendMeshes[i].addColors(colors);
    }
    
    // create a mesh containing everything - we need this for smoothing
    final = handMesh;
    for(int i = 0; i < 4; i++) {
        final.append(fingerBaseMeshes[i]);
        final.append(knuckleMeshes[i]);
        final.append(fingertipMeshes[i]);
        final.append(upperGapMeshes[i]);
        final.append(lowerGapMeshes[i]);
    }
    
    if(saveFinalMesh) final.save("models/mesh_ExtraKnuckles_final.ply");
}

void ExtraKnuckles::draw (const ofTexture& texture) {
	
	texture.bind();
	if (bUseButterfly){
		
		butterflySubdivider.fixMesh (final, refinedMesh);
		refinedMesh.drawFaces();
		
        final.drawFaces();
        
        for(int i = 0; i < 4; i++) {
            fingerBaseMeshes[i].drawFaces();
            knuckleMeshes[i].drawFaces();
            fingertipMeshes[i].drawFaces();
            
            lowerGapMeshes[i].drawFaces();
            upperGapMeshes[i].drawFaces();
            
            blendMeshes[i].drawFaces();
        }
		
	} else {
		final.drawFaces();     // was drawBase();
		//blendMesh.drawFaces(); // was drawBlends();
	}
	texture.unbind();
}

void ExtraKnuckles::drawBlends() {
	
}

ofMesh& ExtraKnuckles::getModifiedMesh() {
    return final;
}

void ExtraKnuckles::saveMeshes(){
	
}
