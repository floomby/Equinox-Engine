#include "engine.hpp"
#include "sound.hpp"

Base *Api::context = nullptr;

#include "api_util.hpp"

unsigned long ApiUtil::getCallbackID() {
    return ++ApiUtil::callbackCounter;
}

thread_local std::queue<CallbackID> ApiUtil::callbackIds;

static std::mutex dispatchLock;
static std::mutex threadDispatchLock;

void ApiUtil::doCallbackDispatch(CallbackID id, const ApiProtocol& data) {
    dispatchLock.lock();
    auto it = ApiUtil::callbacks.find(id);
    if (it == ApiUtil::callbacks.end()) {
        std::cerr << "Missing callback (this probably indicates engine programmer error)" << std::endl;
        dispatchLock.unlock();
        return;
    }
    auto tmp = it->second;
    ApiUtil::callbacks.erase(it);
    dispatchLock.unlock();
    if (tmp.second == LuaWrapper::threadLuaInstance) {
        tmp.first(data);
        return;
    }
    std::scoped_lock l(threadDispatchLock);
    ApiUtil::otherThreadsData.insert({ tmp.second, { tmp.first, data }});
}

void LuaWrapper::dispatchCallbacks() {
    std::vector<std::pair<std::function<void (const ApiProtocol&)>, const ApiProtocol>> todo;
    assert(LuaWrapper::threadLuaInstance);
    threadDispatchLock.lock();
    // I think the stl has something to make this a one liner and more efficiently, but I am too dumb rn to figure it out
    auto it = ApiUtil::otherThreadsData.find(LuaWrapper::threadLuaInstance);
    while (it != ApiUtil::otherThreadsData.end()) {
        if (Api::context->authState.frame > it->second.second.frame) {
            todo.push_back(it->second);
            ApiUtil::otherThreadsData.erase(it++);
        } else it++;
    }
    threadDispatchLock.unlock();
    for (const auto [func, data] : todo) {
        func(data);
    }
}

#define lock_and_get_iterator std::scoped_lock l(context->authState.lock); \
    auto itx = context->authState.getInstance(unitID); \
    if (itx == context->authState.instances.end() || (*itx)->id != unitID) throw std::runtime_error("Invalid instance id"); \
    auto it = *itx;

void Api::cmd_move(const InstanceID unitID, const glm::vec3& destination, const InsertionMode mode, bool userDerived) {
    ApiProtocol data { ApiProtocolKind::COMMAND, 0, "", { CommandKind::MOVE, unitID, { destination, {}, unitID }, mode, userDerived }};
    context->send(data);
}

void Api::cmd_stop(const InstanceID unitID, const InsertionMode mode) {
    ApiProtocol data { ApiProtocolKind::COMMAND, 0, "", { CommandKind::STOP, unitID, { {}, {}, unitID }, mode, true }};
    context->send(data);
}

void Api::cmd_createInstance(const std::string& name, const glm::vec3& position, const glm::quat& heading, TeamID team, std::function<void (InstanceID)> cb) {
    // I will fix this to work with c++ callbacks, don't worry, but this is a check to make sure that we are just getting lua callbacks like we expect
    assert(ApiUtil::callbackIds.size() == 1);
    auto cbID = ApiUtil::callbackIds.front();
    ApiUtil::callbackIds.pop();

    ApiProtocol data { ApiProtocolKind::COMMAND, 0, "", { CommandKind::CREATE, 0, { position, heading, (uint32_t)team }, InsertionMode::NONE, true }};
    strncpy(data.buf, name.c_str(), ApiTextBufferSize);
    data.buf[ApiTextBufferSize - 1] = '\0';
    data.callbackID = cbID;
    context->send(data);

    if (cbID) {
        assert(LuaWrapper::threadLuaInstance);
        std::scoped_lock l(dispatchLock);
        ApiUtil::callbacks.insert({ cbID, { [cb](const ApiProtocol& data) { cb(data.command.id); }, LuaWrapper::threadLuaInstance }});
    }
}

