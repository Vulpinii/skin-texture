#version 450 core
layout (local_size_x = 16, local_size_y = 16) in;
layout (rgba32f, binding = 0) uniform image2D img_output;
layout (binding = 1) uniform sampler2D img_in;
layout (binding = 2) uniform sampler2D img_mask;
//layout (binding = 3) uniform sampler2D img_mask;

uniform vec2 sunPos;

vec4 getSample (in sampler2D img, in vec2 uv, in vec2 resolution){
    ivec2       position = ivec2(uv.xy);
    vec2        screenNormalized = vec2(position) / vec2(resolution);
    return      texture2D(img, screenNormalized);
}

vec2 getTexCoords (in vec2 uv, in vec2 resolution){
    ivec2       position = ivec2(uv.xy);
    return      vec2(position) / vec2(resolution);
}

const float exposure = 0.009f;
const float decay = 0.92;
const float density  = 0.905;
const float weight  = 0.36;

void main()
{
    vec4 pixel = vec4(0,0,0,1);     // final image pixel
    ivec2 pixel_coords = ivec2(gl_GlobalInvocationID);  // pixel coordinate
    ivec2 img_resolution = imageSize(img_output);       // image resolution
    vec2 st = pixel_coords.xy;      // fragCoords


    vec4 color =  vec4(0);
    color.a = 1.0;


    int NUM_SAMPLES = 80;
    vec2 tc = vec2(pixel_coords)/vec2(img_resolution);
    vec2 deltatexCoord = (tc - sunPos);
    deltatexCoord *= (1.0/ float(NUM_SAMPLES))*density;
    float illuminationDecay = 1.0f;

    vec4 godRayColor = vec4(0.0);//texture2D(img_mask , tc) ;//(texture2D(img_mask , tc) + texture2D(img_mask_sun, tc))/2.0f;
    for(int i = 0 ; i< NUM_SAMPLES ; i++)
    {
        tc-= deltatexCoord;
        vec4 samp = texture2D(img_mask , tc) ;//(texture2D(img_mask , tc) + texture2D(img_mask_sun, tc))/2.0f;
        samp *= illuminationDecay*weight;
        godRayColor += samp;
        illuminationDecay *= decay;
    }
    vec4 realColor = getSample(img_in, pixel_coords, img_resolution);
    color = vec4(vec3(godRayColor.r, godRayColor.g, godRayColor.b) * exposure, godRayColor.a) + realColor;


    //-------------[ REGISTER FINAL IMAGE ]-------------//
    imageStore(img_output, pixel_coords, color);
}