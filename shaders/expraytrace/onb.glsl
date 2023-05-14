//////////// ONB //////////////

vec3[3] build_onb_from_w(vec3 n)
{
    vec3[3] axis;
    axis[2] = normalize(n);
    vec3 a = (abs(axis[2].x) > 0.9) ? vec3(0,1,0) : vec3(1,0,0);
    axis[1] = normalize(cross(axis[2], a));
    axis[0] = cross(axis[2], axis[1]);
    return axis;
}

vec3 onb_local(vec3[3] uvw, vec3 a)
{
    return a.x * uvw[0] + a.y * uvw[1] + a.z * uvw[2];
}

///////////////////////////////