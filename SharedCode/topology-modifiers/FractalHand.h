// Experimental fractal hand scene!
// work in progress

#pragma once

#include "TopologyModifier.h"
#include "MeshUtils.h"

class FractalHand : public TopologyModifier {
protected:
    
    vector<pair<ofIndexType, ofIndexType> > stitchIndices;
    
    vector<ofMesh> fingerMeshes;
    vector<ofMesh> fingertipMeshes;
    vector<ofMesh> miniHandMeshes;
    
    ofMesh final;
	
	ofxButterfly    butterflySubdivider;
    ofMesh          refinedMesh;
	bool			bUseButterfly;
    
    int sourceBlendOpIndex;
    int destBlendOpIndex;
    
public:
    string getName() const;
    void update(const ofMesh& mesh);
    ofMesh& getModifiedMesh();
    void drawBlends();
	void saveMeshes();
	void initialize();
	void draw(const ofTexture& texture);
};