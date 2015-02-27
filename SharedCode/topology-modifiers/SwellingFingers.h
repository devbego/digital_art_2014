// experimental swelling fingers/fat fingers scene!
// work in progress !!
// rispoli 1-26-15

#pragma once

#include "TopologyModifier.h"
#include "MeshUtils.h"

class SwellingFingers : public TopologyModifier {
protected:
    
    ofPolyline removalRegion;
    vector<pair<ofIndexType, ofIndexType> > stitchIndices;
    
    vector<ofMesh> fingerMeshes;
    
    ofMesh blendMesh;
    
    ofMesh final;
	
	ofxButterfly    butterflySubdivider;
    ofMesh          refinedMesh;
	bool			bUseButterfly; 
    
    float exponentialSwellAmt;
    
public:
    string getName() const;
    void update(const ofMesh& mesh);
    ofMesh& getModifiedMesh();
    void drawBlends();
	void saveMeshes();
	void initialize();
	void draw(const ofTexture& texture);
};