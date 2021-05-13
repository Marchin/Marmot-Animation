inline f32 
getTime() {
    return (f32)glfwGetTime();
}

internal inline void 
glClearError() {
    while (glGetError() != GL_NO_ERROR);
}

internal inline bool
glLogCall(const char* pFunction, const char* pFile, s32 line) {
    while (GLenum error = glGetError()) {
        printf("[OpenGL Error] (%d): %s %s: %d\n", error, pFunction, pFile, line);
        
        return false;
    }
    
    return true;
}

internal void 
checkShaderCompileErrors(u32 shader, const char* pType) {
	s32 success;
	char infoLog[1024];
	if (strcmp(pType, "PROGRAM") != 0) {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            printf("ERROR::SHADER_COMPILATION_ERROR of type: %s\n%s\n\
                   -------------------------------------------------------\n", pType, infoLog);
		} else {
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                printf("ERROR::PROGRAM_LINKING_ERROR of type: %s\n%s\n\
                       -------------------------------------------------------\n", 
                       pType, infoLog);
			}
		}
	}
}

internal u32 
setupShader(const char* pMaterialPath, u32 type) {
	FILE* pFile = fopen(pMaterialPath, "r");
    char* pShaderCode;
	u32 id;
    
    if (pFile) {
        fseek(pFile, 0, SEEK_END);
        u32 size = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
        pShaderCode = (char*)malloc(size + 1);
        if (pShaderCode && size) {
            fread(pShaderCode, sizeof(char), size, pFile);
        }
        while (pShaderCode[size - 1] < '\0' && size > 0) {
            --size; // NOTE(Marchin): for some reason some text files have 
            //non-valid characters at EOF, so we get rid of them
        }
        pShaderCode[size] = '\0'; 
        fclose(pFile);
    } else {
        perror("Error opening file\n");
    }
	glCall(id = glCreateShader(type));
	glCall(glShaderSource(id, 1, &pShaderCode, NULL));
	glCall(glCompileShader(id));
    
    char vertexMsg[] = "VERTEX";
    char fragmentMsg[] = "FRAGMENT";
    char geometryMsg[] = "GEOMETRY";
    char tessEvaluationMsg[] = "TESS_EVALUATION";
    char tessControlMsg[] = "TESS_CONTROL";
    char computeMsg[] = "COMPUTE";
    char* pMsg = 0;
	if (type == GL_VERTEX_SHADER) pMsg = vertexMsg;
	else if (type == GL_FRAGMENT_SHADER) pMsg = fragmentMsg;
	else if (type == GL_GEOMETRY_SHADER) pMsg = geometryMsg;
	else if (type == GL_TESS_CONTROL_SHADER) pMsg = tessEvaluationMsg;
	else if (type == GL_TESS_EVALUATION_SHADER) pMsg = tessControlMsg;
	else if (type == GL_COMPUTE_SHADER) pMsg = computeMsg;
	checkShaderCompileErrors(id, pMsg);
    
	return id;
}

void
setShaderUniformSize(Shader* pShader, u32 size) {
    pShader->size = size;
    pShader->pHashLocationCache = (meow_hash*)malloc(sizeof(meow_hash)*size);
    pShader->pUniformLocationCache = (s32*)malloc(sizeof(s32)*size);
    for (u32 iUniform = 0; iUniform < size; ++iUniform) {
        pShader->pUniformLocationCache[iUniform] = -1;
    }
}

void
initComputeShader(Shader* pShader, const char* pComputePath, u32 dataSize) {
    u32 compute = setupShader(pComputePath, GL_COMPUTE_SHADER);
    glCall(pShader->id = glCreateProgram());
	glCall(glAttachShader(pShader->id, compute));
    
	glCall(glLinkProgram(pShader->id));
	checkShaderCompileErrors(pShader->id, "PROGRAM");
    
    setShaderUniformSize(pShader, dataSize);
    
    glCall(glDeleteShader(compute));
}

