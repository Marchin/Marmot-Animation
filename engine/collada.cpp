const char geometryTag[] = "<geometry";
const char meshTag[] = "<mesh";
const char floatArrayTag[] = "<float_array";
const char accessorSourceTag[] = "<accessor source";
const char trianglesTag[] = "<triangles";
const char pTag[] = "<p>";
const char openNodeTag[] = "<node";
const char closeNodeTag[] = "</node";
const char instanceGeometryTag[] = "<instance_geometry";
const char instanceMaterialTag[] = "<instance_material";
const char matrixTag[] = "<matrix";
const char imageIDTag[] = "<image id";
const char effectIDTag[] = "<effect id";
const char initFromTag[] = "<init_from";
const char materialTag[] = "<material";
#if 0
const char scaleTag[] = "<scale";
const char rotationTag[] = "<rotate";
const char translateTag[] = "<translate";
#endif

struct MaterialParserHelper {
    meow_hash materialHash;
    meow_hash effectHash;
};

struct EffectParserHelper {
    meow_hash effectHash;
    meow_hash textureHash;
};

struct TextureParserHelper {
    meow_hash textureHash;
    Texture* pTexture;
};

b32
drawNode(void* pData, Renderer* pRenderer) {
    Node* pNode = (Node*)pData;
    drawParsedModel(pNode->pModel, pRenderer, pNode->pTexture);
    
    return true;
}

#define SEEK_LINE_STARTING_WITH(text) \
ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile); \
while (strncmp(ptr, (text), strlen(text)) != 0) { \
ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile); \
} \

