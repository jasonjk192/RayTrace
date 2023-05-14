/////// Scene Elements ////////

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
};

bool hit_box(Box b, Ray r, float t_min, float t_max, inout HitRecord rec)
{
    vec3 invDir = 1.0 / r.direction;

    vec3 f = (b.maxCorner - r.origin) * invDir;
    vec3 n = (b.minCorner - r.origin) * invDir;

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

    vec3 center = (b.maxCorner + b.minCorner) * 0.5;
    vec3 point = rec.p - center;
    vec3 d = (b.maxCorner - b.minCorner) * 0.5;
    float bias = 1.000001;
    vec3 outward_normal = vec3(int(point.x / abs(d.x) * bias), int(point.y / abs(d.y) * bias), int(point.z / abs(d.z) * bias));

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