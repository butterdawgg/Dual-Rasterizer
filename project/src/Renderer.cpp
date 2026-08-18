//External includes
#include "SDL.h"
#include "SDL_surface.h"
#include "SDL_syswm.h"

//Project includes
#include "Renderer.h"
#include "Maths.h"

#include <iostream>

using namespace dae;



Renderer::Renderer(SDL_Window* pWindow) :
    m_pWindow(pWindow)
{
    //Initialize
    SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

    //Create Buffers
    m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
    m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
    m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

    const int depthBufferSize { m_Width * m_Height };
    m_pDepthBufferPixels = new float[depthBufferSize];
    std::fill_n(m_pDepthBufferPixels, depthBufferSize, FLT_MAX);

    // Create camera
    m_Camera = std::make_unique<Camera>(
        Vector3 { 0.0f, 0.0f, 0.0f },
        Vector3 { 0.0f, 0.0f, 0.0f },
        45.0f,
        static_cast<float>(m_Width) / m_Height,
        0.1f,
        100.0f
    );

    // Create meshes
    Mesh vehicleMesh { "resources/vehicle.obj", 0 };
    vehicleMesh.SetPosition({ 0.0f, 0.0f, 50.0f });
    vehicleMesh.SetTransparent(false);

    Mesh fireFxMesh { "resources/fireFX.obj", 1 };
    fireFxMesh.SetPosition({ 0.0f, 0.0f, 50.0f });
    fireFxMesh.SetTransparent(true);

    m_Meshes.push_back(std::move(vehicleMesh));
    m_Meshes.push_back(std::move(fireFxMesh));

    // Create lights
    Light mainLight { Vector3 { .577f, -.577f, .577f }, colors::White, 1.0f };

    m_Lights.push_back(std::move(mainLight));

    // Create materials
    Material vehicleMat {
        ColorRGB { 1.0f, 1.0f, 1.0f },
        ColorRGB { 1.0f, 1.0f, 1.0f },
        ColorRGB { .03f, .03f, .03f },
        7.0f,
        1.0f,
        25.0f,
        Texture::LoadFromFile("resources/vehicle_diffuse.png"),
        Texture::LoadFromFile("resources/vehicle_specular.png"),
        Texture::LoadFromFile("resources/vehicle_gloss.png"),
        Texture::LoadFromFile("resources/vehicle_normal.png")
    };

    Material fireFxMat { };
    fireFxMat.diffuseColor = colors::White;
    fireFxMat.diffuseColorMap.reset(Texture::LoadFromFile("resources/fireFX_diffuse.png"));

    m_Materials.push_back(std::move(vehicleMat));
    m_Materials.push_back(std::move(fireFxMat));

    // Initialize DirectX
    HRESULT dxInitResult { InitializeDX() };

    if (dxInitResult == S_FALSE)
        return;

    m_InitializedDX = true;

    InitializeMeshesDX();
    InitializeMaterialsDX();
}

Renderer::~Renderer()
{
    delete[] m_pDepthBufferPixels;

    CleanupDX();
}

void Renderer::Update(Timer* pTimer)
{
    m_Camera->Update(pTimer);

    if (!m_RotateMesh)
        return;

    const size_t meshesNum { 2 };
    const size_t meshesToRotate[meshesNum] { 0, 1 };

    const float deltaTime { pTimer->GetElapsed() };
    const float rotationSpeed { 0.785398f };

    for (size_t i { 0 }; i < meshesNum; i++)
    {
        size_t id { meshesToRotate[i] };

        if (id > m_Meshes.size())
            continue;

        Mesh& mesh { m_Meshes[id] };

        mesh.SetRotation(mesh.GetRotation() +
            Vector3 { 0.0f, rotationSpeed * deltaTime, 0.0f });
    }
}

void Renderer::Render()
{
    if (m_DXRenderingEnabled)
    {
        RenderDX();
    }
    else
    {
        RenderSoftware();
    }
}



void Renderer::ToggleRenderingBackend(SDL_Scancode key)
{
    if (key != SDL_SCANCODE_F1)
        return;

    m_DXRenderingEnabled = !m_DXRenderingEnabled;

    if (!m_UniformClearColor)
        m_ClearColor = m_DXRenderingEnabled ? colors::ConrflowerBlue : colors::LightGray;

    std::cout << "[Uniform] Switched rendering backend to: " <<
        (m_DXRenderingEnabled ? "DirectX" : "Software") << "\n";
}

