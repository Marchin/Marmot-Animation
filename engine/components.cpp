internal u32 drawnCount;

////////////////////////////////

//Components

////////////////////////////////

internal Component*
createComponent(ComponentID componentID) {
    Component* result = {};
    switch (componentID) {
        case NONE: {
            result = (Component*)malloc(sizeof(Node));
            memset(result, 0, sizeof(Node));
        } break;
        default: {
            result = 0;
        } break;
    }
    
    return result;
}

ENGINE_API inline Component*
getComponent(ComponentID componentID, Component** pComponents, s32 componentsSize) {
    for (s32 i = 0; i < componentsSize; ++i) {
        if (pComponents[i]->id == componentID) {
            return pComponents[i];
        }
    }
    
    return 0;
}

//The components system works similar to the scene graph in that every component
//stores the needed draw and update functions, and we add 4 components everytime
//it runs out of space in order to save allocations.
//Every component has a componentID at the beginning of the structure so that,
//when casted, it can obtain what kind of component it is and the respective
//draw and update functions, which are called through the scene graph
ENGINE_API inline Component*
addComponent(ComponentID componentID, Transform* pTransform) {
    u32 componentsCapacity = pTransform->componentsCapacity;
    if (pTransform->componentsCount == componentsCapacity) {
        componentsCapacity += DEFAULT_CHILDREN_ADDED;
        
        pTransform->pComponents = (Component**)realloc(pTransform->pComponents, 
                                                       componentsCapacity * sizeof(Component*));
        memset(pTransform->pComponents, 0, componentsCapacity * sizeof(Component*));
        pTransform->componentsCapacity = componentsCapacity;
    }
    for (u32 i = 0; i < componentsCapacity; ++i) {
        if (pTransform->pComponents[i] == NULL) {
            pTransform->pComponents[i] = createComponent(componentID);
            ++pTransform->componentsCount;
            return pTransform->pComponents[i];
        }
    }
    
    return 0;
}

ENGINE_API inline void
removeComponent(ComponentID componentID, Component** pComponents, s32 componentsSize) {
    for (s32 i = 0; i < componentsSize; ++i) {
        if (pComponents[i] && pComponents[i]->id == componentID) {
            free(pComponents[i]);
            pComponents[i] = 0;
        }
    }
}

////////////////////////////////

//Transform

////////////////////////////////

//The scene graph is implemented inside the transform system, 
//it works by having pointers to their respective draw and update functions if needed
//and it keeps a reference to the entity itself to send it to those functions

/* 
ENGINE_API inline void 
transformUpdateMC(Transform* pTransform) {
    pTransform->model = 
        pTransform->positionMatrix * pTransform->rotationMatrix * pTransform->scaleMatrix;
}
 */

ENGINE_API void
initTransform(Transform* pTransform) {
    *pTransform = {};
    pTransform->position = HMM_Vec3T(0.f);
    pTransform->eulerAngles = HMM_Vec3T(0.f);
    pTransform->scale = HMM_Vec3T(1.f);
    pTransform->isRotored = true;
    pTransform->rotor = rot3(0.f, 0.f, 0.f, 1.f);
    pTransform->passedBSP = true;
}

ENGINE_API inline void
reserveChildren(Transform* pTransform, u32 amount) {
    pTransform->maxAmountOfChildren += amount;
    pTransform->pChildren = (Transform**)realloc(pTransform->pChildren, 
                                                 pTransform->maxAmountOfChildren * sizeof(Transform*));
}

ENGINE_API inline void
addChild(Transform* pChild, Transform* pParent) {
    u32 maxAmountOfChildren = pParent->maxAmountOfChildren;
    if (pParent->childrenCount == maxAmountOfChildren) {
        //We add more children than needed in case the user needs more
        //The memory, if wasted, is still cheaper than the allocation process
        maxAmountOfChildren += DEFAULT_CHILDREN_ADDED; 
        pParent->pChildren = (Transform**)realloc(pParent->pChildren, 
                                                  maxAmountOfChildren * sizeof(Transform*));
        pParent->maxAmountOfChildren = maxAmountOfChildren;
    }
    pParent->pChildren[pParent->childrenCount++] = pChild;
    pChild->pParent = pParent;
}

ENGINE_API inline void
removeChild(Transform* pChild) {
    Transform* pParent = pChild->pParent;
    
    if (pParent) {
        u32 childrenCount = pParent->childrenCount;
        for (u32 iTransform = 0; iTransform < childrenCount; ++iTransform) {
            if (pParent->pChildren[iTransform] == pChild) {
                pParent->pChildren[iTransform] = 0;
                --pParent->childrenCount;
                pChild->pParent = 0;
                break;
            }
        }
    }
}

ENGINE_API inline void 
transformSetPosition(Transform* pTransform, f32 x, f32 y, f32 z) {
    pTransform->position = HMM_Vec3(x, y, z);
}

