#pragma once

#include "instance.hpp"
#include "utilities.hpp"

namespace Pathgen {
    enum class Kind {
        OMNI,
        COUNT
    };

    const float arrivalDeltaContinuing = 1.0f;
    const float arrivalDeltaStopping = 0.05f;
    const float steeringHeadingUpdateThreshhold = 0.00000000001f;

    void applySteering(Instance& inst, const glm::vec3& steering) {
        auto steeringAcceleration = clampLength(steering, inst.entity->dv);
        // acceleration = steering_force / mass
        inst.dP = clampLength(inst.dP + steeringAcceleration, inst.entity->v_m);
        inst.position += inst.dP;
        if (length(steeringAcceleration) < steeringHeadingUpdateThreshhold) return;

        auto rot = rotationVector({ 1.0f, 0.0f, 0.0f}, steeringAcceleration);
        // auto wantedAngular = log(rot * inverse(inst.heading));
        inst.heading = slerp(inst.heading, rot, 0.05f);

        // inst.heading = normalize(glm::quat({1.0, steeringAcceleration.x, steeringAcceleration.y, steeringAcceleration.z }));
        // inst.heading = normalize(glm::quat({1.0f, 0.f, 0.f, 0.0f }));
    }

    float seek(Instance& inst, const glm::vec3& dest) {
        auto offset = dest - inst.position;
        auto dist = length(offset);
        auto desiredVelocity = offset / dist * inst.entity->v_m;
        applySteering(inst, desiredVelocity - inst.dP);
        return dist;
    }

    float arrive(Instance& inst, const glm::vec3& dest) {
        auto offset = dest - inst.position;
        auto dist = length(offset);
        // I need to figure out the braking distance, clearly doing the integration to find the straigh approach braking distance was not the right thing
        auto ramped = inst.entity->maxSpeed * (dist / 100.0f);
        auto clipped = std::min(ramped, inst.entity->maxSpeed);
        auto desired = (clipped / dist) * offset;
        // std::cout << desired - inst.dP << std::endl;
        applySteering(inst, desired - inst.dP);
        return dist;
    }

    void stop(Instance& inst) {
        applySteering(inst, -inst.dP);
    }

    const float kick = 0.1f;

    void collide(Instance& a, Instance& b) {
        auto v = a.position - b.position;
        auto d2 = length2(v);
        auto db2 = sq(a.entity->boundingRadius + b.entity->boundingRadius);
        if (d2 < db2) {
            auto amount = db2 - d2;
            a.dP += amount * kick * normalize(v);
            b.dP -= amount * kick * normalize(v);
        }
    }
}

