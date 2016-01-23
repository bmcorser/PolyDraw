#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/SmoothedTransform.h>

class ThirdPersonCamera: public LogicComponent
{

URHO3D_OBJECT(ThirdPersonCamera, LogicComponent);

public:
    /// Construct.
    ThirdPersonCamera(Context* context);

    /// Register object factory and attributes.
    static void RegisterObject(Context* context);

    virtual void Start();
    void Update(float timeStep);
    virtual void FixedUpdate(float timeStep) {};

    void SetTargetNode(Node* proposedTarget);
    void SetRadiusLimits(float minDistance, float maxDistance);

    SharedPtr<Camera> camera;

private:

    SharedPtr<Node> containerNode;
    SharedPtr<Node> yawNode;
    SharedPtr<Node> pitchNode;
    SharedPtr<Node> cameraNode;
    SharedPtr<Node> balanceNode;

    SharedPtr<SmoothedTransform> targetTransform;
    SharedPtr<SmoothedTransform> pitchTransform;
    SharedPtr<SmoothedTransform> yawTransform;

    SharedPtr<DebugRenderer> debugRenderer;

    float minRadius;
    float maxRadius;
    float radius;

    float currentPitch;
    float currentYaw;

    Vector3 target;
    Vector3 position;

};
