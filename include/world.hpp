#pragma once

#include "base.hpp"
#include "types.hpp"
#include "dynamic_bitset.hpp"
#include "component_pool.hpp"

#include <atomic>
#include <vector>
#include <unordered_map>
#include <memory>
#include <queue>
#include <bitset>
#include <set>

namespace ecs {
    
    class World {
    public:
        World(uint32_t default_entities_capacity = DEFAULT_ENTITIES_CAPACITY, 
            uint32_t default_entity_capacity = DEFAULT_ENTITY_CAPACITY, 
            uint32_t default_component_pools_capacity = DEFAULT_COMPONENT_POOLS_CAPACITY) {
            
            resize_entities(default_entities_capacity);
            resize_entity(default_entity_capacity);
            reserve_component_pools(default_component_pools_capacity);
        }
        
    public: // Entities

        entity CreateEntity() {
            assert(entities_count_ < entities_capacity_ && "Too many active entities in world");

            entity newEntity = *destroyed_entities_.begin();
            destroyed_entities_.erase(newEntity);

            entities_count_++;
            return newEntity;
        }
        
        void DestroyEntity(entity entity) {
            assert(entity < entities_capacity_ && "Entity out of world range");
            assert(entities_count_ > 0 && "All entities already destroyed");

            signatures[entity].reset();

            assert(!destroyed_entities_.contains(entity) && "Entity already destroyed");
            destroyed_entities_.insert(entity);
            entities_count_--;
        }
        
    public: // Component Pools

        template <typename TComponent>
        void RegisterComponent() {
            type_index componentType = TypeIndexator<TComponent>::value();
            assert(!component_pools_.contains(componentType) && "Component already registered");

            auto pool = std::make_shared<ComponentPool<TComponent>>(entities_capacity_);
            auto interfacePool = std::static_pointer_cast<AbstractComponentPool>(pool);
            component_pools_.insert_or_assign(componentType, interfacePool);
        };

        template <typename TComponent>
        void UnregisterComponent() {
            type_index componentType = TypeIndexator<TComponent>::value();
            assert(component_pools_.contains(componentType) && "Component doesn't registered");

            component_pools_.erase(componentType);
        };

    public: // Components

        template <typename TComponent>
        void AddComponent(entity entity, TComponent component) {
            auto pool = GetComponentPool<TComponent>();
            pool->InsertComponent(entity, component);

            dynamic_bitset signature = signatures[entity];
            //signature.set(0, );
        };
        
        template <typename TComponent>
        void RemoveComponent(entity entity) {
            
        };
        
        template <typename TComponent>
        TComponent& GetComponent(entity entity) {
            auto pool = GetComponentPool<TComponent>();
            return pool->GetComponent(entity);
        };

    private:
        template <typename TComponent>
        std::shared_ptr<ComponentPool<TComponent>> GetOrCreateComponentPool() {
            type_index componentType = TypeIndexator<TComponent>::value();

            if (component_pools_.find(componentType) == component_pools_.end()) {
                RegisterComponent<TComponent>();
                return component_pools_[componentType];
            }
            
            std::shared_ptr<AbstractComponentPool> componentPool = component_pools_[componentType];
            return std::static_pointer_cast<ComponentPool<TComponent>>(componentPool);
        }

        template <typename TComponent>
        std::shared_ptr<ComponentPool<TComponent>> GetComponentPool() {
            type_index componentType = TypeIndexator<TComponent>::value();

            assert(component_pools_.find(componentType) != component_pools_.end() && "ComponentPool can't found, register component");
            
            std::shared_ptr<AbstractComponentPool> componentPool = component_pools_[componentType];
            return std::static_pointer_cast<ComponentPool<TComponent>>(componentPool);
        }
        
    public: // Readonly Data

    public: // Memory Data Modification
        void resize_entities(uint32_t new_size) {
            if (new_size == entities_capacity_) return;

            if (entities_capacity_ < new_size) {
                for (entity entity = entities_capacity_; entity < new_size; entity++) {
                    destroyed_entities_.insert(entity);
                }
            }
            else {
                for (entity entity = new_size - 1; entity >= entities_capacity_; entity--) {
                    assert(destroyed_entities_.contains(entity) && "Can't erase created entities");
                    destroyed_entities_.erase(entity);
                }
            }

            signatures.resize(new_size);
            entities_capacity_ = new_size;
        }
        void resize_entity(uint32_t new_size) {
            if (new_size == entities_capacity_) return;

            for (auto i = 0; i < signatures.size(); i++) {
                auto& signature = signatures[i];
                signature.resize(new_size);
            }

            entity_capacity_ = new_size;
        }
        void reserve_component_pools(uint32_t new_capacity) {
            if (new_capacity == pools_capacity_) return;
            assert(component_pools_.size() <= new_capacity && "New capacity will erase registered components");

            component_pools_.reserve(new_capacity);
            pools_capacity_ = new_capacity;
        }

    private: // Data
        uint32_t entities_capacity_ = 0;
        uint32_t entity_capacity_ = 0;
        uint32_t pools_capacity_ = 0;

        std::vector<dynamic_bitset> signatures;
        std::set<entity> destroyed_entities_;
        uint32_t entities_count_ = 0;

        std::unordered_map<type_index, std::shared_ptr<AbstractComponentPool>> component_pools_;
    };
}