#include "Common.h"
#include "ThirdPersonCamera.h"
#include <Urho3D/DebugNew.h>



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
    Node* node = GetNode();
    Scene* scene = GetScene();
    debugRenderer = scene->GetComponent<DebugRenderer>();

    minRadius = 10;
    maxRadius = 20;
    currentPitch = 30;
    currentYaw = 40;
    radius = 50;
    target = Vector3::ZERO;

    containerNode = node->CreateChild("OrbitalCameraContainer");
    debugRenderer->AddNode(containerNode);
    targetTransform = containerNode->CreateComponent<SmoothedTransform>();

        yawNode = containerNode->CreateChild("OrbitalCameraYaw");
        yawNode->SetRotation(Quaternion(currentYaw, Vector3::UP));
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
    Input* input = GetSubsystem<Input>();
    containerNode->SetPosition(centre.Lerp(target, timeStep));

    // Update class
    cameraPosition = cameraNode->GetWorldPosition();
    centre = containerNode->GetWorldPosition();

    /* can't see this if we're not using the debugViewport
    camera->DrawDebugGeometry(debugRenderer, true);
    debugRenderer->AddLine(cameraPosition, target, Color::GREEN);
    debugRenderer->AddLine(cameraPosition, centre, Color::RED);
    */

    UI* ui = GetSubsystem<UI>();
    if (ui->GetFocusElement())
        return;

    const float MOUSE_SENSITIVITY = 0.1f;

    Cursor* cursor = ui->GetCursor();
    cursor->SetVisible(!input->GetQualifierDown(QUAL_CTRL));

    if (!ui->GetCursor()->IsVisible())
    {
        IntVector2 mouseMove = input->GetMouseMove();
        currentYaw += MOUSE_SENSITIVITY * mouseMove.x_;
        currentPitch += MOUSE_SENSITIVITY * mouseMove.y_;
        currentPitch = Clamp(currentPitch, -90.0f, 90.0f);
        yawNode->SetRotation(Quaternion(currentYaw, Vector3::UP));
        pitchNode->SetRotation(Quaternion(currentPitch, Vector3::RIGHT));
    }
}

void ThirdPersonCamera::SetTargetNode(Node* node) 
{
    target = node->GetPosition();
}