void Renderer::ToggleMeshRotation(SDL_Scancode key)
{
    if (key != SDL_SCANCODE_F2)
        return;

    m_RotateMesh = !m_RotateMesh;

    std::cout << "[Uniform] Switched mesh rotation to: " <<
        (m_RotateMesh ? "enabled" : "disabled") << "\n";
}

void Renderer::ToggleUniformClearColor(SDL_Scancode key)
{
    if (key != SDL_SCANCODE_F10)
        return;

    m_UniformClearColor = !m_UniformClearColor;

    if (m_UniformClearColor)
        m_ClearColor = colors::DarkGray;
    else
        m_ClearColor = m_DXRenderingEnabled ? colors::ConrflowerBlue : colors::LightGray;

    std::cout << "[Uniform] Switched uniform clear color to: " <<
        (m_UniformClearColor ? "enabled" : "disabled") << "\n";
}



void Renderer::ToggleMeshRendering(SDL_Scancode key)
{
    if (key != SDL_SCANCODE_F3)
        return;

    const size_t meshesNum { 1 };
    const size_t meshesToToggle[meshesNum] { 1 };
    const char* meshText { "FireFX" };

    for (size_t i { 0 }; i < meshesNum; i++)
    {
        size_t id { meshesToToggle[i] };

        if (id > m_Meshes.size())
            continue;

        Mesh& mesh { m_Meshes[id] };

        mesh.SetForceRenderingOff(!mesh.GetForceRenderingOff());
    }

    std::cout << "[Uniform] Toggled rendering for meshes: " << meshText << "\n";
}

void Renderer::ToggleSamplingMode(SDL_Scancode key)
{
    if (key != SDL_SCANCODE_F4)
        return;

    switch (m_SamplingMode)
    {
        case SamplingMode::Point:
            m_SamplingMode = SamplingMode::Linear;
            std::cout << "[DirectX only] Switched texture sampling mode to: Linear\n";
            break;
        case SamplingMode::Linear:
            m_SamplingMode = SamplingMode::Anisotropic;
            std::cout << "[DirectX only] Switched texture sampling mode to: Anisotropic\n";
            break;
        case SamplingMode::Anisotropic:
            m_SamplingMode = SamplingMode::Point;
            std::cout << "[DirectX only] Switched texture sampling mode to: Point\n";
            break;
        default:
            break;
    }
}



void Renderer::ToggleRenderMode(SDL_Scancode key)
{
    if (key != SDL_SCANCODE_F5)
        return;

    switch (m_RenderMode)
    {
        case RenderMode::ObservedArea:
            m_RenderMode = RenderMode::DiffuseOnly;
            std::cout << "[Software only] Switched render mode to: DiffuseOnly\n";
            break;
        case RenderMode::DiffuseOnly:
            m_RenderMode = RenderMode::SpecularOnly;
            std::cout << "[Software only] Switched render mode to: SpecularOnly\n";
            break;
        case RenderMode::SpecularOnly:
            m_RenderMode = RenderMode::Combined;
            std::cout << "[Software only] Switched render mode to: Combined\n";
            break;
        case RenderMode::Combined:
            m_RenderMode = RenderMode::ObservedArea;
            std::cout << "[Software only] Switched render mode to: ObservedArea\n";
            break;
        default:
            break;
    }
}

void Renderer::ToggleNormalMapping(SDL_Scancode key)
{
    if (key != SDL_SCANCODE_F6)
        return;

    m_NormalMapping = !m_NormalMapping;

    std::cout << "[Uniform] Switched normal maps to: " <<
        (m_NormalMapping ? "enabled" : "disabled") << "\n";
}

void Renderer::ToggleDrawDepth(SDL_Scancode key)
{
    if (key != SDL_SCANCODE_F7)
        return;

    m_DrawDepth = !m_DrawDepth;

    std::cout << "[Software only] Switched drawing depth buffer to: " <<
        (m_DrawDepth ? "enabled" : "disabled") << "\n";
}



bool Renderer::SaveBufferToImage() const
{
    return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}



