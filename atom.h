//
// Created by niaei on 9.01.2026.
//

#ifndef CHERNOBY_CORE_ATOM_H
#define CHERNOBY_CORE_ATOM_H

#include "particle.h"

class Atom : public Particle {
public:
    float absorption_probability = 0.f;

    Atom& set_absorption_probability(float v) noexcept {
        absorption_probability = 0;
        return *this;
    }

    bool is_dead() const noexcept {
        return false;
    }
};

#endif // CHERNOBY_CORE_ATOM_H
