/*
===========================================================================
    Copyright (C) 2018-2022 Adriano Di Dio.
    
    MOHLevelViewer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MOHLevelViewer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MOHLevelViewer.  If not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/ 
#include "MOHLevelViewer.h"

void VAOFree(VAO_t *VAO)
{
    VAO_t *Temp;
    while( VAO ) {
        glDeleteBuffers(1, VAO->VBOId);
        glDeleteVertexArrays(1, VAO->VAOId);
        Temp = VAO;
        VAO = VAO->Next;
        free(Temp);
    }
}
void VAOUpdate(VAO_t *VAO,int *Data,int DataSize,int NumElements)
{
    int NextSlot;
    NextSlot = VAO->CurrentSize * VAO->Stride;
    if( NextSlot > VAO->Size ) {
        DPrintf("VAOUpdate:Overflow detected...\n");
        assert(1!=1);
        return;
    }
    glBindBuffer(GL_ARRAY_BUFFER, VAO->VBOId[0]);
    glBufferSubData(GL_ARRAY_BUFFER, VAO->CurrentSize * VAO->Stride, DataSize, Data);
    glBindBuffer(GL_ARRAY_BUFFER,0);
    VAO->CurrentSize += NumElements;
}
VAO_t *VAOInitXYZUVRGB(float *Data,int DataSize,int Stride,int VertexOffset,int TextureOffset,int ColorOffset,short TSB,int TextureId,int Count)
{
    VAO_t *VAO;
    
    VAO = malloc(sizeof(VAO_t));
    
    glGenVertexArrays(1, &VAO->VAOId[0]);
    glBindVertexArray(VAO->VAOId[0]);
        
    glGenBuffers(1, VAO->VBOId);
    glBindBuffer(GL_ARRAY_BUFFER, VAO->VBOId[0]);
            
    glBufferData(GL_ARRAY_BUFFER, DataSize,Data, GL_STATIC_DRAW);
        
    glVertexAttribPointer(0,3,GL_FLOAT,false,Stride,BUFFER_OFFSET(VertexOffset));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,false,Stride,BUFFER_OFFSET(TextureOffset));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,3,GL_FLOAT,false,Stride,BUFFER_OFFSET(ColorOffset));
    glEnableVertexAttribArray(2);

    VAO->TSB = TSB;
    VAO->TextureId = TextureId;
    VAO->Next = NULL;
    VAO->Count = Count;
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    
    return VAO;
}
VAO_t *VAOInitXYZUVRGBCLUTColorModeInteger(int *Data,int DataSize,int Stride,int VertexOffset,int TextureOffset,int ColorOffset,int CLUTOffset,int ColorModeOffset,
                                           int Count)
{
    VAO_t *VAO;
    
    VAO = malloc(sizeof(VAO_t));
    
    glGenVertexArrays(1, &VAO->VAOId[0]);
    glBindVertexArray(VAO->VAOId[0]);
        
    glGenBuffers(1, VAO->VBOId);
    glBindBuffer(GL_ARRAY_BUFFER, VAO->VBOId[0]);
            
    glBufferData(GL_ARRAY_BUFFER, DataSize,Data, GL_DYNAMIC_DRAW);
        
    glVertexAttribIPointer(0,3,GL_INT,Stride,BUFFER_INT_OFFSET(VertexOffset));
    glEnableVertexAttribArray(0);
    glVertexAttribIPointer(1,2,GL_INT,Stride,BUFFER_INT_OFFSET(TextureOffset));
    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(2,3,GL_INT,Stride,BUFFER_INT_OFFSET(ColorOffset));
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(3,2,GL_INT,Stride,BUFFER_INT_OFFSET(CLUTOffset));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(4,1,GL_INT,Stride,BUFFER_INT_OFFSET(ColorModeOffset));
    glEnableVertexAttribArray(4);
    VAO->TSB = -1;
    VAO->TextureId = -1;
    VAO->Next = NULL;
    VAO->CurrentSize = 0;
    VAO->Stride = Stride;
    VAO->Size = DataSize;
    VAO->Count = Count;
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    
    return VAO;

}
VAO_t *VAOInitXYUVRGB(float *Data,int DataSize,int Stride,int VertexOffset,int TextureOffset,int ColorOffset,short TSB,int TextureId,
    bool StaticDraw)
{
    VAO_t *VAO;
    
    VAO = malloc(sizeof(VAO_t));
    
    glGenVertexArrays(1, &VAO->VAOId[0]);
    glBindVertexArray(VAO->VAOId[0]);
        
    glGenBuffers(1, VAO->VBOId);
    glBindBuffer(GL_ARRAY_BUFFER, VAO->VBOId[0]);
            
    glBufferData(GL_ARRAY_BUFFER, DataSize,Data, StaticDraw ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
        
    glVertexAttribPointer(0,2,GL_FLOAT,false,Stride,BUFFER_OFFSET(VertexOffset));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,false,Stride,BUFFER_OFFSET(TextureOffset));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,3,GL_FLOAT,false,Stride,BUFFER_OFFSET(ColorOffset));
    glEnableVertexAttribArray(2);

    VAO->TSB = TSB;
    VAO->TextureId = TextureId;
    VAO->Next = NULL;
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    
    return VAO;
}

VAO_t *VAOInitXYRGB(float *Data,int DataSize,int Stride,int VertexOffset,int ColorOffset,bool StaticDraw)
{
    VAO_t *VAO;
    
    VAO = malloc(sizeof(VAO_t));
    
    glGenVertexArrays(1, &VAO->VAOId[0]);
    glBindVertexArray(VAO->VAOId[0]);
        
    glGenBuffers(1, VAO->VBOId);
    glBindBuffer(GL_ARRAY_BUFFER, VAO->VBOId[0]);
            
    glBufferData(GL_ARRAY_BUFFER, DataSize,Data, StaticDraw ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
        
    glVertexAttribPointer(0,2,GL_FLOAT,false,Stride,BUFFER_OFFSET(VertexOffset));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,false,Stride,BUFFER_OFFSET(ColorOffset));
    glEnableVertexAttribArray(1);

    VAO->TSB = -1;
    VAO->TextureId = -1;
    VAO->Next = NULL;
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    
    return VAO;
}

VAO_t *VAOInitXYUV(float *Data,int DataSize,int Stride,int VertexOffset,int TextureOffset,short TSB,int TextureId,bool StaticDraw)
{
    VAO_t *VAO;
    
    VAO = malloc(sizeof(VAO_t));
    
    glGenVertexArrays(1, &VAO->VAOId[0]);
    glBindVertexArray(VAO->VAOId[0]);
        
    glGenBuffers(1, VAO->VBOId);
    glBindBuffer(GL_ARRAY_BUFFER, VAO->VBOId[0]);
            
    glBufferData(GL_ARRAY_BUFFER, DataSize,Data, StaticDraw ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
        
    glVertexAttribPointer(0,2,GL_FLOAT,false,Stride,BUFFER_OFFSET(VertexOffset));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,false,Stride,BUFFER_OFFSET(TextureOffset));
    glEnableVertexAttribArray(1);

    VAO->TSB = TSB;
    VAO->TextureId = TextureId;
    VAO->Next = NULL;
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    
    return VAO;
}

VAO_t *VAOInitXYZUV(float *Data,int DataSize,int Stride,int VertexOffset,int TextureOffset,short TSB,int TextureId,int Count)
{
    VAO_t *VAO;
    
    VAO = malloc(sizeof(VAO_t));
    
    glGenVertexArrays(1, &VAO->VAOId[0]);
    glBindVertexArray(VAO->VAOId[0]);
        
    glGenBuffers(1, VAO->VBOId);
    glBindBuffer(GL_ARRAY_BUFFER, VAO->VBOId[0]);
            
    glBufferData(GL_ARRAY_BUFFER, DataSize,Data, GL_STATIC_DRAW);
        
    glVertexAttribPointer(0,3,GL_FLOAT,false,Stride,BUFFER_OFFSET(VertexOffset));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,false,Stride,BUFFER_OFFSET(TextureOffset));
    glEnableVertexAttribArray(1);

    VAO->TSB = TSB;
    VAO->TextureId = TextureId;
    VAO->Count = Count;
    VAO->Next = NULL;
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    
    return VAO;
}

VAO_t *VAOInitXYZRGB(float *Data,int DataSize,int Stride,int VertexOffset,int ColorOffset,int DynamicDraw)
{
    VAO_t *VAO;
    
    VAO = malloc(sizeof(VAO_t));
    
    glGenVertexArrays(1, &VAO->VAOId[0]);
    glBindVertexArray(VAO->VAOId[0]);
        
    glGenBuffers(1, VAO->VBOId);
    glBindBuffer(GL_ARRAY_BUFFER, VAO->VBOId[0]);
            
    glBufferData(GL_ARRAY_BUFFER, DataSize,Data, DynamicDraw == 1 ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        
    glVertexAttribPointer(0,3,GL_FLOAT,false,Stride,BUFFER_OFFSET(VertexOffset));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,false,Stride,BUFFER_OFFSET(ColorOffset));
    glEnableVertexAttribArray(1);

    VAO->TSB = -1;
    VAO->TextureId = -1;
    VAO->Next = NULL;
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    
    return VAO;
}

VAO_t *VAOInitXYZRGBIBO(float *Data,int DataSize,int Stride,unsigned short *Index,int IndexSize,int VertexOffset,int ColorOffset)
{
    VAO_t *VAO;
    
    VAO = malloc(sizeof(VAO_t));
    
    glGenVertexArrays(1, &VAO->VAOId[0]);
    glBindVertexArray(VAO->VAOId[0]);
        
    glGenBuffers(1, VAO->VBOId);
    glBindBuffer(GL_ARRAY_BUFFER, VAO->VBOId[0]);
    glBufferData(GL_ARRAY_BUFFER, DataSize,Data, GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,false,Stride,BUFFER_OFFSET(VertexOffset));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,false,Stride,BUFFER_OFFSET(ColorOffset));
    glEnableVertexAttribArray(1);
    
    glGenBuffers(1, VAO->IBOId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VAO->IBOId[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexSize,Index,GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    

    VAO->TSB = -1;
    VAO->TextureId = -1;
    VAO->Next = NULL;
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    
    return VAO;
}

VAO_t *VAOInitXYZ(float *Data,int DataSize,int Stride,int VertexOffset,int Count)
{
    VAO_t *VAO;
    
    VAO = malloc(sizeof(VAO_t));
    
    glGenVertexArrays(1, &VAO->VAOId[0]);
    glBindVertexArray(VAO->VAOId[0]);
        
    glGenBuffers(1, VAO->VBOId);
    glBindBuffer(GL_ARRAY_BUFFER, VAO->VBOId[0]);
            
    glBufferData(GL_ARRAY_BUFFER, DataSize,Data, GL_STATIC_DRAW);
        
    glVertexAttribPointer(0,3,GL_FLOAT,false,Stride,BUFFER_OFFSET(VertexOffset));
    glEnableVertexAttribArray(0);

    VAO->TSB = -1;
    VAO->TextureId = -1;
    VAO->Count = Count;
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    glBindVertexArray(0);
    
    return VAO;
}
