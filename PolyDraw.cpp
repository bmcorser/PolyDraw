#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Navigation/Navigable.h>
#include <Urho3D/Navigation/NavigationMesh.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include "PolyDraw.h"
#include "ThirdPersonCamera.h"

#include <Urho3D/DebugNew.h>

URHO3D_DEFINE_APPLICATION_MAIN(Navigation)

Navigation::Navigation(Context* context) :
    Sample(context),
    drawDebug_(false)
{
    // UNCOMMENT TO EXPLOD
    ThirdPersonCamera::RegisterObject(context_);
}


void Navigation::Start()
{
    // Execute base class startup
    Sample::Start();

    // Create the scene content
    CreateScene();

    // Create the UI content
    CreateUI();

    // Setup the viewport for displaying the scene
    SetupViewport();

    // Hook up to the frame update and render post-update events
    SubscribeToEvents();
}

void Navigation::CreateScene()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);
    scene_->SetSmoothingConstant(1.0f);
    scene_->SetSnapThreshold(1.0f);
    /*
    */

    // Create octree, use default volume (-1000, -1000, -1000) to (1000, 1000, 1000)
    // Also create a DebugRenderer component so that we can draw debug geometry
    scene_->CreateComponent<Octree>();
    debugRenderer = scene_->CreateComponent<DebugRenderer>();

    // Create a Zone component for ambient lighting & fog control
    Node* zoneNode = scene_->CreateChild("Zone");
    Zone* zone = zoneNode->CreateComponent<Zone>();
    zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
    zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
    zone->SetFogStart(100.0f);
    zone->SetFogEnd(300.0f);

    // Create a directional light to the world. Enable cascaded shadows on it
    Node* lightNode = scene_->CreateChild("DirectionalLight");
    lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
    Light* light = lightNode->CreateComponent<Light>();
    light->SetLightType(LIGHT_DIRECTIONAL);
    light->SetCastShadows(true);
    light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
    // Set cascade splits at 10, 50 and 200 world units, fade shadows out at 80% of maximum shadow distance
    light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));

    orbitalCameraNode = scene_->CreateChild("CameraRoot");
    orbitalCamera = orbitalCameraNode->CreateComponent<ThirdPersonCamera>();

    /*
    */
    Color colours[17];
    colours[0] = Color(0, 31, 63);
    colours[1] = Color(0, 116, 217);
    colours[2] = Color(127, 219, 255);
    colours[3] = Color(57, 219, 255);
    colours[4] = Color(57, 204, 204);
    colours[5] = Color(61, 153, 112);
    colours[6] = Color(46, 204, 64);
    colours[7] = Color(1, 255, 112);
    colours[8] = Color(255, 220, 0);
    colours[9] = Color(255, 133, 27);
    colours[10] = Color(255, 65, 54);
    colours[11] = Color(133, 20, 75);
    colours[12] = Color(240, 18, 190);
    colours[13] = Color(177, 13, 201);
    colours[14] = Color(17, 17, 17);
    colours[15] = Color(170, 170, 170);
    colours[16] = Color(221, 221, 221);
    // Create bodies
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(1000, 100000);
    std::uniform_int_distribution<> colourChoice(0, 16);
    Vector3 pos;
    Vector<Vector3> created;

    // initial body (for camera target)
    pos = Vector3::ZERO;
    created.Push(pos);
    orbitalCamera->SetTargetNode(CreateMushroom(pos, colours[colourChoice(eng)]));

    // make a bunch of spread out bodies
    bool tooClose;
    int i = 0;
    while (i < 100) {
        pos += Vector3(float(distr(eng)), float(distr(eng)), float(distr(eng)));
        pos /= 1000;
        tooClose = false;
        for (int j=0; j < created.Size(); ++j) {
            if ((created[j] - pos).Length() < 10) {
                tooClose = true;
            }
        }
        if (tooClose == true)
            continue;
        Color colour = colours[colourChoice(eng)];
        created.Push(pos);
        CreateMushroom(pos, colour);
        ++i;
    }

    debugCameraNode = scene_->CreateChild("DebugCamera");
    debugCamera = debugCameraNode->CreateComponent<Camera>();
    debugCamera->SetFarClip(300.0f);
    debugCameraNode->SetPosition(Vector3(-40.0f, 50.0f, -20.0f));
    debugCameraNode->SetRotation(Quaternion(40.0f, 60.0f, 0.0f));

    Renderer* renderer = GetSubsystem<Renderer>();

    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    /*
    */
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, orbitalCamera->camera));
    SharedPtr<Viewport> debugViewport(new Viewport(context_, scene_, debugCameraNode->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
}

void Navigation::CreateUI()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();

    // Create a Cursor UI element because we want to be able to hide and show it at will. When hidden, the mouse cursor will
    // control the camera, and when visible, it will point the raycast target
    XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    SharedPtr<Cursor> cursor(new Cursor(context_));
    cursor->SetStyleAuto(style);
    ui->SetCursor(cursor);

    // Set starting position of the cursor at the rendering window center
    Graphics* graphics = GetSubsystem<Graphics>();
    cursor->SetPosition(graphics->GetWidth() / 2, graphics->GetHeight() / 2);

    /*
    // Construct new Text object, set string to display and font to use
    Text* instructionText = ui->GetRoot()->CreateChild<Text>();
    instructionText->SetText(
        "Click on plane to create new targets\n"
        "Click on targets to follow them\n"
        "Hold CTRL to rotate around target\n"
    );
    instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);
    // The text has multiple rows. Center them in relation to each other
    instructionText->SetTextAlignment(HA_CENTER);

    // Position the text relative to the screen center
    instructionText->SetHorizontalAlignment(HA_CENTER);
    instructionText->SetVerticalAlignment(VA_CENTER);
    instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
    */
}

