#version 450 core

layout(std430, binding = 3) buffer Densities { float densities[]; };
layout(std430, binding = 8) buffer GridCellStart { uint cellStart[]; };
layout(std430, binding = 9) buffer GridCellEnd { uint cellEnd[]; };