void Api::cmd_destroyInstance(InstanceID unitID) {
    ApiProtocol data { ApiProtocolKind::COMMAND, 0, "", { CommandKind::DESTROY, unitID, { {}, {}, 0 }, InsertionMode::NONE, true }};
    context->send(data);
}

void Api::cmd_setTargetLocation(InstanceID unitID, glm::vec3&& location, InsertionMode mode, bool userDerived) {
    ApiProtocol data { ApiProtocolKind::COMMAND, 0, "", { CommandKind::TARGET_LOCATION, 0, { location, {}, 0 }, mode, userDerived }};
    context->send(data);
}

void Api::cmd_setTargetID(InstanceID unitID, InstanceID targetID, InsertionMode mode, bool userDerived) {
    ApiProtocol data { ApiProtocolKind::COMMAND, 0, "", { CommandKind::TARGET_LOCATION, 0, { {}, {}, targetID }, mode, userDerived }};
    context->send(data);
}

void Api::cmd_build(InstanceID unitID, const char *what, InsertionMode mode) {
    if (!Api::context->currentScene->entities.contains(what)) throw std::runtime_error("Invalid build order");
    ApiProtocol data { ApiProtocolKind::COMMAND, 0, "", { CommandKind::BUILD, unitID, { {}, {}, 0 }, mode, true }};
    strncpy(data.command.data.buf, what, CommandDataBufSize);
    data.command.data.buf[CommandDataBufSize - 1] = '\0';
    context->send(data);
}

void Api::cmd_buildStation(InstanceID unitID, const glm::vec3& where, InsertionMode mode, const char *what) {
    switch (mode) {
        case InsertionMode::BACK:
            Api::cmd_move(unitID, where, InsertionMode::BACK, true);
            Api::cmd_build(unitID, what, InsertionMode::BACK);
            break;
        case InsertionMode::OVERWRITE:
            Api::cmd_move(unitID, where, InsertionMode::OVERWRITE, true);
            Api::cmd_build(unitID, what, InsertionMode::BACK);
            break;
        case InsertionMode::FRONT:
            Api::cmd_move(unitID, where, InsertionMode::FRONT, true);
            Api::cmd_build(unitID, what, InsertionMode::SECOND);
            break;
        default:
            throw std::runtime_error("Unsupported insertion mode for building station");
    }
}

void Api::cmd_setState(InstanceID unitID, const char *state, uint32_t value, InsertionMode mode) {
    ApiProtocol data { ApiProtocolKind::COMMAND, 0, "", { CommandKind::STATE, unitID, { {}, {}, value }, mode, true }};
    strncpy(data.command.data.buf, state, CommandDataBufSize);
    data.command.data.buf[CommandDataBufSize - 1] = '\0';
    context->send(data);
}

void Api::cmd_setIntrinsicState(InstanceID unitID, IntrinicStates state, uint32_t value, InsertionMode mode) {
    ApiProtocol data { ApiProtocolKind::COMMAND, 0, "", { CommandKind::INTRINSIC_STATE, unitID, { {}, {}, value, "", state }, mode, true }};
    context->send(data);
}

uint32_t Api::eng_getState(InstanceID unitID, const char *name) {
    lock_and_get_iterator
    return it->customState[name];
}

uint32_t Api::engS_getState(Instance *unit, const char *name) {
    return unit->customState[name];
}

void Api::eng_createBallisticProjectile(Entity *projectileEntity, const glm::vec3& position, const glm::vec3& normedDirection, InstanceID parentID) {
    std::scoped_lock l(context->authState.lock);
    Instance *inst;
    if (context->headless) {
        inst = new Instance(projectileEntity, context->authState.counter++, 0);
    } else {
        inst = new Instance(projectileEntity, context->currentScene->textures.data() + projectileEntity->textureIndex,
            context->currentScene->models.data() + projectileEntity->modelIndex, context->authState.counter++, 0, true);
    }
    inst->dP = normedDirection * projectileEntity->maxSpeed;
    inst->position = position;
    inst->heading = { 1.0f, 0.0f, 0.0f, 0.0f };
    inst->parentID = parentID;
    context->authState.instances.push_back(inst);
}