ENGINE_API inline void 
transformTranslate(Transform* pTransform, f32 x, f32 y, f32 z) {
    pTransform->position += HMM_Vec3(x, y, z);
}

ENGINE_API inline void 
transformRotate(Transform* pTransform, f32 angle, hmm_vec3 axis) {
    if (pTransform->isRotored) {
        Rotor3 rot = rotorFromAngleAndAxis(HMM_ToRadians(angle), axis);
        pTransform->rotor = (rot * pTransform->rotor);
    } else {
        pTransform->eulerAngles += angle * axis;
        pTransform->eulerAngles %= 360.f;
    }
}

ENGINE_API inline void
transformScale(Transform* pTransform, f32 x, f32 y, f32 z) {
    pTransform->scale = HMM_Vec3(x, y, z);
}

void
transformDraw(Transform* pTransform, Renderer* pRenderer, hmm_mat4 parentModel) {
    b32 drawn = true;
    pRenderer->pCamera->model = parentModel;
    if (!pTransform->passedBSP) {
        return;
    }
    if (pTransform->draw) {
        drawn = pTransform->draw(pTransform->pEntity, pRenderer);
    }
    u32 componentCount = pTransform->componentsCount;
    for (u32 iComponent = 0; iComponent < componentCount; ++iComponent) {
        if (pTransform->pComponents[iComponent] && pTransform->pComponents[iComponent]->draw) {
            pTransform->pComponents[iComponent]->draw(pTransform->pComponents[iComponent], pRenderer);
        }
    }
    if (drawn) {
        u32 childrenCount = pTransform->childrenCount;
        for (u32 iTransform = 0, i = 0; i < childrenCount; ++iTransform) {
            Transform* pChild = pTransform->pChildren[iTransform];
            if (pChild == 0) { continue; }
            transformDraw(pChild, pRenderer, parentModel*generateModel(pChild));
            ++i;
        }
    }
    if (pTransform->pParent == NULL) {
        //printf("%d\n", drawnCount);
        drawnCount = 0;
    }
}

ENGINE_API inline void
compareBounds(BoxBounds* pCompared, const BoxBounds* pReference) {
    if (pReference->minX < pCompared->minX) {
        pCompared->minX = pReference->minX;
    }
    if (pReference->maxX > pCompared->maxX) {
        pCompared->maxX = pReference->maxX;
    }
    if (pReference->minY < pCompared->minY) {
        pCompared->minY = pReference->minY;
    }
    if (pReference->maxY > pCompared->maxY) {
        pCompared->maxY = pReference->maxY;
    }
    if (pReference->minZ < pCompared->minZ) {
        pCompared->minZ = pReference->minZ;
    }
    if (pReference->maxZ > pCompared->maxZ) {
        pCompared->maxZ = pReference->maxZ;
    }
}

ENGINE_API void
transformUpdate(Transform* pTransform, const f32 deltaTime, hmm_mat4 parentModel) {
    if (pTransform->update) {
        pTransform->update(pTransform->pEntity, deltaTime);
    }
#if 0
    pTransform->eulerAngles = getRotatedVector(HMM_NormalizeVec3(V3{1.f, 1.f, 1.f}), pTransform->rotor);
    pTransform->eulerAngles.x = asin(pTransform->eulerAngles.x)*360.0f/(2.0f*PI32);
    pTransform->eulerAngles.y = asin(pTransform->eulerAngles.y)*360.0f/(2.0f*PI32);
    pTransform->eulerAngles.z = asin(pTransform->eulerAngles.z)*360.0f/(2.0f*PI32);
#else
    Rotor3 rot = pTransform->rotor;
    f64 sinr_cosp = 2 * (rot.a * rot.dx + rot.dy * rot.dz);
    f64 cosr_cosp = 1 - 2 * (rot.dx * rot.dx + rot.dy * rot.dy);
    pTransform->eulerAngles.x = (f32)atan2(sinr_cosp, cosr_cosp)*360.0f/(2.0f*PI32);
    
    // pitch (y-axis rotation)
    f64 sinp = 2 * (rot.a * rot.dy - rot.dz * rot.dx);
    if (HMM_ABS(sinp) >= 1)  {
        f32 sign = (sinp >= 0.0f) ? 1.0f : -1.0f;
        pTransform->eulerAngles.y = sign*(PI32 / 2)*360.0f/(2.0f*PI32); // use 90 degrees if out of range
    } else  {
        pTransform->eulerAngles.y = (f32)asin(sinp)*360.0f/(2.0f*PI32);
    }
    
    // yaw (z-axis rotation)
    f64 siny_cosp = 2 * (rot.a * rot.dz + rot.dx * rot.dy);
    f64 cosy_cosp = 1 - 2 * (rot.dy * rot.dy + rot.dz * rot.dz);
    pTransform->eulerAngles.z = (f32)atan2(siny_cosp, cosy_cosp)*360.0f/(2.0f*PI32);
#endif
    u32 componentCount = pTransform->componentsCount;
    for (u32 iComponent = 0; iComponent < componentCount; ++iComponent) {
        if (pTransform->pComponents[iComponent] && 
            pTransform->pComponents[iComponent]->update) {
            
            pTransform->pComponents[iComponent]->update(pTransform->pComponents[iComponent], 
                                                        deltaTime);
        }
    }
    u32 childrenCount = pTransform->childrenCount;
    for (u32 iTransform = 0, i = 0; i < childrenCount; ++iTransform){
        Transform* pChild = pTransform->pChildren[iTransform];
        pChild->bounds = DEFAULT_BOUNDS;
        if (pChild == 0) { continue; }
        transformUpdate(pChild, deltaTime, parentModel*generateModel(pChild));
        compareBounds(&pTransform->bounds, &pChild->bounds);
        ++i;
    }
}

