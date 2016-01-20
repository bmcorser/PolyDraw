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
    wheel_ = 0;
    minFollow_ = 2;
    maxFollow_ = 5;
    pitch_ = 30;
    yaw_ = 45;
    follow_ = 10;
    curFollow_ = follow_;
    followVel_ = 0;
    pos = Vector3::ZERO;
    newPos = Vector3::ZERO;
    posVelocity = Vector3::ZERO;

    Node* cameraRootNode = GetScene()->GetChild("CameraRoot");
    cameraAngleNode = cameraRootNode->GetChild("CameraAngle");
    cameraNode = cameraAngleNode->GetChild("Camera");
    camera = cameraNode->GetComponent<Camera>();

    cameraRootNode->SetRotation(Quaternion(yaw_, Vector3(0,1,0)));
    cameraNode->SetPosition(Vector3(0,0, -follow_));
    cameraAngleNode->SetRotation(Quaternion(pitch_, Vector3(1,0,0)));
    cameraRootNode->SetPosition(Vector3::ZERO);

    target_ = GetScene()->GetChild("Jack");

}

void ThirdPersonCamera::Update(float timeStep)
{
    Input* input = GetSubsystem<Input>();
    UI* ui = GetSubsystem<UI>();

    ui->GetCursor()->SetVisible(!input->GetQualifierDown(QUAL_CTRL));
    // Do not move if the UI has a focused element (the console)
    if (ui->GetFocusElement())
        return;
    if (!ui->GetCursor()->IsVisible())
    {
        wheel_ = input->GetMouseMoveWheel();
        newPos = target_->GetWorldPosition();
        const float MOVE_SPEED = 800.0f;
        const float MOUSE_SENSITIVITY = 0.1f;
        IntVector2 mouseMove = input->GetMouseMove();

        yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
        pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
        pitch_ = Clamp(pitch_, -90.0f, 90.0f);

        if (wheel_) follow_ -= wheel_ * timeStep * 100;

        if (follow_ < minFollow_) follow_ = minFollow_;
        if (follow_ > maxFollow_) follow_ = maxFollow_;


        SpringFollow(timeStep);
        SpringPosition(timeStep);
        GetNode()->SetPosition(pos);

        GetNode()->SetRotation(Quaternion(yaw_, Vector3::UP));
        cameraNode->SetPosition(Vector3(0.0f, 0.0f, -curFollow_));
        cameraAngleNode->SetRotation(Quaternion(pitch_, Vector3::RIGHT));
    }
}

void ThirdPersonCamera::SpringFollow(float timeStep) 
{
    float deltaFollow = follow_ - curFollow_;
    //float af = 9*deltaFollow - 6 * followVel_;
    float af = 18 * deltaFollow - 12 * followVel_;

    followVel_ = followVel_ + timeStep* af;
    curFollow_ = curFollow_ + timeStep * followVel_;
}

void ThirdPersonCamera::SpringPosition(float timeStep) 
{
    Vector3 d = newPos - pos;
    Vector3 a = d * Vector3(8,8,8) - posVelocity * Vector3(5,5,5);
    //Vector3 a = d * Vector3(8, 8, 8) - posVelocity * Vector3(10, 10, 10);
    posVelocity = posVelocity + a * Vector3(timeStep, timeStep, timeStep);
    pos = pos + posVelocity * Vector3(timeStep, timeStep, timeStep);
}

void ThirdPersonCamera::FixedUpdate(float timeStep) {}


void ThirdPersonCamera::SetTargetNode(Node* target) 
{
    target_ = target;
}

void ThirdPersonCamera::SetMinMaxDistance(float minDistance, float maxDistance)
{
    minFollow_ = minDistance;
    maxFollow_ = maxDistance;
}