void
initShader(Shader* pShader, const char* pName,
           const char* pVertexPath, const char* pFragmentPath,
           const char* pGeometryPath, 
           const char* pTessControlPath, const char* pTessEvaluationPath) {
    
    u32 vertex = 0;
    u32 fragment = 0;
    u32 geometry = 0;
    u32 tessellationControl = 0;
    u32 tessellationEvaluation = 0;
    
    strcpy(pShader->name, pName);
    
    vertex = setupShader(pVertexPath, GL_VERTEX_SHADER);
    fragment = setupShader(pFragmentPath, GL_FRAGMENT_SHADER);
    if (pGeometryPath != NULL) {
        geometry = setupShader(pGeometryPath, GL_GEOMETRY_SHADER);
    }
    if (pTessControlPath != NULL) {
        tessellationControl = setupShader(pTessControlPath, GL_TESS_CONTROL_SHADER);
    }
    if (pTessEvaluationPath != NULL) {
        tessellationEvaluation = setupShader(pTessEvaluationPath, GL_TESS_EVALUATION_SHADER);
    }
    
    glCall(pShader->id = glCreateProgram());
    glCall(glAttachShader(pShader->id, vertex));
    glCall(glAttachShader(pShader->id, fragment));
    if (pGeometryPath != 0) {
        glCall(glAttachShader(pShader->id, geometry));
    }
    if (pTessControlPath != 0) {
        glCall(glAttachShader(pShader->id, tessellationControl));
    }
    if (pTessEvaluationPath != 0) {
        glCall(glAttachShader(pShader->id, tessellationEvaluation));
    }
    glCall(glLinkProgram(pShader->id));
    checkShaderCompileErrors(pShader->id, "PROGRAM");
    
    glCall(glDeleteShader(vertex));
    glCall(glDeleteShader(fragment));
    if (pGeometryPath != 0) {
        glCall(glDeleteShader(geometry));
    }
    if (pTessControlPath != 0) {
        glCall(glDeleteShader(tessellationControl));
    }
    if (pTessEvaluationPath != 0) {
        glCall(glDeleteShader(tessellationEvaluation));
    }
    
    pShader->size = UNIFORMS_MAX;
    setShaderUniformSize(pShader, UNIFORMS_MAX);
}

inline void 
shaderBindID(u32 shaderID) {
    glCall(glUseProgram(shaderID));
}

s32 
getUniformLocation(Shader* pShader, const char* pName) {
    meow_hash hash = MeowHash_Accelerated(0, sizeof(pName), (const void*)pName);
    b32 found = false;
    s32 iHash;
    s32 firstFreeSlotIndex = -1;
    s32 size = pShader->size;
    for (iHash = 0; iHash < size; ++iHash) {
        if (MeowHashesAreEqual(pShader->pHashLocationCache[iHash], hash)) {
            found = true;
            break;
        } else {
            if (firstFreeSlotIndex == -1 && pShader->pUniformLocationCache[iHash] == -1) {
                firstFreeSlotIndex = iHash;
            }
        }
    }
    if (found) {
        return pShader->pUniformLocationCache[iHash];
    } else {
        glCall(s32 location = glGetUniformLocation(pShader->id, pName));
        if (location == -1) {
            printf("Warning: uniform '%s' doesn't exist\n", pName);
        }
        if (firstFreeSlotIndex == -1) {
            printf("Error: You exceeded the max number of uniforms in a shader allowed\n");
        } else {
            pShader->pHashLocationCache[firstFreeSlotIndex] = hash;
            pShader->pUniformLocationCache[firstFreeSlotIndex] = location;
        }
        return location;
    }
}

inline void 
shaderSetBool(Shader* pShader, const char* pName, b32 value) {
    glCall(glUniform1i(getUniformLocation(pShader, pName), value));
}

