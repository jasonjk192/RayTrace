//////////////////// Random number generation ////////////////////

uint baseHash(uvec2 p)
{
    p = 1103515245U * ((p >> 1U) ^ (p.yx));
    uint h32 = 1103515245U * ((p.x) ^ (p.y>>3U));
    return h32 ^ (h32 >> 16);
}

float hash1(inout float seed) {
    uint n = baseHash(floatBitsToUint(vec2(seed += 0.1,seed += 0.1)));
    return float(n) / float(0xffffffffU);
}

vec2 hash2(inout float seed) {
    uint n = baseHash(floatBitsToUint(vec2(seed += 0.1,seed += 0.1)));
    uvec2 rz = uvec2(n, n * 48271U);
    return vec2(rz.xy & uvec2(0x7fffffffU)) / float(0x7fffffff);
}

vec3 hash3(inout float seed)
{
    uint n = baseHash(floatBitsToUint(vec2(seed += 0.1, seed += 0.1)));
    uvec3 rz = uvec3(n, n * 16807U, n * 48271U);
    return vec3(rz & uvec3(0x7fffffffU)) / float(0x7fffffff);
}

uint lowbias32(uint x)
{
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

float lowbiashash(uint seed) { return float(lowbias32(seed)) / float(0xffffffffU); }

float random(vec2 st)
{
    //return fract(sin(dot(st.xy, vec2(12.9898,78.233))) * 43758.5453123);
    uvec2 ST = uvec2(st);
    return lowbiashash( ST.x + lowbias32(ST.y));
}

float random(vec2 st, float amin, float amax)
{
    return amin + (amax-amin)*random(st);
}

vec3 random_vec3(vec2 st)
{
    return vec3(random(st), random(st * 0.33), random(st * 0.54));
}

vec3 random_vec3(vec2 st, float amin, float amax)
{
    //return vec3(random(st, amin, amax), random(st * 0.13, amin, amax), random(st * 0.14, amin, amax));
    return (amax - amin) * random_vec3(st) + vec3(amin);
}

vec3 random_in_unit_sphere(vec2 st)
{ 
    return normalize(random_vec3(st, -1, 1));
}

vec3 random_in_unit_sphere(inout float seed)
{
    vec3 h = hash3(seed) * vec3(2.0, 6.28318530718, 1.0) - vec3(1.0, 0.0, 0.0);
    float phi = h.y;
    float r = pow(h.z, 1.0/3.0);
	return r * vec3(sqrt(1.0 - h.x * h.x) * vec2(sin(phi), cos(phi)), h.x);
}

vec3 random_in_hemisphere(vec3 normal, vec2 st)
{
    vec3 in_unit_sphere = random_in_unit_sphere(st);
    if (dot(in_unit_sphere, normal) > 0.0)
        return in_unit_sphere;
    else
        return -in_unit_sphere;
}

vec3 random_in_hemisphere(vec3 normal, inout float seed)
{
    vec3 in_unit_sphere = random_in_unit_sphere(seed);
    if (dot(in_unit_sphere, normal) > 0.0)
        return in_unit_sphere;
    else
        return -in_unit_sphere;
}

vec3 random_to_sphere(float radius, float distance_squared, inout float seed)
{
    float r1 = hash1(seed);
    float r2 = hash1(seed);
    float z = 1 + r2 * (sqrt(1 - radius * radius / distance_squared) - 1);

    float phi = 2 * PI * r1;
    float x = cos(phi) * sqrt(1 - z * z);
    float y = sin(phi) * sqrt(1 - z * z);

    return vec3(x, y, z);
}

vec3 random_cosine_direction(inout float seed)
{
    float r1 = hash1(seed);
    float r2 = hash1(seed);
    float z = sqrt(1-r2);

    float phi = 2*PI*r1;
    float x = cos(phi)*sqrt(r2);
    float y = sin(phi)*sqrt(r2);

    return vec3(x, y, z);
}

//////////////////////////////////////////////////////////////////