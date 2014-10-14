#pragma once

#include "TopologyModifier.h"
#include "MeshUtils.h"

class PlusOne : public TopologyModifier {
protected:
    ofMesh extraMesh;
    
    ofMesh leftBaseMesh, rightBaseMesh;
    
    ofPolyline splitPath;
    ofPolyline extraLeftPath, extraRightPath;
    ofPolyline splitLeftPath, splitRightPath;
    
    ofMesh final;
	
	ofxButterfly    butterflySubdividerFinal;
	ofxButterfly    butterflySubdividerExtra;
    ofMesh          refinedMeshFinal;
	ofMesh          refinedMeshExtra;
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