void Navigation::SetupViewport()
{
}

void Navigation::SubscribeToEvents()
{
    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Navigation, HandleUpdate));

    // Subscribe HandlePostRenderUpdate() function for processing the post-render update event, during which we request
    // debug geometry
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(Navigation, HandlePostRenderUpdate));
}

void Navigation::MoveCamera(float timeStep)
{
    Input* input = GetSubsystem<Input>();
    UI* ui = GetSubsystem<UI>();

    const float MOVE_SPEED = 70.0f;
    const float MOUSE_SENSITIVITY = 0.1f;

    // Do not move if the UI has a focused element (the console)
    if (ui->GetFocusElement())
        return;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    // Only move the camera when the cursor is hidden
    if (!ui->GetCursor()->IsVisible())
    {
        // render ray for debugging
        IntVector2 pos = ui->GetCursorPosition();
        Graphics* graphics = GetSubsystem<Graphics>();

        // debugging camera
        IntVector2 mouseMove = input->GetMouseMove();
        yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
        pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
        pitch_ = Clamp(pitch_, -90.0f, 90.0f);
        debugCameraNode->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

        Ray cameraRay = orbitalCamera->camera->GetScreenRay((float)pos.x_ / graphics->GetWidth(), (float)pos.y_ / graphics->GetHeight());
        PODVector<RayQueryResult> results;
        RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, 250.0f, DRAWABLE_GEOMETRY);
        scene_->GetComponent<Octree>()->RaycastSingle(query);
        if (results.Size())
        {
            RayQueryResult& result = results[0];
            // debugRenderer->AddLine(result.position_, cameraRay.origin_, 1);
        }

    }
    if (ui->GetCursor()->IsVisible())
    {
        if (input->GetMouseButtonPress(MOUSEB_LEFT))
            if(input->GetQualifierDown(QUAL_SHIFT)) {
                ConnectBodies();
            } else {
                AddOrRemoveObject();
            }
    }

    // Read WASD keys and move the camera scene node to the corresponding di
    if (input->GetKeyDown('W'))
        debugCameraNode->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('S'))
        debugCameraNode->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('A'))
        debugCameraNode->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('D'))
        debugCameraNode->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);

}

