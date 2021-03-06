// This is an auto generated file changes can be overwritten durring build process
#include "../lua_wrapper.hpp"
#include "../api.hpp"
#include "../api_util.hpp"

static int cmd_moveWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    if(!lua_istable(ls, 2)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 3> v1;
    if (lua_objlen(ls, 2) != 3) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 3; i++) {
        lua_rawgeti(ls, 2, i);
        luaL_checknumber(ls, -1);
        v1[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::vec3 a1(v1[0], v1[1], v1[2]);
    auto a2 = (InsertionMode)luaL_checkinteger(ls, 3);
    auto a3 = lua_toboolean(ls, 4);
    Api::cmd_move(a0, a1, a2, a3);
    return 0;
}

static void cmd_moveExport(lua_State *ls) {
    lua_pushcfunction(ls, cmd_moveWrapper);
    lua_setglobal(ls, "cmd_move");
}

static int cmd_setTargetIDWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto a1 = (InstanceID)luaL_checkinteger(ls, 2);
    auto a2 = (InsertionMode)luaL_checkinteger(ls, 3);
    auto a3 = lua_toboolean(ls, 4);
    Api::cmd_setTargetID(a0, a1, a2, a3);
    return 0;
}

static void cmd_setTargetIDExport(lua_State *ls) {
    lua_pushcfunction(ls, cmd_setTargetIDWrapper);
    lua_setglobal(ls, "cmd_setTargetID");
}

static int cmd_createInstanceWrapper(lua_State *ls) {
    luaL_checkstring(ls, 1);
    auto a0 = lua_tostring(ls, 1);
    if(!lua_istable(ls, 2)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 3> v1;
    if (lua_objlen(ls, 2) != 3) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 3; i++) {
        lua_rawgeti(ls, 2, i);
        luaL_checknumber(ls, -1);
        v1[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::vec3 a1(v1[0], v1[1], v1[2]);
    if(!lua_istable(ls, 3)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 4> v2;
    if (lua_objlen(ls, 3) != 4) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 4; i++) {
        lua_rawgeti(ls, 3, i);
        luaL_checknumber(ls, -1);
        v2[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::quat a2(v2[3], v2[0], v2[1], v2[2]);
    auto a3 = (TeamID)luaL_checkinteger(ls, 4);
    CallbackID id4 = 0;
    if (!lua_isnil(ls, -1)) {
        id4 = ApiUtil::getCallbackID();
        lua_getglobal(ls, "Server_callbacks");
        if (!lua_istable(ls, -1)) throw std::runtime_error("Server_callbacks should be a table (did you forget to enable callbacks on this thread?)");
        lua_insert(ls, -2);
        lua_pushinteger(ls, id4);
        lua_insert(ls, -2);
        lua_settable(ls, -3);
    }
    lua_pop(ls, 1);
    ApiUtil::callbackIds.push(id4);
    auto a4 = ApiUtil::luaCallbackDispatcher<std::function<void (unsigned int)>>(id4);
    Api::cmd_createInstance(a0, a1, a2, a3, a4);
    return 0;
}

static void cmd_createInstanceExport(lua_State *ls) {
    lua_pushcfunction(ls, cmd_createInstanceWrapper);
    lua_setglobal(ls, "cmd_createInstance");
}

static int cmd_stopWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto a1 = (InsertionMode)luaL_checkinteger(ls, 2);
    Api::cmd_stop(a0, a1);
    return 0;
}

static void cmd_stopExport(lua_State *ls) {
    lua_pushcfunction(ls, cmd_stopWrapper);
    lua_setglobal(ls, "cmd_stop");
}

static int cmd_destroyInstanceWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    Api::cmd_destroyInstance(a0);
    return 0;
}

static void cmd_destroyInstanceExport(lua_State *ls) {
    lua_pushcfunction(ls, cmd_destroyInstanceWrapper);
    lua_setglobal(ls, "cmd_destroyInstance");
}

static int cmd_buildWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    auto a2 = (InsertionMode)luaL_checkinteger(ls, 3);
    Api::cmd_build(a0, a1, a2);
    return 0;
}

static void cmd_buildExport(lua_State *ls) {
    lua_pushcfunction(ls, cmd_buildWrapper);
    lua_setglobal(ls, "cmd_build");
}

static int cmd_buildStationWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    if(!lua_istable(ls, 2)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 3> v1;
    if (lua_objlen(ls, 2) != 3) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 3; i++) {
        lua_rawgeti(ls, 2, i);
        luaL_checknumber(ls, -1);
        v1[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::vec3 a1(v1[0], v1[1], v1[2]);
    auto a2 = (InsertionMode)luaL_checkinteger(ls, 3);
    luaL_checkstring(ls, 4);
    auto a3 = lua_tostring(ls, 4);
    Api::cmd_buildStation(a0, a1, a2, a3);
    return 0;
}

static void cmd_buildStationExport(lua_State *ls) {
    lua_pushcfunction(ls, cmd_buildStationWrapper);
    lua_setglobal(ls, "cmd_buildStation");
}

static int cmd_setStateWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    auto a2 = (uint32_t)luaL_checkinteger(ls, 3);
    auto a3 = (InsertionMode)luaL_checkinteger(ls, 4);
    Api::cmd_setState(a0, a1, a2, a3);
    return 0;
}

static void cmd_setStateExport(lua_State *ls) {
    lua_pushcfunction(ls, cmd_setStateWrapper);
    lua_setglobal(ls, "cmd_setState");
}

static int cmd_setIntrinsicStateWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto a1 = (IntrinicStates)luaL_checkinteger(ls, 2);
    auto a2 = (uint32_t)luaL_checkinteger(ls, 3);
    auto a3 = (InsertionMode)luaL_checkinteger(ls, 4);
    Api::cmd_setIntrinsicState(a0, a1, a2, a3);
    return 0;
}

