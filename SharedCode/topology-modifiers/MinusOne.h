#pragma once

#include "TopologyModifier.h"
#include "MeshUtils.h"

class MinusOne : public TopologyModifier {
protected:
    
    ofPolyline removalRegion;
    vector<pair<ofIndexType, ofIndexType> > stitchIndices;
    
    ofMesh blendMesh;
    
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