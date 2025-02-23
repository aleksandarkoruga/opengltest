// //////////////////////////////////////////////////////////////////////
// DO NOT CHANGE

#version 430 core


layout(location = 0) uniform int ssboSize; // Size of the SSBO (number of elements)
layout(location = 3) uniform vec2 resolution;
layout(location = 4) uniform sampler2D previousFrame;
layout(location = 5) uniform sampler2D previousFrame2;

layout(location = 6) uniform int     lastWriteIdx; //last write index from audio thread to ssbo DataBuffer
layout(std430, binding = 2) buffer DataBuffer {
    float data[];
};

layout(location = 0) out vec4 fragColor;  // Output to the framebuffer

// //////////////////////////////////////////////////////////////////////


// Wave equation parameters
const float damping = 0.999; // Slight damping to prevent infinite energy
const float delta = 1.0;
const float space = 1.0;
const float sideAtten = 1.0;
const float pi2=6.28318530718;

//ported from https://www.shadertoy.com/view/wdtyDH
void main()
{
    vec2 onePix = 1.0 / resolution;
    vec2 coord = gl_FragCoord.xy * onePix;

    vec2 oPixS = onePix*space;

    // Read previous states (no clamping to [-1, 1] here)
    vec3 u_prev = texture(previousFrame, coord).rgb ;  // u(t-Δt)
    vec3 u_prev2 = texture(previousFrame2, coord).rgb ; // u(t-2Δt)

    float pressure = u_prev.x;
    float pVel = u_prev.y;

    // Spatial derivatives (Laplacian)
    float p_left = texture(previousFrame, coord - vec2(oPixS.x, 0.0)).x ;
    float p_right = texture(previousFrame, coord + vec2(oPixS.x, 0.0)).x ;
    float p_up = texture(previousFrame, coord + vec2(0.0, oPixS.y)).x ;
    float p_down = texture(previousFrame, coord - vec2(0.0, oPixS.y)).x ;
  /*
    if(coord.x < 2*onePix.x)
        p_right= sideAtten*p_left ;
        
    if(coord.y < 2*onePix.y)
        p_up= sideAtten*p_down  ;

    if(coord.x > 1.0-2*onePix.x)
        p_left= sideAtten*p_right ;

    if(coord.y > 1.0-2*onePix.y)
        p_down= sideAtten*p_up ;
*/
     // Apply horizontal wave function
    pVel += delta*0.25 * (-4.0 * pressure + p_right + p_left + p_up + p_down )  ;
    
    // Change pressure by pressure velocity
    pressure += delta * pVel ;
    
    // "Spring" motion. This makes the waves look more like water waves and less like sound waves.
    pVel -= 0.005 * delta * pressure;
    
    // Velocity damping so things eventually calm down
    pVel *= 1.0 - 0.002 * delta;
    
    // Pressure damping to prevent it from building up forever.
    pressure *= damping;
    
    

    // Add excitation from SSBO 
    int index = int(coord.x * ssboSize);

    //float moIdx = mod(float(index)+lastWriteIdx,float(ssboSize));

    float excitation =  smoothstep(0.0,1.0,1.0- 500.0* abs( (coord.y-0.5)));     //abs( (coord.y-0.5) - 0.25*data[index]);
    
    
    
    //cos(coord.x*pi2+ (coord.y-0.5) * float(resolution.y)*data[index]  )
    
    pressure+=excitation*data[index];
    //x = pressure. y = pressure velocity. Z and W = X and Y gradient
    fragColor = vec4(pressure, pVel, (p_right - p_left) / 2.0, (p_up - p_down) / 2.0);
}