void Api::eng_createGuidedProjectile(Entity *projectileEntity, const glm::vec3& position, const glm::vec3& normedDirection, InstanceID parentID, TeamID teamID) {
    std::scoped_lock l(context->authState.lock);
    Instance *inst;
    if (context->headless) {
        inst = new Instance(projectileEntity, context->authState.counter++, teamID);
    } else {
        inst = new Instance(projectileEntity, context->currentScene->textures.data() + projectileEntity->textureIndex,
            context->currentScene->models.data() + projectileEntity->modelIndex, context->authState.counter++, teamID, true);
    }
    inst->dP = normalize(normedDirection - position) * projectileEntity->maxSpeed;
    inst->position = position;
    inst->heading = rotationVector({ 1.0, 0.0, 0.0 }, normedDirection - position);
    inst->parentID = parentID;
    context->authState.instances.push_back(inst);
}

void Api::eng_createBeam(uint32_t color, float damage, const glm::vec3& from, const glm::vec3& to, InstanceID parentID) {
    std::scoped_lock l(context->authState.lock);
    context->authState.beamDatum.push_back({ parentID, damage });
    auto col = util_colorIntToVec(color);
    context->authState.beams.push_back({ from, to, col, col });
}

void Api::eng_echo(const char *message) {
    std::string msg("  > ");
    msg += message;
    msg += '\n';
    std::cout << msg << std::flush;
}

std::vector<InstanceID> Api::eng_getSelectedInstances() {
    assert(!context->headless);
    std::scoped_lock l(static_cast<Engine *>(context)->apiLocks[Engine::APIL_SELECTION]);
    return static_cast<Engine *>(context)->idsSelected;
}

int Api::eng_getTeamID(InstanceID unitID) {
    lock_and_get_iterator
    return it->id;
}

void Api::eng_setInstanceHealth(InstanceID unitID, float health) {
    lock_and_get_iterator
    it->health = health;
}

float Api::eng_getInstanceHealth(InstanceID unitID) {
    lock_and_get_iterator
    return it->health;
}

float Api::engS_getInstanceHealth(Instance *unit) {
    return unit->health;
}

double Api::eng_getInstanceResources(InstanceID unitID) {
    lock_and_get_iterator
    return it->resources;
}

double Api::engS_getInstanceResources(Instance *unit) {
    return unit->resources;
}

const std::string& Api::eng_getInstanceEntityName(InstanceID unitID) {
    lock_and_get_iterator
    return it->entity->name;
}

const std::string& Api::engS_getInstanceEntityName(Instance *unit) {
    return unit->entity->name;
}

InstanceID Api::engS_getInstanceID(Instance *unit) {
    return unit->id;
}

float Api::engS_getRandomF() {
    return context->authState.realDistribution(context->authState.randomGenerator);
}

void Api::eng_quit() {
    context->quit();
}

float Api::eng_fps() {
    assert(!context->headless);
    return static_cast<Engine *>(context)->fps;
}

// Important note: keybindings are not like server callbacks an are only called in the gui thread
void Api::eng_declareKeyBinding(int key) {
    assert(!context->headless);
    if (!LuaWrapper::isGuiThread) std::cerr << "Warning: Manipulating key bindings from a thread which is not the gui thread is probably not what you want to do.\n    The behavior might not be what you expect. They are not thread agnostic like the callback system." << std::endl;
    auto ctx = static_cast<Engine *>(context);
    std::scoped_lock l(ctx->apiLocks[Engine::APIL_KEYBINDINGS]);
    ctx->luaKeyBindings.insert(key);
}

void Api::eng_undeclareKeyBinding(int key) {
    assert(!context->headless);
    if (!LuaWrapper::isGuiThread) std::cerr << "Warning: Manipulating key bindings from a thread which is not the gui thread is probably not what you want to do.\n    The behavior might not be what you expect. They are not thread agnostic like the callback system." << std::endl;
    auto ctx = static_cast<Engine *>(context);
    std::scoped_lock l(ctx->apiLocks[Engine::APIL_KEYBINDINGS]);
    ctx->luaKeyBindings.erase(key);
}

