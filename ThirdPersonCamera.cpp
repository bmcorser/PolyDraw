#include "Common.h"
#include "ThirdPersonCamera.h"


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
    wheel_ = 0;
    minFollow_ = 2;
    maxFollow_ = 5;
    pitch_ = 30;
    yaw_ = 45;
    follow_ = 2;
    curFollow_ = follow_;
    followVel_ = 0;
    pos = Vector3::ZERO;
    newPos = Vector3::ZERO;
    posVelocity = Vector3::ZERO;

    Node* root_ = GetScene()->GetChild("Camera");
    angleNode_ = root_->CreateChild("angleNode");
    cameraNode_ = angleNode_->CreateChild("cameraNode");
    camera_ = cameraNode_->GetComponent<Camera>();

    GetNode()->SetRotation(Quaternion(yaw_, Vector3(0,1,0)));
    cameraNode_->SetPosition(Vector3(0,0, -follow_));
    angleNode_->SetRotation(Quaternion(pitch_, Vector3(1,0,0)));
    GetNode()->SetPosition(Vector3::ZERO);

    target_ = GetScene()->GetChild("Jack", true);

}

void ThirdPersonCamera::Update(float timeStep)
{
    Input* input = GetSubsystem<Input>();
    const float MOVE_SPEED = 800.0f;
    const float MOUSE_SENSITIVITY = 0.1f;
    IntVector2 mouseMove = input->GetMouseMove();

    yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
    pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
    pitch_ = Clamp(pitch_, -90.0f, 90.0f);

    if (wheel_) follow_ -= wheel_ * timeStep * 100;

    if (follow_ < minFollow_) follow_ = minFollow_;
    if (follow_ > maxFollow_) follow_ = maxFollow_;


    static bool hasObstacle = false;

    SpringFollow(timeStep);
    curFollow_ = CameraTestObstacles(curFollow_, hasObstacle);
    if (hasObstacle) 
    {
        follow_ = curFollow_;
        hasObstacle = false;
    }

    SpringPosition(timeStep);
    GetNode()->SetPosition(pos);

    GetNode()->SetRotation(Quaternion(yaw_, Vector3::UP));
    cameraNode_->SetPosition(Vector3(0.0f, 0.0f, -curFollow_));
    angleNode_->SetRotation(Quaternion(pitch_, Vector3::RIGHT));
}

float ThirdPersonCamera::CameraTestObstacles ( float followDistance, bool& hasObstacle )
{
    hasObstacle = false;
    Quaternion cd = cameraNode_->GetWorldRotation();
    Vector3 dir = cameraNode_->GetWorldPosition() - target_->GetWorldPosition();
    dir.Normalize();

    //Ray fromTargetToCam = Ray(target_->GetWorldPosition(), cd*-Vector3::ONE );
    Ray fromTargetToCam = Ray(target_->GetWorldPosition(), dir);


    PODVector<RayQueryResult> results;
    RayOctreeQuery query(results, fromTargetToCam, RAY_TRIANGLE, followDistance, DRAWABLE_GEOMETRY, -1);

    Octree* octree = GetNode()->GetScene()->GetComponent<Octree>();
    octree->RaycastSingle(query);

    if (results.Size())
    {
        RayQueryResult& result = results[0];

        Variant v = result.node_->GetVar("fx");
        if (v.GetInt() == 1) // if this some fx not clip follow 
        {
    
        }
        else 
        {
            if (result.distance_ >= 0) 
            {
                hasObstacle = true;
                return Min(result.distance_, followDistance)-0.5f;
            }
        }
    }
    return followDistance;
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

void ThirdPersonCamera::FixedUpdate(float timeStep)
{
    newPos = target_->GetWorldPosition();
    Input* input = GetSubsystem<Input>();

    wheel_ = input->GetMouseMoveWheel();

    static bool isAlredyR = false;
    if (input->GetKeyDown('R')) 
    {
        if (isAlredyR == false) 
        {
            isAlredyR = true;   
        }
    }
    else 
    {
        isAlredyR = false;
    }

    static bool isAlredyT = false;
    if (input->GetKeyDown('T')) 
    {
        if (isAlredyT == false) 
        {
            isAlredyT = true;   
        }
    }
    else 
    {
        isAlredyT = false;
    }

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
