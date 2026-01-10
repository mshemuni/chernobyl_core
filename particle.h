#ifndef CHERNOBY_CORE_PARTICLE_H
#define CHERNOBY_CORE_PARTICLE_H

#include "v2d.h"
#include <cmath>
#include <random>
#include <optional>
#include <vector>
#include <tuple>
#include <algorithm>

class Particle {
public:
    // --- State ---
    V2D position;
    V2D velocity;
    V2D acceleration;

    static constexpr float ENERGY_POWER = 0.7f;
    static constexpr float ENERGY_MULTIPLIER = 10.f;

    // --- Physical / lifecycle parameters ---
    float time_lived = 0.f;
    float time_to_live = 1.f;

    int mass = 1;
    float sigma = 1.f;

    float attraction_strength = 0.f;
    float absorption_probability = 0.1f;
    float decay_probability = 0.f;
    bool unstable = false;

    // --- Construction ---
    explicit Particle(
        const V2D& position,
        V2D velocity = V2D::random(),
        V2D acceleration = V2D()
    ) noexcept
        : position(position),
          velocity(velocity),
          acceleration(acceleration)
    {}

    // ============================================================
    // Setters (fluent, validated)
    // ============================================================

    Particle& set_position(const V2D& p) noexcept {
        position = p;
        return *this;
    }

    Particle& set_velocity(const V2D& v) noexcept {
        velocity = v;
        return *this;
    }

    Particle& set_acceleration(const V2D& a) noexcept {
        acceleration = a;
        return *this;
    }

    Particle& set_time_lived(float v) noexcept {
        time_lived = (v < 0.f) ? 0.f : v;
        return *this;
    }

    Particle& set_time_to_live(float v) noexcept {
        if (v > 0.f) time_to_live = v;
        return *this;
    }

    Particle& set_mass(int v) noexcept {
        if (v > 0) mass = v;
        return *this;
    }


    Particle& set_sigma(float v) noexcept {
        if (v > 0.f) sigma = v;
        return *this;
    }

    Particle& set_attraction_strength(float v) noexcept {
        if (v >= 0.f) attraction_strength = v;
        return *this;
    }

    Particle& set_absorption_probability(float v) noexcept {
        if (v < 0.f) v = 0.f;
        if (v > 1.f) v = 1.f;
        absorption_probability = v;
        return *this;
    }

    Particle& set_decay_probability(float v) noexcept {
        if (v < 0.f) v = 0.f;
        if (v > 1.f) v = 1.f;
        decay_probability = v;
        return *this;
    }

    Particle& set_unstable(bool v) noexcept {
        unstable = v;
        return *this;
    }

    // ============================================================
    // Derived quantities
    // ============================================================

    float radius() const noexcept {
        return std::sqrt(
            static_cast<float>(mass) /
            (static_cast<float>(M_PI) * sigma)
        );
    }

    float energy() const noexcept {
        return ENERGY_MULTIPLIER *
               std::pow(static_cast<float>(mass), ENERGY_POWER);
    }

    float stiffness() const noexcept {
        return 1.f / energy();
    }

    // ============================================================
    // Lifecycle
    // ============================================================

    void age(float dt) noexcept {
        time_lived += dt;
    }

    bool is_dead() const noexcept {
        return time_lived >= time_to_live;
    }

    bool is_outside(int width, int height) const noexcept {
        return position.x <= 0.f || position.x >= width ||
               position.y <= 0.f || position.y >= height;
    }

    // ============================================================
    // Motion / integration
    // ============================================================

    void integrate(float dt) noexcept {
        velocity = velocity.add(acceleration.scale(dt));
        position = position.add(velocity.scale(dt));
        acceleration = V2D();
    }

    // ============================================================
    // Interaction
    // ============================================================

    bool is_collided(const Particle& other) const noexcept {
        float d = position.distance(other.position);
        return d <= (radius() + other.radius());
    }

    void attract_to(const Particle& other) noexcept {
        V2D dir = other.position.subtract(position);
        float dist = dir.magnitude();

        if (dist <= 0.f)
            return;

        constexpr float SOFTENING = 0.01f;

        float accel_mag =
            attraction_strength * static_cast<float>(other.mass) /
            (dist * dist + SOFTENING);

        acceleration = acceleration.add(
            dir.unit().scale(accel_mag)
        );
    }

    void bounce(Particle& other, float restitution = 1.f) noexcept {
        if (!is_collided(other))
            return;

        if (mass <= 0 || other.mass <= 0)
            return;

        V2D normal = position.subtract(other.position).unit();
        V2D rel_vel = velocity.subtract(other.velocity);

        float vel_n = rel_vel.dot(normal);
        if (vel_n > 0.f)
            return;

        float inv_m1 = 1.f / static_cast<float>(mass);
        float inv_m2 = 1.f / static_cast<float>(other.mass);

        float j =
            -(1.f + restitution) * vel_n /
            (inv_m1 + inv_m2);

        V2D impulse = normal.scale(j);

        velocity = velocity.add(impulse.scale(inv_m1));
        other.velocity = other.velocity.subtract(impulse.scale(inv_m2));
    }

    // ============================================================
    // Fission (mass-based)
    // ============================================================

    using FissionSplit = std::tuple<int,int,int,float>;

    std::vector<FissionSplit> bohr_wheeler(std::optional<float> energy_input = std::nullopt) const
    {
        if (mass < 3) {
            return {
                FissionSplit{ mass, 0, 0, 1.0f }
            };
        }

        float deltaA0 = 0.18f * std::pow(mass, 2.f / 3.f);
        if (energy_input.has_value()) {
            float symmetry_factor =
                std::max(0.6f, 1.f - energy_input.value() / 100.f);
            deltaA0 *= symmetry_factor;
        }

        float base_nu = 2.5f;
        if (energy_input.has_value())
            base_nu += 0.02f * energy_input.value();

        float max_delta = deltaA0 * 2.f;
        float step = std::max(1.f, deltaA0 / 3.f);

        std::vector<FissionSplit> results;
        std::vector<float> weights;

        for (float deltaA = -max_delta; deltaA <= max_delta; deltaA += step) {
            int nu = static_cast<int>(
                std::round(base_nu + std::abs(deltaA) * 0.1f)
            );

            int A1 = static_cast<int>(
                std::round((mass - nu) / 2.f + deltaA)
            );
            int A2 = (mass - nu) - A1;

            if (A1 <= 0 || A2 <= 0)
                continue;

            float weight =
                std::exp(-(deltaA * deltaA) /
                         (2.f * deltaA0 * deltaA0));

            results.emplace_back(A1, A2, nu, weight);
            weights.push_back(weight);
        }

        float total_weight = 0.f;
        for (float w : weights) total_weight += w;

        std::vector<FissionSplit> normalized;
        for (size_t i = 0; i < results.size(); ++i) {
            auto [A1, A2, nu, _] = results[i];
            normalized.emplace_back(
                A1, A2, nu, weights[i] / total_weight
            );
        }

        return normalized;
    }

    FissionSplit split(std::optional<float> energy_input = std::nullopt) const {
        auto splits = bohr_wheeler(energy_input);

        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(0.f, 1.f);

        float r = dist(gen);
        float cum_prob = 0.f;

        for (const auto& s : splits) {
            cum_prob += std::get<3>(s);
            if (r <= cum_prob)
                return s;
        }

        return splits.back();
    }


    // ============================================================
    // Factory helpers
    // ============================================================

    static Particle random() {
        return Particle(V2D::random());
    }

    static Particle from_values(float x, float y) {
        return Particle(V2D(x, y));
    }
};

#endif // CHERNOBY_CORE_PARTICLE_H