int Api::eng_getScreenWidth() {
    assert(!context->headless);
    return static_cast<Engine *>(context)->swapChainExtent.width;
}

int Api::eng_getScreenHeight() {
    assert(!context->headless);
    return static_cast<Engine *>(context)->swapChainExtent.height;
}

// Idk if I actually want these 2
void Api::eng_setCursorEntity(InstanceID unitID, const std::string& name) {
    assert(!context->headless);
    static_cast<Engine *>(context)->setCursorEntity(name, unitID);
}

void Api::eng_clearCursorEntity() {
    assert(!context->headless);
    static_cast<Engine *>(context)->removeCursorEntity();
}

bool Api::eng_entityIsStation(const std::string& name) {
    return context->currentScene->entities.at(name)->isStation;
}

bool Api::eng_getCollidability(InstanceID unitID) {
    lock_and_get_iterator
    return it->hasCollision;
}

bool Api::engS_getCollidability(Instance *unit) {
    return unit->hasCollision;
}

void Api::eng_setCollidability(InstanceID unitID, bool collidability) {
    lock_and_get_iterator
    it->hasCollision = collidability;
}

void Api::engS_setCollidability(Instance *unit, bool collidability) {
    unit->hasCollision = collidability;
}

bool Api::eng_instanceCanBuild(InstanceID unitID) {
    lock_and_get_iterator
    return !it->entity->buildOptions.empty();
}

bool Api::engS_instanceCanBuild(Instance *unit) {
    return !unit->entity->buildOptions.empty();
}

bool Api::eng_isEntityIdle(InstanceID unitID) {
    lock_and_get_iterator
    auto cit = find_if(it->commandList.begin(), it->commandList.end(), [](auto x){ return x.userDerived; });
    return cit == it->commandList.end();
}

bool Api::engS_isEntityIdle(Instance *unit) {
    auto cit = find_if(unit->commandList.begin(), unit->commandList.end(), [](auto x){ return x.userDerived; });
    return cit == unit->commandList.end();
}

std::vector<std::string>& Api::eng_getInstanceBuildOptions(InstanceID unitID) {
    lock_and_get_iterator
    return it->entity->buildOptions;
}

std::vector<std::string>& Api::engS_getInstanceBuildOptions(Instance *unit) {
    return unit->entity->buildOptions;
}

std::vector<std::string>& Api::eng_getEntityBuildOptions(Entity *entity) {
    return entity->buildOptions;
}

Entity *Api::eng_getInstanceEntity(InstanceID unitID) {
    lock_and_get_iterator
    return it->entity;
}

Entity *Api::engS_getInstanceEntity(Instance *unit) {
    return unit->entity;
}

Entity *Api::eng_getEntityFromName(const char *name) {
    const auto ent = context->currentScene->entities.find(name);
    if (ent == context->currentScene->entities.end()) {
        std::string msg = "Unable to find entity ";
        msg += name;
        throw std::runtime_error(msg);
    }
    return ent->second;
}

float Api::eng_getEngageRange(InstanceID unitID) {
    lock_and_get_iterator
    return it->entity->preferedEngageRange;
}

float Api::engS_getEngageRange(Instance *unit) {
    return unit->entity->preferedEngageRange;
}

// TODO The sound api stuff needs to be made threadsafe
std::vector<std::string> Api::eng_listAudioDevices() {
    if (!context->sound) throw std::runtime_error("Sound is not enabled in this context");
    return context->sound->listDevices();
}

void Api::eng_pickAudioDevice(const char *name) {
    if (!context->sound) throw std::runtime_error("Sound is not enabled in this context");
    context->sound->setDevice(name);
}

void Api::eng_playSound(const char *name) {
    if (!context->sound) throw std::runtime_error("Sound is not enabled in this context");
    context->sound->playSound(name);
}

void Api::eng_playSound3d(const char *name, const glm::vec3& position, const glm::vec3& velocity) {
    if (!context->sound) throw std::runtime_error("Sound is not enabled in this context");
    context->sound->playSound(name, position, velocity, true);
}