static void cmd_setIntrinsicStateExport(lua_State *ls) {
    lua_pushcfunction(ls, cmd_setIntrinsicStateWrapper);
    lua_setglobal(ls, "cmd_setIntrinsicState");
}

static int eng_createBallisticProjectileWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Entity*)lua_topointer(ls, 1);
    if(!lua_istable(ls, 2)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 3> v1;
    if (lua_objlen(ls, 2) != 3) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 3; i++) {
        lua_rawgeti(ls, 2, i);
        luaL_checknumber(ls, -1);
        v1[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::vec3 a1(v1[0], v1[1], v1[2]);
    if(!lua_istable(ls, 3)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 3> v2;
    if (lua_objlen(ls, 3) != 3) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 3; i++) {
        lua_rawgeti(ls, 3, i);
        luaL_checknumber(ls, -1);
        v2[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::vec3 a2(v2[0], v2[1], v2[2]);
    auto a3 = (InstanceID)luaL_checkinteger(ls, 4);
    Api::eng_createBallisticProjectile(a0, a1, a2, a3);
    return 0;
}

static void eng_createBallisticProjectileExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_createBallisticProjectileWrapper);
    lua_setglobal(ls, "eng_createBallisticProjectile");
}

static int eng_createGuidedProjectileWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Entity*)lua_topointer(ls, 1);
    if(!lua_istable(ls, 2)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 3> v1;
    if (lua_objlen(ls, 2) != 3) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 3; i++) {
        lua_rawgeti(ls, 2, i);
        luaL_checknumber(ls, -1);
        v1[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::vec3 a1(v1[0], v1[1], v1[2]);
    if(!lua_istable(ls, 3)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 3> v2;
    if (lua_objlen(ls, 3) != 3) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 3; i++) {
        lua_rawgeti(ls, 3, i);
        luaL_checknumber(ls, -1);
        v2[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::vec3 a2(v2[0], v2[1], v2[2]);
    auto a3 = (InstanceID)luaL_checkinteger(ls, 4);
    auto a4 = (TeamID)luaL_checkinteger(ls, 5);
    Api::eng_createGuidedProjectile(a0, a1, a2, a3, a4);
    return 0;
}

static void eng_createGuidedProjectileExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_createGuidedProjectileWrapper);
    lua_setglobal(ls, "eng_createGuidedProjectile");
}

static int eng_createBeamWrapper(lua_State *ls) {
    auto a0 = (uint32_t)luaL_checkinteger(ls, 1);
    auto a1 = (float)luaL_checknumber(ls, 2);
    if(!lua_istable(ls, 3)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 3> v2;
    if (lua_objlen(ls, 3) != 3) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 3; i++) {
        lua_rawgeti(ls, 3, i);
        luaL_checknumber(ls, -1);
        v2[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::vec3 a2(v2[0], v2[1], v2[2]);
    if(!lua_istable(ls, 4)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 3> v3;
    if (lua_objlen(ls, 4) != 3) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 3; i++) {
        lua_rawgeti(ls, 4, i);
        luaL_checknumber(ls, -1);
        v3[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::vec3 a3(v3[0], v3[1], v3[2]);
    auto a4 = (InstanceID)luaL_checkinteger(ls, 5);
    Api::eng_createBeam(a0, a1, a2, a3, a4);
    return 0;
}

static void eng_createBeamExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_createBeamWrapper);
    lua_setglobal(ls, "eng_createBeam");
}

static int eng_echoWrapper(lua_State *ls) {
    luaL_checkstring(ls, 1);
    auto a0 = lua_tostring(ls, 1);
    Api::eng_echo(a0);
    return 0;
}

static void eng_echoExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_echoWrapper);
    lua_setglobal(ls, "eng_echo");
}

static int eng_getTeamIDWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto r = Api::eng_getTeamID(a0);
    lua_pushinteger(ls, r);    return 1;
}

static void eng_getTeamIDExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getTeamIDWrapper);
    lua_setglobal(ls, "eng_getTeamID");
}

static int eng_getSelectedInstancesWrapper(lua_State *ls) {
    auto r = Api::eng_getSelectedInstances();
    lua_createtable(ls, r.size(), 0);
    for (int i = 0; i < r.size(); i++) {
        lua_pushinteger(ls, r[i]);
        lua_rawseti(ls, -2, i + 1);
    }
    return 1;
}

static void eng_getSelectedInstancesExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getSelectedInstancesWrapper);
    lua_setglobal(ls, "eng_getSelectedInstances");
}

static int eng_setInstanceHealthWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto a1 = (float)luaL_checknumber(ls, 2);
    Api::eng_setInstanceHealth(a0, a1);
    return 0;
}

static void eng_setInstanceHealthExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_setInstanceHealthWrapper);
    lua_setglobal(ls, "eng_setInstanceHealth");
}

static int eng_getInstanceHealthWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto r = Api::eng_getInstanceHealth(a0);
    lua_pushnumber(ls, r);    return 1;
}

static void eng_getInstanceHealthExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getInstanceHealthWrapper);
    lua_setglobal(ls, "eng_getInstanceHealth");
}

static int engS_getInstanceHealthWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto r = Api::engS_getInstanceHealth(a0);
    lua_pushnumber(ls, r);    return 1;
}

static void engS_getInstanceHealthExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getInstanceHealthWrapper);
    lua_setglobal(ls, "engS_getInstanceHealth");
}

