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
    
public:
    string getName() const;
    void update(const ofMesh& mesh);
    ofMesh& getModifiedMesh();
    void drawBlends();
};