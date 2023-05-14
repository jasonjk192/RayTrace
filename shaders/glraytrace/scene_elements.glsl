/////// Scene Elements ////////

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