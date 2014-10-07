#pragma once

#include "HandMeshBuilder.h"
#include "TopologyModifier.h"

class TopologyModifierManager {
private:
    TopologyModifier* curScene;
    vector<TopologyModifier*> scenes;
    
public:
    void setup();
    void setScene(int sceneIndex);
    int getSceneCount() const;
    void update(HandMeshBuilder& handMeshBuilder);
    void draw(const ofTexture& texture);
};