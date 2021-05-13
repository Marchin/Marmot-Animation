
inline char*
skipWhitespace(char* string) {
    char* pResult = string;
    while (*pResult  == ' ') {
        ++pResult;
    }
    
    return pResult;
}

inline char*
readLineAndSkipWhitespace(char* buffer, u32 bufferSize, FILE* pFile) {
    fgets(buffer, bufferSize, pFile);
    char* pResult = skipWhitespace(buffer);
    
    return pResult;
}

inline char*
seekCharacter(char character, char* buffer) {
    char* pResult = 0;
    if (buffer != 0) {
        while (*buffer && *buffer != character) {
            ++buffer;
        }
        pResult = buffer;
    }
    return pResult;
}

inline char*
seekString(const char* string, char* buffer) {
    char* pResult = 0;
    u32 counter = 0;
    u32 strLength = (u32)strlen(string);
    if (buffer != 0 && strLength != 0) {
        while (*buffer) {
            counter = (*buffer == string[counter]) ? counter + 1 : 0;
            ++buffer;
            if (counter == strLength) {
                buffer -= strLength;
                break;
            }
        }
        pResult = buffer;
    }
    return pResult;
}


inline char*
seekNextString(char* string) {
    char* pResult = 0;
    
    if (string != 0) {
        b32 oldStringFinished = false;
        while (*string != 0 && *string++ != ' ') { }
        if (*string != 0) {
            while (*string != 0 && *string == ' ') {
                ++string;
            }
            pResult = string;
        }
    }
    
    return pResult;
}

const char geometryTag[] = "<geometry";
const char meshTag[] = "<mesh";
const char floatArrayTag[] = "<float_array";
const char accessorSourceTag[] = "<accessor source";
const char trianglesTag[] = "<triangles";
const char pTag[] = "<p>";
const char openNodeTag[] = "<node";
const char closeNodeTag[] = "</node";
const char instanceGeometryTag[] = "<instance_geometry";
const char matrixTag[] = "<matrix";
const char scaleTag[] = "<scale";
const char rotationTag[] = "<rotate";
const char translateTag[] = "<translate";

b32
drawNode(void* pData, Renderer* pRenderer) {
    Node* pNode = (Node*)pData;
    drawParsedModel(pNode->pModel, pRenderer);
    
    return true;
}

