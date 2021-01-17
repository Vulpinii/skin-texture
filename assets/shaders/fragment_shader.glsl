#version 410 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 MaskColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;
uniform vec3 objectColor; // skin color
uniform vec3 freck_col;
uniform float freck_scale;
uniform float freck_frequency;

// math
const float PI = 3.14159265359;
const float DEG_TO_RAD = PI / 180.0;

float Random( in vec3 value){
    return fract(sin(dot(value,vec3(1274.0546,1156.01549,1422.65229)))*15554.0);
}

float Noise3D( in vec3 uv ){
    vec3 index = floor(uv);
    vec3 frac = fract(uv);

    float a = Random(index);
    float b = Random(index+vec3(1.0,0.0,0.0));
    float c = Random(index+vec3(0.0,1.0,0.0));
    float d = Random(index+vec3(1.0,1.0,0.0));

    float f = Random(index+vec3(0.0,0.0,1.0));
    float g = Random(index+vec3(1.0,0.0,1.0));
    float h = Random(index+vec3(0.0,1.0,1.0));
    float i = Random(index+vec3(1.0,1.0,1.0));

    frac = frac*frac*(3.0 - 2.0 * frac);

    return mix(mix(mix(a,b,frac.x),mix(c,d,frac.x),frac.y),
    mix(mix(f,g,frac.x),mix(h,i,frac.x),frac.y),
    frac.z);
}

float FBMNoise3D6( in vec3 uv){
    float fbm = Noise3D(uv*0.1)*0.5;
    fbm += Noise3D(uv*0.2)*0.25;
    fbm += Noise3D(uv*0.4)*0.125;
    fbm += Noise3D(uv*0.8)*0.0625;
    fbm += Noise3D(uv*0.16)*0.03125;
    fbm += Noise3D(uv*0.32)*0.03125;
    return fbm;
}
// based on
// https://colinbarrebrisebois.com/2011/03/07/gdc-2011-approximating-translucency-for-a-fast-cheap-and-convincing-subsurface-scattering-look/

float hash( float n )
{
    return fract(sin(n)*3538.5453);
}

float thickness( in vec3 p, in vec3 n, float maxDist, float falloff )
{
    const int nbIte = 7;
    const float nbIteInv = 1./float(nbIte);
    float ao = 0.0;

    for( int i=0; i<nbIte; i++ )
    {
        float l = hash(float(i))*maxDist;
        vec3 rd = normalize(-n)*l;
        ao += (l) / pow(1.+l, falloff);
    }

    return clamp( 1.-ao*nbIteInv, 0., 1.);
}

vec3 sss(vec3 skin, in vec3 p, in vec3 n, in vec3 ro, in vec3 rd )
{
    p.xz = mod(p.xz+100., 200.)-100.;

    vec3 ldir1 = normalize(lightPos-p);
    float latt1 = pow( length(lightPos-p)*.15, 3. ) / (pow(1.125-FBMNoise3D6(p*100.0f), 0.25)*1.45+.35);
    float thi = thickness(p, n, 6., 0.6);
    vec3 diff1 = lightColor * (max(dot(n,ldir1),0.) ) / latt1;

    vec3 col =  diff1;
    float trans1 =  pow( clamp( dot(-rd, -ldir1+n), 0., 1.), 1.) + 1.;
    col = skin * 0.008*(trans1/latt1)*thi ;
    return col;//max(col, col * FBMNoise3D6(p*100.0f));
}


//----SKIN----
// Partially inspired by works of Joseph Kubiak
float subsurface_scale = 1.1;
float subsurface_frequency = 2.2;
float skin_scale = 0.5;
float skin_frequency = 50.0;
float base_skin_amt = 0.98;
vec3 subsurface_color = vec3(0.639, 0.058, 0);
vec3 surface_col = vec3(1.0, 1.0, 1.0);

// Simplex 2D noise by Inigo Quilez
vec2 hash( vec2 p ) // replace this by something better
{
    p = vec2( dot(p,vec2(127.1,311.7)),
    dot(p,vec2(269.5,183.3)) );

    return -1.0 + 2.0*fract(sin(p)*43758.5453123);
}
float noise( in vec2 p )
{
    const float K1 = 0.366025404; // (sqrt(3)-1)/2;
    const float K2 = 0.211324865; // (3-sqrt(3))/6;

    vec2 i = floor( p + (p.x+p.y)*K1 );

    vec2 a = p - i + (i.x+i.y)*K2;
    vec2 o = step(a.yx,a.xy);
    vec2 b = a - o + K2;
    vec2 c = a - 1.0 + 2.0*K2;

    vec3 h = max( 0.5-vec3(dot(a,a), dot(b,b), dot(c,c) ), 0.0 );

    vec3 n = h*h*h*h*vec3( dot(a,hash(i+0.0)), dot(b,hash(i+o)), dot(c,hash(i+1.0)));

    return dot( n, vec3(70.0) );

}
float n_noise(in vec2 p)
{
    return 0.5 + 0.5 * noise(p);
}

//	Simplex 3D Noise by Ian McEwan, Ashima Arts
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}
float snoise(vec3 v){
    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy) );
    vec3 x0 =   v - i + dot(i, C.xxx) ;

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );

    //  x0 = x0 - 0. + 0.0 * C
    vec3 x1 = x0 - i1 + 1.0 * C.xxx;
    vec3 x2 = x0 - i2 + 2.0 * C.xxx;
    vec3 x3 = x0 - 1. + 3.0 * C.xxx;

    // Permutations
    i = mod(i, 289.0 );
    vec4 p = permute( permute( permute(
    i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
    + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
    + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

    // Gradients
    // ( N*N points uniformly over a square, mapped onto an octahedron.)
    float n_ = 1.0/7.0; // N=7
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );

    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);

    //Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 42.0*dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
    dot(p2,x2), dot(p3,x3) ) );
}



void main()
{
    //----------------------------[ HUMAN SKIN ]----------------------------//
    float subsurface_radius = subsurface_scale / 2.0;
    float freck_radius = freck_scale / 2.0;
    vec3 uv = FragPos*10.0f;
    float subsurface_distance = snoise(uv * subsurface_frequency);
    float subsurface = 1.0 - min(1.0, subsurface_distance / subsurface_radius);
    float skin_value = snoise(uv * skin_frequency)/42.0 ;//* skin_scale;
    float freck_distance = n_noise(FragPos.zy/FragPos.x * freck_frequency);
    float freck = 1.0 - min(1.0, freck_distance / freck_radius);
    vec3 col = (subsurface_color * subsurface);
    col = mix(col,  objectColor , base_skin_amt);
    col  = mix(col, surface_col, skin_value);
    col = mix(col, freck_col, freck);

    //-----------------------------[ LIGHTING ]-----------------------------//
    // basic lighgting
    float ambientStrength = 0.001; vec3 ambient = ambientStrength * lightColor;
    vec3 norm = normalize(Normal); vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0); vec3 diffuse = diff * lightColor;
    float specularStrength = 0.01; vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;
    // SubSurface Scattering lighting
    vec3 sssCol = sss(objectColor, FragPos, Normal, viewPos, normalize(viewPos - FragPos));
    FragColor = vec4(mix(sssCol, sssCol*lightColor, 0.85) + ambient + specular,1.0);

    FragColor.xyz *= col;
    MaskColor = vec4(0,0,0,1);
}