void Api::eng_mute(bool mute) {
    if (!context->sound) throw std::runtime_error("Sound is not enabled in this context");
    context->sound->muted = mute;
}

bool Api::eng_isMuted() {
    if (!context->sound) throw std::runtime_error("Sound is not enabled in this context");
    return context->sound->muted;
}

void Api::eng_setInstanceCustomState(InstanceID unitID, std::string key, int value) {
    lock_and_get_iterator
    it->customState[key] = value;
}

void Api::engS_setInstanceCustomState(Instance *unit, std::string key, int value) {
    std::scoped_lock l(context->authState.lock);
    unit->customState[key] = value;
}

std::optional<int> Api::eng_getInstanceCustomState(InstanceID unitID, std::string key) {
    lock_and_get_iterator
    auto mit = it->customState.find(key);
    if (mit == it->customState.end()) return {};
    return std::optional<int>(mit->second);
}

std::optional<int> Api::engS_getInstanceCustomState(Instance *unit, std::string key) {
    std::scoped_lock l(context->authState.lock);
    auto mit = unit->customState.find(key);
    if (mit == unit->customState.end()) return {};
    return std::optional<int>(mit->second);
}

void Api::eng_removeInstanceCustomState(InstanceID unitID, std::string key) {
    lock_and_get_iterator
    auto mit = it->customState.find(key);
    if (mit == it->customState.end()) return;
    it->customState.erase(mit);
}

void Api::engS_removeInstanceCustomState(Instance *unit, std::string key) {
    std::scoped_lock l(context->authState.lock);
    auto mit = unit->customState.find(key);
    if (mit == unit->customState.end()) return;
    unit->customState.erase(mit);
}

glm::vec3 Api::eng_getInstancePosition(InstanceID unitID) {
    lock_and_get_iterator
    return it->position;
}

const glm::vec3& Api::engS_getInstancePosition(Instance *unit) {
    return unit->position;
}

Instance *Api::engS_getClosestEnemy(Instance *unit) {
    // Idk what my policy in these things being null is??
    if (!unit) return nullptr;
    auto closest = std::numeric_limits<float>::max();
    Instance *ret = nullptr;
    for (auto other : *context->authState.instRef) {
        // bool something = false;
        // if (other && other->team == 2) {
        //     std::cout << "here" << std::endl;
        //     something = true;
        // }
        if (!other || other->entity->isProjectile || other == unit || other->team == unit->team) {
            continue;
        }
        auto dist2 = distance2(unit->position, other->position);
        // if (something) {
        //     std::cout << "why not continue: " << dist2 << std::endl;
        //     std::cout << unit->position << " :: " << other->position << std::endl;
        //     std::cout << unit->entity->name << std::endl;
        // }
        if (dist2 < closest) {
            ret = other;
            closest = dist2;
        }
    }
    return ret;
}

glm::quat Api::eng_getInstanceHeading(InstanceID unitID) {
    lock_and_get_iterator
    return it->heading;
}

const glm::quat& Api::engS_getInstanceHeading(Instance *unit) {
    return unit->heading;
}

unsigned long Api::eng_frame() {
    return context->authState.frame;
}

void Api::gui_setVisibility(const char *name, bool visibility) {
    assert(!context->headless);
    GuiCommandData *what = new GuiCommandData();
    what->str = name;
    what->action = visibility;
    what->flags = GUIF_NAMED;
    context->gui->submitCommand({ Gui::GUI_VISIBILITY, what });
}

void Api::gui_setLabelText(const std::string& name, const std::string& text) {
    assert(!context->headless);
    GuiCommandData *what = new GuiCommandData();
    what->str = name;
    what->str2 = text;
    what->flags = GUIF_NAMED;
    context->gui->submitCommand({ Gui::GUI_TEXT, what });
}

void Api::gui_addPanel(const char *root, const char *tableName) {
    assert(!context->headless);
    GuiCommandData *what = new GuiCommandData();
    what->str = root;
    what->str2 = tableName;
    what->flags = GUIF_NAMED | GUIF_LUA_TABLE;
    context->gui->submitCommand({ Gui::GUI_LOAD, what });
    GuiCommandData *what2 = new GuiCommandData();
    what2->str = tableName;
    what2->action = GUIA_VISIBLE;
    what2->flags = GUIF_PANEL_NAME;
    context->gui->submitCommand({ Gui::GUI_VISIBILITY, what2 });
}

