#version 440 core

out vec4 FragColor;
in vec3 FragPos;
in vec3 Normal;
in float Speed;

uniform vec3 color;
uniform vec3 camPos;

vec3 lightDir = normalize(vec3(2.5f, 2.5f, 2.0f));

float shininess = 48;
vec3 ambient = vec3(0.3);
vec3 diffuse = vec3(0.9);
vec3 specular = vec3(0.6);

vec3 currentColor = vec3(0.0);

vec3 CalcDirLight(vec3 lightDir, vec3 normal, vec3 viewDir);
vec3 GetColorFromSpeed(float speed);

void main() {
    vec3 viewDir = normalize(camPos - FragPos);
    currentColor = GetColorFromSpeed(Speed);
    vec3 result = CalcDirLight(lightDir, normalize(Normal), viewDir);
    FragColor = vec4(currentColor, 1.0);
    //FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(vec3 lightDirection, vec3 normal, vec3 viewDirection) {
    vec3 L = normalize(lightDirection);
    // diffuse shading
    float diff = max(dot(normal, L), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-L, normal);
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), shininess);
    // combine results
    vec3 _ambient = ambient * currentColor;
    vec3 _diffuse = diffuse * diff * currentColor;
    vec3 _specular = specular * spec * currentColor;
    return (_ambient + _diffuse + _specular);
}

vec3 GetColorFromSpeed(float speed){
    float minSpeed = 0.0;
    float maxSpeed = 20.0;
    float x = clamp((speed - minSpeed) / (maxSpeed - minSpeed), 0.0, 1.0);
    x = mix(0.12, 1.0, x);

    // turbo polynomial approximation coefficients
    const vec4 kRedVec4 = vec4(0.13572138, 4.61539260, -42.66032258, 132.13108234);
    const vec4 kGreenVec4 = vec4(0.09140261, 2.19418839, 4.84296658, -14.18503333);
    const vec4 kBlueVec4 = vec4(0.10667330, 12.64194608, -60.58204836, 110.36276771);
    const vec2 kRedVec2 = vec2(-152.94239396, 69.11619443);
    const vec2 kGreenVec2 = vec2(-4.27729857, 2.82956604);
    const vec2 kBlueVec2 = vec2(-89.90310912, 27.34824973);

    vec3 v4 = vec3(x, x * x, x * x * x);
    vec3 v2 = v4 * v4.z; // x^4, x^5, x^6

    // dot products calculate the polynomial curve for each channel
    float r = kRedVec4.x + dot(kRedVec4.yzw, v4) + dot(kRedVec2, v2.xy);
    float g = kGreenVec4.x + dot(kGreenVec4.yzw, v4) + dot(kGreenVec2, v2.xy);
    float b = kBlueVec4.x + dot(kBlueVec4.yzw, v4) + dot(kBlueVec2, v2.xy);

    return clamp(vec3(r, g, b), 0.0, 1.0);
}