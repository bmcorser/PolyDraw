#include "CelestialBody.h"

CelestialBody::CelestialBody(Context* context) : LogicComponent(context)
{
    SetUpdateEventMask(USE_FIXEDUPDATE | USE_UPDATE);
}

void CelestialBody::RegisterObject(Context* context)
{
    context->RegisterFactory<CelestialBody>();
}

void CelestialBody::Start()
{
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
    rootNode = GetNode();
    scene = GetScene();
    debugRenderer = scene->GetComponent<DebugRenderer>();
}

void CelestialBody::Update(float timeStep)
{
}

Node* CelestialBody::Create(Vector3 pos) // CelestialBodyPreset::PLAIN
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Node* mushroomNode = scene->CreateChild("Mushroom");
    mushroomNode->SetPosition(pos);
    mushroomNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));
    mushroomNode->SetScale(2.0f + Random(15));
    StaticModel* mushroomObject = mushroomNode->CreateComponent<StaticModel>();
    Model* mushroomModel = cache->GetResource<Model>("Models/Sphere.mdl");
    mushroomObject->SetModel(mushroomModel);
    VertexBuffer* buffer = mushroomModel->GetGeometry(0, 0)->GetVertexBuffer(0);
    unsigned char* vertexData = (unsigned char*)buffer->Lock(0, buffer->GetVertexCount());
    unsigned numVertices = buffer->GetVertexCount();
    unsigned vertexSize = buffer->GetVertexSize();
    PODVector<Vector3> vertices;
    for (unsigned i = 0; i < 1 /* numVertices */; ++i)
    {
        Vector3& vertex = *reinterpret_cast<Vector3*>(vertexData + i * vertexSize);
        // vertex.x_ += 3;
    }
    buffer->Unlock();
    Material* material = new Material(context_);
    // ("Materials/DefaultMaterial.xml");
    if (material) {
        material->SetShaderParameter("MatDiffColor", colours[0].ToVector4() / Vector4(255, 255, 255, 1));
        material->SetShaderParameter("MatSpecColor", colours[0].ToVector4() / Vector4(255, 255, 255, 1));
        mushroomObject->SetMaterial(material);
    }
    mushroomObject->SetCastShadows(true);

    return mushroomNode;
}
