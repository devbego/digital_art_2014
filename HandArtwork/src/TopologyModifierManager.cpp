#include "TopologyModifierManager.h"

#include "PlusOne.h"
#include "MinusOne.h"

void TopologyModifierManager::setup() {
    scenes.push_back(new PlusOne());
    scenes.push_back(new MinusOne());
    curScene = scenes.front();
}
void TopologyModifierManager::setScene(int sceneIndex) {
    curScene = scenes[sceneIndex];
}
int TopologyModifierManager::getSceneCount() const {
    return scenes.size();
}
void TopologyModifierManager::update(HandMeshBuilder& handMeshBuilder) {
    ofMesh& mesh = handMeshBuilder.getMesh();
    curScene->update(mesh);
}
void TopologyModifierManager::draw(const ofTexture& texture) {
    curScene->draw(texture);
}