static int eng_getInstanceResourcesWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto r = Api::eng_getInstanceResources(a0);
    lua_pushnumber(ls, r);    return 1;
}

static void eng_getInstanceResourcesExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getInstanceResourcesWrapper);
    lua_setglobal(ls, "eng_getInstanceResources");
}

static int engS_getInstanceResourcesWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto r = Api::engS_getInstanceResources(a0);
    lua_pushnumber(ls, r);    return 1;
}

static void engS_getInstanceResourcesExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getInstanceResourcesWrapper);
    lua_setglobal(ls, "engS_getInstanceResources");
}

static int eng_getInstanceEntityNameWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto r = Api::eng_getInstanceEntityName(a0);
    lua_pushstring(ls, r.c_str());    return 1;
}

static void eng_getInstanceEntityNameExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getInstanceEntityNameWrapper);
    lua_setglobal(ls, "eng_getInstanceEntityName");
}

static int engS_getInstanceEntityNameWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto r = Api::engS_getInstanceEntityName(a0);
    lua_pushstring(ls, r.c_str());    return 1;
}

static void engS_getInstanceEntityNameExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getInstanceEntityNameWrapper);
    lua_setglobal(ls, "engS_getInstanceEntityName");
}

static int engS_getInstanceIDWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto r = Api::engS_getInstanceID(a0);
    lua_pushinteger(ls, r);    return 1;
}

static void engS_getInstanceIDExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getInstanceIDWrapper);
    lua_setglobal(ls, "engS_getInstanceID");
}

static int eng_getCollidabilityWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto r = Api::eng_getCollidability(a0);
    lua_pushboolean(ls, r);    return 1;
}

static void eng_getCollidabilityExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getCollidabilityWrapper);
    lua_setglobal(ls, "eng_getCollidability");
}

static int engS_getCollidabilityWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto r = Api::engS_getCollidability(a0);
    lua_pushboolean(ls, r);    return 1;
}

static void engS_getCollidabilityExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getCollidabilityWrapper);
    lua_setglobal(ls, "engS_getCollidability");
}

static int eng_setCollidabilityWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto a1 = lua_toboolean(ls, 2);
    Api::eng_setCollidability(a0, a1);
    return 0;
}

static void eng_setCollidabilityExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_setCollidabilityWrapper);
    lua_setglobal(ls, "eng_setCollidability");
}

static int engS_setCollidabilityWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto a1 = lua_toboolean(ls, 2);
    Api::engS_setCollidability(a0, a1);
    return 0;
}

static void engS_setCollidabilityExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_setCollidabilityWrapper);
    lua_setglobal(ls, "engS_setCollidability");
}

static int eng_instanceCanBuildWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto r = Api::eng_instanceCanBuild(a0);
    lua_pushboolean(ls, r);    return 1;
}

static void eng_instanceCanBuildExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_instanceCanBuildWrapper);
    lua_setglobal(ls, "eng_instanceCanBuild");
}

static int engS_instanceCanBuildWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto r = Api::engS_instanceCanBuild(a0);
    lua_pushboolean(ls, r);    return 1;
}

static void engS_instanceCanBuildExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_instanceCanBuildWrapper);
    lua_setglobal(ls, "engS_instanceCanBuild");
}

static int eng_getInstanceBuildOptionsWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto r = Api::eng_getInstanceBuildOptions(a0);
    lua_createtable(ls, r.size(), 0);
    for (int i = 0; i < r.size(); i++) {
        lua_pushstring(ls, r[i].c_str());
        lua_rawseti(ls, -2, i + 1);
    }
    return 1;
}

static void eng_getInstanceBuildOptionsExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getInstanceBuildOptionsWrapper);
    lua_setglobal(ls, "eng_getInstanceBuildOptions");
}

static int engS_getInstanceBuildOptionsWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto r = Api::engS_getInstanceBuildOptions(a0);
    lua_createtable(ls, r.size(), 0);
    for (int i = 0; i < r.size(); i++) {
        lua_pushstring(ls, r[i].c_str());
        lua_rawseti(ls, -2, i + 1);
    }
    return 1;
}

static void engS_getInstanceBuildOptionsExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getInstanceBuildOptionsWrapper);
    lua_setglobal(ls, "engS_getInstanceBuildOptions");
}

static int eng_getEntityBuildOptionsWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Entity*)lua_topointer(ls, 1);
    auto r = Api::eng_getEntityBuildOptions(a0);
    lua_createtable(ls, r.size(), 0);
    for (int i = 0; i < r.size(); i++) {
        lua_pushstring(ls, r[i].c_str());
        lua_rawseti(ls, -2, i + 1);
    }
    return 1;
}

static void eng_getEntityBuildOptionsExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getEntityBuildOptionsWrapper);
    lua_setglobal(ls, "eng_getEntityBuildOptions");
}

static int eng_getInstanceEntityWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto r = Api::eng_getInstanceEntity(a0);
    lua_pushlightuserdata(ls, r);    return 1;
}

static void eng_getInstanceEntityExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getInstanceEntityWrapper);
    lua_setglobal(ls, "eng_getInstanceEntity");
}

static int engS_getInstanceEntityWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto r = Api::engS_getInstanceEntity(a0);
    lua_pushlightuserdata(ls, r);    return 1;
}

static void engS_getInstanceEntityExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getInstanceEntityWrapper);
    lua_setglobal(ls, "engS_getInstanceEntity");
}

