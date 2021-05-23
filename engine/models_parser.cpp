
 void
parseModel(ModelData* pModelData, char* path) {
    assert(pModelData);
    // zero struct pModelData
    
    FILE* pFile = fopen(path, "r");
    char buffer[512];
    
    if (pFile) {
        fseek(pFile, 0, SEEK_END);
        u32 size = ftell(pFile);
        fseek(pFile, 0, SEEK_SET);
        
        fgets(buffer, 512, pFile);
        do {
            if (buffer[0] == 'v') {
                if (buffer[1] == ' ') {
                    ++pModelData->vertexCount;
                } else if (buffer[1] == 't') {
                    ++pModelData->uvCount;
                }
            } else if (buffer[0] == 'f' && buffer[1] == ' ') {
                ++pModelData->facesCount;
            }
            fgets(buffer, 512, pFile);
        } while (!feof(pFile));
        
        fseek(pFile, 0, SEEK_SET);
        pModelData->pVertices = (V3*)malloc(pModelData->vertexCount*sizeof(V3));
        pModelData->pUV = (V2*)malloc(pModelData->uvCount*sizeof(V2));
        pModelData->pFaces = (Face*)malloc(pModelData->facesCount*sizeof(Face));
        u32 iVertex = 0;
        u32 iUV = 0;
        u32 iFace = 0;
        fgets(buffer, 512, pFile);
        do {
            if (buffer[0] == 'v') {
                if (buffer[1] == ' ') {
                    V3* pVertex = &pModelData->pVertices[iVertex];
                    u32 startIndex = 2;
                    for (u32 iCoord = 0; iCoord < 3; ++iCoord) {
                        pVertex->Elements[iCoord] = (f32)atof(buffer + startIndex);
                        while (buffer[startIndex] != ' ' && buffer[startIndex] != '\n') {
                            ++startIndex;
                        }
                        ++startIndex;
                    }
                    ++iVertex;
                } else if (buffer[1] == 't') {
                    V2* pUV = &pModelData->pUV[iUV];
                    u32 startIndex = 3;
                    for (u32 iCoord = 0; iCoord < 2; ++iCoord) {
                        pUV->Elements[iCoord] = (f32)atof(buffer + startIndex);
                        while (buffer[startIndex] != ' ' && buffer[startIndex] != '\n') {
                            ++startIndex;
                        }
                        ++startIndex;
                    }
                    ++iUV;
                }
            }
            if (buffer[0] == 'f' && buffer[1] == ' ') {
                u32 index = 0;
                u32 startIndex = 2;
                do {
                    pModelData->pFaces[iFace].vertexIndices[index] = atoi(buffer + startIndex++) - 1;
                    if (buffer[startIndex] == '/') {
                        pModelData->pFaces[iFace].uvIndices[index] = atoi(buffer + ++startIndex) - 1;
                    }
                    while (buffer[startIndex] != ' ' && buffer[startIndex] != '\n') {
                        ++startIndex;
                    }
                    if (buffer[startIndex] != '\n') {
                        ++startIndex;
                    }
                    ++index;
                    assert(index <= 3);
                } while (buffer[startIndex] != '\n');
                ++iFace;
            }
            fgets(buffer, 512, pFile);
        } while (!feof(pFile));
    }
}

void
initParsedModel(ModelData* pModelData) {
    pModelData->component.id = MODEL_DATA;
    
    initShader(&pModelData->shader, "Texture", "..//resources//shaders//vModel.glsl", 
               "..//resources//shaders//fModel.glsl");
    
    initVA(&pModelData->va);
    vaBind(pModelData->va);
    
#if 0
    pModelData->transform.pEntity = pModelData;
    pModelData->transform.draw = drawParsedModel;
    initTransform(&pModelData->transform);
#else
#endif
    
    u32 dataSize = pModelData->vertexCount*sizeof(pModelData->pVertices[0]) *
        pModelData->uvCount*sizeof(pModelData->pUV[0])* sizeof(f32);
    f32* pData = (f32*)malloc(dataSize);
    for (u32 iFace = 0, index = 0; iFace < pModelData->facesCount; ++iFace) {
        for (u32 iVertex = 0; iVertex < 3; ++iVertex) {
            pData[index++] = pModelData->pVertices[pModelData->pFaces[iFace].vertexIndices[iVertex]].x;
            pData[index++] = pModelData->pVertices[pModelData->pFaces[iFace].vertexIndices[iVertex]].y;
            pData[index++] = pModelData->pVertices[pModelData->pFaces[iFace].vertexIndices[iVertex]].z;
            
            pData[index++] = pModelData->pUV[pModelData->pFaces[iFace].uvIndices[iVertex]].u;
            pData[index++] = pModelData->pUV[pModelData->pFaces[iFace].uvIndices[iVertex]].v;
        }
    }
    initVB(&pModelData->vb, pData, dataSize);
    vbBind(pModelData->vb);
    VertexBufferLayout layout = {};
    u32 layoutsAmount = 2;
    layout.pElements = (VertexBufferElement*)malloc(layoutsAmount*sizeof(VertexBufferElement));
    memset(layout.pElements, 0, layoutsAmount*sizeof(VertexBufferElement));
    layout.elementsMaxSize = layoutsAmount;
    vbLayoutPushFloat(&layout, 3);
    vbLayoutPushFloat(&layout, 2);
    
    vaAddBuffer(pModelData->va, pModelData->vb, &layout);
    free(layout.pElements);
    vaUnbind();
}

b32
drawParsedModel(ModelData* pModelData, Renderer* pRenderer, Texture* pTexture) {
    shaderBindID(pModelData->shader.id);
    Mat4 mvp = getModelViewProj(pRenderer);
    shaderSetMat4(&pModelData->shader, "uModelViewProjection", &mvp);
    if (pTexture) {
        textureBindID(pTexture->id, 0);
    }
    shaderSetInt(&pModelData->shader, "texture_diffuse1", 0);
    vaBind(pModelData->va);
    drawBuffer(0, pModelData->facesCount*3);
    vaUnbind();
    
    return true;
}