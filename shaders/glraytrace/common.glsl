//////////// Camera ////////////

struct Camera
{
  vec3 origin;
  vec3 horizontal;
  vec3 vertical;
  vec3 lower_left_corner;
};

////////////////////////////////

///////////// Ray //////////////

struct Ray
{
  vec3 origin;
  vec3 direction;
  float time;
};

vec3 at(Ray r, float dist) { return r.direction * dist + r.origin; }

////////////////////////////////

////////// Material ////////////

struct Material
{
    int type;
    vec3 albedo;
    float fuzz;
    float ir;
};

///////////////////////////////

///////// Hit Record //////////

struct HitRecord
{
  vec3 p;
  vec3 normal;
  float t;
  bool front_face;
  Material material;
};

void set_face_normal(Ray r, vec3 outward_normal, inout HitRecord rec)
{
  rec.front_face = dot(r.direction, outward_normal) < 0;
  rec.normal = (rec.front_face) ? outward_normal : -1 * outward_normal;
}

///////////////////////////////

/////// Scatter Record ////////

struct ScatterRecord {
    Ray specular_ray;
    bool is_specular;
    vec3 attenuation;
    float pdf;
};

///////////////////////////////