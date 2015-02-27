#include "Stubby.h"

const IndexSet pinkyIndices  = IndexSet() /IndexRange(21,41);
const IndexSet ringIndices   = IndexSet() /IndexRange(42,62);
const IndexSet middleIndices = IndexSet() /IndexRange(63,83);
const IndexSet indexIndices  = IndexSet() /IndexRange(84,104);

const IndexSet allFingerIndices = IndexSet()
/IndexRange(21,41)
/IndexRange(42,62)
/IndexRange(63,83)
/IndexRange(84,104);

const IndexSet blendRemoval = IndexSet()
/IndexRange(3,8);

const IndexSet knuckeIndices = IndexSet() /IndexRange(3,11);

string Stubby::getName() const {
    return "Stubby";
}

void Stubby::update(const ofMesh& mesh) {
    ofMesh handMesh = mesh;
    
    // create individual meshes for each finger
    pinkyMesh  = copySubmesh(handMesh, pinkyIndices);
    ringMesh   = copySubmesh(handMesh, ringIndices);
    middleMesh = copySubmesh(handMesh, middleIndices);
    indexMesh  = copySubmesh(handMesh, indexIndices);
    
    // take fingers off of hand mesh
    handMesh = removeSubmesh(handMesh, allFingerIndices);
    
    // so now we have 4 finger meshes and a palm mesh.
    
    // save the piece of finger we're going to remove to use as a blend
    pinkyBlendMesh  = copySubmesh(pinkyMesh,  knuckeIndices);
    ringBlendMesh   = copySubmesh(ringMesh,   knuckeIndices);
    middleBlendMesh = copySubmesh(middleMesh, knuckeIndices);
    indexBlendMesh  = copySubmesh(indexMesh,  knuckeIndices);
    // we only need the first half of it though
    pinkyBlendMesh  = removeSubmesh(pinkyBlendMesh, blendRemoval);
    ringBlendMesh   = removeSubmesh(ringBlendMesh, blendRemoval);
    middleBlendMesh = removeSubmesh(middleBlendMesh, blendRemoval);
    indexBlendMesh  = removeSubmesh(indexBlendMesh, blendRemoval);
    
    // set the edge of the blends to be transparent for a smooth transition
    vector<ofFloatColor> colors(6);
    colors[0] = ofFloatColor(1,1);
    colors[1] = ofFloatColor(1,1);
    colors[2] = ofFloatColor(1,1);
    colors[3] = ofFloatColor(1,0);
    colors[4] = ofFloatColor(1,0);
    colors[5] = ofFloatColor(1,0);
    pinkyBlendMesh.addColors(colors);
    ringBlendMesh.addColors(colors);
    middleBlendMesh.addColors(colors);
    indexBlendMesh.addColors(colors);
    
    // remove the piece of the finger
    pinkyMesh  = removeSubmesh(pinkyMesh,  knuckeIndices);
    ringMesh   = removeSubmesh(ringMesh,   knuckeIndices);
    middleMesh = removeSubmesh(middleMesh, knuckeIndices);
    indexMesh  = removeSubmesh(indexMesh,  knuckeIndices);
    
    // stitch the fingers back together
    int toStitchLeft[] = {6,7,8};
    int toStitchRight[] = {3,4,5};
    int toStitchCount = 3;
    stitchIndices.clear();
    for(int i = 0; i < toStitchCount; i++) {
        stitchIndices.push_back(pair<ofIndexType, ofIndexType>(toStitchLeft[i], toStitchRight[i]));
    }
    pinkyMesh  = stitch(pinkyMesh,  stitchIndices, STITCH_FROM_RIGHT);
    ringMesh   = stitch(ringMesh,   stitchIndices, STITCH_FROM_RIGHT);
    middleMesh = stitch(middleMesh, stitchIndices, STITCH_FROM_RIGHT);
    indexMesh  = stitch(indexMesh,  stitchIndices, STITCH_FROM_RIGHT);
    
    // smooth out fingers
    ofVec3f pinkyTopAveragedVertex = (pinkyMesh.getVertex(2) + pinkyMesh.getVertex(11)) * 0.5;
    ofVec3f pinkyMidAveragedVertex = (pinkyMesh.getVertex(1) + pinkyMesh.getVertex(10)) * 0.5;
    ofVec3f pinkyBottomAveragedVertex = (pinkyMesh.getVertex(0) + pinkyMesh.getVertex(9)) * 0.5;
    
    ofVec3f ringTopAveragedVertex = (ringMesh.getVertex(2) + ringMesh.getVertex(11)) * 0.5;
    ofVec3f ringMidAveragedVertex = (ringMesh.getVertex(1) + ringMesh.getVertex(10)) * 0.5;
    ofVec3f ringBottomAveragedVertex = (ringMesh.getVertex(0) + ringMesh.getVertex(9)) * 0.5;
    
    ofVec3f middleTopAveragedVertex = (middleMesh.getVertex(2) + middleMesh.getVertex(11)) * 0.5;
    ofVec3f middleMidAveragedVertex = (middleMesh.getVertex(1) + middleMesh.getVertex(10)) * 0.5;
    ofVec3f middleBottomAveragedVertex = (middleMesh.getVertex(0) + middleMesh.getVertex(9)) * 0.5;
    
    ofVec3f indexTopAveragedVertex = (indexMesh.getVertex(2) + indexMesh.getVertex(11)) * 0.5;
    ofVec3f indexMidAveragedVertex = (indexMesh.getVertex(1) + indexMesh.getVertex(10)) * 0.5;
    ofVec3f indexBottomAveragedVertex = (indexMesh.getVertex(0) + indexMesh.getVertex(9)) * 0.5;
    
    pinkyMesh.setVertex(5, pinkyTopAveragedVertex);
    pinkyMesh.setVertex(8, pinkyTopAveragedVertex);
    pinkyMesh.setVertex(4, pinkyMidAveragedVertex);
    pinkyMesh.setVertex(7, pinkyMidAveragedVertex);
    pinkyMesh.setVertex(3, pinkyBottomAveragedVertex);
    pinkyMesh.setVertex(6, pinkyBottomAveragedVertex);
    
    ringMesh.setVertex(5, ringTopAveragedVertex);
    ringMesh.setVertex(8, ringTopAveragedVertex);
    ringMesh.setVertex(4, ringMidAveragedVertex);
    ringMesh.setVertex(7, ringMidAveragedVertex);
    ringMesh.setVertex(3, ringBottomAveragedVertex);
    ringMesh.setVertex(6, ringBottomAveragedVertex);
    
    middleMesh.setVertex(5, middleTopAveragedVertex);
    middleMesh.setVertex(8, middleTopAveragedVertex);
    middleMesh.setVertex(4, middleMidAveragedVertex);
    middleMesh.setVertex(7, middleMidAveragedVertex);
    middleMesh.setVertex(3, middleBottomAveragedVertex);
    middleMesh.setVertex(6, middleBottomAveragedVertex);
    
    indexMesh.setVertex(5, indexTopAveragedVertex);
    indexMesh.setVertex(8, indexTopAveragedVertex);
    indexMesh.setVertex(4, indexMidAveragedVertex);
    indexMesh.setVertex(7, indexMidAveragedVertex);
    indexMesh.setVertex(3, indexBottomAveragedVertex);
    indexMesh.setVertex(6, indexBottomAveragedVertex);
    
    // smooth out blends
    pinkyBlendMesh.setVertex(2, pinkyTopAveragedVertex);
    pinkyBlendMesh.setVertex(1, pinkyMidAveragedVertex);
    pinkyBlendMesh.setVertex(0, pinkyBottomAveragedVertex);
    
    ringBlendMesh.setVertex(2, ringTopAveragedVertex);
    ringBlendMesh.setVertex(1, ringMidAveragedVertex);
    ringBlendMesh.setVertex(0, ringBottomAveragedVertex);
    
    middleBlendMesh.setVertex(2, middleTopAveragedVertex);
    middleBlendMesh.setVertex(1, middleMidAveragedVertex);
    middleBlendMesh.setVertex(0, middleBottomAveragedVertex);
    
    indexBlendMesh.setVertex(2, indexTopAveragedVertex);
    indexBlendMesh.setVertex(1, indexMidAveragedVertex);
    indexBlendMesh.setVertex(0, indexBottomAveragedVertex);
    
    // align the blend meshes to the fingers
    pinkyBlendMesh.setVertex(3, pinkyMesh.getVertex(9));
    pinkyBlendMesh.setVertex(4, pinkyMesh.getVertex(10));
    pinkyBlendMesh.setVertex(5, pinkyMesh.getVertex(11));
    
    ringBlendMesh.setVertex(3, ringMesh.getVertex(9));
    ringBlendMesh.setVertex(4, ringMesh.getVertex(10));
    ringBlendMesh.setVertex(5, ringMesh.getVertex(11));
    
    middleBlendMesh.setVertex(3, middleMesh.getVertex(9));
    middleBlendMesh.setVertex(4, middleMesh.getVertex(10));
    middleBlendMesh.setVertex(5, middleMesh.getVertex(11));
    
    indexBlendMesh.setVertex(3, indexMesh.getVertex(9));
    indexBlendMesh.setVertex(4, indexMesh.getVertex(10));
    indexBlendMesh.setVertex(5, indexMesh.getVertex(11));
    
    // create a single mesh that contains everything, we need this for the subdivided mesh
    final = handMesh;
    final.append(pinkyMesh);
    final.append(ringMesh);
    final.append(middleMesh);
    final.append(indexMesh);
    
    if(saveFinalMesh) final.save("models/mesh_Stubby_final.ply");
}