static int eng_getEntityFromNameWrapper(lua_State *ls) {
    luaL_checkstring(ls, 1);
    auto a0 = lua_tostring(ls, 1);
    auto r = Api::eng_getEntityFromName(a0);
    lua_pushlightuserdata(ls, r);    return 1;
}

static void eng_getEntityFromNameExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getEntityFromNameWrapper);
    lua_setglobal(ls, "eng_getEntityFromName");
}

static int eng_listAudioDevicesWrapper(lua_State *ls) {
    auto r = Api::eng_listAudioDevices();
    lua_createtable(ls, r.size(), 0);
    for (int i = 0; i < r.size(); i++) {
        lua_pushstring(ls, r[i].c_str());
        lua_rawseti(ls, -2, i + 1);
    }
    return 1;
}

static void eng_listAudioDevicesExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_listAudioDevicesWrapper);
    lua_setglobal(ls, "eng_listAudioDevices");
}

static int eng_pickAudioDeviceWrapper(lua_State *ls) {
    luaL_checkstring(ls, 1);
    auto a0 = lua_tostring(ls, 1);
    Api::eng_pickAudioDevice(a0);
    return 0;
}

static void eng_pickAudioDeviceExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_pickAudioDeviceWrapper);
    lua_setglobal(ls, "eng_pickAudioDevice");
}

static int eng_playSoundWrapper(lua_State *ls) {
    luaL_checkstring(ls, 1);
    auto a0 = lua_tostring(ls, 1);
    Api::eng_playSound(a0);
    return 0;
}

static void eng_playSoundExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_playSoundWrapper);
    lua_setglobal(ls, "eng_playSound");
}

static int eng_playSound3dWrapper(lua_State *ls) {
    luaL_checkstring(ls, 1);
    auto a0 = lua_tostring(ls, 1);
    if(!lua_istable(ls, 2)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 3> v1;
    if (lua_objlen(ls, 2) != 3) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 3; i++) {
        lua_rawgeti(ls, 2, i);
        luaL_checknumber(ls, -1);
        v1[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::vec3 a1(v1[0], v1[1], v1[2]);
    if(!lua_istable(ls, 3)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 3> v2;
    if (lua_objlen(ls, 3) != 3) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 3; i++) {
        lua_rawgeti(ls, 3, i);
        luaL_checknumber(ls, -1);
        v2[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::vec3 a2(v2[0], v2[1], v2[2]);
    Api::eng_playSound3d(a0, a1, a2);
    return 0;
}

static void eng_playSound3dExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_playSound3dWrapper);
    lua_setglobal(ls, "eng_playSound3d");
}

static int eng_muteWrapper(lua_State *ls) {
    auto a0 = lua_toboolean(ls, 1);
    Api::eng_mute(a0);
    return 0;
}

static void eng_muteExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_muteWrapper);
    lua_setglobal(ls, "eng_mute");
}

static int eng_isMutedWrapper(lua_State *ls) {
    auto r = Api::eng_isMuted();
    lua_pushboolean(ls, r);    return 1;
}

static void eng_isMutedExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_isMutedWrapper);
    lua_setglobal(ls, "eng_isMuted");
}

static int eng_setInstanceCustomStateWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    auto a2 = (int)luaL_checkinteger(ls, 3);
    Api::eng_setInstanceCustomState(a0, a1, a2);
    return 0;
}

static void eng_setInstanceCustomStateExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_setInstanceCustomStateWrapper);
    lua_setglobal(ls, "eng_setInstanceCustomState");
}

static int engS_setInstanceCustomStateWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    auto a2 = (int)luaL_checkinteger(ls, 3);
    Api::engS_setInstanceCustomState(a0, a1, a2);
    return 0;
}

static void engS_setInstanceCustomStateExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_setInstanceCustomStateWrapper);
    lua_setglobal(ls, "engS_setInstanceCustomState");
}

static int eng_getInstanceCustomStateWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    auto r = Api::eng_getInstanceCustomState(a0, a1);
    if (r.has_value())
        lua_pushinteger(ls, r.value());
    else
        lua_pushnil(ls);
    return 1;
}

static void eng_getInstanceCustomStateExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getInstanceCustomStateWrapper);
    lua_setglobal(ls, "eng_getInstanceCustomState");
}

static int engS_getInstanceCustomStateWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    auto r = Api::engS_getInstanceCustomState(a0, a1);
    if (r.has_value())
        lua_pushinteger(ls, r.value());
    else
        lua_pushnil(ls);
    return 1;
}

static void engS_getInstanceCustomStateExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getInstanceCustomStateWrapper);
    lua_setglobal(ls, "engS_getInstanceCustomState");
}

static int eng_removeInstanceCustomStateWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    Api::eng_removeInstanceCustomState(a0, a1);
    return 0;
}

static void eng_removeInstanceCustomStateExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_removeInstanceCustomStateWrapper);
    lua_setglobal(ls, "eng_removeInstanceCustomState");
}

static int engS_removeInstanceCustomStateWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    Api::engS_removeInstanceCustomState(a0, a1);
    return 0;
}

static void engS_removeInstanceCustomStateExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_removeInstanceCustomStateWrapper);
    lua_setglobal(ls, "engS_removeInstanceCustomState");
}

static int eng_getInstancePositionWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto r = Api::eng_getInstancePosition(a0);
    lua_createtable(ls, 3, 0);
    lua_pushnumber(ls, r.x);
    lua_rawseti(ls, -2, 1);
    lua_pushnumber(ls, r.y);
    lua_rawseti(ls, -2, 2);
    lua_pushnumber(ls, r.z);
    lua_rawseti(ls, -2, 3);
    lua_getglobal(ls, "Vec3_mt");
    lua_setmetatable(ls, -2);
    return 1;
}