void Renderer::RenderSoftware()
{
    // Lock back buffer
    SDL_LockSurface(m_pBackBuffer);

    // Clear back buffer
    std::fill_n(m_pBackBufferPixels, m_Width * m_Height,
        ToUint32(m_ClearColor, m_pBackBuffer->format));

    // Clear depth buffer
    std::fill_n(m_pDepthBufferPixels, m_Width * m_Height, FLT_MAX);

    // Get view and projection matrices
    const Matrix viewMatrix { m_Camera->GetViewMatrix() };
    const Matrix projMatrix { m_Camera->GetProjMatrix() };

    // Get camera position
    const Vector3 cameraPos { -viewMatrix[3][0],
        -viewMatrix[3][1], -viewMatrix[3][2] };

    // Render loop
    for (const auto& mesh : m_Meshes)
    {
        if (mesh.GetForceRenderingOff())
            continue;

        if (mesh.IsTransparent())
            continue;

        // Frustum culling
        const Bounds& meshBounds { mesh.GetWorldBounds() };
        if (!m_Camera->IsInFrustum(meshBounds))
            continue;



        /* ------------------------ */
        /* --- PROJECTION STAGE --- */
        /* ------------------------ */

        // Get model space vertices
        auto& verticesLocal { mesh.GetVertices() };

        // Get indices
        auto& indices { mesh.GetIndices() };

        // Get primitive topology
        auto topology { mesh.GetPrimitiveTopology() };

        // Get world matrix
        const Matrix worldMatrix { mesh.GetWorldMatrix() };

        // Transform vertices
        std::vector<Vertex_Out> vertices { verticesLocal.size() };

        for (size_t i { 0 }; i < vertices.size(); i++)
        {
            Vertex_Out& vertex { vertices[i] };
            vertex = verticesLocal[i].ToVertexOut();

            // World transformation
            vertex.position = worldMatrix.TransformPoint(vertex.position);
            vertex.normal = worldMatrix.TransformVector(vertex.normal);
            vertex.tangent = worldMatrix.TransformVector(vertex.tangent);

            // View transformation
            vertex.viewDirection = (cameraPos - vertex.position.GetXYZ()).SafeNormalized();
            vertex.position = viewMatrix.TransformPoint(vertex.position);

            // Projection transformation
            vertex.position = projMatrix.TransformPoint(vertex.position);

            // Perspective divide
            vertex.position.x /= vertex.position.w;
            vertex.position.y /= vertex.position.w;
            vertex.position.z /= vertex.position.w;

            // Transform into screen space
            vertex.position.x = (vertex.position.x + 1.0f) * 0.5f * m_Width;
            vertex.position.y = (1.0f - vertex.position.y) * 0.5f * m_Height;
        }



        /* --------------------------- */
        /* --- RASTERIZATION STAGE --- */
        /* --------------------------- */

        size_t size { };
        size_t increment { };

        switch (topology)
        {
            case Topology::TriangleList:
                size = indices.size();
                increment = 3;
                break;
            case Topology::TriangleStrip:
                size = indices.size() - 2;
                increment = 1;
                break;
            default:
                break;
        }

        for (size_t i { 0 }; i < size; i += increment)
        {
            // Temporary vertex pointers to avoid copies
            const Vertex_Out* pV0 { };
            const Vertex_Out* pV1 { };
            const Vertex_Out* pV2 { };

            // Get the three vertices of the triangle based on primitive topology
            if (topology == Topology::TriangleList)
            {
                pV0 = &vertices[indices[i]];
                pV1 = &vertices[indices[i + 1]];
                pV2 = &vertices[indices[i + 2]];
            }
            else if (topology == Topology::TriangleStrip)
            {
                if ((i & 1) == 0)
                {
                    pV0 = &vertices[indices[i]];
                    pV1 = &vertices[indices[i + 1]];
                    pV2 = &vertices[indices[i + 2]];
                }
                else
                {
                    // flip winding
                    pV0 = &vertices[indices[i + 1]];
                    pV1 = &vertices[indices[i]];
                    pV2 = &vertices[indices[i + 2]];
                }
            }
            else
            {
                continue;
            }

             // Bind the vertex references
            const Vertex_Out& v0 { *pV0 };
            const Vertex_Out& v1 { *pV1 };
            const Vertex_Out& v2 { *pV2 };

            // Get vertex positions
            const Vector4 p0 { v0.position };
            const Vector4 p1 { v1.position };
            const Vector4 p2 { v2.position };

            // Near plane culling
            if (p0.w <= 0.0f) continue;
            if (p1.w <= 0.0f) continue;
            if (p2.w <= 0.0f) continue;

            // Compute the screen space bounding box of the triangle
            Vector2 screenSpaceMin { FLT_MAX, FLT_MAX };
            Vector2 screenSpaceMax { -FLT_MAX, -FLT_MAX };

            if (p0.x < screenSpaceMin.x) screenSpaceMin.x = p0.x;
            if (p0.y < screenSpaceMin.y) screenSpaceMin.y = p0.y;
            if (p0.x > screenSpaceMax.x) screenSpaceMax.x = p0.x;
            if (p0.y > screenSpaceMax.y) screenSpaceMax.y = p0.y;

            if (p1.x < screenSpaceMin.x) screenSpaceMin.x = p1.x;
            if (p1.y < screenSpaceMin.y) screenSpaceMin.y = p1.y;
            if (p1.x > screenSpaceMax.x) screenSpaceMax.x = p1.x;
            if (p1.y > screenSpaceMax.y) screenSpaceMax.y = p1.y;

            if (p2.x < screenSpaceMin.x) screenSpaceMin.x = p2.x;
            if (p2.y < screenSpaceMin.y) screenSpaceMin.y = p2.y;
            if (p2.x > screenSpaceMax.x) screenSpaceMax.x = p2.x;
            if (p2.y > screenSpaceMax.y) screenSpaceMax.y = p2.y;

            // Clamp AABB to screen bounds
            const int minX { std::max(static_cast<int>(std::floor(screenSpaceMin.x)), 0) };
            const int minY { std::max(static_cast<int>(std::floor(screenSpaceMin.y)), 0) };
            const int maxX { std::min(static_cast<int>(std::ceil(screenSpaceMax.x)), m_Width - 1) };
            const int maxY { std::min(static_cast<int>(std::ceil(screenSpaceMax.y)), m_Height - 1) };

            // Skip if the trianlge is outside of the screen
            if (minX > maxX || minY > maxY)
                continue;

            // Precompute edge function denominator
            float denom { (p1.y - p2.y) * (p0.x - p2.x) + (p2.x - p1.x) * (p0.y - p2.y) };

            // Cull backfaces and degenerate triangles
            if (denom <= 0.0f)
                continue;

            // Precompute values for perspective-correct interpolation
            const float invW0 { 1.0f / p0.w };
            const float invW1 { 1.0f / p1.w };
            const float invW2 { 1.0f / p2.w };

            // Iterate over only pixels inside the screen
            for (int px = minX; px <= maxX; px++)
            {
                for (int py = minY; py <= maxY; py++)
                {
                    // Pixel center
                    float x = px + 0.5f;
                    float y = py + 0.5f;

                    // Index of the pixel in depth and back buffers
                    const int pixelBufferId { (py * m_Width) + px };

                    // Compute barycentric coordinates
                    float w0 = ((p1.y - p2.y) * (x - p2.x) + (p2.x - p1.x) * (y - p2.y)) / denom;
                    float w1 = ((p2.y - p0.y) * (x - p2.x) + (p0.x - p2.x) * (y - p2.y)) / denom;
                    float w2 = 1.0f - w0 - w1;

                    // Skip if pixel outside of the triangle
                    if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f)
                        continue;

                    // Compute interpolated w and its inverse
                    const float w {
                        w0 * invW0 +
                        w1 * invW1 +
                        w2 * invW2
                    };

                    const float invW { 1.0f / w };

                    // Compute depth
                    const float depth {
                        (w0 * p0.z * invW0 +
                            w1 * p1.z * invW1 +
                            w2 * p2.z * invW2) * invW
                    };

                    // Depth test
                    const float prevDepth { m_pDepthBufferPixels[pixelBufferId] };

                    if (depth > prevDepth || depth < 0.0f || depth > 1.0f)
                        continue;

                    // Write into the depth buffer
                    m_pDepthBufferPixels[pixelBufferId] = depth;



                    /* --------------------- */
                    /* --- SHADING STAGE --- */
                    /* --------------------- */

                    // Skip shading if drawing depth is enabled
                    if (m_DrawDepth)
                        continue;

                    // Interpolate uv coordinates
                    const Vector2 uv {
                        (w0 * v0.uv * invW0 +
                            w1 * v1.uv * invW1 +
                            w2 * v2.uv * invW2) *
                            invW
                    };

                    // Interpolate normal
                    const Vector3 n {
                        ((w0 * (v0.normal * invW0) +
                            w1 * (v1.normal * invW1) +
                            w2 * (v2.normal * invW2)) *
                            invW).SafeNormalized()
                    };

                    // Interpolate tangent
                    Vector3 t {
                        ((w0 * (v0.tangent * invW0) +
                            w1 * (v1.tangent * invW1) +
                            w2 * (v2.tangent * invW2)) *
                            invW).SafeNormalized()
                    };

                    // Ensure orthogonal
                    t = (t - n * Vector3::Dot(n, t)).SafeNormalized();

                    // Interpolate view direction
                    const Vector3 v {
                        ((w0 * (v0.viewDirection * invW0) +
                            w1 * (v1.viewDirection * invW1) +
                            w2 * (v2.viewDirection * invW2)) *
                            invW).SafeNormalized()
                    };

                    // Get the material
                    const Material& material { m_Materials[mesh.GetMaterialID()] };

                    ColorRGB finalColor { m_RenderMode == RenderMode::Combined ?
                        material.GetAmbient(uv) : colors::Black };

                    Vector3 normal { m_NormalMapping ? material.GetNormal(n, t, uv) : n };

                    // Calculate shading
                    for (auto& light : m_Lights)
                    {
                        const Vector3 toLight { -light.direction };

                        switch (m_RenderMode)
                        {
                            case RenderMode::ObservedArea:
                                finalColor += material.GetObservedArea(normal, toLight);
                                break;
                            case RenderMode::DiffuseOnly:
                                finalColor += material.GetDiffuse(normal, toLight, uv);
                                break;
                            case RenderMode::SpecularOnly:
                                finalColor += material.GetSpecular(normal, toLight, v, uv);
                                break;
                            case RenderMode::Combined:
                                finalColor += (material.GetDiffuse(normal, toLight, uv) +
                                    material.GetSpecular(normal, toLight, v, uv)) *
                                    light.color * light.intensity;
                                break;
                            default:
                                break;
                        }
                    }

                    // Write to buffer
                    m_pBackBufferPixels[pixelBufferId] =
                        ToUint32(finalColor, m_pBackBuffer->format);
                }
            }
        }
    }

    if (m_DrawDepth)
        DrawDepthBuffer();

    // Update SDL surface
    SDL_UnlockSurface(m_pBackBuffer);
    SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
    SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::DrawDepthBuffer()
{
    for (int i = 0; i < m_Width * m_Height; ++i)
    {
        float depth = m_pDepthBufferPixels[i];

        if (depth < FLT_MAX)
        {
            float remappedDepth = (depth - 0.995f) / (1.0f - 0.995f);
            remappedDepth = std::clamp(remappedDepth, 0.0f, 1.0f);

            ColorRGB depthColor { remappedDepth, remappedDepth, remappedDepth };
            m_pBackBufferPixels[i] = ToUint32(depthColor, m_pBackBuffer->format);
        }
    }
}