void
parseCollada(ModelStructure* pModelStructure, char* path) {
    const u32 kBufferSize = 8*1024;
    assert(pModelStructure);
    
    FILE* pFile = fopen(path, "r");
    char buffer[kBufferSize];
    
    if (pFile) {
        u32 materialCount = 0;
        u32 effectCount = 0;
        u32 textureCount = 0;
        u32 modelCount = 0;
        u32 nodeCount = 0;
        char* ptr = 0;
        while (!feof(pFile)) {
            ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
            if (strncmp(ptr, materialTag, strlen(materialTag)) == 0) {
                ++materialCount;
            }
            if (strncmp(ptr, effectIDTag, strlen(effectIDTag)) == 0) {
                ++effectCount;
            }
            if (strncmp(ptr, imageIDTag, strlen(imageIDTag)) == 0) {
                ++textureCount;
            }
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
        pModelStructure->textureCount = textureCount;
        pModelStructure->pModels = (ModelData*)malloc(modelCount * sizeof(ModelData));
        pModelStructure->pNodes = (Node*)malloc(nodeCount * sizeof(Node));
        pModelStructure->pTextures = (Texture*)malloc(textureCount * sizeof(Texture));
        
        initTransform(&pModelStructure->transform);
        strcpy(pModelStructure->transform.name, "Structure");
        
        MaterialParserHelper* pMaterialPHs = (MaterialParserHelper*)malloc(materialCount*sizeof(MaterialParserHelper));
        EffectParserHelper* pEffectPHs = (EffectParserHelper*)malloc(effectCount*sizeof(EffectParserHelper));
        TextureParserHelper* pTexturePHs = (TextureParserHelper*)malloc(textureCount*sizeof(TextureParserHelper));
        
        Transform* pCurrParent = &pModelStructure->transform;
        ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
        u32 currModelIndex = 0;
        u32 currNodeIndex = 0;
        u32 currMaterialIndex = 0;
        u32 currEffectIndex = 0;
        u32 currTextureIndex = 0;
        do {
            if (strncmp(ptr, materialTag, strlen(materialTag)) == 0) {
                ptr = seekAfterCharacter('\"', ptr);
                pMaterialPHs[currMaterialIndex].materialHash = 
                    MeowHash_Accelerated(0, seekCharacter('\"', ptr) - ptr, ptr);
                ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                ptr = seekAfterCharacter('#', ptr);
                pMaterialPHs[currMaterialIndex].effectHash = MeowHash_Accelerated(0, seekCharacter('\"', ptr) - ptr, ptr);
                ++currMaterialIndex;
            } else if (strncmp(ptr, effectIDTag, strlen(effectIDTag)) == 0) {
                ptr = seekAfterCharacter('\"', ptr);
                pEffectPHs[currEffectIndex].effectHash = MeowHash_Accelerated(0, seekCharacter('\"', ptr) - ptr, ptr);
                SEEK_LINE_STARTING_WITH(initFromTag);
                ptr = seekAfterCharacter('>', ptr);
                pEffectPHs[currEffectIndex].textureHash = MeowHash_Accelerated(0, seekCharacter('<', ptr) - ptr, ptr);
                ++currEffectIndex;
            } else if (strncmp(ptr, imageIDTag, strlen(imageIDTag)) == 0) {
                ptr = seekAfterCharacter('\"', ptr);
                pTexturePHs[currTextureIndex].textureHash = 
                    MeowHash_Accelerated(0, seekCharacter('\"', ptr) - ptr, ptr);
                ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                ptr = seekAfterCharacter('>', ptr);
                char textureNameBuffer[512];
                u64 size = seekCharacter('<', ptr) - ptr;
                memcpy(textureNameBuffer, ptr, size);
                textureNameBuffer[size] = 0;
                initTexture(pModelStructure->pTextures + currTextureIndex, textureNameBuffer, true);
                pTexturePHs[currTextureIndex].pTexture = pModelStructure->pTextures + currTextureIndex;
                ++currTextureIndex;
            } else if (strncmp(ptr, geometryTag, strlen(geometryTag)) == 0) {
                ModelData modelData = {};
                
                ptr = seekString("id", ptr);
                ptr += 4;
                modelData.idHash = MeowHash_Accelerated(0, seekCharacter('\"', ptr) - ptr, ptr);
                ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                if (strncmp(ptr, meshTag, strlen(meshTag)) == 0) {
                    SEEK_LINE_STARTING_WITH(floatArrayTag);
                    
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
                    
#if 0
                    // TODO(Marchin): wtf was this for?
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    while (strncmp(ptr, accessorSourceTag, strlen(accessorSourceTag)) != 0) {
                        ptr = seekString("id=\"", ptr);
                        char* idStart = ptr;
                        ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                    }
                    
                    const char strideText[] = "stride";
                    ptr = seekString(&strideText[0], ptr);
                    ptr += strlen(strideText) + 2 /* =" */;
                    assert(atoi(ptr) == 3);
#endif
                    
                    SEEK_LINE_STARTING_WITH(floatArrayTag);
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
                    
                    SEEK_LINE_STARTING_WITH(floatArrayTag);
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
                    
                    SEEK_LINE_STARTING_WITH(trianglesTag);
                    ptr = seekString(&countText[0], ptr);
                    ptr += strlen(countText) + 2 /* =" */;
                    u32 facesCount = atoi(ptr);
                    modelData.facesCount = facesCount;
                    modelData.pFaces = (Face*)malloc(facesCount*sizeof(Face));
                    
                    SEEK_LINE_STARTING_WITH(pTag);
                    
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
                pNode->pTexture = 0;
                
                initTransform(&pNode->transform);
                ptr = seekString("name", ptr);
                ptr += 6;
                char* name = ptr;
                ptr = seekCharacter('\"', ptr);
                
                strncpy(pNode->transform.name, name, ptr - name);
                
#if 0
                
                SEEK_LINE_STARTING_WITH(scaleTag);
                ptr = seekCharacter('>', ptr) + 1;
                for (u32 i = 0; i < 3; ++i) {
                    pNode->transform.scale.Elements[i] = (f32)atof(ptr);
                    ptr = seekNextString(ptr);
                }
                
                V3 eulerAngles = {};
                SEEK_LINE_STARTING_WITH(rotationTag);
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
                
                SEEK_LINE_STARTING_WITH(translateTag);
                ptr = seekCharacter('>', ptr) + 1;
                for (u32 i = 0; i < 3; ++i) {
                    pNode->transform.position.Elements[i] = (f32)atof(ptr);
                    ptr = seekNextString(ptr);
                }
#else
                SEEK_LINE_STARTING_WITH(matrixTag);
                
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
                pNode->transform.pEntity = pNode;
                pNode->transform.draw = drawNode;
                addChild(&pNode->transform, pCurrParent);
                pCurrParent = &pNode->transform;
                
                b32 abortParsing = false;
                ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                while (strncmp(ptr, (instanceGeometryTag), strlen(instanceGeometryTag)) != 0) {
                    if (strncmp(ptr, closeNodeTag, strlen(closeNodeTag)) == 0) {
                        abortParsing = true;
                        break;
                    }
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                } 
                
                if (abortParsing) {
                    continue;
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
                
                ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                while (strncmp(ptr, (instanceMaterialTag), strlen(instanceMaterialTag)) != 0) {
                    if (strncmp(ptr, closeNodeTag, strlen(closeNodeTag)) == 0) {
                        abortParsing = true;
                        break;
                    }
                    ptr = readLineAndSkipWhitespace(buffer, kBufferSize, pFile);
                } 
                
                if (abortParsing) {
                    continue;
                }
                
                ptr = seekAfterCharacter('\"', ptr);
                meow_hash materialHash = MeowHash_Accelerated(0, seekCharacter('\"', ptr) - ptr, ptr);
                
                MaterialParserHelper* pMaterialPH = 0;
                for (u32 iMaterial = 0; iMaterial < materialCount; ++iMaterial) {
                    if (MeowHashesAreEqual(pMaterialPHs[iMaterial].materialHash, materialHash)) {
                        pMaterialPH = pMaterialPHs + iMaterial;
                        break;
                    }
                }
                
                if (pMaterialPH) {
                    EffectParserHelper* pEffectPH = 0;
                    for (u32 iEffect = 0; iEffect < effectCount; ++iEffect) {
                        if (MeowHashesAreEqual(pEffectPHs[iEffect].effectHash, pMaterialPH->effectHash)) {
                            pEffectPH = pEffectPHs + iEffect;
                            break;
                        }
                    }
                    
                    if (pEffectPH) {
                        TextureParserHelper* pTexturePH = 0;
                        for (u32 iTexture = 0; iTexture < textureCount; ++iTexture) {
                            if (MeowHashesAreEqual(pTexturePHs[iTexture].textureHash, pEffectPH->textureHash)) {
                                pTexturePH = pTexturePHs + iTexture;
                                break;
                            }
                        }
                        
                        if (pTexturePH) {
                            pNode->pTexture = pTexturePH->pTexture;
                        }
                    }
                }
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