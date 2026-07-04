#version 450 core

layout(std430, binding = 0) buffer ParticlePositions { float positions[]; };
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
uniform float densityMultiplier;

uniform float spacing;
uniform uint tableSize;

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

int floatToIntCoord(float val) {
    return int(floor(val / spacing));
}

uint coordToHash(ivec3 coord) {
    int h = (coord.x * 92837111) ^ (coord.y * 689287499) ^ (coord.z * 283923481);
    return uint(h & 0x7FFFFFFF) % tableSize;
}

vec3 getPositionVec3(uint id) {
    uint base = id * 3;
    return vec3(positions[base], positions[base+1], positions[base+2]);
}

ivec3 positionToCoord(vec3 p) {
    return ivec3(
        floatToIntCoord(p.x),
        floatToIntCoord(p.y),
        floatToIntCoord(p.z)
    );
}

float sampleDensityAt(vec3 pos) {
    vec3 cellSpacePos = pos / spacing - 0.5;
    vec3 cellFloor = floor(cellSpacePos);
    vec3 frac = cellSpacePos - cellFloor;

    float total = 0.0;
    for (int dx = 0; dx <= 1; dx++) {
        for (int dy = 0; dy <= 1; dy++) {
            for (int dz = 0; dz <= 1; dz++) {
                ivec3 coord = ivec3(cellFloor) + ivec3(dx, dy, dz);
                uint hash = coordToHash(coord);
                uint start = cellStart[hash];
                uint end = cellEnd[hash];

                float cellDensity = 0.0;
                for (uint j = start; j < end; j++) {
                    ivec3 particleCoord = positionToCoord(getPositionVec3(j));
                    if (particleCoord == coord) {
                        cellDensity += densities[j];
                    }
                }

                float wx = (dx == 1) ? frac.x : (1.0 - frac.x);
                float wy = (dy == 1) ? frac.y : (1.0 - frac.y);
                float wz = (dz == 1) ? frac.z : (1.0 - frac.z);
                total += cellDensity * wx * wy * wz;
            }
        }
    }
    return total;
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
    vec3 dir = normalize(endPos - startPos);
    float stepSize = distance(startPos, endPos) / float(stepCount);

    vec3 pos = startPos + dir * 0.001;
    float totalDensity = 0.0;
    uint totalCount = 0;
    for (uint i = 0; i < stepCount; i++){
        totalDensity += sampleDensityAt(pos);
        totalCount++;
        pos += dir * stepSize;
    }

    totalDensity *= densityMultiplier;
    totalDensity = clamp(totalDensity, 0.0, 1.0);

    //if (totalDensity == 0.0) discard;

    FragColor = vec4(vec3(1.0) * totalDensity, totalDensity); 
    //FragColor = vec4(1.0, 1.0, 1.0, totalDensity);

    //FragColor = vec4(rayDir * 0.5 + 0.5, 1.0);
}