HRESULT Renderer::InitializeDX()
{
    D3D_FEATURE_LEVEL featureLevel { D3D_FEATURE_LEVEL_11_1 };
    uint32_t createDeviceFlag { 0 };

    #if defined(DEBUG) || defined(_DEBUG)
    createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
    #endif // #if defined(DEBUG) || defined(_DEBUG)

    // Create DXGI Factory
    IDXGIFactory1* pDxgiFactory { };
    HRESULT result { CreateDXGIFactory1(__uuidof(IDXGIFactory1),
        reinterpret_cast<void**> (&pDxgiFactory)) };
    if (FAILED(result))
        return S_FALSE;

    // Enumerate adapters and select the dGPU
    IDXGIAdapter* adapter = nullptr;
    IDXGIAdapter* selectedAdapter = nullptr;

    for (UINT i { 0 }; pDxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        DXGI_ADAPTER_DESC desc;
        adapter->GetDesc(&desc);
        std::wcout << L"Adapter: " << i << L": " << desc.Description << L"\n";

        if (desc.VendorId != 0x8086) // Intel vendor ID
        {
            selectedAdapter = adapter;
            break;
        }
    }

    // Fallback to the first adapter if no dGPU found
    if (selectedAdapter == nullptr)
    {
        if (pDxgiFactory->EnumAdapters(0, &adapter) != DXGI_ERROR_NOT_FOUND)
            selectedAdapter = adapter;
        else
            return S_FALSE;
    }

    // Create device and device context
    result = D3D11CreateDevice(selectedAdapter, D3D_DRIVER_TYPE_UNKNOWN, 0,
        createDeviceFlag, &featureLevel, 1, D3D11_SDK_VERSION,
        &m_pDevice, nullptr, &m_pDeviceContext);

    if (FAILED(result))
        return S_FALSE;

    // Set up swapchain description
    DXGI_SWAP_CHAIN_DESC swapChainDesc { };
    swapChainDesc.BufferDesc.Width = m_Width;
    swapChainDesc.BufferDesc.Height = m_Height;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32 bit format
    swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Buffers will be bound to Output Merger as a render target
    swapChainDesc.BufferCount = 1;
    swapChainDesc.Windowed = true;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    // Get window handle from SDL
    SDL_SysWMinfo sysWMInfo { };
    SDL_GetVersion(&sysWMInfo.version);
    SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
    swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

    // Create swapchain
    result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);

    if (FAILED(result))
        return S_FALSE;

    // Set up depth stencil description
    D3D11_TEXTURE2D_DESC depthStencilDesc { };
    depthStencilDesc.Width = m_Width;
    depthStencilDesc.Height = m_Height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    // Set up depth stencil view description
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc { };
    depthStencilViewDesc.Format = depthStencilDesc.Format;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    // Create depth stencil
    result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
    if (FAILED(result))
        return S_FALSE;

    // Create depth stencil view
    result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
    if (FAILED(result))
        return S_FALSE;

    // Create render target
    result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**> (&m_pRenderTargetBuffer));

    if (FAILED(result))
        return S_FALSE;

    // Create render target view
    result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer,
        nullptr, &m_pRenderTargetView);

    if (FAILED(result))
        return S_FALSE;

    // Bind render target and depth stencil to the output merger
    m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

    // Set up viewport
    D3D11_VIEWPORT viewPort { };
    viewPort.Width = static_cast<float> (m_Width);
    viewPort.Height = static_cast<float> (m_Height);
    viewPort.TopLeftX = 0.f;
    viewPort.TopLeftY = 0.f;
    viewPort.MinDepth = 0.f;
    viewPort.MaxDepth = 1.f;

    // Set viewport to device context
    m_pDeviceContext->RSSetViewports(1, &viewPort);

    // Create the effect
    const wchar_t* shaderFilepath { L"resources/default.fx" };

    // Set shader flags
    DWORD shaderFlags = 0;
    #if defined(_DEBUG) || defined(DEBUG)
    shaderFlags |= D3DCOMPILE_DEBUG;
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
    #endif

    ID3DBlob* pErrorBlob = nullptr;

    HRESULT hr = D3DX11CompileEffectFromFile(
        shaderFilepath,
        nullptr,
        nullptr,
        shaderFlags,
        0,
        m_pDevice,
        &m_pEffect,
        &pErrorBlob
    );

    if (FAILED(hr))
    {
        if (pErrorBlob)
        {
            std::cout << static_cast<char*>(pErrorBlob->GetBufferPointer());
            pErrorBlob->Release();
        }

        return S_FALSE;
    }

    // Get techniques
    m_pOpaqueTechnique = m_pEffect->GetTechniqueByName("OpaqueTechnique");
    m_pTransparentTechnique = m_pEffect->GetTechniqueByName("TransparentTechnique");

    // Get first pass description
    D3DX11_PASS_DESC passDesc { };
    m_pOpaqueTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

    // Create input layout description
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, normal), D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(Vertex, tangent), D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    // Create input layout
    m_pDevice->CreateInputLayout(
        layout,
        _countof(layout),
        passDesc.pIAInputSignature,
        passDesc.IAInputSignatureSize,
        &m_pInputLayout
    );

    // Get effect flags
    m_pUseNormalMap = m_pEffect->GetVariableByName("gUseNormalMap")->AsScalar();
    m_pSamplingMode = m_pEffect->GetVariableByName("gSamplingMode")->AsScalar();

    return S_OK;
}

