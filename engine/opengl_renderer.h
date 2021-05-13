#ifndef ENGINE_RENDER_H
#define ENGINE_RENDER_H

#ifdef LOG
#define glCall(x) glClearError();\
x;\
assert(glLogCall(#x, __FILE__, __LINE__));
#else
#define glCall(x) x
#endif
#define UNIFORMS_MAX 96
                 
                 global const char* gpDiffuse = "texture_diffuse";
                 global const char* gpSpecular = "texture_specular";
                 global const char* gpNormal = "texture_normal";
                 global const char* gpReflection = "texture_reflection";
                 
                 struct Shader;
                 struct ENGINE_API Shader {
                     meow_hash* pHashLocationCache;
                     s32* pUniformLocationCache;
                     u32 size;
                     char name[64];
                     u32 id;
                 };
                 
                 struct Texture;
                 struct ENGINE_API Texture {
                     u32 id;
                     s32 width;
                     s32 height;
                 };
                 
                 struct ElementBuffer {
                     u32 ebObject;
                     u32 count;
                 };
                 
                 struct VertexBufferElement {
                     u32 type;
                     u32 count;
                     u8 normalized;
                 };
                 
                 struct VertexBufferLayout {
                     VertexBufferElement* pElements;
                     u32 elementsCount;
                     u32 elementsMaxSize;
                     u32 stride;
                 };
                 
                 struct Window;
                 struct ENGINE_API Window {
                     void* pInstance;
                     s32 width;
                     s32 height;
                     const char* pName;
                 };
                 
                 struct Camera;
                 
                 struct Renderer;
                 struct ENGINE_API Renderer {
                     Camera* pCamera;
                     Window* pWindow;
                 };
                 
                 inline void
                 bindTexture(s32 id) {
                     glCall(glBindTexture(GL_TEXTURE_2D, id));
                 }
                 
                 ENGINE_API inline f32 getTime();
                 ENGINE_API void setShaderUniformSize(Shader* pShader, u32 size);
                 ENGINE_API void initComputeShader(Shader* pShader, const char* pComputePath, u32 dataSize);
                 ENGINE_API void initShader(Shader* pShader, const char* pName,
                                            const char* pVertexPath, const char* pFragmentPath,
                                            const char* pGeometryPath = 0, 
                                            const char* pTessControlPath = 0, 
                                            const char* pTessEvaluationPath = 0);
                 ENGINE_API inline void shaderBindID(u32 shaderID);
                 ENGINE_API s32 getUniformLocation(Shader* pShader, const char* pName);
                 ENGINE_API inline void shaderSetBool(Shader* pShader, const char* pName, b32 value);
                 ENGINE_API inline void shaderSetInt(Shader* pShader, const char* pName, s32 value);
                 ENGINE_API inline void shaderSetFloat(Shader* pShader, const char* pName, f32 value);
                 ENGINE_API inline void shaderSetVec3(Shader* pShader, const char* pName, hmm_vec3* pVector);
                 ENGINE_API inline void shaderSetVec4(Shader* pShader, const char* pName, hmm_vec4* pVector); 
                 ENGINE_API inline void shaderSetMat4(Shader* pShader, const char* pName, hmm_mat4* pMat4);
                 ENGINE_API void initTexture(Texture* pTexture, u32 width, u32 height);
                 ENGINE_API void initTexture(Texture* pTexture,
                                             const char* pImgPath, 
                                             b32 flipVertical = false, 
                                             s32 TextureWrap_S = CLAMP_TO_EDGE, 
                                             s32 TextureWrap_T = CLAMP_TO_EDGE,
                                             s32 TextureMinFilter = LINEAR, 
                                             s32 TextureMagFilter = LINEAR);
                 ENGINE_API inline void freeTexture(u32* pTextureID);
                 ENGINE_API inline void textureBindID(u32 textureID, u32 slot);
                 ENGINE_API inline void initEB(u32* pEBObject,  u32* pData, u32 count);
                 ENGINE_API inline void freeEB(u32* pEBObject);
                 ENGINE_API inline void ebBind(u32 ebObject);
                 ENGINE_API inline void ebUnbind();
                 ENGINE_API inline void bindBuffer(u32 id, u32 type);
                 ENGINE_API inline void bindBufferBase(u32 ssbo, u32 position);
                 ENGINE_API inline u32 initBuffer(u32 type, u32 size);
                 ENGINE_API inline u32 vbElementGetSizeOfType(u32 type);
                 ENGINE_API void vbLayoutPushFloat(VertexBufferLayout* pVBLayout, u32 count);
                 ENGINE_API void vbLayoutPushUint(VertexBufferLayout* pVBLayout, u32 count);
                 ENGINE_API void vbLayoutPushUchar(VertexBufferLayout* pVBLayout, u32 count);
                 ENGINE_API inline void initVB(u32* pVB);
                 ENGINE_API inline void initVB(u32* pVB, const void* pData, u32 size);
                 ENGINE_API inline void freeVB(u32* pVB);
                 ENGINE_API inline void vbBind(u32 vb);
                 ENGINE_API inline void vbUnbind();
                 ENGINE_API inline void vbSetData(u32 vb, const void* pData, u32 size);
                 ENGINE_API inline void initVA(u32* pVA);
                 ENGINE_API inline void freeVA(u32* pVA);
                 ENGINE_API inline void vaBind(u32 va);
                 ENGINE_API inline void vaUnbind();
                 ENGINE_API void vaAddBuffer(u32 va, u32 vb, VertexBufferLayout* pLayout);
                 ENGINE_API void vaAddBufferByLocation(u32 va, u32 vb, 
                                                       VertexBufferLayout* pLayout, u32 location);
                 
                 //WINDOW
                 ENGINE_API b32 stopWindow(Window* pWindow);
                 ENGINE_API inline b32 windowShouldClose(Window* pWindow);
                 ENGINE_API inline b32 isKeyPressed(Renderer* pRenderer, u32 key);
                 ENGINE_API inline void getMousePos(Window* pWindow, f64* pX, f64* pY);
                 
                 //RENDERER
                 ENGINE_API inline void fillColor(f32 red, f32 green, f32 blue);
                 ENGINE_API void updateProjection(Camera* pCamera);
                 ENGINE_API inline b32 startRenderer(Renderer* pRenderer, Window* pWindow, Camera* pCamera);
                 ENGINE_API inline b32 stopRenderer();
                 ENGINE_API inline void drawBuffer(u32 offset, u32 count);
                 ENGINE_API inline void drawBufferStrip(u32 offset, u32 count);
                 ENGINE_API inline void drawBufferFan(u32 offset, u32 count);
                 ENGINE_API inline void drawElements(u32 count);
                 ENGINE_API inline void drawElementsStrip(u32 count);
                 ENGINE_API inline void drawElementsQuad(u32 count);
                 ENGINE_API inline void multiplyModelMatrix(Renderer* pRenderer, hmm_mat4* pTransformation);
                 ENGINE_API inline hmm_mat4 getViewProj(Renderer* pRenderer);
                 ENGINE_API inline hmm_mat4 getModelView(Renderer* pRenderer);
                 ENGINE_API inline hmm_mat4 getModelViewProj(Renderer* pRenderer);
                 ENGINE_API inline hmm_vec3 getCameraPosition(Renderer* pRenderer);
                 ENGINE_API inline f32 getCameraWidth(Renderer* pRenderer);
                 ENGINE_API inline f32 getCameraHeight(Renderer* pRenderer);
#endif //ENGINE_RENDER_H