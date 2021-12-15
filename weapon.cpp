#include "api.hpp"
#include "weapon.hpp"
#include "net.hpp"

bool Weapon::hasEntity() {
    return false;
}

PlasmaCannon::PlasmaCannon(std::shared_ptr<Entity> projectileEntity) {
    entity = projectileEntity;
    entity->weapon = this;
}

void PlasmaCannon::fire(const glm::vec3& position, const glm::vec3& normedDirection, InstanceID parentID) {
    Api::eng_createBallisticProjectile(entity.get(), position, normedDirection, parentID);
}

bool PlasmaCannon::hasEntity() {
    return true;
}

Target::Target() { }

Target::Target(const glm::vec3& location)
: isLocation(true), location(location) { }

Target::Target(InstanceID targetID)
: isUnit(true), targetID(targetID) { }

WeaponInstance::WeaponInstance(Weapon *instanceOf, InstanceID parentID)
: instanceOf(instanceOf), timeSinceFired(0.0f), parentID(parentID) { }

#include <iostream>

void WeaponInstance::fire(const glm::vec3& position, const glm::vec3& direction) {
    if (timeSinceFired > instanceOf->reload[reloadIndex]) {
        instanceOf->fire(position, direction, parentID);
        timeSinceFired = 0.0f;
        reloadIndex = (reloadIndex + 1) % instanceOf->reload.size();
    }
    timeSinceFired += Config::Net::secondsPerTick;
}