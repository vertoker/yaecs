
#include "ecs.hpp"

#include <iostream>

struct A {};
struct B : A { int v1; };
struct Position {
    float x, y, z;
    Position() : x{0}, y{0}, z{0} { }
    Position(float x, float y, float z) 
        : x{x}, y{y}, z{z} { }
};

class PositionSystem : public ecs::IRunSystem {
public:
    void run() override {
        for (auto it = begin<Position>(); it != end<Position>(); ++it) {
            auto& component = *it;
            component.x += 1;
            component.y += 1;
            component.z += 1;
        }
    }
};

int main(int argc, char* argv[]) {
    std::cout << ecs::TypeIndexator<A>::value();
    std::cout << ecs::TypeIndexator<B>::value();
    //std::cout << ecs::TypeIndexator<C>::value();
    std::cout << ecs::TypeIndexator<B>::value();
    std::cout << ecs::TypeIndexator<A>::value();
    std::cout << std::endl;

    // World
    
    ecs::World ecsWorld{3, 2};
    
    ecs::entity entity1 = ecsWorld.CreateEntity();
    ecs::entity entity2 = ecsWorld.CreateEntity();
    ecs::entity entity3 = ecsWorld.CreateEntity();

    ecsWorld.RegisterComponent<A>();
    //ecsWorld.RegisterComponent<B>();
    ecsWorld.RegisterComponent<Position>();

    ecsWorld.InsertComponent(entity1, A{});
    //ecsWorld.AddComponent(entity1, A{});
    //ecsWorld.InsertComponent(entity2, B{.v1=1});
    ecsWorld.InsertComponent(entity2, Position());

    // Systems

    ecs::Systems ecsSystems{ecsWorld};
    ecs::SystemExecuteCollection<ecs::IRunSystem> runSystems{};

    auto posSystem = ecsSystems.RegisterSystem<PositionSystem>();
    runSystems.AddSystem(posSystem);

    for (auto it = ecsWorld.begin<Position>(); it != ecsWorld.end<Position>(); ++it) {
        auto& component = *it;
        std::cout << component.x;
    }
    std::cout << std::endl;

    runSystems.execute();

    for (auto it = ecsWorld.begin<Position>(); it != ecsWorld.end<Position>(); ++it) {
        auto& component = *it;
        std::cout << component.x;
    }
    std::cout << std::endl;

    return 0;
}