ENGINE_API inline b32
passesBSP(const BoxBounds* pBounds, const Plane* pPlane, 
          const Renderer* pRenderer, const f32 cameraDistance) {
    BoxBounds bounds = *pBounds;
    f32 minX = bounds.minX;
    f32 minY = bounds.minY;
    f32 minZ = bounds.minZ;
    f32 maxX = bounds.maxX;
    f32 maxY = bounds.maxY;
    f32 maxZ = bounds.maxZ;
    
    hmm_vec3 points[8] = {
        minX, minY, minZ,
        minX, minY, maxZ,
        minX, maxY, minZ,
        minX, maxY, maxZ,
        maxX, minY, minZ,
        maxX, minY, maxZ,
        maxX, maxY, minZ,
        maxX, maxY, minZ,
    }; 
    
    for (u32 iPoint = 0; iPoint < 8; ++iPoint) {
        f32 signedDistanceToPoint = HMM_DotVec3(pPlane->normal, points[iPoint]) + pPlane->d;
        if (haveSameSign(signedDistanceToPoint, cameraDistance)) {
            
            return true;
        }
    }
    return true;
}

ENGINE_API void
transformCheckBSP(Transform* pTransform, const Plane* pPlane, 
                  const Renderer* pRenderer, const f32 cameraDistance,
                  b32 firstRound) {
    
    if (pTransform->passedBSP || firstRound) {
        pTransform->passedBSP = passesBSP(&pTransform->bounds, pPlane, pRenderer, cameraDistance);
        if (!pTransform->passedBSP) { return; }
        u32 childrenCount = pTransform->childrenCount;
        for (u32 iTransform = 0, i = 0; i < childrenCount; ++iTransform){
            Transform* pChild = pTransform->pChildren[iTransform];
            if (pChild == 0) { continue; }
            transformCheckBSP(pChild, pPlane, pRenderer, 
                              cameraDistance, firstRound);
            ++i;
        }
    }
}

ENGINE_API void
checkBSPPlanes(Transform* pScence, const Renderer* pRenderer, const Level* pLevel) {
    const u32 planesCount = pLevel->bspPlaneCount;
    for (u32 iPlane = 0; iPlane < planesCount; ++iPlane) {
        Plane plane = pLevel->pBSPPlanes[iPlane];
        f32 cameraDistance = -HMM_DotVec3(plane.normal, -1.f*pRenderer->pCamera->transform.position) + plane.d;
        transformCheckBSP(pScence, &plane, pRenderer, 
                          cameraDistance, iPlane == 0);
    }
}

void
generateWalls(Transform* pTransform, Level* pLevel, hmm_mat4 parentModel) {
    // NOTE(Marchin): if the first transform sent has any parents, their model matrix are ignored
    if (strncmp(pTransform->name, "wall", 4) == 0) {
        u32 planesCount = pLevel->bspPlaneCount++;
        if (planesCount >= pLevel->maxBSPPlanes) {
            pLevel->maxBSPPlanes = planesCount + DEFAULT_CHILDREN_ADDED;
            pLevel->pBSPPlanes = (Plane*)realloc(pLevel->pBSPPlanes,
                                                 pLevel->maxBSPPlanes*sizeof(Plane));
        }
        Plane plane;
        plane.normal =  HMM_NormalizeVec3((parentModel * HMM_Vec4v(VEC3_Y, 0.f)).XYZ);
        plane.dot = (parentModel * HMM_Vec4(0.f, 0.f, 0.f, 1.f)).XYZ;
        plane.d = -HMM_DotVec3(plane.normal, plane.dot);
        pLevel->pBSPPlanes[planesCount] = plane;
    }
    u32 childrenCount = pTransform->childrenCount;
    for (u32 iTransform = 0, i = 0; i < childrenCount; ++iTransform){
        Transform* pChild = pTransform->pChildren[iTransform];
        if (pChild == 0) { continue; }
        generateWalls(pChild, pLevel, parentModel * generateModel(pChild));
        ++i;
    }
}
