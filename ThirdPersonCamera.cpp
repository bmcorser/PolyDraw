#include "Common.h"
#include "ThirdPersonCamera.h"


ThirdPersonCamera::ThirdPersonCamera(Context* context) : LogicComponent(context)
{
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_FIXEDUPDATE | USE_UPDATE);
}

void ThirdPersonCamera::RegisterObject(Context* context)
{
    context->RegisterFactory<ThirdPersonCamera>();
}

void ThirdPersonCamera::Start()
{
    Scene* scene = GetScene();
    debugRenderer = scene->GetComponent<DebugRenderer>();

    wheel_ = 0;
    minFollow_ = 10;
    maxFollow_ = 20;
    pitch_ = 30;
    yaw_ = 45;
    follow_ = 50;
    curFollow_ = follow_;
    followVel_ = 0;
    pos = Vector3::ZERO;
    newPos = Vector3::ZERO;
    posVelocity = Vector3::ZERO;

    Node* cameraRootNode = scene->GetChild("CameraRoot", true);
    cameraAngleNode = cameraRootNode->GetChild("CameraAngle", true);
    cameraNode = cameraAngleNode->GetChild("Camera", true);
    camera = cameraNode->GetComponent<Camera>();

    cameraRootNode->SetRotation(Quaternion(yaw_, Vector3(0,1,0)));
    cameraNode->SetPosition(Vector3(0,0, -follow_));
    cameraAngleNode->SetRotation(Quaternion(pitch_, Vector3(1,0,0)));
    cameraRootNode->SetPosition(Vector3::ZERO);

    target_ = scene->GetChild("Jack");

}

void ThirdPersonCamera::Update(float timeStep)
{
    UI* ui = GetSubsystem<UI>();
    camera->DrawDebugGeometry(debugRenderer, true);

    // Do not move if the UI has a focused element (the console)
    if (ui->GetFocusElement())
        return;

    Input* input = GetSubsystem<Input>();

    newPos = target_->GetWorldPosition();
    const float MOUSE_SENSITIVITY = 0.1f;

    ui->GetCursor()->SetVisible(!input->GetQualifierDown(QUAL_CTRL));

    if (!ui->GetCursor()->IsVisible())
    {
        wheel_ = input->GetMouseMoveWheel();
        IntVector2 mouseMove = input->GetMouseMove();
        yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
        pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
        pitch_ = Clamp(pitch_, -90.0f, 90.0f);

        if (wheel_) follow_ -= wheel_ * timeStep * 100;
    }

    if (follow_ < minFollow_) follow_ = minFollow_;
    if (follow_ > maxFollow_) follow_ = maxFollow_;


    SpringFollow(timeStep);
    SpringPosition(timeStep);
    GetNode()->SetPosition(pos);

    GetNode()->SetRotation(Quaternion(yaw_, Vector3::UP));
    cameraNode->SetPosition(Vector3(0.0f, 0.0f, -curFollow_));
    cameraAngleNode->SetRotation(Quaternion(pitch_, Vector3::RIGHT));
}

void ThirdPersonCamera::SpringFollow(float timeStep) 
{
    float deltaFollow = follow_ - curFollow_;
    float af = 28 * deltaFollow - 22 * followVel_;

    followVel_ = followVel_ + timeStep * af;
    curFollow_ = curFollow_ + timeStep * followVel_;
}

void ThirdPersonCamera::SpringPosition(float timeStep) 
{
    Vector3 d = newPos - pos;
    Vector3 a = d * Vector3(8,8,8) - posVelocity * Vector3(5,5,5);
    posVelocity = posVelocity + a * Vector3(timeStep, timeStep, timeStep);
    pos = pos + posVelocity * Vector3(timeStep, timeStep, timeStep);
}

void ThirdPersonCamera::SetTargetNode(Node* target) 
{
    target_ = target;
}

void ThirdPersonCamera::SetMinMaxDistance(float minDistance, float maxDistance)
{
    minFollow_ = minDistance;
    maxFollow_ = maxDistance;
}
