#pragma once

// Force discrete GPU (NVIDIA/AMD) on laptops with hybrid graphics
// This is a global export that tells Windows to prefer high-performance GPU
// over integrated graphics for OpenGL/DirectX rendering.
//
// Include this header in your main.cpp to automatically enable discrete GPU.

#ifdef _WIN32
extern "C" {
    // NVIDIA Optimus: Force NVIDIA GPU
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    
    // AMD PowerXpress: Force AMD discrete GPU  
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif
