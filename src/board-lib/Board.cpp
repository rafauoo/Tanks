//
// Created by tomek on 24.05.2022.
//

#include <cmath>

#include "../core-lib/include/EventQueue.h"
#include "../core-lib/include/Event.h"

#include "include/Board.h"
#include "include/Grid.h"
#include "include/GridBuilder.h"
#include "include/TileManager.h"
#include "../tank-lib/include/EntityController.h"

#include "../tank-lib/include/Tank.h"
#include "../tank-lib/include/Bullet.h"


Board::Board() : entityController_(std::make_unique<EntityController>()), grid_(std::make_unique<Grid>()) {}

void Board::setTankMoving(const std::shared_ptr<Tank> &target, bool isMoving) {
    entityController_->setTankMoving(target, isMoving);
}

void Board::setTankDirection(const std::shared_ptr<Tank> &tank, Direction targetDirection) {
    Direction initialDirection = tank->getFacing();

    if (initialDirection == targetDirection) {
        return;
    }

    if (!(abs(static_cast<int>(initialDirection - targetDirection)) == 2) &&
        !snapTankToGrid(tank, true, true)) {  // rotated by 180 deg
        return;
    }

    entityController_->setTankDirection(tank, targetDirection);
    eventQueue_->registerEvent(std::make_unique<Event>(Event::TankRotated, tank));


}

bool Board::snapTankToGrid(const std::shared_ptr<Tank> &target, bool snap_x, bool snap_y) {
    float initial_x = target->getX();
    float initial_y = target->getY();

    if (snap_x)
        target->setX(std::round(target->getX()));
    if (snap_y)
        target->setY(std::round(target->getY()));

    if (!validateEntityPosition(target)) {
        target->setX(initial_x);
        target->setY(initial_y);

        return false;
    }

    eventQueue_->registerEvent(std::make_unique<Event>(Event::EntityMoved, target));

    return true;
}

void Board::setGrid(std::unique_ptr<Grid> grid) {
    grid_ = std::move(grid);
}

void Board::deleteTile(unsigned int x, unsigned int y) {
    if (!TileManager::isTileDestructible(grid_->getTileAtPosition(x, y))) {
        return;
    }
    grid_->deleteTile(x, y);
}

void Board::moveAllEntities() {
    for (const std::shared_ptr<Entity> &entity: *(entityController_->getAllEntities())) {
        moveEntity(entity);
    }
}

bool Board::moveEntity(const std::shared_ptr<Entity> &target) {
    if (!target->move()) {
        return false;
    }
    eventQueue_->registerEvent(std::make_unique<Event>(Event::EntityMoved, target));
    if (!validateEntityPosition(target)) {
        eventQueue_->registerEvent(createCollisionEvent(target));
        return false;
    }
    return true;
}

bool Board::fireTank(const std::shared_ptr<Tank> &target) {
    std::optional<std::shared_ptr<Bullet>> newBullet = target->createBullet();

    if (!newBullet.has_value()) {
        return false;
    }

    if (!validateEntityPosition(std::static_pointer_cast<Entity>(newBullet.value()))) {
        eventQueue_->registerEvent(createCollisionEvent(target));
    }

    std::shared_ptr<Entity> addedEntity = entityController_->addEntity(newBullet.value());
    eventQueue_->registerEvent(std::make_unique<Event>(Event::EntitySpawned, addedEntity));
    return true;
}

bool Board::spawnTank(unsigned int x, unsigned int y, Tank::TankType type, Direction facing) {
    if (type == Tank::PlayerTank) {
        return spawnPlayer(x, y, facing);
    }

    std::shared_ptr<Tank> newTank = entityController_->createTank(x, y, type, facing);

    std::shared_ptr<Entity> spawnedTank = entityController_->addEntity(newTank);
    eventQueue_->registerEvent(std::make_unique<Event>(Event::EntitySpawned, spawnedTank));

    if (!validateEntityPosition(spawnedTank)) {
        eventQueue_->registerEvent(createCollisionEvent(spawnedTank));
        return false;
    }
    return true;
}

bool Board::spawnPlayer(unsigned int x, unsigned int y, Direction facing) {
    std::shared_ptr<Entity> newTank = entityController_->createTank(x, y, Tank::PlayerTank, facing);

    std::shared_ptr<Entity> spawnedTank = entityController_->addEntity(newTank);
    eventQueue_->registerEvent(std::make_unique<Event>(Event::PlayerSpawned, spawnedTank));

    if (!validateEntityPosition(spawnedTank)) {
        eventQueue_->registerEvent(createCollisionEvent(spawnedTank));
        return false;
    }
    return true;
}

