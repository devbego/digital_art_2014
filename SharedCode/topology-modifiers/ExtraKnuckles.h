#pragma once

#include "TopologyModifier.h"
#include "MeshUtils.h"

class ExtraKnuckles : public TopologyModifier {
protected:
    
    vector<pair<ofIndexType, ofIndexType> > stitchIndices;
    
    vector<ofMesh> fingerMeshes;
    
    vector<ofMesh> fingerBaseMeshes;
    vector<ofMesh> fingertipMeshes;
    vector<ofMesh> knuckleMeshes;
    
    vector<ofMesh> lowerGapMeshes;
    vector<ofMesh> upperGapMeshes;
    
    vector<ofMesh> blendMeshes;
    
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