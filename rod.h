#ifndef CHERNOBY_CORE_ROD_H
#define CHERNOBY_CORE_ROD_H

#include "v2d.h"
#include "particle.h"
#include <algorithm>

class Rod {
public:
    V2D start;
    V2D end;


    static constexpr float LOWER_AMOUNT = 0.01f;
    float thickness = 15;

    // ------------------------------------------------------------
    // Construction
    // ------------------------------------------------------------

    constexpr Rod() noexcept = default;

    constexpr Rod(const V2D& start, const V2D& end) noexcept
        : start(start), end(end) {}

    // ------------------------------------------------------------
    // Setters
    // ------------------------------------------------------------

    Rod& set_start(const V2D& s) noexcept {
        start = s;
        return *this;
    }

    Rod& set_end(const V2D& e) noexcept {
        end = e;
        return *this;
    }

    Rod& set_thickness(float t) noexcept {
        if (t>0) thickness=t;
        return *this;
    }

    // ------------------------------------------------------------
    // Collision
    // ------------------------------------------------------------

    bool is_collided(const Particle& p) const noexcept {
        V2D d = end.subtract(start);
        float len2 = d.dot(d);

        if (len2 <= 0.f) {
            return start.distance(p.position) <= p.radius();
        }

        float t = (p.position.subtract(start)).dot(d) / len2;
        t = std::clamp(t, 0.f, 1.f);

        V2D closest = start.add(d.scale(t));

        return closest.distance(p.position) <= p.radius();
    }

};

#endif // CHERNOBY_CORE_ROD_H
