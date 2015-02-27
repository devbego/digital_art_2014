#pragma once

#include "TopologyModifier.h"
#include "MeshUtils.h"

class OppositeThumb : public TopologyModifier {
protected:
    
    ofPolyline removalRegion;
    vector<pair<ofIndexType, ofIndexType> > stitchIndices;
    
    ofMesh origThumbMesh;
    ofMesh thumbMesh;
    ofMesh gapFillMesh;
    
    ofMesh extraThumbBlendMesh;
    ofMesh replacementEdgeMesh;
    
    ofMesh final;
	
	ofxButterfly    butterflySubdivider;
    ofMesh          refinedMesh;
	bool			bUseButterfly;
    
    float edgeStretch;
    
    bool removeThumb;
    
public:
    string getName() const;
    void update(const ofMesh& mesh);
    ofMesh& getModifiedMesh();
    void drawBlends();
	void saveMeshes();
	void initialize();
	void draw(const ofTexture& texture);
};