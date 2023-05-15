/////// Scene Elements ////////

mat4 rotationAxisAngle( vec3 v, float angle )
{
    float s = sin( angle );
    float c = cos( angle );
    float ic = 1.0 - c;

    return mat4( v.x*v.x*ic + c,     v.y*v.x*ic - s*v.z, v.z*v.x*ic + s*v.y, 0.0,
                 v.x*v.y*ic + s*v.z, v.y*v.y*ic + c,     v.z*v.y*ic - s*v.x, 0.0,
                 v.x*v.z*ic - s*v.y, v.y*v.z*ic + s*v.x, v.z*v.z*ic + c,     0.0,
			     0.0,                0.0,                0.0,                1.0 );
}

mat4 translate( float x, float y, float z )
{
    return mat4( 1.0, 0.0, 0.0, 0.0,
				 0.0, 1.0, 0.0, 0.0,
				 0.0, 0.0, 1.0, 0.0,
				 x,   y,   z,   1.0 );
}

mat4 translate( vec3 v )
{
    return mat4( 1.0, 0.0, 0.0, 0.0,
				 0.0, 1.0, 0.0, 0.0,
				 0.0, 0.0, 1.0, 0.0,
				 v.x, v.y, v.z, 1.0 );
}

/////////// Sphere ////////////

struct Sphere
{
    vec3 center;
    float radius;
};

vec3 random_point_on_sphere(vec3 center, float radius, vec3 o, inout float seed)
{
    vec3 direction = center - o;
    float distance_squared = dot(direction, direction);
    vec3[3] uvw = build_onb_from_w(direction);
    return onb_local(uvw, random_to_sphere(radius, distance_squared, seed));
}

bool hit_sphere(Sphere s, Ray r, float t_min, float t_max, inout HitRecord rec)
{
	vec3 oc = r.origin - s.center;
    float a = dot(r.direction, r.direction);
    float half_b = dot(oc, r.direction);
    float c = dot(oc, oc) - s.radius*s.radius;

    float discriminant = half_b*half_b - a*c;
    if (discriminant < 0) return false;
    float sqrtd = sqrt(discriminant);

    float root = (-half_b - sqrtd) / a;
    if (root < t_min || t_max < root)
    {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
            return false;
    }

    rec.t = root;
    rec.p = at(r,rec.t);
    set_face_normal(r, (rec.p - s.center) / s.radius, rec);

    return true;
}

///////////////////////////////

///////////// Box /////////////

struct Box
{
    vec3 minCorner;
    vec3 maxCorner;
    mat4 transformationMatrix;
};

bool hit_box(Box b, Ray r, float t_min, float t_max, inout HitRecord rec)
{
    vec3 center = (b.maxCorner + b.minCorner) * 0.5;
    mat4 invBoxMat = inverse(b.transformationMatrix);

    vec3 rd = (invBoxMat * vec4(r.direction,0.0)).xyz;
	vec3 ro = (invBoxMat * vec4(r.origin + center,1.0)).xyz;

    vec3 invDir = 1.0 / rd;
    vec3 f = (b.maxCorner - ro - center) * invDir;
    vec3 n = (b.minCorner - ro - center) * invDir;

    vec3 tmax = max(f, n);
    vec3 tmin = min(f, n);

    float t1 = min(t_max, min(tmax.x, min(tmax.y, tmax.z)));
    float t0 = max(t_min, max(tmin.x, max(tmin.y, tmin.z)));

    if(t1>=t0)
    {
        if(t0>0) rec.t = t0;
        else rec.t = t1;
    }
    else return false;
    rec.p = at(r,rec.t);
    
    vec3 point = (invBoxMat * vec4(rec.p + center, 1)).xyz;
    vec3 d = (b.maxCorner - b.minCorner) * 0.5;
    float bias = 1.000002;
    vec3 outward_normal = vec3(int(point.x / abs(d.x) * bias), int(point.y / abs(d.y) * bias), int(point.z / abs(d.z) * bias));
    outward_normal = (vec4(outward_normal, 0) * invBoxMat).xyz;
    set_face_normal(r, outward_normal, rec);
    
    return true;
}

///////////////////////////////

//////////// Plane ////////////

struct Plane
{
    float dist;
    vec3 normal;
};

bool hit_plane(Plane p, Ray r, float t_min, float t_max, inout HitRecord rec)
{
    float denom = dot(p.normal, r.direction);
    if (abs(denom) > 0.0001f)
    {
        float t = -(dot(p.normal, r.origin) + p.dist) / denom;
        if (t < t_min || t_max < t) return false;
        if (t > 0)
        {
            rec.t = t;
            rec.p = at(r,rec.t);
            set_face_normal(r, p.normal, rec);
            return true;
        }
    }
    return false;
}

///////////////////////////////

////////// Rectangle //////////

struct Rectangle
{
    vec2 minCorner;
    vec2 maxCorner;
    float dist;
    mat4 transformationMatrix;
};

bool hit_rectangle(Rectangle rect, Ray r, float t_min, float t_max, inout HitRecord rec)
{
    //vec3 center = vec3((rect.maxCorner + rect.minCorner) * 0.5, 0);
    //mat4 invMat = inverse(rect.transformationMatrix);

    //vec3 rd = (invMat * vec4(r.direction,0.0)).xyz;
	//vec3 ro = (invMat * vec4(r.origin + center,1.0)).xyz;
    
    vec3 rd = r.direction;
    vec3 ro = r.origin;

    vec3 invDir = 1.0 /  rd;
    float t = (rect.dist - ro.z) * invDir.z;
    if (t < t_min || t_max < t) return false;

    vec3 point = ro + rd * t;

    bool hit = point.x <= rect.maxCorner.x && point.x >= rect.minCorner.x &&
               point.y <= rect.maxCorner.y && point.y >= rect.minCorner.y;

    if(hit)
    {
        rec.t = t;
        rec.p = at(r,rec.t);
        set_face_normal(r, vec3(0,0,1), rec);
    }
    return hit;
}

///////////////////////////////

vec3 pdf_cosine_generate(vec3[3] uvw, inout float seed)
{
    return onb_local(uvw, random_cosine_direction(seed));
}

float pdf_cosine_value(vec3[3] uvw, vec3 direction)
{
    float cosine = dot(normalize(direction), uvw[2]);
    return (cosine <= 0) ? 0 : cosine / PI;
}

vec3 pdf_hittable_sphere_generate(Sphere s, vec3 o, inout float seed)
{
    vec3 direction = s.center - o;
    float distance_squared = dot(direction, direction);
    vec3[3] uvw = build_onb_from_w(direction);
    return onb_local(uvw, random_to_sphere(s.radius, distance_squared, seed));
}

float pdf_hittable_sphere_value(Sphere s, vec3 o, vec3 direction)
{
    HitRecord rec;
    if (!hit_sphere(s, Ray(o, direction, 0), 0.001, inf, rec)) return 0;

    vec3 dir = s.center - o;
    float cos_theta_max = sqrt(1 - s.radius * s.radius / dot(dir, dir));
    float solid_angle = 2 * PI * (1 - cos_theta_max);

    return  1 / solid_angle;
}