inline void
shaderSetInt(Shader* pShader, const char* pName, s32 value) {
    glCall(glUniform1i(getUniformLocation(pShader, pName), value));
}

inline void
shaderSetFloat(Shader* pShader, const char* pName, f32 value) {
    glCall(glUniform1f(getUniformLocation(pShader, pName), value));
}

inline void
shaderSetVec3(Shader* pShader, const char* pName, hmm_vec3* pVector) {
    glCall(glUniform3fv(getUniformLocation(pShader, pName), 1, &pVector->x));
}

inline void
shaderSetVec4(Shader* pShader, const char* pName, hmm_vec4* pVector) {
    glCall(glUniform4fv(getUniformLocation(pShader, pName), 1, &pVector->X));
}

inline void
shaderSetMat4(Shader* pShader, const char* pName, hmm_mat4* pMat4) {
    glCall(glUniformMatrix4fv(getUniformLocation(pShader, pName), 
                              1, GL_FALSE, (f32*)pMat4));
}

void
initTexture(Texture* pTexture, u32 width, u32 height) {
    pTexture->width = width;
    pTexture->height = height;
    glCall(glGenTextures(1, &pTexture->id));
    glCall(glBindTexture(GL_TEXTURE_2D, pTexture->id));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    glCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
                        0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
    glCall(glBindTexture(GL_TEXTURE_2D, 0));
}

void
initTexture(Texture* pTexture,
            const char* pImgPath, b32 flipVertical, 
            s32 TextureWrap_S, s32 TextureWrap_T,
            s32 TextureMinFilter, s32 TextureMagFilter) {
    
    glCall(glGenTextures(1, &pTexture->id));
    glCall(glBindTexture(GL_TEXTURE_2D, pTexture->id));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, TextureWrap_S));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, TextureWrap_T));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureMinFilter));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureMagFilter));
    stbi_set_flip_vertically_on_load(flipVertical);
    
    s32 nrChannels;
    u8* pData = stbi_load(pImgPath, 
                          &pTexture->width, &pTexture->height, 
                          &nrChannels, 0);
    if (pData) {
        GLenum format;
        if (nrChannels == 1) {
            format = GL_RED;
        } else if (nrChannels == 3) {
            format = GL_RGB;
        } else if (nrChannels == 4) {
            format = GL_RGBA;
        } else {
            format = 0; // NOTE(Marchin): i arbitrary put 0 here, i should check later if it doesn't cause problems
        }
        glCall(glTexImage2D(GL_TEXTURE_2D, 0, format, 
                            pTexture->width, pTexture->height,
                            0, format, GL_UNSIGNED_BYTE, pData));
        glCall(glGenerateMipmap(GL_TEXTURE_2D));
    } else {
        printf("Failed to load texture\n");
    }
    glCall(glBindTexture(GL_TEXTURE_2D, 0));
    stbi_image_free(pData);
}

inline void
freeTexture(u32* pTextureID) {
    glCall(glDeleteTextures(1, pTextureID));
}

inline void
textureBindID(u32 textureID, u32 slot) {
    glCall(glActiveTexture(GL_TEXTURE0 + slot));
    glCall(glBindTexture(GL_TEXTURE_2D, textureID));
}

inline void
initEB(u32* pEBObject,  u32* pData, u32 count) {
    glCall(glGenBuffers(1, pEBObject));
    glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *pEBObject));
    glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                        count * sizeof(u32), 
                        pData, 
                        GL_STATIC_DRAW));
}

inline void
freeEB(u32* pEBObject) {
    glCall(glDeleteBuffers(1, pEBObject));
}

inline void 
ebBind(u32 ebObject) {
    glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebObject));
}

