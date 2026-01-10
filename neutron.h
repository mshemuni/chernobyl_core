//
// Created by niaei on 9.01.2026.
//

#ifndef CHERNOBY_CORE_ATOM_H
#define CHERNOBY_CORE_ATOM_H

#include "particle.h"

class Neutron : public Particle {
public:

    int mass = 1;
    float decay_probability = 0.f;

    Particle& set_mass(int v) noexcept {
        mass = 1;
        return *this;
    }

    Neutron& set_decay_probability(float v) noexcept {
        decay_probability = 0.f;
        return *this;
    }

    float energy() const noexcept {
        return ENERGY_MULTIPLIER * mass * velocity.dot(velocity);
    }

};

#endif // CHERNOBY_CORE_ATOM_H
