#pragma once

#include "Common.h"

using namespace Urho3D;

class CelestialBody: public LogicComponent
{

URHO3D_OBJECT(CelestialBody, LogicComponent);

public:
    /// Construct.
    CelestialBody(Context* context);

    /// Register object factory and attributes.
    static void RegisterObject(Context* context);
    virtual void Start();
    void Update(float timeStep);
    virtual void FixedUpdate(float timeStep) {};

    Node* Create(Vector3 pos);

private:

    SharedPtr<DebugRenderer> debugRenderer;
    SharedPtr<Node> rootNode;
    SharedPtr<Scene> scene;
    Color colours[17];

};
