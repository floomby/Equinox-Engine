#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "utilities.hpp"

namespace Networking { class Session; }

// I am making the executive decision that ids are unique
class Team {
public:
    Team();
    Team(TeamID id, const std::string& displayName, std::optional<std::shared_ptr<Networking::Session>> = {});
    TeamID id;
    std::string displayName;
    double resourceUnits = 0.0f;
    std::optional<std::shared_ptr<Networking::Session>> session;

    inline bool operator==(TeamID other) const { return id == other; }
    inline bool operator==(const std::string& other) const { return displayName == other; }
    inline bool operator==(const Team& other) const { return displayName == other.displayName && id == other.id; }
    inline bool operator<(const Team& other) const { if (id == other.id) return displayName < other.displayName; return id < other.id; };
private:

};