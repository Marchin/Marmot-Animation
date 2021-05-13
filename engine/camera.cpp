ENGINE_API void 
initCamera(Camera* pCamera, hmm_vec3 position, hmm_vec3 up, f32 yaw, f32 pitch) {
	initTransform(&pCamera->transform);
	pCamera->transform.position = position;
    pCamera->transform.rotor = rot3(0.f, 0.f, 0.f, 1.f);
    //pCamera->transform.rotor = lookAt(position, HMM_Vec3(0.0f, 0.0f, -1.0f));
	pCamera->worldUp = HMM_Vec3(0.f, 1.f, 0.f);
	pCamera->yaw = yaw;
	pCamera->pitch = pitch;
    pCamera->movementSpeed = SPEED;
    pCamera->mouseSensitivity = SENSITIVITY;
    pCamera->zoom = ZOOM;
    pCamera->halfCamHeight = 10.f; 
    pCamera->halfCamWidth = 10.f; 
    pCamera->fov = 45;
    pCamera->minDist = 0.1f;
    pCamera->maxDist = 100.f;
    pCamera->aspectRatio = 800.f/600.f;
    pCamera->projectionType = PERSPECTIVE;
    pCamera->model = HMM_Mat4d(1.f);
    pCamera->firstMouseMovement = true;
    updateCameraVectors(pCamera);
    updateProjection(pCamera);
}

ENGINE_API void
intiCamera(Camera* pCamera,
           f32 posX, f32 posY, f32 posZ, 
           f32 upX, f32 upY, f32 upZ, 
           f32 yaw, f32 pitch) {
    
	initTransform(&pCamera->transform);
	pCamera->transform.position = HMM_Vec3(posX, posY, posZ);
    pCamera->transform.rotor = rot3(0.f, 0.f, 0.f, 1.f);
    //pCamera->transform.rotor = lookAt(pCamera->transform.position, HMM_Vec3(0.0f, 0.0f, 1.0f));
    pCamera->worldUp = HMM_Vec3(upX, upY, upZ);
    pCamera->yaw = yaw;
    pCamera->pitch = pitch;
    pCamera->movementSpeed = SPEED;
    pCamera->mouseSensitivity = SENSITIVITY;
    pCamera->zoom = ZOOM;
    pCamera->firstMouseMovement = true;
    updateCameraVectors(pCamera);
}

ENGINE_API hmm_mat4 
getViewMatrix(Camera* pCamera) {
    V3 up = getRotatedVector(VEC3_Y, pCamera->transform.rotor);
    V3 front = getRotatedVector(-1.f*VEC3_Z, pCamera->transform.rotor);
#if 0
    V3 right = getRotatedVector(VEC3_X, pCamera->transform.rotor);
    V3 right2 = HMM_Cross(front, VEC3_Y);
    f32 aup = HMM_Length(up);
    f32 afront = HMM_Length(front);
    f32 arotor = rotorLength(pCamera->transform.rotor);
#endif
    return HMM_LookAt(pCamera->transform.position,
                      pCamera->transform.position + front,
                      up);
}

ENGINE_API void 
moveCamera(Camera* pCamera, hmm_vec3 direction, f32 deltaTime) {
    f32 velocity = pCamera->movementSpeed * deltaTime;
    pCamera->transform.position += direction * velocity;
}

ENGINE_API void
cameraMouseMovement(Camera* pCamera, f64 xPos, f64 yPos, b32 constrainPitch) {
    if (pCamera->firstMouseMovement) {
        pCamera->lastMousePos.x = (f32)xPos;
        pCamera->lastMousePos.y = (f32)yPos;
        pCamera->firstMouseMovement = false;
    }
    
    f32 xOffset = (f32)xPos - pCamera->lastMousePos.x;
    f32 yOffset = (f32)yPos - pCamera->lastMousePos.y;
    pCamera->lastMousePos.x = (f32)xPos;
    pCamera->lastMousePos.y = (f32)yPos;
    
    xOffset *= pCamera->mouseSensitivity;
    yOffset *= pCamera->mouseSensitivity;
    
    pCamera->yaw += xOffset;
    pCamera->pitch += yOffset;
    
    // Make sure that when pitch is out of bounds, screen doesn't get flipped
#if 1
    if (constrainPitch) {
        if (pCamera->pitch > 89.0f) {
            pCamera->pitch = 89.0f;
        } if (pCamera->pitch < -89.0f) {
            pCamera->pitch = -89.0f;
        }
    }
#endif
    
    updateCameraVectors(pCamera);
}