void Renderer::InitializeMeshesDX()
{
    for (auto& mesh : m_Meshes)
    {
        auto& dx { mesh.GetMeshDX() };

        dx.numIndices = static_cast<uint32_t>(mesh.GetIndices().size());

        // Vertex buffer
        D3D11_BUFFER_DESC vbDesc { };
        vbDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vbDesc.ByteWidth = UINT(sizeof(Vertex) * mesh.GetVertices().size());
        vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA vbData { };
        vbData.pSysMem = mesh.GetVertices().data();

        m_pDevice->CreateBuffer(&vbDesc, &vbData, &dx.pVertexBuffer);

        // Index buffer
        D3D11_BUFFER_DESC ibDesc { };
        ibDesc.Usage = D3D11_USAGE_IMMUTABLE;
        ibDesc.ByteWidth = UINT(sizeof(uint32_t) * mesh.GetIndices().size());
        ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

        D3D11_SUBRESOURCE_DATA ibData { };
        ibData.pSysMem = mesh.GetIndices().data();

        m_pDevice->CreateBuffer(&ibDesc, &ibData, &dx.pIndexBuffer);
    }
}

void Renderer::InitializeMaterialsDX()
{
    for (auto& mat : m_Materials)
    {
        auto& dx { mat.GetMaterialDX() };

        // Create shader resource views for material textures
        if (mat.diffuseColorMap)
            mat.diffuseColorMap->CreateShaderResourceView(m_pDevice);
        if (mat.specularMap)
            mat.specularMap->CreateShaderResourceView(m_pDevice);
        if (mat.glossinessMap)
            mat.glossinessMap->CreateShaderResourceView(m_pDevice);
        if (mat.normalMap)
            mat.normalMap->CreateShaderResourceView(m_pDevice);

        // Get all the effect variables
        dx.pMatWorld = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
        dx.pMatViewProj = m_pEffect->GetVariableByName("gViewProj")->AsMatrix();

        dx.pCameraPosition = m_pEffect->GetVariableByName("gCameraPosition")->AsVector();

        dx.pNumLights = m_pEffect->GetVariableByName("gNumLights")->AsScalar();
        dx.pLightsArray = m_pEffect->GetVariableByName("gLights");

        dx.pDiffuseColor = m_pEffect->GetVariableByName("gDiffuseColor")->AsVector();
        dx.pSpecularColor = m_pEffect->GetVariableByName("gSpecularColor")->AsVector();
        dx.pAmbientColor = m_pEffect->GetVariableByName("gAmbientColor")->AsVector();
        dx.pDiffuseReflectance = m_pEffect->GetVariableByName("gDiffuseReflectance")->AsScalar();
        dx.pSpecularReflectance = m_pEffect->GetVariableByName("gSpecularReflectance")->AsScalar();
        dx.pGlossiness = m_pEffect->GetVariableByName("gGlossiness")->AsScalar();

        dx.pDiffuseMap = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
        dx.pNormalMap = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
        dx.pSpecularMap = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
        dx.pGlossinessMap = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
    }
}

