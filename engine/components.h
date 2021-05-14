#ifndef ENTITIES_H
#define ENTITIES_H

#define DEFAULT_CHILDREN_ADDED 4

enum ENGINE_API ComponentID {
    NONE,
    MODEL_DATA,
    NODE,
};

struct Component {
    ComponentID id;
    b32(*draw)(void* pComponent, Renderer* pRenderer);
    void(*update)(void* pComponent, f32 deltaTime);
};

struct ENGINE_API BoxBounds {
    union {
        struct {
            f32 minX;
            f32 minY;
            f32 minZ;
        };
        V3 min;
    };
    union {
        struct {
            f32 maxX;
            f32 maxY;
            f32 maxZ;
        };
        V3 max;
    };
};

global const BoxBounds DEFAULT_BOUNDS = {
    FLT_MAX, FLT_MAX, FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX};

struct ENGINE_API Transform {
    V3 position;
    V3 eulerAngles;
    V3 scale;
    
    BoxBounds bounds;
    
    Component** pComponents;
    u32 componentsCount;
    u32 componentsCapacity;
    
    b32(*draw)(void* pEntity, Renderer* pRenderer);
    void(*update)(void* pEntity, f32 deltaTime);
    Transform* pParent;
    Transform** pChildren;
    void* pEntity;
    u32 childrenCount;
    u32 maxAmountOfChildren;
    
    b32 passedBSP;
    char name[128];
    
    Rotor3 rotor;
    b32 isRotored;
}; 

//COMPONENT
ENGINE_API inline Component* getComponent(ComponentID componentID, 
                                          Component** pComponents, s32 componentsSize);
ENGINE_API inline Component* addComponent(ComponentID componentID, Transform* pTransform);
ENGINE_API inline void removeComponent(ComponentID componentID, 
                                       Component** pComponents, s32 componentsSize);
//TRANSFORM
ENGINE_API inline void reserveChildren(Transform* pTransform, u32 amount);
ENGINE_API inline void addChild(Transform* pChild, Transform* pParent);
ENGINE_API inline void removeChild(Transform* pChild);
ENGINE_API void initTransform(Transform* pTransform);
ENGINE_API inline void transformSetPosition(Transform* pTransform, f32 x, f32 y, f32 z);
ENGINE_API inline void transformTranslate(Transform* pTransform, f32 x, f32 y, f32 z);
ENGINE_API inline void transformRotate(Transform* pTransform, f32 angle, hmm_vec3 axis);
ENGINE_API inline void transformScale(Transform* pTransform, f32 x, f32 y, f32 z);
ENGINE_API void transformDraw(Transform* pTransform, Renderer* pRenderer, hmm_mat4 model);
ENGINE_API void transformUpdate(Transform* pTransform, const f32 deltaTime, hmm_mat4 model); 
ENGINE_API void checkBSPPlanes(Transform* pScence, 
                               const Renderer* pRenderer,
                               const Level* pLevel);
ENGINE_API void generateWalls(Transform* pTransform, Level* pLevel);

#endif //ENTITIES_H