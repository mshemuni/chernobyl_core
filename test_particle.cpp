//
// Created by niaei on 10.01.2026
//

#include "particle.h"
#include <cassert>
#include <cmath>
#include <iostream>

constexpr float EPS = 1e-5f;

bool feq(float a, float b, float eps = EPS) {
    return std::fabs(a - b) < eps;
}

int main() {

    // ============================================================
    // Construction & setters
    // ============================================================

    {
        Particle p(V2D(1.f, 2.f));
        assert(p.mass == 1);

        p.set_mass(10);
        assert(p.mass == 10);

        p.set_mass(-5);   // invalid
        assert(p.mass == 10);  // unchanged
    }

    // ============================================================
    // Derived quantities
    // ============================================================

    {
        Particle p(V2D(0.f, 0.f));
        p.set_mass(9);
        p.set_sigma(1.f);

        float expected_radius = std::sqrt(9.f / static_cast<float>(M_PI));
        assert(feq(p.radius(), expected_radius));

        float expected_energy =
            Particle::ENERGY_MULTIPLIER *
            std::pow(9.f, Particle::ENERGY_POWER);

        assert(feq(p.energy(), expected_energy));
        assert(feq(p.stiffness(), 1.f / expected_energy));
    }

    // ============================================================
    // Lifecycle
    // ============================================================

    {
        Particle p(V2D(0.f, 0.f));
        p.set_time_to_live(2.f);

        p.age(1.f);
        assert(!p.is_dead());

        p.age(1.f);
        assert(p.is_dead());
    }

    // ============================================================
    // Integration
    // ============================================================

    {
        Particle p(V2D(0.f, 0.f), V2D(1.f, 0.f));
        p.set_acceleration(V2D(1.f, 0.f));

        p.integrate(1.f);

        assert(feq(p.velocity.x, 2.f));
        assert(feq(p.position.x, 2.f));
        assert(feq(p.acceleration.magnitude(), 0.f));
    }

    // ============================================================
    // Collision detection
    // ============================================================

    {
        Particle a(V2D(0.f, 0.f));
        Particle b(V2D(0.1f, 0.f));

        a.set_mass(1);
        b.set_mass(1);

        assert(a.is_collided(b));
    }

    // ============================================================
    // Attraction
    // ============================================================

    {
        Particle a(V2D(0.f, 0.f));
        Particle b(V2D(1.f, 0.f));

        a.set_attraction_strength(1.f);
        b.set_mass(10);

        a.attract_to(b);

        assert(a.acceleration.x > 0.f);
        assert(feq(a.acceleration.y, 0.f));
    }

    // ============================================================
    // Elastic bounce (1D sanity)
    // ============================================================

    {
        Particle a(V2D(0.f, 0.f), V2D(1.f, 0.f));
        Particle b(V2D(0.5f, 0.f), V2D(-1.f, 0.f));

        a.set_mass(1);
        b.set_mass(1);

        a.bounce(b, 1.f);

        // velocities should swap (equal masses, elastic)
        assert(a.velocity.x < 0.f);
        assert(b.velocity.x > 0.f);
    }

    // ============================================================
    // Fission: Bohr–Wheeler
    // ============================================================

    {
        Particle p(V2D(0.f, 0.f));
        p.set_mass(236);

        auto splits = p.bohr_wheeler();
        assert(!splits.empty());

        float prob_sum = 0.f;
        for (auto& [A1, A2, nu, prob] : splits) {
            assert(A1 > 0);
            assert(A2 > 0);
            assert(nu >= 0);
            assert(feq(A1 + A2 + nu, 236.f));
            prob_sum += prob;
        }

        assert(feq(prob_sum, 1.f, 1e-3f));
    }

    // ============================================================
    // Fission: stochastic split
    // ============================================================

    {
        Particle p(V2D(0.f, 0.f));
        p.set_mass(235);

        auto [A1, A2, nu, prob] = p.split();

        assert(A1 > 0);
        assert(A2 > 0);
        assert(nu >= 0);
        assert(feq(static_cast<float>(A1 + A2 + nu), 235.f));
        assert(prob > 0.f && prob <= 1.f);
    }

    // ============================================================
    // Fission invalid mass
    // ============================================================

    {
        Particle p(V2D(0.f, 0.f));
        p.set_mass(1);

        bool thrown = false;
        try {
            p.bohr_wheeler();
        } catch (const std::invalid_argument&) {
            thrown = true;
        }

        assert(thrown);
    }

    std::cout << "All Particle tests passed \n";
    return 0;
}