void Renderer::RenderDX()
{
    if (!m_InitializedDX)
        return;

    // Clear render target and depth buffer
    const float clearColor[4] { m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, 1.0f };
    m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, clearColor);
    m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView,
        D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    // Get matrices
    const Matrix viewMatrix { m_Camera->GetViewMatrix() };
    const Matrix projMatrix { m_Camera->GetProjMatrix() };
    const Matrix viewProjMatrix { viewMatrix * projMatrix };
    const Vector3 cameraPos { m_Camera->GetPosition() };

    // Render lambda
    auto renderMesh = [&](Mesh& mesh, bool useTransparentTechnique)
    {
        if (mesh.GetForceRenderingOff())
            return;

        // Frustum culling
        const Bounds& meshBounds { mesh.GetWorldBounds() };
        if (!m_Camera->IsInFrustum(meshBounds))
            return;

        // Get DX specific types
        auto& mat { m_Materials[mesh.GetMaterialID()] };
        auto& meshDX { mesh.GetMeshDX() };
        auto& matDX { mat.GetMaterialDX() };

        // Set matrices
        const Matrix worldMatrix { mesh.GetWorldMatrix() };
        matDX.pMatWorld->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
        matDX.pMatViewProj->SetMatrix(reinterpret_cast<const float*>(&viewProjMatrix));

        // Set camera position
        if (matDX.pCameraPosition)
            matDX.pCameraPosition->SetFloatVector(reinterpret_cast<const float*>(&cameraPos));

        // Set textures
        if (mat.diffuseColorMap && matDX.pDiffuseMap)
            matDX.pDiffuseMap->SetResource(mat.diffuseColorMap->GetShaderResourceView());
        if (mat.normalMap && matDX.pNormalMap)
            matDX.pNormalMap->SetResource(mat.normalMap->GetShaderResourceView());
        if (mat.specularMap && matDX.pSpecularMap)
            matDX.pSpecularMap->SetResource(mat.specularMap->GetShaderResourceView());
        if (mat.glossinessMap && matDX.pGlossinessMap)
            matDX.pGlossinessMap->SetResource(mat.glossinessMap->GetShaderResourceView());

        // Set material properties
        if (matDX.pDiffuseColor)
            matDX.pDiffuseColor->SetFloatVector(reinterpret_cast<const float*>(&mat.diffuseColor));
        if (matDX.pSpecularColor)
            matDX.pSpecularColor->SetFloatVector(reinterpret_cast<const float*>(&mat.specularColor));
        if (matDX.pAmbientColor)
            matDX.pAmbientColor->SetFloatVector(reinterpret_cast<const float*>(&mat.ambientColor));
        if (matDX.pDiffuseReflectance)
            matDX.pDiffuseReflectance->SetFloat(mat.diffuseReflectance);
        if (matDX.pSpecularReflectance)
            matDX.pSpecularReflectance->SetFloat(mat.specularReflectance);
        if (matDX.pGlossiness)
            matDX.pGlossiness->SetFloat(mat.glossiness);

        // Set light array
        if (matDX.pNumLights && matDX.pLightsArray)
        {
            // Set number of lights (clamp to MAX_LIGHTS)
            int numLights { std::min(static_cast<int>(m_Lights.size()), 8) };
            matDX.pNumLights->SetInt(numLights);

            // Set light array data
            if (numLights > 0)
            {
                matDX.pLightsArray->SetRawValue(m_Lights.data(), 0,
                    sizeof(Light) * numLights);
            }
        }

        // Set effect flags
        if (m_pUseNormalMap)
            m_pUseNormalMap->SetBool(m_NormalMapping);
        if (m_pSamplingMode)
            m_pSamplingMode->SetInt(static_cast<int>(m_SamplingMode));

        // Set topology and buffers
        m_pDeviceContext->IASetPrimitiveTopology(
            mesh.GetPrimitiveTopology() == Topology::TriangleStrip ?
            D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP :
            D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_pDeviceContext->IASetInputLayout(m_pInputLayout);

        constexpr UINT stride = sizeof(Vertex);
        constexpr UINT offset = 0;
        m_pDeviceContext->IASetVertexBuffers(0, 1, &meshDX.pVertexBuffer, &stride, &offset);
        m_pDeviceContext->IASetIndexBuffer(meshDX.pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

        // Select technique based on transparency
        ID3DX11EffectTechnique* technique = useTransparentTechnique ?
            m_pTransparentTechnique : m_pOpaqueTechnique;

        // Draw (single pass for all of the lights)
        D3DX11_TECHNIQUE_DESC techDesc;
        technique->GetDesc(&techDesc);

        for (UINT p = 0; p < techDesc.Passes; p++)
        {
            technique->GetPassByIndex(p)->Apply(0, m_pDeviceContext);
            m_pDeviceContext->DrawIndexed(meshDX.numIndices, 0, 0);
        }
    };

    // Render opaque meshes first (with depth write)
    for (auto& mesh : m_Meshes)
    {
        if (!mesh.IsTransparent())
            renderMesh(mesh, false);
    }

    // Render transparent meshes second (without depth write and with blending)
    for (auto& mesh : m_Meshes)
    {
        if (mesh.IsTransparent())
            renderMesh(mesh, true);
    }

    // Present
    m_pSwapChain->Present(0, 0);
}

void Renderer::CleanupDX()
{
    if (m_pDeviceContext) m_pDeviceContext->ClearState();

    if (m_pDepthStencilView) m_pDepthStencilView->Release();
    if (m_pDepthStencilBuffer) m_pDepthStencilBuffer->Release();

    if (m_pRenderTargetView) m_pRenderTargetView->Release();
    if (m_pRenderTargetBuffer) m_pRenderTargetBuffer->Release();

    if (m_pSwapChain) m_pSwapChain->Release();

    if (m_pDeviceContext) m_pDeviceContext->Release();
    if (m_pDevice) m_pDevice->Release();

    for (auto& mesh : m_Meshes)
    {
        auto& dx = mesh.GetMeshDX();
        if (dx.pVertexBuffer) dx.pVertexBuffer->Release();
        if (dx.pIndexBuffer) dx.pIndexBuffer->Release();
    }

    if (m_pInputLayout) m_pInputLayout->Release();
    if (m_pEffect) m_pEffect->Release();
}