static void eng_getInstancePositionExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getInstancePositionWrapper);
    lua_setglobal(ls, "eng_getInstancePosition");
}

static int engS_getInstancePositionWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto r = Api::engS_getInstancePosition(a0);
    lua_createtable(ls, 3, 0);
    lua_pushnumber(ls, r.x);
    lua_rawseti(ls, -2, 1);
    lua_pushnumber(ls, r.y);
    lua_rawseti(ls, -2, 2);
    lua_pushnumber(ls, r.z);
    lua_rawseti(ls, -2, 3);
    lua_getglobal(ls, "Vec3_mt");
    lua_setmetatable(ls, -2);
    return 1;
}

static void engS_getInstancePositionExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getInstancePositionWrapper);
    lua_setglobal(ls, "engS_getInstancePosition");
}

static int engS_getClosestEnemyWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto r = Api::engS_getClosestEnemy(a0);
    lua_pushlightuserdata(ls, r);    return 1;
}

static void engS_getClosestEnemyExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getClosestEnemyWrapper);
    lua_setglobal(ls, "engS_getClosestEnemy");
}

static int eng_getInstanceHeadingWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto r = Api::eng_getInstanceHeading(a0);
    lua_createtable(ls, 4, 0);
    lua_pushnumber(ls, r.x);
    lua_rawseti(ls, -2, 1);
    lua_pushnumber(ls, r.y);
    lua_rawseti(ls, -2, 2);
    lua_pushnumber(ls, r.z);
    lua_rawseti(ls, -2, 3);
    lua_pushnumber(ls, r.w);
    lua_rawseti(ls, -2, 4);
    lua_getglobal(ls, "Quat_mt");
    lua_setmetatable(ls, -2);
    return 1;
}

static void eng_getInstanceHeadingExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getInstanceHeadingWrapper);
    lua_setglobal(ls, "eng_getInstanceHeading");
}

static int engS_getInstanceHeadingWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto r = Api::engS_getInstanceHeading(a0);
    lua_createtable(ls, 4, 0);
    lua_pushnumber(ls, r.x);
    lua_rawseti(ls, -2, 1);
    lua_pushnumber(ls, r.y);
    lua_rawseti(ls, -2, 2);
    lua_pushnumber(ls, r.z);
    lua_rawseti(ls, -2, 3);
    lua_pushnumber(ls, r.w);
    lua_rawseti(ls, -2, 4);
    lua_getglobal(ls, "Quat_mt");
    lua_setmetatable(ls, -2);
    return 1;
}

static void engS_getInstanceHeadingExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getInstanceHeadingWrapper);
    lua_setglobal(ls, "engS_getInstanceHeading");
}

static int eng_frameWrapper(lua_State *ls) {
    auto r = Api::eng_frame();
    lua_pushinteger(ls, r);    return 1;
}

static void eng_frameExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_frameWrapper);
    lua_setglobal(ls, "eng_frame");
}

static int engS_getRandomFWrapper(lua_State *ls) {
    auto r = Api::engS_getRandomF();
    lua_pushnumber(ls, r);    return 1;
}

static void engS_getRandomFExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getRandomFWrapper);
    lua_setglobal(ls, "engS_getRandomF");
}

static int eng_quitWrapper(lua_State *ls) {
    Api::eng_quit();
    return 0;
}

static void eng_quitExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_quitWrapper);
    lua_setglobal(ls, "eng_quit");
}

static int eng_fpsWrapper(lua_State *ls) {
    auto r = Api::eng_fps();
    lua_pushnumber(ls, r);    return 1;
}

static void eng_fpsExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_fpsWrapper);
    lua_setglobal(ls, "eng_fps");
}

static int eng_declareKeyBindingWrapper(lua_State *ls) {
    auto a0 = (int)luaL_checkinteger(ls, 1);
    Api::eng_declareKeyBinding(a0);
    return 0;
}

static void eng_declareKeyBindingExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_declareKeyBindingWrapper);
    lua_setglobal(ls, "eng_declareKeyBinding");
}

static int eng_undeclareKeyBindingWrapper(lua_State *ls) {
    auto a0 = (int)luaL_checkinteger(ls, 1);
    Api::eng_undeclareKeyBinding(a0);
    return 0;
}

static void eng_undeclareKeyBindingExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_undeclareKeyBindingWrapper);
    lua_setglobal(ls, "eng_undeclareKeyBinding");
}

static int eng_getScreenWidthWrapper(lua_State *ls) {
    auto r = Api::eng_getScreenWidth();
    lua_pushinteger(ls, r);    return 1;
}

static void eng_getScreenWidthExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getScreenWidthWrapper);
    lua_setglobal(ls, "eng_getScreenWidth");
}

static int eng_getScreenHeightWrapper(lua_State *ls) {
    auto r = Api::eng_getScreenHeight();
    lua_pushinteger(ls, r);    return 1;
}

static void eng_getScreenHeightExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getScreenHeightWrapper);
    lua_setglobal(ls, "eng_getScreenHeight");
}

static int eng_setCursorEntityWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    Api::eng_setCursorEntity(a0, a1);
    return 0;
}

static void eng_setCursorEntityExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_setCursorEntityWrapper);
    lua_setglobal(ls, "eng_setCursorEntity");
}

static int eng_clearCursorEntityWrapper(lua_State *ls) {
    Api::eng_clearCursorEntity();
    return 0;
}

static void eng_clearCursorEntityExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_clearCursorEntityWrapper);
    lua_setglobal(ls, "eng_clearCursorEntity");
}

