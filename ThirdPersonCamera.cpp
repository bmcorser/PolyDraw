#include "Common.h"
#include "ThirdPersonCamera.h"
#include <Urho3D/DebugNew.h>



ThirdPersonCamera::ThirdPersonCamera(Context* context) : LogicComponent(context)
{
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    // SetUpdateEventMask(USE_FIXEDUPDATE | USE_UPDATE);
}

void ThirdPersonCamera::RegisterObject(Context* context)
{
    context->RegisterFactory<ThirdPersonCamera>();
}

void ThirdPersonCamera::Start()
{
    Node* node = GetNode();
    Scene* scene = GetScene();
    debugRenderer = scene->GetComponent<DebugRenderer>();

    minRadius = 10;
    maxRadius = 20;
    currentPitch = 30;
    currentYaw = 45;
    radius = 50;

    containerNode = node->CreateChild("OrbitalCameraContainer");
    targetTransform = containerNode->CreateComponent<SmoothedTransform>();

        yawNode = containerNode->CreateChild("OrbitalCameraYaw");
        yawNode->SetRotation(Quaternion(0, currentYaw, 0));
        yawTransform = yawNode->CreateComponent<SmoothedTransform>();

            pitchNode = yawNode->CreateChild("OrbitalCameraPitch");
            pitchNode->SetRotation(Quaternion(currentPitch, 0, 0));
            pitchTransform = pitchNode->CreateComponent<SmoothedTransform>();

                cameraNode = pitchNode->CreateChild("OrbitalCamera");
                cameraNode->SetPosition(Vector3(0, 0, -radius));
                cameraNode->SetRotation(Quaternion(0, 0, 0));

                    camera = cameraNode->CreateComponent<Camera>();  // public

                balanceNode = pitchNode->CreateChild("OrbitalCameraBalance");
                balanceNode->SetPosition(Vector3(0, 0, radius));


}

void ThirdPersonCamera::Update(float timeStep)
{
    targetTransform->Update(0.1, 0.1);
    UI* ui = GetSubsystem<UI>();
    camera->DrawDebugGeometry(debugRenderer, true);
    Vector3 cameraPosition = cameraNode->GetPosition();
    Vector3 targetPosition = containerNode->GetPosition();
    debugRenderer->AddLine(cameraPosition, targetPosition, 1);

    // Do not move if the UI has a focused element (the console)
    if (ui->GetFocusElement())
        return;

    Input* input = GetSubsystem<Input>();

    const float MOUSE_SENSITIVITY = 0.1f;

    Cursor* cursor = ui->GetCursor();
    cursor->SetVisible(!input->GetQualifierDown(QUAL_CTRL));

    if (!ui->GetCursor()->IsVisible() && !targetTransform->IsInProgress())
    {
        IntVector2 mouseMove = input->GetMouseMove();
        currentYaw += MOUSE_SENSITIVITY * mouseMove.x_;
        currentPitch += MOUSE_SENSITIVITY * mouseMove.y_;
        currentPitch = Clamp(currentPitch, -90.0f, 90.0f);

        yawNode->SetRotation(Quaternion(0, currentYaw, 0));
        pitchNode->SetRotation(Quaternion(currentPitch, 0, 0));
    }

    /*
    float wheel = input->GetMouseMoveWheel();
    radius += wheel * k;
    if (radius < minRadius) radius = minRadius;
    if (radius > maxRadius) radius = maxRadius;
    cameraNode->SetPosition(0, radius, 0);
    balanceNode->SetPosition(0, -radius, 0);
    */

}

void ThirdPersonCamera::SetTargetNode(Node* node) 
{
    URHO3D_LOGINFO("delta: " + (target - node->GetPosition()).ToString());
    target = node->GetPosition();
    targetTransform->SetTargetPosition(target);
    /*
    Vector3 diff = target - pos;
    Quaternion newRotation;
    newRotation.FromLookRotation(diff, Vector3::UP);
    targetTransform->SetTargetRotation(newRotation);
    */
}

void ThirdPersonCamera::SetRadiusLimits(float min, float max)
{
    minRadius = min;
    maxRadius = max;
}
