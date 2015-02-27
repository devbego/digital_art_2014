// Stubby scene, aka stubbyfingers, one less knuckle
// rispoli 1-26-15

#pragma once

#include "TopologyModifier.h"
#include "MeshUtils.h"

class Stubby : public TopologyModifier {
protected:
    
    ofPolyline removalRegion;
    vector<pair<ofIndexType, ofIndexType> > stitchIndices;
    
    ofMesh pinkyMesh;
    ofMesh ringMesh;
    ofMesh middleMesh;
    ofMesh indexMesh;
    
    ofMesh pinkyBlendMesh;
    ofMesh ringBlendMesh;
    ofMesh middleBlendMesh;
    ofMesh indexBlendMesh;
    
    ofMesh final;
	
	ofxButterfly    butterflySubdivider;
    ofMesh          refinedMesh;
	bool			bUseButterfly;
    
public:
    string getName() const;
    void update(const ofMesh& mesh);
    ofMesh& getModifiedMesh();
    void drawBlends();
	void saveMeshes();
	void initialize();
	void draw(const ofTexture& texture);
};