ENGINE_API void
cameraMouseScroll(Camera* pCamera, f32 yoffset) {
    if (pCamera->zoom >= 1.0f && pCamera->zoom <= 45.0f) {
        pCamera->zoom -= yoffset;
    }
    if (pCamera->zoom <= 1.0f) {
        pCamera->zoom = 1.0f;
    }
    if (pCamera->zoom >= 45.0f) {
        pCamera->zoom = 45.0f;
    }
}

ENGINE_API void
updateCameraVectors(Camera* pCamera) {
#if 0
    V3 up = getRotatedVector(VEC3_Y, pCamera->transform.rotor);
    V3 right = getRotatedVector(VEC3_X, pCamera->transform.rotor);
    V3 front = getRotatedVector(VEC3_Z, pCamera->transform.rotor);
#else
    V3 up = VEC3_Y;
    V3 right = VEC3_X;
    V3 front = VEC3_Z;
#endif
    
#if 0
    Rotor3 rotor = pCamera->transform.rotor;
#else
    Rotor3 rotor = rot3(0.f, 0.f, 0.f, 1.f);
#endif
    rotor *= rotorFromAngleAndAxis(HMM_ToRadians(pCamera->yaw), up);
    rotor *= rotorFromAngleAndAxis(HMM_ToRadians(pCamera->pitch), right);
#if 0
    rotor *= rotorFromAngleAndAxis(HMM_ToRadians(pCamera->roll), front);
#endif
    
    pCamera->transform.rotor = rotorNormalize(rotor);
    pCamera->model = generateModel(&pCamera->transform);
    
#if 0
    pCamera->yaw = 0.f;
    pCamera->pitch = 0.f;
#endif
}

