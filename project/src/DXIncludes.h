#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <d3dx11effect.h>

// undefine the min and max macros
#ifndef NOMINMAX
#ifdef max(a, b)
#undef max(a, b)
#endif // max(a, b)
#ifdef min(a, b)
#undef min(a, b)
#endif // min(a, b)
#endif // !NOMINMAX