static int eng_entityIsStationWrapper(lua_State *ls) {
    luaL_checkstring(ls, 1);
    auto a0 = lua_tostring(ls, 1);
    auto r = Api::eng_entityIsStation(a0);
    lua_pushboolean(ls, r);    return 1;
}

static void eng_entityIsStationExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_entityIsStationWrapper);
    lua_setglobal(ls, "eng_entityIsStation");
}

static int eng_isEntityIdleWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto r = Api::eng_isEntityIdle(a0);
    lua_pushboolean(ls, r);    return 1;
}

static void eng_isEntityIdleExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_isEntityIdleWrapper);
    lua_setglobal(ls, "eng_isEntityIdle");
}

static int engS_isEntityIdleWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto r = Api::engS_isEntityIdle(a0);
    lua_pushboolean(ls, r);    return 1;
}

static void engS_isEntityIdleExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_isEntityIdleWrapper);
    lua_setglobal(ls, "engS_isEntityIdle");
}

static int eng_getStateWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    auto r = Api::eng_getState(a0, a1);
    lua_pushinteger(ls, r);    return 1;
}

static void eng_getStateExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getStateWrapper);
    lua_setglobal(ls, "eng_getState");
}

static int engS_getStateWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    auto r = Api::engS_getState(a0, a1);
    lua_pushinteger(ls, r);    return 1;
}

static void engS_getStateExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getStateWrapper);
    lua_setglobal(ls, "engS_getState");
}

static int eng_getEngageRangeWrapper(lua_State *ls) {
    auto a0 = (InstanceID)luaL_checkinteger(ls, 1);
    auto r = Api::eng_getEngageRange(a0);
    lua_pushnumber(ls, r);    return 1;
}

static void eng_getEngageRangeExport(lua_State *ls) {
    lua_pushcfunction(ls, eng_getEngageRangeWrapper);
    lua_setglobal(ls, "eng_getEngageRange");
}

static int engS_getEngageRangeWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (Instance*)lua_topointer(ls, 1);
    auto r = Api::engS_getEngageRange(a0);
    lua_pushnumber(ls, r);    return 1;
}

static void engS_getEngageRangeExport(lua_State *ls) {
    lua_pushcfunction(ls, engS_getEngageRangeWrapper);
    lua_setglobal(ls, "engS_getEngageRange");
}

static int gui_setVisibilityWrapper(lua_State *ls) {
    luaL_checkstring(ls, 1);
    auto a0 = lua_tostring(ls, 1);
    auto a1 = lua_toboolean(ls, 2);
    Api::gui_setVisibility(a0, a1);
    return 0;
}

static void gui_setVisibilityExport(lua_State *ls) {
    lua_pushcfunction(ls, gui_setVisibilityWrapper);
    lua_setglobal(ls, "gui_setVisibility");
}