bool Board::validateEntityPosition(const std::shared_ptr<Entity> &target) {
    if (target->getX() < 0 || target->getY() < 0) {
        return false;
    }

    auto min_x = static_cast<unsigned int>(floorf(target->getX()));   // TODO USE DOUBLE
    auto max_x = static_cast<unsigned int>(ceilf(target->getX() + target->getSizeX() - 1));
    auto min_y = static_cast<unsigned int>(floorf(target->getY()));
    auto max_y = static_cast<unsigned int>(ceilf(target->getY() + target->getSizeY() - 1));

    try {
        for (unsigned int i = min_x; i <= max_x; i++)
            for (unsigned int j = min_y; j <= max_y; j++) {
                TileType tile = grid_->getTileAtPosition(i, j);
                if (tile != NullTile && TileManager::isTileCollidable(tile)) {
                    return false;
                }
            }
    } catch (OutOfGridException &) {
        return false;
    }

    if (entityController_->checkEntityCollisions(target)) {
        return false;
    }

    return true;
}

void Board::killAllEnemyEntities() {   // FIXME this should be in EntityController
    std::vector<std::shared_ptr<Entity>> *entityVector = entityController_->getAllEntities();
    for (auto iter = entityVector->rbegin(); iter != entityVector->rend(); iter++) {
        std::shared_ptr<Entity> entity = *iter;
        Tank *possiblyATank = dynamic_cast<Tank *>(entity.get());
        if (possiblyATank == nullptr) {  // bullet
            auto *possiblyABullet = dynamic_cast<Bullet *>(entity.get());
            if (possiblyABullet != nullptr && possiblyABullet->isFriendly()) {
                continue;
            }
            entityController_->removeEntity(entity);
            continue;

        } else if (possiblyATank->getType() == Tank::PlayerTank) {
            continue;
        }

        entityController_->killTank(std::static_pointer_cast<Tank>(entity));
    }
}

void Board::removeAllEntities() {
    entityController_->clear();
}

unsigned int Board::getSizeX() {
    return grid_->getSizeX();
}

unsigned int Board::getSizeY() {
    return grid_->getSizeY();
}

std::unique_ptr<Event> Board::createCollisionEvent(std::shared_ptr<Entity> entity) {
    // wdym typechecking is a bad thing

    Event::CollisionMember member1;
    Event::CollisionMember member2;

    // set member 1
    if (dynamic_cast<PlayerTank *>(entity.get()) != nullptr) {
        // player
        member1 = Event::PlayerTankCollisionInfo{
                std::static_pointer_cast<PlayerTank>(entity)};

    } else if (dynamic_cast<Tank *>(entity.get()) != nullptr) {
        // enemy
        member1 = Event::EnemyTankCollisionInfo{
                std::static_pointer_cast<Tank>(entity)};

    } else if (dynamic_cast<Bullet *>(entity.get())!= nullptr &&
               dynamic_cast<Bullet *>(entity.get())->isFriendly()) {
        // friendly bullet
        member1 = Event::FriendlyBulletCollisionInfo{
                std::static_pointer_cast<Bullet>(entity)};

    } else if (dynamic_cast<Bullet *>(entity.get())!= nullptr) {
        // enemy bullet
        member1 = Event::EnemyBulletCollisionInfo{
                std::static_pointer_cast<Bullet>(entity)};
    }

    // set member 2
    std::optional<std::shared_ptr<Entity>> collidingEntity = entityController_->findEntityAtPosition(entity->getX(),
                                                                                                     entity->getY(),
                                                                                                     entity);
    if (!collidingEntity.has_value()) {
        // board
        member2 = Event::BoardCollisionInfo{
                static_cast<unsigned int>(entity->getX()),
                static_cast<unsigned int>(entity->getY()),
                grid_.get()};

    } else {
        entity = collidingEntity.value();
        if (dynamic_cast<PlayerTank *>(entity.get()) != nullptr) {
            // player
            member2 = Event::PlayerTankCollisionInfo{
                    std::static_pointer_cast<PlayerTank>(entity)};
            // swap to guarantee player being the first member
            std::swap(member1, member2);

        } else if (dynamic_cast<Tank *>(entity.get()) != nullptr) {
            // enemy
            member2 = Event::EnemyTankCollisionInfo{
                    std::static_pointer_cast<Tank>(entity)};

        } else if (dynamic_cast<Bullet *>(entity.get())!= nullptr &&
                   dynamic_cast<Bullet *>(entity.get())->isFriendly()) {
            // friendly bullet
            member2 = Event::FriendlyBulletCollisionInfo{
                    std::static_pointer_cast<Bullet>(entity)};
            // swap to guarantee friendly bullet being the first member
            std::swap(member1, member2);

        } else if (dynamic_cast<Bullet *>(entity.get())!= nullptr) {
            // enemy bullet
            member2 = Event::EnemyBulletCollisionInfo{
                    std::static_pointer_cast<Bullet>(entity)};
        }
    }

    return std::move(std::make_unique<Event>(Event::Collision, member1, member2));
}

void Board::loadLevel(unsigned int levelNum) {
    removeAllEntities();
    setGrid(std::move(GridBuilder::buildLevel(levelNum)));  // TODO init player tank
    eventQueue_->registerEvent(std::make_unique<Event>(Event::LevelLoaded, levelNum));
}