
#pragma once

#include <variant>

#include "../lib/mathlib.h"
#include "../lib/spectrum.h"
#include "../util/hdr_image.h"

#include "samplers.h"

namespace PT {

struct BSDF_Sample {

    Spectrum emissive;
    Spectrum attenuation;
    Vec3 direction;
    float pdf;

    void transform(const Mat4& T) {
        direction = T.rotate(direction);
    }
};

struct BSDF_Lambertian {

    BSDF_Lambertian(Spectrum albedo) : albedo(albedo) {
    }

    BSDF_Sample sample(Vec3 out_dir) const;
    Spectrum evaluate(Vec3 out_dir, Vec3 in_dir) const;

    Spectrum albedo;
    Samplers::Hemisphere::Uniform sampler;
};

struct BSDF_Mirror {

    BSDF_Mirror(Spectrum reflectance) : reflectance(reflectance) {
    }

    BSDF_Sample sample(Vec3 out_dir) const;
    Spectrum evaluate(Vec3 out_dir, Vec3 in_dir) const;

    Spectrum reflectance;
};

struct BSDF_Refract {

    BSDF_Refract(Spectrum transmittance, float ior)
        : transmittance(transmittance), index_of_refraction(ior) {
    }

    BSDF_Sample sample(Vec3 out_dir) const;
    Spectrum evaluate(Vec3 out_dir, Vec3 in_dir) const;

    Spectrum transmittance;
    float index_of_refraction;
};

struct BSDF_Glass {

    BSDF_Glass(Spectrum transmittance, Spectrum reflectance, float ior)
        : transmittance(transmittance), reflectance(reflectance), index_of_refraction(ior) {
    }

    BSDF_Sample sample(Vec3 out_dir) const;
    Spectrum evaluate(Vec3 out_dir, Vec3 in_dir) const;

    Spectrum transmittance;
    Spectrum reflectance;
    float index_of_refraction;
};

struct BSDF_Diffuse {

    BSDF_Diffuse(Spectrum radiance) : radiance(radiance) {
    }

    BSDF_Sample sample(Vec3 out_dir) const;
    Spectrum evaluate(Vec3 out_dir, Vec3 in_dir) const;

    Spectrum radiance;
    Samplers::Hemisphere::Uniform sampler;
};

struct BSDF_Phong {

    BSDF_Phong(Spectrum albedo, float glossiness) : albedo(albedo), glossiness(glossiness) {
    }

    BSDF_Sample sample(Vec3 out_dir) const;
    Spectrum evaluate(Vec3 out_dir, Vec3 in_dir) const;

    Spectrum albedo;
    float glossiness;
};

struct BSDF_Blinn {

    BSDF_Blinn(Spectrum albedo, float glossiness) : albedo(albedo), glossiness(glossiness) {
    }

    BSDF_Sample sample(Vec3 out_dir) const;
    Spectrum evaluate(Vec3 out_dir, Vec3 in_dir) const;

    Spectrum albedo;
    float glossiness;
};

struct BSDF_Retro {

    BSDF_Retro(Spectrum reflectance, float glossiness) : reflectance(reflectance), glossiness(glossiness) {
    }

    BSDF_Sample sample(Vec3 out_dir) const;
    Spectrum evaluate(Vec3 out_dir, Vec3 in_dir) const;

    Spectrum reflectance;
    float glossiness;
};

struct BSDF_Metal {

    BSDF_Metal(Spectrum reflectance, float glossiness) : reflectance(reflectance), glossiness(glossiness) {
    }

    BSDF_Sample sample(Vec3 out_dir) const;
    Spectrum evaluate(Vec3 out_dir, Vec3 in_dir) const;

    Spectrum reflectance;
    float glossiness;
};

class BSDF {
public:
    BSDF(BSDF_Lambertian&& b) : underlying(std::move(b)) {
    }
    BSDF(BSDF_Mirror&& b) : underlying(std::move(b)) {
    }
    BSDF(BSDF_Glass&& b) : underlying(std::move(b)) {
    }
    BSDF(BSDF_Diffuse&& b) : underlying(std::move(b)) {
    }
    BSDF(BSDF_Refract&& b) : underlying(std::move(b)) {
    }
    BSDF(BSDF_Phong&& b) : underlying(std::move(b)) {
    }
    BSDF(BSDF_Blinn&& b) : underlying(std::move(b)) {
    }
    BSDF(BSDF_Retro&& b) : underlying(std::move(b)) {
    }
    BSDF(BSDF_Metal&& b) : underlying(std::move(b)) {
    }

    BSDF(const BSDF& src) = delete;
    BSDF& operator=(const BSDF& src) = delete;
    BSDF& operator=(BSDF&& src) = default;
    BSDF(BSDF&& src) = default;

    BSDF_Sample sample(Vec3 out_dir) const {
        return std::visit(overloaded{[&out_dir](const auto& b) { return b.sample(out_dir); }},
                          underlying);
    }

    Spectrum evaluate(Vec3 out_dir, Vec3 in_dir) const {
        return std::visit(
            overloaded{[&out_dir, &in_dir](const auto& b) { return b.evaluate(out_dir, in_dir); }},
            underlying);
    }

    bool is_discrete() const {
        return std::visit(overloaded{[](const BSDF_Lambertian&) { return false; },
                                     [](const BSDF_Mirror&) { return true; },
                                     [](const BSDF_Glass&) { return true; },
                                     [](const BSDF_Diffuse&) { return false; },
                                     [](const BSDF_Refract&) { return true; },
                                     [](const BSDF_Phong&) { return true; },
                                     [](const BSDF_Blinn&) { return true; },
                                     [](const BSDF_Retro&) { return true; },
                                     [](const BSDF_Metal&) {return true;}},
                          underlying);
    }

    bool is_sided() const {
        return std::visit(overloaded{[](const BSDF_Lambertian&) { return false; },
                                     [](const BSDF_Mirror&) { return false; },
                                     [](const BSDF_Glass&) { return true; },
                                     [](const BSDF_Diffuse&) { return false; },
                                     [](const BSDF_Refract&) { return true; },
                                     [](const BSDF_Phong&) { return true; },
                                     [](const BSDF_Blinn&) { return true; },
                                     [](const BSDF_Retro&) { return true; },
                                     [](const BSDF_Metal&) { return true;}},
                          underlying);
    }

private:
    std::variant<BSDF_Lambertian, BSDF_Mirror, BSDF_Glass, BSDF_Diffuse, BSDF_Refract, BSDF_Phong, BSDF_Blinn, BSDF_Retro, BSDF_Metal> underlying;
};

Vec3 reflect(Vec3 dir);
Vec3 refract(Vec3 out_dir, float index_of_refraction, bool& was_internal);

} // namespace PT
