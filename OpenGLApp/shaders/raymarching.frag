#version 450 core

layout(std430, binding = 3) buffer Densities { float densities[]; };
layout(std430, binding = 8) buffer GridCellStart { uint cellStart[]; };
layout(std430, binding = 9) buffer GridCellEnd { uint cellEnd[]; };

in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 camPos;
uniform mat4 invView;

void main(){
    // converting screen to world
    vec4 ndcPosNear = vec4(TexCoords * 2.0 - 1.0, -1.0, 1.0);
    vec4 ndcPosFar  = vec4(TexCoords * 2.0 - 1.0,  1.0, 1.0);

    vec4 worldPosNear = invView * ndcPosNear;
    vec4 worldPosFar  = invView * ndcPosFar;

    worldPosNear /= worldPosNear.w;
    worldPosFar  /= worldPosFar.w;

    vec3 rayOrigin = worldPosNear.xyz; 
    vec3 rayDir = normalize(worldPosFar.xyz - worldPosNear.xyz);

    /* ray marching code */
    
    FragColor = vec4(rayDir * 0.5 + 0.5, 1.0);
}