inline void 
ebUnbind() {
    glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

inline void
bindBuffer(u32 id, u32 type) {
    glCall(glBindBuffer(type, id));
}

inline void
bindBufferBase(u32 ssbo, u32 position) {
    glCall(glBindBufferBase(GL_SHADER_STORAGE_BUFFER, position, ssbo));
}

inline u32
initBuffer(u32 type, u32 size) {
    u32 id;
    glCall(glGenBuffers(1, &id));
    glCall(glBindBuffer(type, id));
    glCall(glBufferData(type, size, NULL, GL_STATIC_DRAW));
    
    return id;
}

inline void
bufferDataDraw(u32 bufferType, u32 length) {
    glCall(glBufferData(bufferType, length, NULL, GL_STREAM_DRAW));
}

inline void*
mapBufferRangeWrite(u32 bufferType, u32 offset, u32 length) {
    return glMapBufferRange(bufferType, offset, length, 
                            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

inline void
memoryBarrier() {
    glCall(glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT));
}

inline void
unmapBuffer(u32 bufferType) {
    glCall(glUnmapBuffer(bufferType));
}

inline void
setDepthTestState(b32 state) {
    if (state) {
        glCall(glEnable(GL_DEPTH_TEST));
    } else {
        glCall(glDisable(GL_DEPTH_TEST));
    }
}

inline u32 
vbElementGetSizeOfType(u32 type) {
    u32 result = 0;
    switch (type) {
        case GL_FLOAT: {
            return 4;
        } break;
        case GL_UNSIGNED_INT: {
            return 4;
        } break;
        case GL_UNSIGNED_BYTE: {
            return 1;
        } break;
        default: {
            assert(false);
        }
    }
    return 0;
}

void 
vbLayoutPushFloat(VertexBufferLayout* pVBLayout, u32 count) {
    if (pVBLayout->elementsCount >= pVBLayout->elementsMaxSize) {
        perror("You exceeded the max amount of elements in a VBLayout\n");
    } else {
        *(pVBLayout->pElements + pVBLayout->elementsCount)
            = { GL_FLOAT, count, GL_FALSE };
        pVBLayout->stride += count * vbElementGetSizeOfType(GL_FLOAT);
        ++pVBLayout->elementsCount;
    }
}

void
vbLayoutPushUint(VertexBufferLayout* pVBLayout, u32 count) {
    if (pVBLayout->elementsCount >= pVBLayout->elementsMaxSize) {
        perror("You exceeded the max amount of elements in a VBLayout\n");
    } else {
        *(pVBLayout->pElements + pVBLayout->elementsCount)
            = { GL_UNSIGNED_INT, count, GL_FALSE };
        pVBLayout->stride += count * vbElementGetSizeOfType(GL_UNSIGNED_INT);
        ++pVBLayout->elementsCount;
    }
}

void 
vbLayoutPushUchar(VertexBufferLayout* pVBLayout, u32 count) {
    if (pVBLayout->elementsCount >= pVBLayout->elementsMaxSize) {
        perror("You exceeded the max amount of elements in a VBLayout\n");
    } else {
        *(pVBLayout->pElements + pVBLayout->elementsCount)
            = { GL_UNSIGNED_BYTE, count, GL_FALSE };
        pVBLayout->stride += count * vbElementGetSizeOfType(GL_UNSIGNED_BYTE);
        ++pVBLayout->elementsCount;
    }
}

inline void
initVB(u32* pVB) {
    glCall(glGenBuffers(1, pVB));
}

inline void
initVB(u32* pVB, const void* pData, u32 size) {
    glCall(glGenBuffers(1, pVB));
    glCall(glBindBuffer(GL_ARRAY_BUFFER, *pVB));
    glCall(glBufferData(GL_ARRAY_BUFFER, size, pData, GL_STATIC_DRAW));
}

inline void
freeVB(u32* pVB) {
    glCall(glDeleteBuffers(1, pVB));
}

inline void 
vbBind(u32 vb) {
    glCall(glBindBuffer(GL_ARRAY_BUFFER, vb));
}

inline void 
vbUnbind() {
    glCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

inline void 
vbSetData(u32 vb, const void* pData, u32 size) {
    glCall(glBindBuffer(GL_ARRAY_BUFFER, vb));
    glCall(glBufferData(GL_ARRAY_BUFFER, size, pData, GL_STATIC_DRAW));
}

inline void 
initVA(u32* pVA) {
    glCall(glGenVertexArrays(1, pVA));
}

inline void 
freeVA(u32* pVA) {
    glCall(glDeleteVertexArrays(1, pVA));
}

inline void
vaBind(u32 va) {
    glCall(glBindVertexArray(va));
}

inline void 
vaUnbind() {
    glCall(glBindVertexArray(0));
}

void 
vaAddBuffer(u32 va, u32 vb, VertexBufferLayout* pLayout) {
    vaBind(va);
    vbBind(vb);
    VertexBufferElement* pElements = pLayout->pElements;
    size_t offset = 0;
    for (u32 iElement = 0; iElement < pLayout->elementsCount; ++iElement) {
        VertexBufferElement* pElement = pElements + iElement;
        glCall(glVertexAttribPointer(iElement, 
                                     pElement->count, pElement->type, pElement->normalized,
                                     pLayout->stride, (void*)offset));
        glCall(glEnableVertexAttribArray(iElement));
        
        offset += pElement->count * vbElementGetSizeOfType(pElement->type);
    }
}

void 
vaAddBufferByLocation(u32 va, u32 vb, VertexBufferLayout* pLayout, u32 location) {
    vaBind(va);
    vbBind(vb);
    VertexBufferElement* pElements = pLayout->pElements;
    size_t offset = 0;
    VertexBufferElement* pElement = pElements;
    glCall(glVertexAttribPointer(location, 
                                 pElement->count, pElement->type, pElement->normalized,
                                 0, (void*)offset));
    glCall(glEnableVertexAttribArray(location));
}

inline void
dispatch(u32 x, u32 y, u32 z) {
    glCall(glDispatchCompute(x, y, z));
}

////////////////////////////////

//WINDOW

////////////////////////////////

inline void
framebufferSizeCallback(GLFWwindow* pWindow, s32 width, s32 height) {
    glViewport(0, 0, width, height);
}

b32 
startWindow(Window* pWindow) {
    if (!glfwInit()) {
        return false;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    pWindow->pInstance = glfwCreateWindow(pWindow->width, pWindow->height, 
                                          pWindow->pName, NULL, NULL);
    if (pWindow->pInstance == 0) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent((GLFWwindow*)pWindow->pInstance);
    glfwSetFramebufferSizeCallback((GLFWwindow*)pWindow->pInstance, 
                                   framebufferSizeCallback);
    glfwSetInputMode((GLFWwindow*)pWindow->pInstance, 
                     GLFW_CURSOR, 
                     GLFW_CURSOR_DISABLED);  
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return false;
    }
    return true;
}

b32  
stopWindow(Window* pWindow) {
    if (pWindow->pInstance != 0) {
        glfwDestroyWindow((GLFWwindow*)pWindow->pInstance);
    }
    pWindow->pInstance = 0;
    
    glfwTerminate();
    return true;
}

inline b32 
windowShouldClose(Window* pWindow) {
    if (pWindow->pInstance) {
        return glfwWindowShouldClose((GLFWwindow*)pWindow->pInstance);
    } else {
        return true;
    }
}

inline void 
pollEventsFromWindow(Window* pWindow) {
    if (glfwGetKey((GLFWwindow*)pWindow->pInstance, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glCall(glfwSetWindowShouldClose((GLFWwindow*)pWindow->pInstance, true));
    }
    glCall(glfwPollEvents());
}

inline b32
isKeyPressed(Renderer* pRenderer, u32 key) {
    if (glfwGetKey((GLFWwindow*)pRenderer->pWindow->pInstance, key) == GLFW_PRESS) {
        return true;
    }
    return false;
}

inline void
getMousePos(Window* pWindow, f64* pX, f64* pY) {
    glfwGetCursorPos((GLFWwindow*)pWindow->pInstance, pX, pY);
}


////////////////////////////////

//RENDERER

////////////////////////////////

inline b32 
startRenderer(Renderer* pRenderer, Window* pWindow, Camera* pCamera) {
    assert(pCamera != 0);
    initCamera(pCamera, HMM_Vec3(0.f, 5.f, 28.f));
    pRenderer->pCamera = pCamera;
    pRenderer->pWindow = pWindow;
    glCall(glEnable(GL_BLEND));
    glCall(glEnable(GL_DEPTH_TEST));
    //glCullFace(GL_BACK);
    //glDepthFunc(GL_EQUAL);
    glCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    //glCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    
    return true;
}

inline b32 
stopRenderer() {
    return true;
}

inline void 
clearRenderer() {
    glCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

inline void
fillColor(f32 red, f32 green, f32 blue) {
    glCall(glClearColor(red, green, blue, 1.f));
}

inline void
swapBuffers(Window* pWindow) {
    glCall(glfwSwapBuffers((GLFWwindow*)pWindow->pInstance));
}

inline void
drawBuffer(u32 offset, u32 count) {
    glCall(glDrawArrays(GL_TRIANGLES, offset, count));
}

inline void
drawBufferInstanced(u32 offset, u32 vertexCount, u32 instances) {
    glCall(glDrawArraysInstanced(GL_TRIANGLES, offset, vertexCount, instances));
}

inline void
drawPointsInstanced(u32 offset, u32 vertexCount, u32 instances) {
    glCall(glDrawArraysInstanced(GL_POINTS, offset, vertexCount, instances));
}

inline void 
drawBufferStrip(u32 offset, u32 count) {
    glCall(glDrawArrays(GL_TRIANGLE_STRIP, offset, count));
}

inline void
drawBufferFan(u32 offset, u32 count) {
    glCall(glDrawArrays(GL_TRIANGLE_FAN, offset, count));
}

inline void
drawElements(u32 count) {
    glCall(glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, 0));
}

inline void
drawElementsStrip(u32 count) {
    glCall(glDrawElements(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_INT, 0));
}

inline hmm_mat4
generateModel(Transform* pTransform)  {
    hmm_mat4 result = HMM_Scale(pTransform->scale);
    if (pTransform->isRotored) {
        result = getMat4FromRotor(pTransform->rotor) * result;
    } else {
        result = HMM_Rotate(pTransform->eulerAngles.x, VEC3_X) * result;
        result = HMM_Rotate(pTransform->eulerAngles.y, VEC3_Y) * result;
        result = HMM_Rotate(pTransform->eulerAngles.z, VEC3_Z) * result;
    }
    result = HMM_Translate(pTransform->position) * result;
    
    return result;
}

inline hmm_mat4 
getViewProj(Renderer* pRenderer) {
    return (pRenderer->pCamera->projection * getViewMatrix(pRenderer->pCamera));
}

inline hmm_mat4 
getModelViewProj(Renderer* pRenderer) {
    return (pRenderer->pCamera->projection *
            getViewMatrix(pRenderer->pCamera) *
            pRenderer->pCamera->model);
}

inline hmm_mat4 
getModelView(Renderer* pRenderer) {
    return (getViewMatrix(pRenderer->pCamera) * pRenderer->pCamera->model);
}

inline hmm_vec3
getCameraPosition(Renderer* pRenderer) {
    return pRenderer->pCamera->transform.position;
}

inline f32 
getCameraWidth(Renderer* pRenderer) {
    return pRenderer->pCamera->halfCamWidth * 2.f;
}

inline f32 
getCameraHeight(Renderer* pRenderer) {
    return pRenderer->pCamera->halfCamHeight * 2.f;
}