void Api::gui_removePanel(const char *panelName) {
    assert(!context->headless);
    GuiCommandData *what = new GuiCommandData();
    what->str = panelName;
    what->flags = GUIF_PANEL_NAME;
    // This is a reminder that I will probabl want to implement this in the future
    what->childIndices = {};
    context->gui->submitCommand({ Gui::GUI_REMOVE, what });
}

void Api::gui_setToggleState(const char *name, uint state) {
    assert(!context->headless);
    GuiCommandData *what = new GuiCommandData();
    what->str = name;
    what->flags = GUIF_NAMED;
    what->action = state;
    context->gui->submitCommand({ Gui::GUI_TOGGLE, what });
}

void Api::state_dumpAuthStateIDs() {
    std::scoped_lock l(context->authState.lock);
    context->authState.dump();
}

void Api::state_giveResources(TeamID team, double resourceUnits) {
    ApiProtocol data { ApiProtocolKind::RESOURCES, team };
    data.dbl = resourceUnits;
    context->send(data);
}

double Api::state_getResources(TeamID teamID) {
    if (teamID > Config::maxTeams) {
        std::string msg = "TeamID is out of bounds (";
        msg += teamID;
        msg += " is greater than max teams)";
        throw std::runtime_error(msg);
    }

    auto tmp = context->authState.teams[teamID];
    if (!tmp) {
        std::string msg = "TeamID is invalid (";
        msg += teamID;
        msg += " has not been declared)";
        throw std::runtime_error(msg);
    }
    return tmp->resourceUnits;
}

std::pair<TeamID, const std::string> Api::state_getTeamIAm() {
    assert(!context->headless);
    return { static_cast<Engine *>(context)->engineSettings.teamIAm.id,
        static_cast<Engine *>(context)->engineSettings.teamIAm.displayName };
}

void Api::net_declareTeam(TeamID teamID, const std::string& name) {
    if (teamID > Config::maxTeams) {
        std::string msg = "TeamID is out of bounds (";
        msg += teamID;
        msg += " is greater than max teams)";
        throw std::runtime_error(msg);
    }
    ApiProtocol data { ApiProtocolKind::TEAM_DECLARATION, teamID };
    strncpy(data.buf, name.c_str(), ApiTextBufferSize);
    data.buf[ApiTextBufferSize - 1] = '\0';
    context->send(data);
}

void Api::net_declareNullTeam(TeamID teamID, const std::string& name) {
    if (teamID > Config::maxTeams) {
        std::string msg = "TeamID is out of bounds (";
        msg += teamID;
        msg += " is greater than max teams)";
        throw std::runtime_error(msg);
    }
    ApiProtocol data { ApiProtocolKind::TEAM_DECLARATION, teamID };
    strncpy(data.buf, name.c_str(), ApiTextBufferSize);
    data.buf[ApiTextBufferSize - 1] = '\0';
    data.flags = APIF_NULL_TEAM;
    context->send(data);
}

void Api::net_pause(bool pause) {
    ApiProtocol data { ApiProtocolKind::PAUSE, (uint64_t)pause };
    context->send(data);
}

glm::vec4 Api::util_colorIntToVec(uint32_t color) {
    return {
        (float)(0xff000000 & color) / 0xff000000,
        (float)(0x00ff0000 & color) / 0x00ff0000,
        (float)(0x0000ff00 & color) / 0x0000ff00,
        (float)(0x000000ff & color) / 0x000000ff,
    };
}

bool Api::util_isNull(void *ptr) {
    return !ptr;
}

glm::quat Api::math_multQuat(const glm::quat& a, const glm::quat& b) {
    return a * b;
}

glm::vec3 Api::math_normVec3(const glm::vec3& v) {
    return glm::normalize(v);
}

glm::vec4 Api::math_normVec4(const glm::vec4& v) {
    return glm::normalize(v);
}
