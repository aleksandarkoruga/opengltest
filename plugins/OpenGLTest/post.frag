#version 430 core

layout(location = 3) uniform vec2 resolution;

layout(location = 4) uniform sampler2D frame;

layout(location = 0) out vec4 fragColor;  // Output to the framebuffer


float grid(vec2 st, float res)
{
  vec2 grid = fract(st*res);
  return 1.0-(step(res,grid.x) * step(res,grid.y));
}


void main() {
    // Calculate aspect ratios
    float texAspect = 1.0; // 1080x1080 is square
    float winAspect = resolution.x / resolution.y;
    
    // Compute UV coordinates with letterboxing
    vec2 uv = gl_FragCoord.xy / resolution;
    uv = uv * 2.0 - 1.0; // Convert to [-1,1]

    if (winAspect > texAspect) {
        // Window is wider than texture: add vertical bars
        uv.x *= winAspect / texAspect;
    } else {
        // Window is taller than texture: add horizontal bars
        uv.y *= texAspect / winAspect;
        
    }

    uv = (uv + 1.0) * 0.5; // Convert back to [0,1]

    // Sample texture (clamp to avoid edge artifacts)
    vec4 data = texture(frame, clamp(uv, 0.0, 1.0)); 
    vec3 normal = normalize(vec3(-data.z, 0.2, -data.w));

    //val.x = float(val.x > 0.0)* val.x;
    //val.y = float(val.y < 0.0)* abs(val.y);
    //val.z =0;
    //val*=1.0; 

   
    float gridLines = clamp(grid((uv+vec2(0.5)+ 0.2 * data.zw) *200.0, 0.15),0.0,1.0)*0.2;
    
    fragColor = vec4(clamp((vec3(0,gridLines,gridLines)+(data.xzw)) ,0.0,1.0),1.0) ;
    fragColor += vec4(1) * pow(max(0.0, dot(normal, normalize(vec3(-3, 10, 3)))), 60.0);
}
