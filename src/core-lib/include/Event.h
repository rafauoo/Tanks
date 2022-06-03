//
// Created by tomek on 26.04.2022.
//

#ifndef PROI_PROJEKT_EVENT_H
#define PROI_PROJEKT_EVENT_H

#include <exception>
#include <string>
#include <memory>

class Tank;
class Entity;
class Menu;
class GameState;
class Grid;
class GameStatistics;

/**
 * Indicates an error encountered while constructing an Event instance.
 */
struct EventConstructionException : public std::exception {
    [[nodiscard]] const char * what () const noexcept override;
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

        EntityEntityCollision,
        EntityGridCollision,

        TankKilled,
        TankRotated,
        TankHit,

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
        enum KeyAction{
            Pressed =0,
            Released
        };
        unsigned int keyCode;
        KeyAction action;
    };

    /**
     * Holds additional event info for tank related events
     * If an event was created as a result of deleting a tank, member variable
     * will point to deallocated memory and should only be used for removing empty
     * references in other objects
     * TRYING TO ACCESS MEMBER VARIABLE'S ATTRS WILL RESULT IN UNDEFINED BEHAVIOR
     */
    struct EntityInfo {  //
        Entity* entity;  // FIXME WILL POINT TO DEALLOCATED MEMORY IF TANK WAS DELETED (create a separate struct with unique_ptr or only pass deleted tank attrs)
    };

    /**
     * Holds additional event info for menu events
     */
    struct MenuInfo {
        Menu* menu;
        unsigned int new_pos;
    };

    /**
     * Holds additional event info for state events
     */
    struct StateInfo {
        GameState* state_;
    };

    /**
     * Holds additional event info for points events
     */
    struct StatsInfo {
        GameStatistics* stats_;
    };

    /**
     * Holds additional event info for entity-entity collision events
     */
    struct EntityEntityCollisionInfo{
        Entity* entity1;
        Entity* entity2;
    };

    /**
     * Holds additional event info for entity-tile collision events
     */
    struct EntityTileCollisionInfo{
        Entity* entity;
        unsigned int x;
        unsigned int y;
    };

    /**
     * Holds additional event info for tile related events
     */
    struct TileInfo {
        unsigned int tile_x;
        unsigned int tile_y;
        Grid* grid;
    };

    /**
     * Holds additional info for level related events
     */
    struct LevelInfo {
        unsigned int levelNumber;
        Grid* grid;
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
        EntityEntityCollisionInfo entityEntityCollisionInfo;
        EntityTileCollisionInfo entityGridCollisionInfo;
        StatsInfo pointsInfo;

        ~info_u(){};  // DO NOT change this to =default, or else it will stop working (must be '{}')
    } info = {};

    // FIXME not so elegant
    Event(EventType, unsigned int ui1);

    Event(EventType e, GameStatistics* statsObject);

    Event(EventType e, GameState* new_state);

    explicit Event(EventType);

    Event(EventType e, Menu* menu, unsigned int new_pos);

    Event(EventType e, Entity* entity);

    Event(EventType e, Entity* entity1, Entity* entity2);

    Event(EventType e, Entity* entity, unsigned int x, unsigned int y);

    Event(EventType e, unsigned int x, unsigned int y, Grid* grid);

    Event(EventType e, unsigned int levelNumber, Grid* grid);

    Event()=delete;
};


#endif //PROI_PROJEKT_EVENT_H
