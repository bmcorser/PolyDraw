#pragma once

#include <Urho3D/Scene/LogicComponent.h>

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

    void SetTargetNode(Node* target);
    void SetMinMaxDistance(float minDistance, float maxDistance);

    SharedPtr<Node> cameraNode;

private:

    SharedPtr<Node> target_;
    SharedPtr<Node> cameraAngleNode;

    SharedPtr<Camera> camera;
    SharedPtr<DebugRenderer> debugRenderer;

    int wheel_;
    float lastCollisionTime;

    float minFollow_;
    float maxFollow_;

    float pitch_;
    float yaw_;
    float follow_;
    float curFollow_;
    float followVel_;
    Vector3 pos;
    Vector3 newPos;
    Vector3 posVelocity;

    void SpringFollow(float timeStep);
    void SpringPosition(float timeStep);
    float CameraTestObstacles(float followDistance, bool& hasObstacle);

};
