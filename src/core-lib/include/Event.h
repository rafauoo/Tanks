//
// Created by tomek on 26.04.2022.
//

#ifndef PROI_PROJEKT_EVENT_H
#define PROI_PROJEKT_EVENT_H

#include <exception>
#include <string>
#include <memory>
#include <variant>

#include "../../tank-lib/include/Entity.h"
#include "../../tank-lib/include/Tank.h"

class Bullet;

class Entity;

class Menu;

class GameState;

class Grid;

class GameStatistics;

class Bot;

enum Direction : unsigned int;

/**
 * Indicates an error encountered while constructing an Event instance.
 */
struct EventConstructionException : public std::exception {
    [[nodiscard]] const char *what() const noexcept override;
};

/**
 * \brief Represents a single event of type Event::EventType
 *
 * Event types are represented with EventType enum. EventType always needs to be passes to constructor, followed
 * by appropriate args. If incorrect args are provided, an EvenConstructionException is thrown.
 *
 * Event class has multiple constructor that can be used for initializing different events.
 *
 * Events can be generated by any classed and should be passed to an event queue (./EventQueue.h)
 */
class Event {
public:

/**
     * Represents event types
     */
    enum EventType {
        NullEvent = 0,

        KeyPressed,
        KeyReleased,

        EntitySpawned,
        EntityMoved,
        EntityRemoved,

        PlayerSpawned,
        PlayerKilled,

        Collision,

        TankKilled,
        TankRotated,
        TankHit,

        BotDecisionRequest,
        BotSpawnDecision,
        BotMoveDecision,
        BotRotateDecision,
        BotFireDecision,

        TilePlaced,
        TileChanged,
        TileDeleted,

        LevelLoaded,

        MenuSelectionChange,
        MenuEnterClicked,

        StateChanged,

        StatisticsChanged
    };

    EventType type;

    /**
     * Holds additional event info for keyboard related events
     */
    struct KeyEventInfo {
        enum KeyAction {
            Pressed = 0,
            Released
        };
        unsigned int keyCode;
        KeyAction action;
    };

    /**
     * Holds additional event info for tank related events
     */
    struct EntityInfo {  //
        std::shared_ptr<Entity> entity;
    };

    /**
     * Holds additional event info for menu events
     */
    struct MenuInfo {
        Menu *menu;
        unsigned int new_pos;
    };

    /**
     * Holds additional event info for state events
     */
    struct StateInfo {
        GameState *state_;
    };

    /**
     * Holds additional event info for points events
     */
    struct StatsInfo {
        GameStatistics *stats_;
    };

    /**
    * Holds additional event info for tile related events
    */
    struct TileInfo {
        unsigned int tile_x;
        unsigned int tile_y;
        Grid *grid;
    };

    // bot events

    /**
     * Holds additional info about bot requests
     */
    struct BotInfo {  //
        std::shared_ptr<Bot> bot;
    };

    /**
     * Holds all info necessary for spawning a tank (used only with bots)
     */
    struct SpawnDecisionInfo {  //
        unsigned int x;
        unsigned int y;
        Tank::TankType type;
        Direction direction=North;
    };

    /**
     * Holds move instructions for bots
     */
    struct MoveDecisionInfo {  //
        std::shared_ptr<Bot> bot;
        bool flag;
    };

    /**
     * Holds rotation instructions for bots
     */
    struct RotateDecisionInfo {  //
        std::shared_ptr<Bot> bot;
        Direction direction;
    };

    // collision events

    /**
     * Holds additional event info for player tank collision members
     */
    struct PlayerTankCollisionInfo {
        std::shared_ptr<PlayerTank> playerTank;
    };

    /**
     * Holds additional event info for enemy tank collision members
     */
    struct EnemyTankCollisionInfo {
        std::shared_ptr<Tank> enemyTank;
    };

    /**
     * Holds additional event info for friendly bullet collision members
     */
    struct FriendlyBulletCollisionInfo {
        std::shared_ptr<Bullet> friendlyBullet;
    };

    /**
     * Holds additional event info for enemy bullet collision members
     */
    struct EnemyBulletCollisionInfo {
        std::shared_ptr<Bullet> enemyBullet;
    };

    /**
     * Holds additional event info for board collision members
     */
    struct BoardCollisionInfo {
        unsigned int tile_x;
        unsigned int tile_y;
        Grid *grid;
    };

    // #####

    /**
     * Can be one of all collision members
     */
    typedef std::variant<PlayerTankCollisionInfo,
            EnemyTankCollisionInfo,
            FriendlyBulletCollisionInfo,
            EnemyBulletCollisionInfo,
            BoardCollisionInfo> CollisionMember;

    /**
     * Holds two collision members. Individual CollisionInfos are accessible with std::get<T>() or (preferred) std::visit().
     *
     * Collision handling can be provided by creating a handleCollision() function and overloading it with different CollisionInfos.
     * The function can then be called in a lambda expression inside std::visit:
     *
     * std::visit([](auto &&arg1, auto &&arg2){ handleCollision(arg1, arg2); }, collisionEvent.member1, collisionEvent.member2);
     *
     * In practice, friendly bullet and player tank members are always stored as member1, and grid member is always member2
     */
    struct CollisionInfo {
        CollisionMember member1;
        CollisionMember member2;
    };

    /**
     * Holds additional info for level related events
     */
    struct LevelInfo {
        unsigned int levelNumber;
        Grid *grid;
    };

    // ####################################################3
    /**
     * Contains additional event info.
     * Unions member is initialized according to EventType enum passed to the constructor
     * Trying to access any member other than initialized will result in undefined behavior
     */
    union info_u {
        MenuInfo menuInfo;
        StateInfo stateInfo;
        KeyEventInfo keyInfo;
        EntityInfo entityInfo;
        TileInfo tileInfo;
        LevelInfo levelInfo;
        CollisionInfo collisionInfo;
        StatsInfo pointsInfo;
        BotInfo botInfo;
        SpawnDecisionInfo spawnDecisionInfo;
        RotateDecisionInfo rotateDecisionInfo;
        MoveDecisionInfo moveDecisionInfo;
        BotInfo fireDecisionInfo;

        ~info_u() {};  // DO NOT change this to =default, or else it will stop working (must be '{}')
    } info = {};

    // FIXME not so elegant
    Event(EventType, unsigned int ui1);

    Event(EventType e, GameStatistics *statsObject);

    Event(EventType e, GameState *new_state);

    explicit Event(EventType);

    Event(EventType e, Menu *menu, unsigned int new_pos);

    Event(EventType e, std::shared_ptr<Entity> entity);

    Event(EventType e, unsigned int x, unsigned int y, Grid *grid);

    Event(EventType e, unsigned int levelNumber, Grid *grid);

    Event(EventType e, std::shared_ptr<Bot> bot);

    Event(EventType e, unsigned int x, unsigned int y, Tank::TankType tankType, Direction direction=North);

    Event(EventType e, std::shared_ptr<Bot> bot, Direction direction);

    Event(EventType e, std::shared_ptr<Bot> bot, bool flag);

    Event(EventType e, CollisionMember mem1, CollisionMember mem2);

    Event() = delete;
};


#endif //PROI_PROJEKT_EVENT_H
