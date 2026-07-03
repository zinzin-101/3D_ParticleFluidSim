#version 450 core

layout(std430, binding = 3) buffer Densities { float densities[]; };
layout(std430, binding = 8) buffer GridCellStart { uint cellStart[]; };
layout(std430, binding = 9) buffer GridCellEnd { uint cellEnd[]; };

in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 camPos;
uniform mat4 invView;
uniform vec4 planes[6];

uniform float renderScale;
uniform uint stepCount;

bool isIntersectingPlane(vec3 origin, vec3 dir, vec4 plane, vec3 hitPos){
    vec3 n = plane.xyz;
    float d = plane.w;

    vec3 p0 = -d * n;

    float denom = dot(dir, n);
    if (abs(denom) > 0.0001) {
        
        float t = dot(p0 - origin, n) / denom;
        
        if (t >= 0.0) {
            hitPos = origin + t * dir;
            return true;
        }
    }

    return false;
}

void main(){
    vec2 uv = (TexCoords - 0.5) * (1.0 / renderScale) + 0.5;

    // converting screen to world
    vec4 ndcPosNear = vec4(uv * 2.0 - 1.0, -1.0, 1.0);
    vec4 ndcPosFar  = vec4(uv * 2.0 - 1.0,  1.0, 1.0);

    vec4 worldPosNear = invView * ndcPosNear;
    vec4 worldPosFar  = invView * ndcPosFar;

    worldPosNear /= worldPosNear.w;
    worldPosFar  /= worldPosFar.w;

    vec3 rayOrigin = worldPosNear.xyz; 
    vec3 rayDir = normalize(worldPosFar.xyz - worldPosNear.xyz);

    uint intersectCount = 0;
    vec3 startPos = vec3(0.0);
    vec3 endPos = vec3(0.0);
    for (uint i = 0; i < 6; i++){
        if (intersectCount >= 2) break;
        vec3 temp = vec3(0.0);
        if (isIntersectingPlane(rayOrigin, rayDir, planes[i], temp)) {
            if (intersectCount == 0){
                startPos = temp;
            }
            else if (intersectCount == 1){
                endPos = temp;
            }
        }
    }

    if (intersectCount < 2) {
        FragColor = vec4(0.0);
        return;
    }

    /* ray marching code */
    
    FragColor = vec4(rayDir * 0.5 + 0.5, 1.0);
}