void Navigation::ConnectBodies()
{
    // Raycast and check if we hit a mushroom node. If yes, remove it, if no, create a new one
    Vector3 hitPos;
    Drawable* hitDrawable;

    if (Raycast(250.0f, hitPos, hitDrawable))
    {
        Node* hitNode = hitDrawable->GetNode();
        if (hitNode->GetName() == "Mushroom")
        {
            connections_A.Push(orbitalCamera->target);
            connections_B.Push(hitNode->GetPosition());
        }
    }
}
void Navigation::AddOrRemoveObject()
{
    // Raycast and check if we hit a mushroom node. If yes, remove it, if no, create a new one
    Vector3 hitPos;
    Drawable* hitDrawable;

    if (Raycast(250.0f, hitPos, hitDrawable))
    {
        Node* hitNode = hitDrawable->GetNode();
        if (hitNode->GetName() == "Mushroom")
        {
            orbitalCamera->SetTargetNode(hitNode); // send event
        }
        else
        {
            // Node* newNode = CreateMushroom(hitPos);
        }
    }
}

Node* Navigation::CreateMushroom(const Vector3& pos, Color colour)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Node* mushroomNode = scene_->CreateChild("Mushroom");
    mushroomNode->SetPosition(pos);
    mushroomNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
    mushroomNode->SetScale(2.0f + Random(15));
    StaticModel* mushroomObject = mushroomNode->CreateComponent<StaticModel>();
    mushroomObject->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
    Material* material = new Material(context_);
    // ("Materials/DefaultMaterial.xml");
    URHO3D_LOGINFO(colour.ToVector4().ToString());
    if (material) {
        material->SetShaderParameter("MatDiffColor", colour.ToVector4() / Vector4(255, 255, 255, 1));
        material->SetShaderParameter("MatSpecColor", colour.ToVector4() / Vector4(255, 255, 255, 1));
        mushroomObject->SetMaterial(material);
    }
    mushroomObject->SetCastShadows(true);

    return mushroomNode;
}

bool Navigation::Raycast(float maxDistance, Vector3& hitPos, Drawable*& hitDrawable)
{
    hitDrawable = 0;

    UI* ui = GetSubsystem<UI>();
    IntVector2 pos = ui->GetCursorPosition();
    // Check the cursor is visible and there is no UI element in front of the cursor
    if (!ui->GetCursor()->IsVisible() || ui->GetElementAt(pos, true))
        return false;

    Graphics* graphics = GetSubsystem<Graphics>();
    Ray cameraRay = orbitalCamera->camera->GetScreenRay((float)pos.x_ / graphics->GetWidth(), (float)pos.y_ / graphics->GetHeight());
    // Pick only geometry objects, not eg. zones or lights, only get the first (closest) hit
    PODVector<RayQueryResult> results;
    RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, maxDistance, DRAWABLE_GEOMETRY);
    scene_->GetComponent<Octree>()->RaycastSingle(query);
    if (results.Size())
    {
        RayQueryResult& result = results[0];
        hitPos = result.position_;
        hitDrawable = result.drawable_;
        return true;
    }

    return false;
}

void Navigation::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    // Move the camera, scale movement with time step
    MoveCamera(timeStep);
}

void Navigation::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
    /*
    PODVector<Node*> children;
    scene_->GetChildren(children);
    if (children.Size() > 0)
    {
        for (unsigned i = 0; i < children.Size(); ++i)
            if(children[i]->GetComponent<StaticModel>())
                children[i]->GetComponent<StaticModel>()->DrawDebugGeometry(debugRenderer, true);
    }
    */
    if (connections_A.Size() > 0)
    {
        for (int i = 0; i < connections_A.Size(); ++i) {
            debugRenderer->AddLine(connections_A[i], connections_B[i], Color::MAGENTA);
        }
    }
}