static int gui_setLabelTextWrapper(lua_State *ls) {
    luaL_checkstring(ls, 1);
    auto a0 = lua_tostring(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    Api::gui_setLabelText(a0, a1);
    return 0;
}

static void gui_setLabelTextExport(lua_State *ls) {
    lua_pushcfunction(ls, gui_setLabelTextWrapper);
    lua_setglobal(ls, "gui_setLabelText");
}

static int gui_addPanelWrapper(lua_State *ls) {
    luaL_checkstring(ls, 1);
    auto a0 = lua_tostring(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    Api::gui_addPanel(a0, a1);
    return 0;
}

static void gui_addPanelExport(lua_State *ls) {
    lua_pushcfunction(ls, gui_addPanelWrapper);
    lua_setglobal(ls, "gui_addPanel");
}

static int gui_removePanelWrapper(lua_State *ls) {
    luaL_checkstring(ls, 1);
    auto a0 = lua_tostring(ls, 1);
    Api::gui_removePanel(a0);
    return 0;
}

static void gui_removePanelExport(lua_State *ls) {
    lua_pushcfunction(ls, gui_removePanelWrapper);
    lua_setglobal(ls, "gui_removePanel");
}

static int gui_setToggleStateWrapper(lua_State *ls) {
    luaL_checkstring(ls, 1);
    auto a0 = lua_tostring(ls, 1);
    auto a1 = (uint)luaL_checkinteger(ls, 2);
    Api::gui_setToggleState(a0, a1);
    return 0;
}

static void gui_setToggleStateExport(lua_State *ls) {
    lua_pushcfunction(ls, gui_setToggleStateWrapper);
    lua_setglobal(ls, "gui_setToggleState");
}

static int state_dumpAuthStateIDsWrapper(lua_State *ls) {
    Api::state_dumpAuthStateIDs();
    return 0;
}

static void state_dumpAuthStateIDsExport(lua_State *ls) {
    lua_pushcfunction(ls, state_dumpAuthStateIDsWrapper);
    lua_setglobal(ls, "state_dumpAuthStateIDs");
}

static int state_giveResourcesWrapper(lua_State *ls) {
    auto a0 = (TeamID)luaL_checkinteger(ls, 1);
    auto a1 = (double)luaL_checknumber(ls, 2);
    Api::state_giveResources(a0, a1);
    return 0;
}

static void state_giveResourcesExport(lua_State *ls) {
    lua_pushcfunction(ls, state_giveResourcesWrapper);
    lua_setglobal(ls, "state_giveResources");
}

static int state_getResourcesWrapper(lua_State *ls) {
    auto a0 = (TeamID)luaL_checkinteger(ls, 1);
    auto r = Api::state_getResources(a0);
    lua_pushnumber(ls, r);    return 1;
}

static void state_getResourcesExport(lua_State *ls) {
    lua_pushcfunction(ls, state_getResourcesWrapper);
    lua_setglobal(ls, "state_getResources");
}

static int state_getTeamIAmWrapper(lua_State *ls) {
    auto r = Api::state_getTeamIAm();
    lua_pushinteger(ls, r.first);
    lua_pushstring(ls, r.second.c_str());
    return 2;
}

static void state_getTeamIAmExport(lua_State *ls) {
    lua_pushcfunction(ls, state_getTeamIAmWrapper);
    lua_setglobal(ls, "state_getTeamIAm");
}

static int net_declareTeamWrapper(lua_State *ls) {
    auto a0 = (TeamID)luaL_checkinteger(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    Api::net_declareTeam(a0, a1);
    return 0;
}

static void net_declareTeamExport(lua_State *ls) {
    lua_pushcfunction(ls, net_declareTeamWrapper);
    lua_setglobal(ls, "net_declareTeam");
}

static int net_declareNullTeamWrapper(lua_State *ls) {
    auto a0 = (TeamID)luaL_checkinteger(ls, 1);
    luaL_checkstring(ls, 2);
    auto a1 = lua_tostring(ls, 2);
    Api::net_declareNullTeam(a0, a1);
    return 0;
}

static void net_declareNullTeamExport(lua_State *ls) {
    lua_pushcfunction(ls, net_declareNullTeamWrapper);
    lua_setglobal(ls, "net_declareNullTeam");
}

static int net_pauseWrapper(lua_State *ls) {
    auto a0 = lua_toboolean(ls, 1);
    Api::net_pause(a0);
    return 0;
}

static void net_pauseExport(lua_State *ls) {
    lua_pushcfunction(ls, net_pauseWrapper);
    lua_setglobal(ls, "net_pause");
}

static int util_colorIntToVecWrapper(lua_State *ls) {
    auto a0 = (uint32_t)luaL_checkinteger(ls, 1);
    auto r = Api::util_colorIntToVec(a0);
    lua_createtable(ls, 4, 0);
    lua_pushnumber(ls, r.x);
    lua_rawseti(ls, -2, 1);
    lua_pushnumber(ls, r.y);
    lua_rawseti(ls, -2, 2);
    lua_pushnumber(ls, r.z);
    lua_rawseti(ls, -2, 3);
    lua_pushnumber(ls, r.w);
    lua_rawseti(ls, -2, 4);
    lua_getglobal(ls, "Vec4_mt");
    lua_setmetatable(ls, -2);
    return 1;
}

static void util_colorIntToVecExport(lua_State *ls) {
    lua_pushcfunction(ls, util_colorIntToVecWrapper);
    lua_setglobal(ls, "util_colorIntToVec");
}

static int util_isNullWrapper(lua_State *ls) {
    if (!lua_islightuserdata(ls, 1)) throw std::runtime_error("Invalid lua arguments (pointer)");
    auto a0 = (void*)lua_topointer(ls, 1);
    auto r = Api::util_isNull(a0);
    lua_pushboolean(ls, r);    return 1;
}

static void util_isNullExport(lua_State *ls) {
    lua_pushcfunction(ls, util_isNullWrapper);
    lua_setglobal(ls, "util_isNull");
}

static int math_multQuatWrapper(lua_State *ls) {
    if(!lua_istable(ls, 1)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 4> v0;
    if (lua_objlen(ls, 1) != 4) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 4; i++) {
        lua_rawgeti(ls, 1, i);
        luaL_checknumber(ls, -1);
        v0[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::quat a0(v0[3], v0[0], v0[1], v0[2]);
    if(!lua_istable(ls, 2)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 4> v1;
    if (lua_objlen(ls, 2) != 4) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 4; i++) {
        lua_rawgeti(ls, 2, i);
        luaL_checknumber(ls, -1);
        v1[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::quat a1(v1[3], v1[0], v1[1], v1[2]);
    auto r = Api::math_multQuat(a0, a1);
    lua_createtable(ls, 4, 0);
    lua_pushnumber(ls, r.x);
    lua_rawseti(ls, -2, 1);
    lua_pushnumber(ls, r.y);
    lua_rawseti(ls, -2, 2);
    lua_pushnumber(ls, r.z);
    lua_rawseti(ls, -2, 3);
    lua_pushnumber(ls, r.w);
    lua_rawseti(ls, -2, 4);
    lua_getglobal(ls, "Quat_mt");
    lua_setmetatable(ls, -2);
    return 1;
}

static void math_multQuatExport(lua_State *ls) {
    lua_pushcfunction(ls, math_multQuatWrapper);
    lua_setglobal(ls, "math_multQuat");
}

static int math_normVec3Wrapper(lua_State *ls) {
    if(!lua_istable(ls, 1)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 3> v0;
    if (lua_objlen(ls, 1) != 3) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 3; i++) {
        lua_rawgeti(ls, 1, i);
        luaL_checknumber(ls, -1);
        v0[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::vec3 a0(v0[0], v0[1], v0[2]);
    auto r = Api::math_normVec3(a0);
    lua_createtable(ls, 3, 0);
    lua_pushnumber(ls, r.x);
    lua_rawseti(ls, -2, 1);
    lua_pushnumber(ls, r.y);
    lua_rawseti(ls, -2, 2);
    lua_pushnumber(ls, r.z);
    lua_rawseti(ls, -2, 3);
    lua_getglobal(ls, "Vec3_mt");
    lua_setmetatable(ls, -2);
    return 1;
}

static void math_normVec3Export(lua_State *ls) {
    lua_pushcfunction(ls, math_normVec3Wrapper);
    lua_setglobal(ls, "math_normVec3");
}

static int math_normVec4Wrapper(lua_State *ls) {
    if(!lua_istable(ls, 1)) throw std::runtime_error("Invalid lua arguments (table)");
    std::array<float, 4> v0;
    if (lua_objlen(ls, 1) != 4) throw std::runtime_error("C++/Lua vector mismatch");
    for (int i = 1; i <= 4; i++) {
        lua_rawgeti(ls, 1, i);
        luaL_checknumber(ls, -1);
        v0[i - 1] = lua_tonumber(ls, -1);
        lua_pop(ls, 1);
    }
    glm::vec4 a0(v0[0], v0[1], v0[2], v0[3]);
    auto r = Api::math_normVec4(a0);
    lua_createtable(ls, 4, 0);
    lua_pushnumber(ls, r.x);
    lua_rawseti(ls, -2, 1);
    lua_pushnumber(ls, r.y);
    lua_rawseti(ls, -2, 2);
    lua_pushnumber(ls, r.z);
    lua_rawseti(ls, -2, 3);
    lua_pushnumber(ls, r.w);
    lua_rawseti(ls, -2, 4);
    lua_getglobal(ls, "Vec4_mt");
    lua_setmetatable(ls, -2);
    return 1;
}

static void math_normVec4Export(lua_State *ls) {
    lua_pushcfunction(ls, math_normVec4Wrapper);
    lua_setglobal(ls, "math_normVec4");
}

void LuaWrapper::apiExport() {
    luaL_dofile(luaState, "lua/glm_metatables.lua");
    cmd_moveExport(luaState);
    cmd_setTargetIDExport(luaState);
    cmd_createInstanceExport(luaState);
    cmd_stopExport(luaState);
    cmd_destroyInstanceExport(luaState);
    cmd_buildExport(luaState);
    cmd_buildStationExport(luaState);
    cmd_setStateExport(luaState);
    cmd_setIntrinsicStateExport(luaState);
    eng_createBallisticProjectileExport(luaState);
    eng_createGuidedProjectileExport(luaState);
    eng_createBeamExport(luaState);
    eng_echoExport(luaState);
    eng_getTeamIDExport(luaState);
    eng_getSelectedInstancesExport(luaState);
    eng_setInstanceHealthExport(luaState);
    eng_getInstanceHealthExport(luaState);
    engS_getInstanceHealthExport(luaState);
    eng_getInstanceResourcesExport(luaState);
    engS_getInstanceResourcesExport(luaState);
    eng_getInstanceEntityNameExport(luaState);
    engS_getInstanceEntityNameExport(luaState);
    engS_getInstanceIDExport(luaState);
    eng_getCollidabilityExport(luaState);
    engS_getCollidabilityExport(luaState);
    eng_setCollidabilityExport(luaState);
    engS_setCollidabilityExport(luaState);
    eng_instanceCanBuildExport(luaState);
    engS_instanceCanBuildExport(luaState);
    eng_getInstanceBuildOptionsExport(luaState);
    engS_getInstanceBuildOptionsExport(luaState);
    eng_getEntityBuildOptionsExport(luaState);
    eng_getInstanceEntityExport(luaState);
    engS_getInstanceEntityExport(luaState);
    eng_getEntityFromNameExport(luaState);
    eng_listAudioDevicesExport(luaState);
    eng_pickAudioDeviceExport(luaState);
    eng_playSoundExport(luaState);
    eng_playSound3dExport(luaState);
    eng_muteExport(luaState);
    eng_isMutedExport(luaState);
    eng_setInstanceCustomStateExport(luaState);
    engS_setInstanceCustomStateExport(luaState);
    eng_getInstanceCustomStateExport(luaState);
    engS_getInstanceCustomStateExport(luaState);
    eng_removeInstanceCustomStateExport(luaState);
    engS_removeInstanceCustomStateExport(luaState);
    eng_getInstancePositionExport(luaState);
    engS_getInstancePositionExport(luaState);
    engS_getClosestEnemyExport(luaState);
    eng_getInstanceHeadingExport(luaState);
    engS_getInstanceHeadingExport(luaState);
    eng_frameExport(luaState);
    engS_getRandomFExport(luaState);
    eng_quitExport(luaState);
    eng_fpsExport(luaState);
    eng_declareKeyBindingExport(luaState);
    eng_undeclareKeyBindingExport(luaState);
    eng_getScreenWidthExport(luaState);
    eng_getScreenHeightExport(luaState);
    eng_setCursorEntityExport(luaState);
    eng_clearCursorEntityExport(luaState);
    eng_entityIsStationExport(luaState);
    eng_isEntityIdleExport(luaState);
    engS_isEntityIdleExport(luaState);
    eng_getStateExport(luaState);
    engS_getStateExport(luaState);
    eng_getEngageRangeExport(luaState);
    engS_getEngageRangeExport(luaState);
    gui_setVisibilityExport(luaState);
    gui_setLabelTextExport(luaState);
    gui_addPanelExport(luaState);
    gui_removePanelExport(luaState);
    gui_setToggleStateExport(luaState);
    state_dumpAuthStateIDsExport(luaState);
    state_giveResourcesExport(luaState);
    state_getResourcesExport(luaState);
    state_getTeamIAmExport(luaState);
    net_declareTeamExport(luaState);
    net_declareNullTeamExport(luaState);
    net_pauseExport(luaState);
    util_colorIntToVecExport(luaState);
    util_isNullExport(luaState);
    math_multQuatExport(luaState);
    math_normVec3Export(luaState);
    math_normVec4Export(luaState);
}
