#version 430 core

layout(location = 0) uniform int ssboSize; // Size of the SSBO (number of elements)
layout(location = 3) uniform vec2 resolution;
layout(location = 4) uniform sampler2D previousFrame;
layout(location = 5) uniform sampler2D previousFrame2;

layout(std430, binding = 2) buffer DataBuffer {
    float data[];
};

layout(location = 0) out vec4 fragColor;  // Output to the framebuffer



// Wave equation parameters
const float past = 0.0001;     
const float nonlinear = 0.005;
const float damping = 0.999; // Slight damping to prevent infinite energy
const float delta = 1.0;

void main()
{
    vec2 coord = gl_FragCoord.xy / resolution;

    // Read previous states (no clamping to [-1, 1] here)
    vec3 u_prev = texture(previousFrame, coord).rgb ;  // u(t-Δt)
    vec3 u_prev2 = texture(previousFrame2, coord).rgb ; // u(t-2Δt)

    float pressure = u_prev.x;
    float pVel = u_prev.y;

    // Spatial derivatives (Laplacian)
    float p_left = texture(previousFrame, coord - vec2(1.0 / resolution.x, 0.0)).x ;
    float p_right = texture(previousFrame, coord + vec2(1.0 / resolution.x, 0.0)).x ;
    float p_up = texture(previousFrame, coord + vec2(0.0, 1.0 / resolution.y)).x ;
    float p_down = texture(previousFrame, coord - vec2(0.0, 1.0 / resolution.y)).x ;
  

     // Apply horizontal wave function
    pVel += delta*0.25 * (-4.0 * pressure + p_right + p_left + p_up + p_down + u_prev2.y * past)  ;
    
    // Change pressure by pressure velocity
    pressure += delta * pVel + u_prev2.x * past + nonlinear*(cos(u_prev2.y*10.0));
    
    // "Spring" motion. This makes the waves look more like water waves and less like sound waves.
    pVel -= 0.005 * delta * pressure;
    
    // Velocity damping so things eventually calm down
    pVel *= 1.0 - 0.002 * delta;
    
    // Pressure damping to prevent it from building up forever.
    pressure *= damping;
    
    

    // Add excitation from SSBO (optional)
    int index = int(gl_FragCoord.x / resolution.x * ssboSize);
    float excitation = abs( (coord.y-0.5) - 0.25*data[index]);
    excitation=(1.0-clamp(excitation, 0.0, 1.0));
    excitation = step(0.99,excitation);
    //float excitation = (data[index]) * 0.1;
    //excitation = excitation * float(coord.x < 0.51)*float(coord.x > 0.49)* float(coord.y < 0.51)*float(coord.y > 0.49);
    pressure+=excitation;
    //x = pressure. y = pressure velocity. Z and W = X and Y gradient
    fragColor = vec4(pressure, pVel, (p_right - p_left) / 2.0, (p_up - p_down) / 2.0);
}


/*
void mainOld()
{
    vec2 coord = gl_FragCoord.xy / resolution;

    // Read previous states (no clamping to [-1, 1] here)
    vec3 u_prev = texture(previousFrame, coord).rgb ;  // u(t-Δt)
    vec3 u_prev2 = texture(previousFrame2, coord).rgb ; // u(t-2Δt)

    // Spatial derivatives (Laplacian)
    vec3 u_left = texture(previousFrame, coord - vec2(1.0 / resolution.x, 0.0)).rgb ;
    vec3 u_right = texture(previousFrame, coord + vec2(1.0 / resolution.x, 0.0)).rgb ;
    vec3 u_up = texture(previousFrame, coord + vec2(0.0, 1.0 / resolution.y)).rgb ;
    vec3 u_down = texture(previousFrame, coord - vec2(0.0, 1.0 / resolution.y)).rgb ;

    vec3 laplacian = (u_left + u_right + u_up + u_down - 4.0 * u_prev);

    // Wave equation update: u(t) = 2u(t-Δt) - u(t-2Δt) + c²Δt²/Δx² * laplacian(u)
    vec3 u_new = 2.0 * u_prev - u_prev2 + (c * c * dt * dt / (dx * dx)) * laplacian;

    // Apply damping to prevent infinite oscillations
    u_new *= damping;

    // Add excitation from SSBO (optional)
    int index = int(gl_FragCoord.x / resolution.x * ssboSize);
    //float excitation = abs( (coord.y-0.5) - 0.0*data[index]);
    //excitation=(1.0-clamp(excitation, 0.0, 1.0));
    //excitation = step(0.99,excitation);
    float excitation = (data[index]) * 0.1;
    excitation = excitation * float(coord.x < 0.51)*float(coord.x > 0.49)* float(coord.y < 0.51)*float(coord.y > 0.49);


    u_new = (float(abs(excitation)>=0.0001)*vec3(excitation*0.01)) + (   u_new*(float(abs(excitation)<0.0001))  );

    //u_new += 0.01*vec3( cos(laplacian.y*3.1*2*3.14),cos(laplacian.z*2.4*2*3.14),cos(laplacian.x*2*3.14)  );

    // Store the new state in [-1, 1] (no clamping here)
    fragColor = vec4(u_new,1.0);
}
*/