internal void
calculateFrustumPlanes(Camera* pCamera) {
    f32 halfFovRad = (pCamera->fov * 0.5f * PI32) / 180.f;
    
    V3 front = getRotatedVector(VEC3_Z, pCamera->transform.rotor);
    f32 signedNear = pCamera->minDist * ((front.z > 0.f)? 1.f:-1.f);
    f32 signedFar = pCamera->maxDist * ((front.z > 0.f)? 1.f:-1.f);
    f32 nearTop = tanf(halfFovRad) * signedNear;
    f32 nearBottom = -nearTop;
    f32 nearRight = nearTop * pCamera->aspectRatio;
    f32 nearLeft = -nearRight;
    
    f32 farTop = tanf(halfFovRad) * pCamera->maxDist * front.z;
    f32 farBottom = -farTop;
    f32 farRight = farTop * pCamera->aspectRatio;
    f32 farLeft = -farRight; 
    
    hmm_vec3 nearTopLeft  = {nearLeft, nearTop, signedNear};
    hmm_vec3 nearTopRight  = {nearRight, nearTop, signedNear};
    hmm_vec3 nearBottomRight  = {nearRight, nearBottom, signedNear};
    hmm_vec3 farTopRight = {farRight, farTop, signedFar};
    hmm_vec3 farTopLeft = {farLeft, farTop, signedFar};
    hmm_vec3 farBottomRight  = {farRight, farBottom, signedFar};
    
    //normals look towards the inside
    
    //top and bottom are the same vector, same with left and right
    hmm_vec3 nearTopSegment = nearTopLeft - nearTopRight;
    hmm_vec3 nearRightSegment = nearBottomRight - nearTopRight;
    hmm_vec3 topRightSegment = farTopRight - nearTopRight;
    hmm_vec3 topLeftSegment = farTopLeft - nearTopLeft;
    hmm_vec3 bottomRightSegment = farBottomRight - nearBottomRight;
    
    Plane nearPlane;
    nearPlane.normal = HMM_Cross(nearRightSegment, nearTopSegment);
    nearPlane.normal = HMM_NormalizeVec3(nearPlane.normal);
    nearPlane.dot = nearTopRight;
    nearPlane.d = -HMM_DotVec3(nearPlane.normal, nearPlane.dot);
    
    Plane rightPlane;
    rightPlane.normal = HMM_Cross(topRightSegment, nearRightSegment);
    rightPlane.normal = HMM_NormalizeVec3(rightPlane.normal);
    rightPlane.dot = nearTopRight;
    rightPlane.d = -HMM_DotVec3(rightPlane.normal, rightPlane.dot);
    
    Plane leftPlane;
    leftPlane.normal = HMM_Cross(nearRightSegment, topLeftSegment);
    leftPlane.normal = HMM_NormalizeVec3(leftPlane.normal);
    leftPlane.dot = nearTopLeft;
    leftPlane.d = -HMM_DotVec3(leftPlane.normal, leftPlane.dot);
    
    Plane topPlane;
    topPlane.normal = HMM_Cross(nearTopSegment, topRightSegment);
    topPlane.normal = HMM_NormalizeVec3(topPlane.normal);
    topPlane.dot = nearTopRight;
    topPlane.d = -HMM_DotVec3(topPlane.normal, topPlane.dot);
    
    Plane bottomPlane;
    bottomPlane.normal = HMM_Cross(bottomRightSegment, nearTopSegment);
    bottomPlane.normal = HMM_NormalizeVec3(bottomPlane.normal);
    bottomPlane.dot = nearBottomRight;
    bottomPlane.d = -HMM_DotVec3(bottomPlane.normal, bottomPlane.dot);
    
    Plane farPlane;
    //Since we are normalizing, we don't care about the lenght of the vectors
    //That means we can use the same ones from the near plane since the have the same direction
    farPlane.normal = -1.f*nearPlane.normal;
    farPlane.dot = farTopRight;
    farPlane.d = -HMM_DotVec3(farPlane.normal, farPlane.dot);
    
    pCamera->frustumPlanes[0] = nearPlane;
    pCamera->frustumPlanes[1] = rightPlane;
    pCamera->frustumPlanes[2] = leftPlane;
    pCamera->frustumPlanes[3] = topPlane;
    pCamera->frustumPlanes[4] = bottomPlane;
    pCamera->frustumPlanes[5] = farPlane;
}

ENGINE_API void
updateProjection(Camera* pCamera) {
    switch (pCamera->projectionType) {
        case ORTHOGRAPHIC: {
            pCamera->projection = HMM_Orthographic(-pCamera->halfCamWidth, 
                                                   pCamera->halfCamWidth, 
                                                   -pCamera->halfCamHeight, 
                                                   pCamera->halfCamHeight, 
                                                   pCamera->minDist, 
                                                   pCamera->maxDist);
        }break;
        case PERSPECTIVE: {
            pCamera->projection = HMM_Perspective(pCamera->fov,
                                                  pCamera->aspectRatio,
                                                  pCamera->minDist, 
                                                  pCamera->maxDist); 
            calculateFrustumPlanes(pCamera);
        }break;
        default: {
            assert(0);
        }break;
    }
}


inline b32
isPointInsideFrustum(hmm_vec3 point, Camera* pCamera) {
    for (u32 iPlane = 0; iPlane < 6; ++iPlane) {
        //Plane is defined in utils.h, i probably should find a better place for it
        Plane plane = pCamera->frustumPlanes[iPlane];
        hmm_vec3 normal = plane.normal;
        f32 signedDistanceToPoint = HMM_DotVec3(normal, point);
        //plane.d is "how many normals" we need to get to the origin
        //if the sum is negative it means that the point is on the opposite
        //side of the normal direction
        if (signedDistanceToPoint + plane.d < 0) {
            return true;
        }
    }
    
    return true;
}