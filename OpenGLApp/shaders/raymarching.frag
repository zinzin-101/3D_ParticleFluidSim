#version 450 core

layout(std430, binding = 3) buffer Densities { float densities[]; };
layout(std430, binding = 8) buffer GridCellStart { uint cellStart[]; };
layout(std430, binding = 9) buffer GridCellEnd { uint cellEnd[]; };

in vec2 TexCoords;
out vec4 FragColor;

uniform vec3 camPos;
uniform mat4 invViewProj;
uniform vec4 planes[6];

uniform float renderScale;
uniform uint stepCount;

bool intersectConvexVolume(vec3 origin, vec3 dir, out vec3 startPos, out vec3 endPos) {
    float tNear = -1e30;
    float tFar  =  1e30;

    for (int i = 0; i < 6; i++) {
        vec3 n = planes[i].xyz;
        float d = planes[i].w * renderScale;

        float denom = dot(dir, n);
        float t = -(dot(n, origin) + d) / denom;

        if (denom < -0.0001) {
            // ray outside -> inside
            tNear = max(tNear, t);
        } else if (denom > 0.0001) {
            // ray inside -> outside
            tFar = min(tFar, t);
        } else {
            // no hit
            if (dot(n, origin) + d > 0.0) return false;
        }
    }

    if (tNear > tFar) return false;
    if (tFar < 0.0) return false;
    startPos = origin + max(tNear, 0.0) * dir;
    endPos = origin + tFar * dir;
    return true;
}

void main(){
    vec2 uv = TexCoords;

    vec4 clipFar = vec4(uv * 2.0 - 1.0, 1.0, 1.0);
    vec4 worldFar = invViewProj * clipFar;
    worldFar /= worldFar.w;

    vec3 rayOrigin = camPos;
    vec3 rayDir = normalize(worldFar.xyz - camPos);

    vec3 startPosScreen = vec3(0.0);
    vec3 endPosScreen = vec3(0.0);
    if (!intersectConvexVolume(rayOrigin, rayDir, startPosScreen, endPosScreen)) {
        discard;
    }

    vec3 startPos = startPosScreen / renderScale;
    vec3 endPos = endPosScreen / renderScale;

    /* ray marching code */
    
    FragColor = vec4(rayDir * 0.5 + 0.5, 1.0);
}