float cosinepdf(HitRecord rec, Ray scattered)
{
	float cosine = dot(rec.normal, unit_vector(scattered.direction));
    return cosine < 0 ? 0 : cosine/pi;
}