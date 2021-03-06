#pragma once

#include "unit_ai.hpp"
#include "utilities.hpp"
#include "weapon.hpp"

typedef unsigned char stbi_uc;

enum SpecialEntities {
    ENT_ICON,
    ENT_PROJECTILE
};

class UnitAI;
class GuiTexture;

// TODO It would seem good to support empty entities (ie. no model)
class Entity {
    friend class EntityTexture;
    friend class Engine;
    friend class Scene;
public:
    friend std::ostream& operator<<(std::ostream& os, const Entity& entity);

    Entity(SpecialEntities entityType, const char *name = "", const char *model = "", const char *texture = "");
    Entity(const char *model, const char *texture = "", const char *icon = "");
    Entity(const char *name, const char *model, const char *texture, const char *icon);
    std::vector<uint32_t> mapOffsetToIndices(size_t offset);

    Entity(const Entity& other) = delete;
    Entity(Entity&& other) noexcept = delete;
    Entity& operator=(const Entity& other) = delete;
    Entity& operator=(Entity&& other) noexcept = delete;
    ~Entity();
    // temp stuff
    float boundingRadius;
    int textureIndex = 0, iconIndex = 0, modelIndex = 0;

    // Idk why I set defaults here??
    float maxSpeed = 0.5f;
    float mass = 1.0f;
    float thrust = 1.0f;
    float acceleration = 1.0f;
    // maybe this should be a glm::vec3 for yaw, pitch and roll seperately
    float maxOmega = 0.1f;
    double maxHealth = 1.0f;
    double resources = 0.0f;
    std::string name;
    float buildPower = 0.0f;
    float minePower = 0.0f;
    bool canCloak = false;
    float visionRange = 0.0f;
    float radarRange = 0.0f;
    float visionRange2 = 0.0f;
    float radarRange2 = 0.0f;

    float dv;
    float v_m;
    float w_m;

    Weapon *weapon = nullptr;
    bool isProjectile, isUnit, isResource, isGuided = false, isStation = false;
    WeaponKind weaponKind;
    uint framesTillDead = 0;

    std::vector<std::string> weaponNames;
    std::vector<Weapon *> weapons;
    std::vector<glm::vec3> weaponPositions;
    float preferedEngageRange = 0.0f;

    std::vector<std::string> unitAINames;
    std::vector<UnitAI *> ais;

    std::vector<std::string> buildOptions;
    std::string buildIcon;

    std::string deathSound;

    void precompute();
protected:
    stbi_uc *texturePixels;
    stbi_uc *iconPixels;
    int textureWidth, textureHeight, textureChannels;
    int iconWidth, iconHeight, iconChannels;
    std::vector<Utilities::Vertex> vertices;
    std::vector<uint32_t> indices;
    bool hasIcon, hasTexture;
private:
    bool hasConstTexture;
};