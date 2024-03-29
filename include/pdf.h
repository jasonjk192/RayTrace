#ifndef PDF_H
#define PDF_H

#include "common.h"

class pdf 
{
public:
    virtual ~pdf() {}

    virtual float value(const glm::vec3& direction) const = 0;
    virtual glm::vec3 generate() const = 0;
};

class cosine_pdf : public pdf 
{
public:
    cosine_pdf(const glm::vec3& w) { uvw.build_from_w(w); }

    virtual float value(const glm::vec3& direction) const override 
    {
        auto cosine = glm::dot(glm::normalize(direction), uvw.w());
        return (cosine <= 0) ? 0 : cosine / pi;
    }

    virtual glm::vec3 generate() const override
    {
        return uvw.local(random_cosine_direction());
    }

public:
    onb uvw;
};

class hittable_pdf : public pdf 
{
public:
    hittable_pdf(shared_ptr<hittable> p, const glm::vec3& origin) : ptr(p), o(origin) {}

    virtual float value(const glm::vec3& direction) const override {
        return ptr->pdf_value(o, direction);
    }

    virtual glm::vec3 generate() const override {
        return ptr->random(o);
    }

public:
    glm::vec3 o;
    shared_ptr<hittable> ptr;
};

class mixture_pdf : public pdf {
public:
    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) {
        p[0] = p0;
        p[1] = p1;
    }

    virtual float value(const glm::vec3& direction) const override {
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    virtual glm::vec3 generate() const override {
        if (random_float() < 0.5)
            return p[0]->generate();
        else
            return p[1]->generate();
    }

public:
    shared_ptr<pdf> p[2];
};

#endif