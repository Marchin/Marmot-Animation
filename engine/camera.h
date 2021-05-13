#ifndef CAMERA_H
#define CAMERA_H

const f64 YAW = 0.0f;
const f64 PITCH = 0.0f;
const f32 SPEED = 4.f;
const f32 SENSITIVITY = 0.1f;
const f32 ZOOM = 45.0f;

enum Projection {
    ORTHOGRAPHIC,
    PERSPECTIVE
};

struct Transform;

struct ENGINE_API Camera {
    Transform transform;
    V3 worldUp;
    Mat4 projection;
    Mat4 model;
    
	f32 yaw;
	f32 pitch;
	f32 roll;
	
	f32 movementSpeed;
	f32 mouseSensitivity;
	f32 zoom;
    
    f32 halfCamWidth;
	f32 halfCamHeight;
    f32 fov;
    f32 minDist;
    f32 maxDist;
    f32 aspectRatio;
    b32 firstMouseMovement;
    
    hmm_vec2 lastMousePos;
    Projection projectionType;
    
    Plane frustumPlanes[6];
};

ENGINE_API void initCamera(Camera* pCamera,
                           hmm_vec3 position = HMM_Vec3(0.0f, 0.0f, 0.0f), 
                           hmm_vec3 up = HMM_Vec3(0.0f, 1.0f, 0.0f),
                           f32 yaw = YAW, f32 pitch = PITCH);

ENGINE_API void initCamera(Camera* pCamera, 
                           f32 posX, f32 posY, f32 posZ, 
                           f32 upX, f32 upY, f32 upZ, 
                           f32 yaw, f32 pitch);
ENGINE_API  hmm_mat4 getViewMatrix(Camera* pCamera);

ENGINE_API void  moveCamera(Camera* pCamera, hmm_vec3 direction, f32 deltaTime);

ENGINE_API void cameraMouseMovement(Camera* pCamera, 
                                    f64 xOffset, f64 yOffset, 
                                    b32 constrainPitch = true);

ENGINE_API void cameraProcessMouseScroll(Camera* pCamera, f32 xOffset, f32 yOffset);

ENGINE_API void updateCameraVectors(Camera* pCamera);
inline b32 isPointInsideFrustum(hmm_vec3 point, Camera* pCamera);
#endif //CAMERA_H