// RAYMARCHING // /////////////////////////////////////////////////
// /////////////////////////////////////////////////
// /////////////////////////////////////////////////
// UNCOMMENT TO USE AND COMMENT THE main ABOVE/////////////////////////////////////////////////
/*
// Camera
const vec3 camPos = vec3(0.0, 0.0, 5.0);
const vec3 camTarget = vec3(0.0, 0.0, 0.0);
const float fov = 1.0;

// Lighting
const vec3 lightDir = normalize(vec3(0.5, 1.0, 0.5));

// Constants
const float MAX_DIST = 100.0;
const float MIN_DIST = 0.001;
const int MAX_STEPS = 256;

// Basic SDFs
float sdSphere(vec3 p, float r) {
    return length(p) - r;
}

float sdBox(vec3 p, vec3 b) {
    vec3 q = abs(p) - b;
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
}

float sdPlane(vec3 p, vec4 n) {
    return dot(p, n.xyz) + n.w;
}

// Fractals
float mengerSponge(vec3 p) {
    float d = sdBox(p, vec3(1.0));
    float s = 1.0;
    for (int i = 0; i < 4; i++) {
        vec3 a = mod(p * s, 2.0) - 1.0;
        s *= 3.0;
        vec3 r = 1.0 - 3.0 * abs(a);
        float c = sdBox(r, vec3(1.0)) / s;
        d = max(d, c);
    }
    return d;
}

float mandelbulb(vec3 p) {
    vec3 z = p;
    float dr = 1.0;
    float r = 0.0;
    for (int i = 0; i < 10; i++) {
        r = length(z);
        if (r > 2.0) break;
        float theta = acos(z.z / r);
        float phi = atan(z.y, z.x);
        dr = pow(r, 8.0) * 8.0 * dr + 1.0;
        float zr = pow(r, 8.0);
        theta = theta * 8.0;
        phi = phi * 8.0;
        z = zr * vec3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
        z += p;
    }
    return 0.5 * log(r) * r / dr;
}

float apollonianGasket(vec3 p) {
    float scale = 2.0;
    float d = sdSphere(p, 1.0);
    for (int i = 0; i < 5; i++) {
        p = -1.0 + 2.0 * fract(0.5 * p + 0.5);
        float r = length(p);
        if (r < 1.0) {
            p /= r * r;
            d = min(d, sdSphere(p, 1.0 / scale));
        }
        scale *= 2.0;
    }
    return d;
}

// Scene mapping
float map(vec3 p) {
    float d = MAX_DIST;

    // Example: Combine shapes and fractals
    d = min(d, sdSphere(p - vec3(0.0, 0.0, -5.0), 1.0)); // Sphere
    d = min(d, sdBox(p - vec3(2.0, 0.0, -5.0), vec3(0.5))); // Box
    d = min(d, mengerSponge(p - vec3(-2.0, 0.0, -5.0))); // Menger Sponge
    d = min(d, mandelbulb(p - vec3(0.0, 2.0, -5.0))); // Mandelbulb
    d = min(d, apollonianGasket(p - vec3(0.0, -2.0, -5.0))); // Apollonian Gasket

    return d;
}

// Raymarching
float rayMarch(vec3 ro, vec3 rd) {
    float dist = 0.0;
    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 p = ro + rd * dist;
        float d = map(p);
        if (d < MIN_DIST || dist > MAX_DIST) break;
        dist += d;
    }
    return dist;
}

// Normal calculation
vec3 calcNormal(vec3 p) {
    const float eps = 0.001;
    float d = map(p);
    vec3 n = d - vec3(
        map(p - vec3(eps, 0.0, 0.0)),
        map(p - vec3(0.0, eps, 0.0)),
        map(p - vec3(0.0, 0.0, eps))
    );
    return normalize(n);
}

// Lighting
vec3 getLight(vec3 p, vec3 rd) {
    vec3 normal = calcNormal(p);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(rd, reflectDir), 0.0), 32.0);
    return vec3(diff + spec);
}

// Main
void main() {
    vec2 uv = (gl_FragCoord.xy - 0.5 * resolution.xy) / resolution.y;

    // Camera setup
    vec3 ro = camPos;
    vec3 rd = normalize(vec3(uv, -fov));

    // Raymarch
    float dist = rayMarch(ro, rd);
    vec3 p = ro + rd * dist;

    // Lighting
    vec3 color = vec3(0.0);
    if (dist < MAX_DIST) {
        color = getLight(p, rd);
    }

    fragColor = vec4(color, 1.0);
}
*/