ofMesh& Stubby::getModifiedMesh() {
    return final;
}

void Stubby::drawBlends() {
	pinkyBlendMesh.drawFaces();
    ringBlendMesh.drawFaces();
    middleBlendMesh.drawFaces();
    indexBlendMesh.drawFaces();
}

//-------------------------------------------
void Stubby::draw (const ofTexture& texture) {
	
	texture.bind();
	if (bUseButterfly){
		
		butterflySubdivider.fixMesh (final, refinedMesh);
		refinedMesh.drawFaces();
		
        final.drawFaces();
        
        pinkyMesh.drawFaces();
        ringMesh.drawFaces();
        middleMesh.drawFaces();
        indexMesh.drawFaces();
        
		drawBlends();
		
	} else {
		final.drawFaces();     // was drawBase();
		//blendMesh.drawFaces(); // was drawBlends();
	}
	texture.unbind();
}

//-------------------------------------------
void Stubby::saveMeshes(){
    ofMesh entireStubbyHand = final;
    entireStubbyHand.append(pinkyMesh);
    entireStubbyHand.append(ringMesh);
    entireStubbyHand.append(middleMesh);
    entireStubbyHand.append(indexMesh);
    
	entireStubbyHand.save("models/mesh_Stubby_final.ply");
    
    // these were used for debugging...
    
	//blendMesh.save    ("mesh_Stubby_blendMesh.ply");
    
    pinkyMesh.save("models/mesh_Stubby_pinky.ply");
    
    //pinkyBlendMesh.save("mesh_Stubby_pinkyBlend.ply");
    
    //ringMesh.save("mesh_Stubby_ring.ply");
    //middleMesh.save("mesh_Stubby_middle.ply");
    //indexMesh.save("mesh_Stubby_index.ply");
}

//-------------------------------------------
void Stubby::initialize(){
    Stubby::initializeGui();
    this->gui->autoSizeToFitWidgets();
    
	final.load("models/mesh_Stubby_final.ply");
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
