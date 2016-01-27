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
    currentYaw = 0;
    targetYaw = 0;
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
    if (input->GetKeyDown('Q'))
        // Strafe container
        containerNode->SetPosition(centre.Lerp(target, timeStep));

    // Update class
    cameraPosition = cameraNode->GetWorldPosition();
    centre = containerNode->GetWorldPosition();

    camera->DrawDebugGeometry(debugRenderer, true);
    debugRenderer->AddLine(cameraPosition, target, Color::GREEN);
    debugRenderer->AddLine(cameraPosition, centre, Color::RED);
    /*
    */

    UI* ui = GetSubsystem<UI>();
    // Do not move if the UI has a focused element (the console)
    if (ui->GetFocusElement())
        return;

    const float MOUSE_SENSITIVITY = 0.1f;

    Cursor* cursor = ui->GetCursor();
    cursor->SetVisible(!input->GetQualifierDown(QUAL_CTRL));

    if (ui->GetCursor()->IsVisible() && false)
    {
        IntVector2 mouseMove = input->GetMouseMove();
        currentYaw += MOUSE_SENSITIVITY * mouseMove.x_;
        currentPitch += MOUSE_SENSITIVITY * mouseMove.y_;
        currentPitch = Clamp(currentPitch, -90.0f, 90.0f);

    }
    if (input->GetKeyDown('H'))
        currentYaw += 0.5;
        yawNode->SetRotation(Quaternion(currentYaw, Vector3::UP));
        currentYaw = yawNode->GetRotation().YawAngle();
    if (input->GetKeyDown('L'))
        currentYaw -= 0.5;
        yawNode->SetRotation(Quaternion(currentYaw, Vector3::UP));
        currentYaw = yawNode->GetRotation().YawAngle();


    Vector3 diff = target - centre;
    if (diff.Length() > .7f && false) {
        Quaternion targetAngle = Quaternion(cameraPosition.DotProduct(target), cameraPosition.CrossProduct(target));
        Quaternion slerpedTargetAngle = yawNode->GetRotation().Slerp(targetAngle, timeStep);
        yawNode->SetRotation(slerpedTargetAngle);
        currentYaw = slerpedTargetAngle.YawAngle();
    }

    debugRenderer->AddNode(yawNode, 3);
    // debugRenderer->AddNode(pitchNode);
    debugRenderer->AddNode(cameraNode, 1);
    debugRenderer->AddNode(balanceNode, 1);
    /*
    float wheel = input->GetMouseMoveWheel();
    radius += wheel * k;
    if (radius < minRadius) radius = minRadius;
    if (radius > maxRadius) radius = maxRadius;
    cameraNode->SetPosition(0, radius, 0);
    balanceNode->SetPosition(0, -radius, 0);
    */
    // currentYaw = yawNode->GetRotation().YawAngle();
    currentWorldYaw = Quaternion(cameraPosition, centre).YawAngle();
    Quaternion targetYawQuat = Quaternion::IDENTITY;
    targetYawQuat.FromLookRotation(target);
    targetYaw = targetYawQuat.YawAngle();
    URHO3D_LOGINFO("centre vec: " + centre.ToString());
    URHO3D_LOGINFO("target vec: " + target.ToString());
    std::cout << "centre yaw: " << currentWorldYaw << std::endl;
    std::cout << "yaw: " << currentYaw << std::endl;
    std::cout << "target yaw: " << targetYaw << std::endl;
    debugRenderer->AddLine(
        Quaternion(currentYaw, Vector3::LEFT) * Vector3(0, 10, 0), Vector3::ZERO, Color::GREEN);
    debugRenderer->AddLine(
        Quaternion(currentWorldYaw, Vector3::LEFT) * Vector3(0, 10, 0), Vector3::ZERO, Color::YELLOW);
    debugRenderer->AddLine(
        Quaternion(targetYaw, Vector3::LEFT) * Vector3(0, 10, 0), Vector3::ZERO, Color::MAGENTA);
}

void ThirdPersonCamera::SetTargetNode(Node* node) 
{
    target = node->GetPosition();
    // yawNode->SetRotation(Quaternion(0, currentYaw, 0));
}

void ThirdPersonCamera::SetRadiusLimits(float min, float max)
{
    minRadius = min;
    maxRadius = max;
}
