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

uniform float particleMass;
uniform vec3 lightColor;
uniform samplerCube skybox;
uniform float isoLevel;
uniform float surfaceSmoothingRadius;

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

const float PI = 3.14159265358979323846;

float poly6Kernel(float r2, float h) {
    if (r2 >= h * h) return 0.0;
    float h2 = h * h;
    float diff = h2 - r2;
    float h9 = h2 * h2 * h2 * h2 * h;
    return (315.0 / (64.0 * PI * h9)) * diff * diff * diff;
}

vec3 poly6Gradient(vec3 diff, float r2, float h) {
    if (r2 >= h * h) return vec3(0.0);
    float h2 = h * h;
    float diffTerm = h2 - r2;
    float h9 = h2 * h2 * h2 * h2 * h;
    float coeff = -945.0 / (32.0 * PI * h9); 
    return coeff * diffTerm * diffTerm * diff;
}

vec3 spikyGradient(vec3 diff, float r, float h) {
    if (r >= h || r < 0.0001) return vec3(0.0);
    float hr = h - r;
    float coeff = -45.0 / (PI * h*h*h*h*h*h);
    return coeff * hr * hr * (diff / r);
}

float sampleDensityAt(vec3 pos) {
    float h = spacing;
    ivec3 centerCell = ivec3(floor(pos / spacing));

    float total = 0.0;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            for (int dz = -1; dz <= 1; dz++) {
                ivec3 coord = centerCell + ivec3(dx, dy, dz);
                uint hash = coordToHash(coord);
                uint start = cellStart[hash];
                uint end = cellEnd[hash];

                for (uint j = start; j < end; j++) {
                    vec3 particlePos = getPositionVec3(j);
                    if (positionToCoord(particlePos) != coord) continue;

                    vec3 diff = pos - particlePos;
                    float r2 = dot(diff, diff);
                    total += particleMass * poly6Kernel(r2, h);
                }
            }
        }
    }
    return total;
}

uniform float refractionIndexAir;
uniform float refractionIndexFluid;
vec3 refractRay(vec3 incident, vec3 normal, float indexA, float indexB) {
    float relIndex = indexA / indexB;
    float cosIn = -dot(incident, normal);
    if (cosIn < 0.0) {
        normal = -normal;
        cosIn = -cosIn;
    }

    float sinSqrAngle = relIndex * relIndex * (1.0 - cosIn * cosIn);

    if (sinSqrAngle >= 1.0) {
        // total internal reflection
        return reflect(incident, normal);
    }
    return incident * relIndex + normal * (relIndex * cosIn - sqrt(1.0 - sinSqrAngle));
}

float calculateReflectance(vec3 incident, vec3 normal, float indexA, float indexB) {
    float refractRatio = indexA / indexB;
    float cosIn = -dot(incident, normal);
    if (cosIn < 0.0) {
        normal = -normal;
        cosIn = -cosIn;
    }

    float sinSqr = refractRatio * refractRatio * (1.0 - cosIn * cosIn);

    if (sinSqr >= 1.0) {
        // total internal reflection
        return 1.0;
    }

    float cosRefract = sqrt(1.0 - sinSqr);
    float rayPerp = (indexA * cosIn - indexB * cosRefract) / (indexA * cosIn + indexB * cosRefract);
    float rayParallel = (indexB * cosIn - indexA * cosRefract) / (indexB * cosIn + indexA * cosRefract);

    return (rayPerp * rayPerp + rayParallel * rayParallel) * 0.5;
}

struct DensitySample {
    float density;
    vec3 gradient;
};

DensitySample sampleDensityAndGradient(vec3 pos) {
    float h = surfaceSmoothingRadius * spacing;
    float h2 = h * h;
    float coeff = 315.0 / (64.0 * PI * h2*h2*h2*h2*h);
    ivec3 centerCell = ivec3(floor(pos / spacing));

    int cellRadius = int(ceil(surfaceSmoothingRadius));

    DensitySample result;
    result.density = 0.0;
    result.gradient = vec3(0.0);

    for (int dx = -cellRadius; dx <= cellRadius; dx++)
    for (int dy = -cellRadius; dy <= cellRadius; dy++)
    for (int dz = -cellRadius; dz <= cellRadius; dz++) {
        ivec3 coord = centerCell + ivec3(dx, dy, dz);
        uint hash = coordToHash(coord);
        uint start = cellStart[hash];
        uint end = cellEnd[hash];

        for (uint j = start; j < end; j++) {
            vec3 particlePos = getPositionVec3(j);
            if (positionToCoord(particlePos) != coord) continue;

            vec3 diff = pos - particlePos;
            float r2 = dot(diff, diff);
            if (r2 >= h2) continue;

            float diffTerm = h2 - r2;
            result.density += particleMass * coeff * diffTerm * diffTerm * diffTerm;
            result.gradient += particleMass * poly6Gradient(diff, r2, h);
        }
    }
    return result;
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
    vec3 marchDir = dir;
    bool enteredFluid = false;
    vec3 reflectColor = vec3(0.0);
    float surfaceReflectance = 0.0;

    vec3 totalLight = vec3(0.0);
    float totalDensity = 0.0;

    vec3 prevPos = pos;
    float prevDensity = 0.0;

    for (uint i = 0; i < stepCount; i++){
        DensitySample ds = sampleDensityAndGradient(pos);

        if (!enteredFluid && ds.density > isoLevel) {
            float denom = ds.density - prevDensity;
            float t = (abs(denom) > 1e-6) ? (isoLevel - prevDensity) / denom : 1.0;

            vec3 surfacePos = mix(prevPos, pos, clamp(t, 0.0, 1.0));

            DensitySample surfaceSample = sampleDensityAndGradient(surfacePos);
            vec3 grad = surfaceSample.gradient;
            vec3 normal = (dot(grad, grad) > 1e-8) ? normalize(-grad) : -marchDir;  

            surfaceReflectance = calculateReflectance(marchDir, normal, refractionIndexAir, refractionIndexFluid);
            reflectColor = texture(skybox, reflect(marchDir, normal)).rgb;
            
            marchDir = refractRay(marchDir, normal, refractionIndexAir, refractionIndexFluid);
            enteredFluid = true;
        }


        if (ds.density > isoLevel * 0.5){
        //if (enteredFluid){
            totalDensity += ds.density * stepSize;
            vec3 inLight = ds.density * stepSize * vec3(1.0);
            vec3 transmittance = exp(-vec3(lightColor) * totalDensity);
            totalLight += inLight * transmittance;
        }

        prevDensity = ds.density;
        prevPos = pos;
        pos += marchDir * stepSize;
    }

    // tone mapping
    totalLight = totalLight / (totalLight + vec3(1.0));

    vec3 finalColor = mix(totalLight, reflectColor, surfaceReflectance);
    FragColor = vec4(finalColor, clamp(totalDensity * densityMultiplier, 0.0, 1.0));


    //totalDensity *= densityMultiplier;
    //totalDensity = clamp(totalDensity, 0.0, 1.0);
    //FragColor = vec4(totalLight, totalDensity);

    //if (totalDensity == 0.0) discard;

    //FragColor = vec4(vec3(1.0, 0.0, 0.0) * totalDensity, totalDensity); 
    //FragColor = vec4(1.0, 1.0, 1.0, totalDensity);

    //FragColor = vec4(rayDir * 0.5 + 0.5, 1.0);
    
}