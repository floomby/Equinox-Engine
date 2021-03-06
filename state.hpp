#pragma once

#include <boost/random.hpp>
#include <list>
#include <map>
#include <mutex>
#include <stdint.h>
#include <thread>
#include <utility>

#include "econf.h"
#include "instance.hpp"


struct CommandCoroutineType {
    Command *command;
    std::list<Command>::iterator it;
    uint idx;
    std::vector<uint> *which;
    CommandCoroutineType(Command *command, std::list<Command>::iterator it, uint idx, std::vector<uint> *which)
    : command(command), it(it), idx(idx), which(which) {}
};

template<typename StateType> struct CommandGenerator {
    struct Promise;

    // compiler looks for promise_type
    using promise_type = Promise;
    std::coroutine_handle<Promise> coroutine;
    // using StateType = std::tuple<Command, std::vector<uint>::const_iterator, std::list<Command>::iterator>;

    CommandGenerator(std::coroutine_handle<Promise> handle) : coroutine(handle) {}

    ~CommandGenerator() {
        if(coroutine) coroutine.destroy();
    }

    StateType value() {
        return coroutine.promise().val;
    }

    bool next() {
        coroutine.resume();
        return !coroutine.done();
    }

    struct Promise {
        StateType val;

        Promise() : val({ nullptr, std::list<Command>({}).end(), 0, nullptr }){};
        // Promise(Promise const&) = delete;
        // void operator=(Promise const&) = delete;

        CommandGenerator get_return_object() {
            return CommandGenerator { std::coroutine_handle<Promise>::from_promise(*this) };
        }

        std::suspend_always initial_suspend() {
            return {};
        }

        std::suspend_always yield_value(StateType x) {
            val = x;
            return {};
        }

        std::suspend_never return_void() {
            return {};
        }

        std::suspend_always final_suspend() noexcept {
            return {};
        }

        void unhandled_exception() {
            throw;
        }
    };
};

enum class ApiProtocolKind {
    FRAME_ADVANCE,
    SERVER_MESSAGE,
    PAUSE,
    TEAM_DECLARATION,
    COMMAND,
    RESOURCES,
    CALLBACK
};

const uint ApiTextBufferSize = 128;

#define APIF_NONE           0U
#define APIF_NULL_TEAM      1U

struct ApiProtocol {
    ApiProtocolKind kind;
    uint64_t frame;
    char buf[ApiTextBufferSize];
    Command command;
    double dbl;
    CallbackID callbackID;
    uint32_t flags;
};

static_assert(std::is_trivially_copyable<ApiProtocol>::value, "ApiProtocol must be trivially copyable");

#include <iostream>

static const std::vector<std::string> ApiProtocolKinds = enumNames2<ApiProtocolKind>();

namespace std {
    inline ostream& operator<<(ostream& os, const ApiProtocol& data) {
        os << "Kind: " << ApiProtocolKinds[static_cast<int>(data.kind)] << " frame: " << data.frame << " buf: " << data.buf << " " << data.command;
        return os;
    }
}

// strange to have ubo here, but beams are simple and this is a simple way to do it
struct LineUBO {
    alignas(16) glm::vec3 a;
    alignas(16) glm::vec3 b;
    alignas(16) glm::vec4 aColor;
    alignas(16) glm::vec4 bColor;
};

struct BeamData {
    InstanceID parent;
    float damage;
};

class Base;

// I feel like I am doing this all wrong
class AuthoritativeState {
public:
    AuthoritativeState(Base *context);
    ~AuthoritativeState();

    std::vector<Instance *> instances;
    std::vector<Instance *> *instRef;
    std::vector<LineUBO> beams;
    std::vector<BeamData> beamDatum;
    std::atomic<unsigned long> frame = 0;
    InstanceID counter = 100;
    std::array<std::shared_ptr<Team>, Config::maxTeams + 1> teams;

    std::atomic<bool> paused = true;

    void doUpdateTick();
    mutable std::recursive_mutex lock;
    inline std::vector<Instance *>::iterator getInstance(InstanceID id) {
        return find_if(instances.begin(), instances.end(), [id](auto x) -> bool { return x->id == id; });
    }

    bool inline started() const { return frame > 0; }
    void dump() const;
    uint32_t crc() const;

    void process(ApiProtocol *data, std::shared_ptr<Networking::Session> session);
    void emit(const ApiProtocol& data) const;
    void doCallbacks();
    void enableCallbacks();

    boost::random::mt11213b randomGenerator { 37946U };
    boost::random::uniform_real_distribution<float> realDistribution { 0.0f, 1.0f };
private:
    std::queue<std::pair<ApiProtocol, std::shared_ptr<Networking::Session>>> callbacks;
    Base *context;
};

class ObservableState {
public:
    ~ObservableState();
    std::vector<Instance *> instances;
    std::vector<LineUBO> beams;
    uint commandCount(const std::vector<uint>& which);
    CommandGenerator<CommandCoroutineType> getCommandGenerator(std::vector<uint> *which);
    // This update is only for smoothing the animations and stuff (Lag compensation will eventually go in it too once I get around to it)
    void doUpdate(float timeDelta);
    void syncToAuthoritativeState(AuthoritativeState& state);
    std::vector<Team> teams;
};