void
parseCollada(ModelStructure* pModelStructure, char* path) {
    const u32 kBufferSize = 8*1024;
    assert(pModelStructure);
    
    FILE* pFile = fopen(path, "r");
    char buffer[kBufferSize];
    
    if (pFile) {
        u32 modelCount = 0;
        u32 nodeCount = 0;
        char* ptr = 0;
        while (!feof(pFile)) {
            ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
            if (strncmp(ptr, geometryTag, strlen(geometryTag)) == 0) {
                ++modelCount;
            }
            if (strncmp(ptr, openNodeTag, strlen(openNodeTag)) == 0) {
                ++nodeCount;
            }
        }
        fseek(pFile, 0, SEEK_SET);
        
        *pModelStructure = {};
        pModelStructure->modelCount = modelCount;
        pModelStructure->nodeCount = nodeCount;
        pModelStructure->pModels = (ModelData*)malloc(modelCount * sizeof(ModelData));
        pModelStructure->pNodes = (Node*)malloc(nodeCount * sizeof(Node));
        
        initTransform(&pModelStructure->transform);
        strcpy(pModelStructure->transform.name, "Structure");
        Transform* pCurrParent = &pModelStructure->transform;
        
        ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
        u32 currModelIndex = 0;
        u32 currNodeIndex = 0;
        do {
            if (strncmp(ptr, geometryTag, strlen(geometryTag)) == 0) {
                ModelData modelData = {};
                
                ptr = seekString("id", ptr);
                ptr += 4;
                modelData.idHash = MeowHash_Accelerated(0, seekCharacter('\"', ptr) - ptr, ptr);
                ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                if (strncmp(ptr, meshTag, strlen(meshTag)) == 0) {
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    while (strncmp(ptr, floatArrayTag, strlen(floatArrayTag)) != 0) {
                        ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    }
                    
                    const char countText[] = "count";
                    ptr = seekString(&countText[0], ptr);
                    ptr += strlen(countText) + 2 /* =" */;
                    u32 vertexCount = atoi(ptr) / 3;
                    modelData.vertexCount = vertexCount;
                    modelData.pVertices = (V3*)malloc(vertexCount*sizeof(V3));
                    
                    ptr = seekCharacter('>', ptr) + 1;
                    for (u32 iVertex = 0; iVertex < vertexCount; ++iVertex) {
                        V3 vertex = {};
                        vertex.x = (f32)atof(ptr);
                        ptr = seekNextString(ptr);
                        vertex.y = (f32)atof(ptr);
                        ptr = seekNextString(ptr);
                        vertex.z = (f32)atof(ptr);
                        ptr = seekNextString(ptr);
                        modelData.pVertices[iVertex] = vertex;
                    }
                    
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    while (strncmp(ptr, accessorSourceTag, strlen(accessorSourceTag)) != 0) {
                        ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    }
                    
                    const char strideText[] = "stride";
                    ptr = seekString(&strideText[0], ptr);
                    ptr += strlen(strideText) + 2 /* =" */;
                    assert(atoi(ptr) == 3);
                    
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    while (strncmp(ptr, floatArrayTag, strlen(floatArrayTag)) != 0) {
                        ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    }
                    ptr = seekString(&countText[0], ptr);
                    ptr += strlen(countText) + 2 /* =" */;
                    u32 normalCount = atoi(ptr) / 3;
                    modelData.normalCount = normalCount;
                    modelData.pNormals = (V3*)malloc(normalCount*sizeof(V3));
                    
                    ptr = seekCharacter('>', ptr) + 1;
                    for (u32 iNormal = 0; iNormal < normalCount; ++iNormal) {
                        V3 normal = {};
                        normal.x = (f32)atof(ptr);
                        ptr = seekNextString(ptr);
                        normal.y = (f32)atof(ptr);
                        ptr = seekNextString(ptr);
                        normal.z = (f32)atof(ptr);
                        ptr = seekNextString(ptr);
                        modelData.pNormals[iNormal] = normal;
                    }
                    
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    while (strncmp(ptr, floatArrayTag, strlen(floatArrayTag)) != 0) {
                        ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    }
                    ptr = seekString(&countText[0], ptr);
                    ptr += strlen(countText) + 2 /* =" */;
                    u32 uvCount = atoi(ptr) / 2;
                    modelData.uvCount = uvCount;
                    modelData.pUV = (V2*)malloc(uvCount*sizeof(V2));
                    
                    ptr = seekCharacter('>', ptr) + 1;
                    for (u32 iUV = 0; iUV < uvCount; ++iUV) {
                        V2 uv = {};
                        uv.u = (f32)atof(ptr);
                        ptr = seekNextString(ptr);
                        uv.v = (f32)atof(ptr);
                        ptr = seekNextString(ptr);
                        modelData.pUV[iUV] = uv;
                    }
                    
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    while (strncmp(ptr, trianglesTag, strlen(trianglesTag)) != 0) {
                        ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    }
                    ptr = seekString(&countText[0], ptr);
                    ptr += strlen(countText) + 2 /* =" */;
                    u32 facesCount = atoi(ptr);
                    modelData.facesCount = facesCount;
                    modelData.pFaces = (Face*)malloc(facesCount*sizeof(Face));
                    
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    while (strncmp(ptr, pTag, strlen(pTag)) != 0) {
                        ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    }
                    
                    ptr = seekCharacter('>', ptr) + 1;
                    for (u32 iFace = 0; iFace < facesCount; ++iFace) {
                        Face face = {};
                        face.vertexIndices[0] = atoi(ptr);
                        ptr = seekNextString(ptr);
                        face.normalIndices[0] = atoi(ptr);
                        ptr = seekNextString(ptr);
                        face.uvIndices[0] = atoi(ptr);
                        ptr = seekNextString(ptr);
                        face.vertexIndices[1] = atoi(ptr);
                        ptr = seekNextString(ptr);
                        face.normalIndices[1] = atoi(ptr);
                        ptr = seekNextString(ptr);
                        face.uvIndices[1] = atoi(ptr);
                        ptr = seekNextString(ptr);
                        face.vertexIndices[2] = atoi(ptr);
                        ptr = seekNextString(ptr);
                        face.normalIndices[2] = atoi(ptr);
                        ptr = seekNextString(ptr);
                        face.uvIndices[2] = atoi(ptr);
                        ptr = seekNextString(ptr);
                        modelData.pFaces[iFace] = face;
                    }
                    initParsedModel(&modelData);
                    pModelStructure->pModels[currModelIndex++] = modelData;
                }
            } else if (strncmp(ptr, openNodeTag, strlen(openNodeTag)) == 0) {
                Node* pNode = pModelStructure->pNodes + currNodeIndex++;
                
                initTransform(&pNode->transform);
                ptr = seekString("name", ptr);
                ptr += 6;
                char* name = ptr;
                ptr = seekCharacter('\"', ptr);
                
                strncpy(pNode->transform.name, name, ptr - name);
                
#if 0
                ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                while (strncmp(ptr, scaleTag, strlen(scaleTag)) != 0) {
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                }
                ptr = seekCharacter('>', ptr) + 1;
                for (u32 i = 0; i < 3; ++i) {
                    pNode->transform.scale.Elements[i] = (f32)atof(ptr);
                    ptr = seekNextString(ptr);
                }
                
                V3 eulerAngles = {};
                ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                while (strncmp(ptr, rotationTag, strlen(rotationTag)) != 0) {
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                }
                for (u32 i = 0; i < 3; ++i) {
                    ptr = seekCharacter('>', ptr) + 1;
                    ptr = seekNextString(ptr);
                    ptr = seekNextString(ptr);
                    ptr = seekNextString(ptr);
                    eulerAngles.Elements[2 - i] = (f32)atof(ptr);
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                }
                Rotor3 rotorZ = rotorFromAngleAndAxis(eulerAngles.z, VEC3_Z);
                Rotor3 rotorY = rotorFromAngleAndAxis(eulerAngles.y, VEC3_Y);
                Rotor3 rotorX = rotorFromAngleAndAxis(eulerAngles.x, VEC3_X);
                //pNode->transform.rotor = rotorX*rotorY*rotorZ;
                pNode->transform.rotor = rotorZ*rotorY*rotorX;
                
                ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                while (strncmp(ptr, translateTag, strlen(translateTag)) != 0) {
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                }
                ptr = seekCharacter('>', ptr) + 1;
                for (u32 i = 0; i < 3; ++i) {
                    pNode->transform.position.Elements[i] = (f32)atof(ptr);
                    ptr = seekNextString(ptr);
                }
#else
                ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                while (strncmp(ptr, matrixTag, strlen(matrixTag)) != 0) {
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                }
                
                Mat4 transformMatrix;
                ptr = seekCharacter('>', ptr) + 1;
                for (u32 i = 0; i < 4; ++i) {
                    for (u32 j = 0; j < 4; ++j) {
                        transformMatrix.Elements[i][j] = (f32)atof(ptr);
                        ptr = seekNextString(ptr);
                    }
                }
                
                pNode->transform.position.x = transformMatrix.Elements[0][3];
                pNode->transform.position.y = transformMatrix.Elements[1][3];
                pNode->transform.position.z = transformMatrix.Elements[2][3];
                pNode->transform.scale.x = HMM_LengthVec3(HMM_Vec3(transformMatrix.Elements[0][0],
                                                                   transformMatrix.Elements[1][0],
                                                                   transformMatrix.Elements[2][0]));
                pNode->transform.scale.y = HMM_LengthVec3(HMM_Vec3(transformMatrix.Elements[0][1],
                                                                   transformMatrix.Elements[1][1],
                                                                   transformMatrix.Elements[2][1]));
                pNode->transform.scale.z = HMM_LengthVec3(HMM_Vec3(transformMatrix.Elements[0][2],
                                                                   transformMatrix.Elements[1][2],
                                                                   transformMatrix.Elements[2][2]));
                
                
                transformMatrix.Elements[0][3] = 0.0f;
                transformMatrix.Elements[1][3] = 0.0f;
                transformMatrix.Elements[2][3] = 0.0f;
                
                transformMatrix.Elements[0][2] /= pNode->transform.scale.z;
                transformMatrix.Elements[1][2] /= pNode->transform.scale.z;
                transformMatrix.Elements[2][2] /= pNode->transform.scale.z;
                
                transformMatrix.Elements[0][1] /= pNode->transform.scale.y;
                transformMatrix.Elements[1][1] /= pNode->transform.scale.y;
                transformMatrix.Elements[2][1] /= pNode->transform.scale.y;
                
                transformMatrix.Elements[0][0] /= pNode->transform.scale.x;
                transformMatrix.Elements[1][0] /= pNode->transform.scale.x;
                transformMatrix.Elements[2][0] /= pNode->transform.scale.x;
                
                pNode->transform.rotor = getRotorFromMat4(transformMatrix);
#endif
                
                ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                while (strncmp(ptr, instanceGeometryTag, strlen(instanceGeometryTag)) != 0) {
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                }
                ptr = seekString("url", ptr);
                ptr += 6; // url="#
                meow_hash geometryHash = MeowHash_Accelerated(0, seekCharacter('\"', ptr) - ptr, ptr);
                
                for (u32 iModel = 0; iModel < modelCount; ++iModel) {
                    if (MeowHashesAreEqual(geometryHash, pModelStructure->pModels[iModel].idHash)) {
                        pNode->pModel = pModelStructure->pModels + iModel;
                        break;
                    }
                }
                
                pNode->transform.pEntity = pNode;
                pNode->transform.draw = drawNode;
                addChild(&pNode->transform, pCurrParent);
                pCurrParent = &pNode->transform;
            } else if (strncmp(ptr, closeNodeTag, strlen(closeNodeTag)) == 0) {
                pCurrParent = pCurrParent->pParent;
            }
            
            ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
        } while (!feof(pFile));
        
        fclose(pFile);
    }
}
/*
pNode->transform.position.x = transformMatrix.Elements[0][3];
                pNode->transform.position.y = transformMatrix.Elements[1][3];
                pNode->transform.position.z = transformMatrix.Elements[2][3];
                pNode->transform.scale.x = HMM_LengthVec3(HMM_Vec3(transformMatrix.Elements[0][0],
                                                                   transformMatrix.Elements[1][0],
                                                                   transformMatrix.Elements[2][0]));
                pNode->transform.scale.y = HMM_LengthVec3(HMM_Vec3(transformMatrix.Elements[0][1],
                                                                   transformMatrix.Elements[1][1],
                                                                   transformMatrix.Elements[2][1]));
                pNode->transform.scale.z = HMM_LengthVec3(HMM_Vec3(transformMatrix.Elements[0][2],
                                                                   transformMatrix.Elements[1][2],
                                                                   transformMatrix.Elements[2][2]));
                pNode->transform.rotor = getRotorFromMat4(transformMatrix);
                
*/