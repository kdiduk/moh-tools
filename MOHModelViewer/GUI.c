// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*
===========================================================================
    Copyright (C) 2018-2022 Adriano Di Dio.
    
    MOHModelViewer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    MOHModelViewer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with MOHModelViewer.  If not, see <http://www.gnu.org/licenses/>.
===========================================================================
*/

#include "GUI.h"
#include "VRAM.h"
#include "MOHModelViewer.h"

Config_t *GUIFont;
Config_t *GUIFontSize;
Config_t *GUIShowFPS;

const VSyncSettings_t VSyncOptions[] = { 
    
    {
        "Disable",
        0
    },
    {
        "Standard",
        1
    },
    {
        "Adaptive",
        -1
    }
};

int NumVSyncOptions = sizeof(VSyncOptions) / sizeof(VSyncOptions[0]);

void GUIReleaseContext(ImGuiContext *Context)
{    
    igSetCurrentContext(Context);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    igDestroyContext(Context);
}
void GUIFreeDialogList(GUI_t *GUI)
{
    GUIFileDialog_t *Temp;
    
    while( GUI->FileDialogList ) {
        free(GUI->FileDialogList->WindowTitle);
        free(GUI->FileDialogList->Key);
        if( GUI->FileDialogList->Filters ) {
            free(GUI->FileDialogList->Filters);
        }
        IGFD_Destroy(GUI->FileDialogList->Window);
        Temp = GUI->FileDialogList;
        GUI->FileDialogList = GUI->FileDialogList->Next;
        free(Temp);
    }
}
void GUIFree(GUI_t *GUI)
{
    GUIReleaseContext(GUI->DefaultContext);
    GUIReleaseContext(GUI->ProgressBar->Context);
    GUIFreeDialogList(GUI);
    
    if( GUI->ProgressBar->DialogTitle ) {
        free(GUI->ProgressBar->DialogTitle);
    }
    if( GUI->ErrorMessage ) {
        free(GUI->ErrorMessage);
    }
    free(GUI->ConfigFilePath);
    free(GUI->ProgressBar);
    free(GUI);
}

void GUIPrepareModalWindow()
{
    ImGuiIO *IO;
    ImVec2 Pivot; 
    ImVec2 ModalPosition;
    
    IO = igGetIO();
    Pivot.x = 0.5f;
    Pivot.y = 0.5f;
    ModalPosition.x = IO->DisplaySize.x * 0.5;
    ModalPosition.y = IO->DisplaySize.y * 0.5;
    igSetNextWindowPos(ModalPosition, ImGuiCond_Always, Pivot);
}
/*
 * Process a new event from the SDL system only when
 * it is active.
 * Returns 0 if GUI has ignored the event 1 otherwise.
 */
int GUIProcessEvent(GUI_t *GUI,SDL_Event *Event)
{
//     if( !GUIIsActive(GUI) ) {
//         return 0;
//     }
    ImGui_ImplSDL2_ProcessEvent(Event);
    return 1;
}

int GUIIsActive(GUI_t *GUI)
{
    return GUI->NumActiveWindows > 0;
}

/*
 Utilities functions to detect GUI status.
 Push => Signal that a window is visible and update the cursor status to handle it.
 Pop => Signal that a window has been closed and if there are no windows opened then it hides the cursor since it is not needed anymore.
 */
void GUIPushWindow(GUI_t *GUI)
{
    if( !GUI->NumActiveWindows ) {
        //NOTE(Adriano):We have pushed a new window to the stack release the mouse and show
        //the cursor.
        VideoSystemGrabMouse(0);
    }
    GUI->NumActiveWindows++;
}
void GUIPopWindow(GUI_t *GUI)
{
    GUI->NumActiveWindows--;
    if( !GUI->NumActiveWindows ) {
        //NOTE(Adriano):All the windows have been closed...grab the mouse!
        VideoSystemGrabMouse(1);
    }
}
/*
 * Push or Pop a window from the stack based on the handle value.
 */
void GUIUpdateWindowStack(GUI_t *GUI,int HandleValue)
{
    if( HandleValue ) {
        GUIPushWindow(GUI);
    } else {
        GUIPopWindow(GUI);
    }
}
void GUIToggleDebugWindow(GUI_t *GUI)
{
    GUI->DebugWindowHandle = !GUI->DebugWindowHandle;
    GUIUpdateWindowStack(GUI,GUI->DebugWindowHandle);
}
void GUIToggleVideoSettingsWindow(GUI_t *GUI)
{
    GUI->VideoSettingsWindowHandle = !GUI->VideoSettingsWindowHandle;
    GUIUpdateWindowStack(GUI,GUI->VideoSettingsWindowHandle);

}
void GUIToggleLevelSelectWindow(GUI_t *GUI)
{
    GUI->LevelSelectWindowHandle = !GUI->LevelSelectWindowHandle;
    GUIUpdateWindowStack(GUI,GUI->LevelSelectWindowHandle);
}
void GUIBeginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    igNewFrame();   
}

void GUIEndFrame()
{
    igRender();
    ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
}
bool GUICheckBoxWithTooltip(char *Label,bool *Value,char *DescriptionFormat,...)
{
    va_list Arguments;
    int IsChecked;
    IsChecked = igCheckbox(Label,Value);
    if( DescriptionFormat != NULL && igIsItemHovered(ImGuiHoveredFlags_None) ) {
        igBeginTooltip();
        igPushTextWrapPos(igGetFontSize() * 40.0f);
        va_start(Arguments, DescriptionFormat);
        igTextV(DescriptionFormat,Arguments);
        va_end(Arguments);
        igPopTextWrapPos();
        igEndTooltip();
    }
    return IsChecked;
}

void GUISetProgressBarDialogTitle(GUI_t *GUI,const char *Title)
{
    if( GUI->ProgressBar->DialogTitle ) {
        free(GUI->ProgressBar->DialogTitle);
    }
    GUI->ProgressBar->DialogTitle = (Title != NULL) ? StringCopy(Title) : "Loading...";
    //NOTE(Adriano):Forces a refresh...since changing the title disrupts the rendering process.
    GUI->ProgressBar->IsOpen = 0;
}

void GUIProgressBarBegin(GUI_t *GUI,const char *Title)
{
    igSetCurrentContext(GUI->ProgressBar->Context);
    GUI->ProgressBar->IsOpen = 0;
    GUI->ProgressBar->CurrentPercentage = 0.f;
    GUISetProgressBarDialogTitle(GUI,Title);
}
void GUIProgressBarReset(GUI_t *GUI)
{
    if(!GUI) {
        return;
    }
    GUI->ProgressBar->CurrentPercentage = 0;
}
/*
    This function can be seen as a complete rendering loop.
    Each time we increment the progress bar, we check for any pending event that the GUI
    can handle and then we clear the display, show the current progress and swap buffers.
 */
void GUIProgressBarIncrement(GUI_t *GUI,VideoSystem_t *VideoSystem,float Increment,const char *Message)
{
    SDL_Event Event;
    ImGuiViewport *Viewport;
    ImVec2 ScreenCenter;
    ImVec2 Pivot;
    ImVec2 Size;
    
    if( !GUI ) {
        return;
    }
    
    SDL_PumpEvents();
    //NOTE(Adriano):
    //Process any window event that could be generated while showing the progress bar.
    //We need to make sure not to process any mouse/keyboad event otherwise when the progress bar ends the
    //queue may be empty and the GUI won't respond to events properly...
    while( SDL_PeepEvents(&Event, 1, SDL_GETEVENT, SDL_WINDOWEVENT, SDL_WINDOWEVENT) > 0 ) {
        ImGui_ImplSDL2_ProcessEvent(&Event);
    }
    //NOTE(Adriano):Since we are checking for events these function have now an updated view of the current window size.
    Viewport = igGetMainViewport();
    
    ImGuiViewport_GetCenter(&ScreenCenter,Viewport);

    Pivot.x = 0.5f;
    Pivot.y = 0.5f;

    glClear(GL_COLOR_BUFFER_BIT );
    

    GUIBeginFrame();
    
    if( !GUI->ProgressBar->IsOpen ) {
        igOpenPopup_Str(GUI->ProgressBar->DialogTitle,0);
        GUI->ProgressBar->IsOpen = 1;
    }
    
    Size.x = 0.f;
    Size.y = 0.f;
    DPrintf("ScreenCenter is now:%f;%f\n",ScreenCenter.x,ScreenCenter.y);
    igSetNextWindowPos(ScreenCenter, ImGuiCond_Always, Pivot);
    GUI->ProgressBar->CurrentPercentage += Increment;
    if (igBeginPopupModal(GUI->ProgressBar->DialogTitle, NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        igProgressBar((GUI->ProgressBar->CurrentPercentage / 100.f),Size,Message);
        igEnd();
    }
    GUIEndFrame();
    VideoSystemSwapBuffers(VideoSystem);
}

void GUIProgressBarEnd(GUI_t *GUI,VideoSystem_t *VideoSystem)
{
    int Width;
    int Height;
    
    igSetCurrentContext(GUI->DefaultContext);
    GUI->ProgressBar->IsOpen = 0;
    GUI->ProgressBar->CurrentPercentage = 0.f;
    //NOTE(Adriano):Make sure to update the current window size.
    VideoSystemGetCurrentWindowSize(VideoSystem,&Width,&Height);
    if( Width != VidConfigWidth->IValue ) {
        ConfigSetNumber("VideoWidth",Width);
    }
    if( Height != VidConfigHeight->IValue ) {
        ConfigSetNumber("VideoHeight",Height);
    }
}
int GUIGetVSyncOptionValue()
{
    int i;
    for (i = 0; i < NumVSyncOptions; i++) {
        if( VSyncOptions[i].Value == VidConfigVSync->IValue ) {
            return i;
        }
    }
    return 0;
}

void GUISetErrorMessage(GUI_t *GUI,const char *Message)
{
    if( !GUI ) {
        DPrintf("GUISetErrorMessage:Invalid GUI struct\n");
        return;
    }
    if( !Message ) {
        DPrintf("GUISetErrorMessage:Invalid Message.");
        return;
    }
    
    if( GUI->ErrorMessage ) {
        free(GUI->ErrorMessage);
    }
    
    GUI->ErrorMessage = StringCopy(Message);
//     GUIPushWindow(GUI);
}

void GUIFileDialogRender(GUI_t *GUI,GUIFileDialog_t *FileDialog)
{
    ImVec2 MaxSize;
    ImVec2 MinSize;
    ImGuiViewport *Viewport;
    ImVec2 WindowPosition;
    ImVec2 WindowPivot;
    char *DirectoryPath;
    char *FileName;
    void *UserData;
    
    if( !FileDialog ) {
        return;
    }
    if( !IGFD_IsOpened(FileDialog->Window) ) {
        return;
    }
    Viewport = igGetMainViewport();
    WindowPosition.x = Viewport->WorkPos.x;
    WindowPosition.y = Viewport->WorkPos.y;
    WindowPivot.x = 0.f;
    WindowPivot.y = 0.f;
    MaxSize.x = Viewport->WorkSize.x;
    MaxSize.y = Viewport->WorkSize.y;
    MinSize.x = -1;
    MinSize.y = -1;

    igSetNextWindowSize(MaxSize,0);
    igSetNextWindowPos(WindowPosition, ImGuiCond_Always, WindowPivot);
    if (IGFD_DisplayDialog(FileDialog->Window, FileDialog->Key, 
        ImGuiWindowFlags_NoCollapse  | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings, MinSize, MaxSize)) {
        if (IGFD_IsOk(FileDialog->Window)) {
                FileName = IGFD_GetFilePathName(FileDialog->Window);
                DirectoryPath = IGFD_GetCurrentPath(FileDialog->Window);
                UserData = IGFD_GetUserDatas(FileDialog->Window);
                if( FileDialog->OnElementSelected ) {
                    FileDialog->OnElementSelected(FileDialog,GUI,DirectoryPath,FileName,UserData);
                }
                if( DirectoryPath ) {
                    free(DirectoryPath);
                }
                if( FileName ) {
                    free(FileName);
                }
        } else {
            if( FileDialog->OnDialogCancelled ) {
                FileDialog->OnDialogCancelled(FileDialog,GUI);
            }
        }
    }

}
void GUIRenderFileDialogs(GUI_t *GUI)
{
    GUIFileDialog_t *Iterator;
    
    for( Iterator = GUI->FileDialogList; Iterator; Iterator = Iterator->Next ) {
        GUIFileDialogRender(GUI,Iterator);
    }
}
void GUIDrawVideoSettingsWindow(GUI_t *GUI,VideoSystem_t *VideoSystem)
{
    int OldValue;
    int IsSelected;
    int i;
    int CurrentVSyncOption;

    if( !GUI->VideoSettingsWindowHandle ) {
        return;
    }
    ImVec2 ZeroSize;
    ZeroSize.x = ZeroSize.y = 0.f;
    int PreviewIndex = VideoSystem->CurrentVideoMode != -1 ? VideoSystem->CurrentVideoMode : 0;
    if( igBegin("Video Settings",&GUI->VideoSettingsWindowHandle,ImGuiWindowFlags_AlwaysAutoResize) ) {
        CurrentVSyncOption = GUIGetVSyncOptionValue();
        if( igBeginCombo("VSync Options",VSyncOptions[CurrentVSyncOption].DisplayValue,0) ) {
            for (i = 0; i < NumVSyncOptions; i++) {
                IsSelected = (CurrentVSyncOption == i);
                if (igSelectable_Bool(VSyncOptions[i].DisplayValue, IsSelected,0,ZeroSize)) {
                    if( CurrentVSyncOption != i ) {
                        OldValue = VidConfigVSync->IValue;
                        if( VideoSystemSetSwapInterval(VSyncOptions[i].Value) < 0 ) {
                            VideoSystemSetSwapInterval(OldValue);
                        }
                    }
                }
          
            }
            igEndCombo();
        }
        igSeparator();
        //NOTE(Adriano):Only in Fullscreen mode we can select the video mode we want.
        if( VidConfigFullScreen->IValue ) {
            igText("Video Mode");
            if( igBeginCombo("##Resolution", VideoSystem->VideoModeList[PreviewIndex].Description, 0) ) {
                for( i = 0; i < VideoSystem->NumVideoModes; i++ ) {
                    int IsSelected = ((VideoSystem->VideoModeList[i].Width == VidConfigWidth->IValue) && 
                        (VideoSystem->VideoModeList[i].Height == VidConfigHeight->IValue)) ? 1 : 0;
                    if( igSelectable_Bool(VideoSystem->VideoModeList[i].Description,IsSelected,0,ZeroSize ) ) {
                        VideoSystemSetVideoSettings(VideoSystem,i);
                    }
                    if( IsSelected ) {
                        igSetItemDefaultFocus();
                    }
                }
                igEndCombo();
            }
            igSeparator();
        }
        if( igCheckbox("Fullscreen Mode",(bool *) &VidConfigFullScreen->IValue) ) {
            DPrintf("VidConfigFullScreen:%i\n",VidConfigFullScreen->IValue);
            VideoSystemSetVideoSettings(VideoSystem,-1);
        }
    }
    igEnd();
    if( !GUI->VideoSettingsWindowHandle ) {
        GUIUpdateWindowStack(GUI,GUI->VideoSettingsWindowHandle);
    }
}
void GUIDrawDebugWindow(GUI_t *GUI,Camera_t *Camera,VideoSystem_t *VideoSystem)
{
    SDL_version LinkedVersion;
    SDL_version CompiledVersion;
    
    if( !GUI->DebugWindowHandle ) {
        return;
    }

    if( igBegin("Debug Settings",&GUI->DebugWindowHandle,ImGuiWindowFlags_AlwaysAutoResize) ) {
        if( igCollapsingHeader_TreeNodeFlags("Debug Statistics",0) ) {
            igText("NumActiveWindows:%i",GUI->NumActiveWindows);
            igSeparator();
            igText("OpenGL Version: %s",glGetString(GL_VERSION));
            SDL_GetVersion(&LinkedVersion);
            SDL_VERSION(&CompiledVersion);
            igText("SDL Compiled Version: %u.%u.%u",CompiledVersion.major,CompiledVersion.minor,CompiledVersion.patch);
            igText("SDL Linked Version: %u.%u.%u",LinkedVersion.major,LinkedVersion.minor,LinkedVersion.patch);
            igSeparator();
            igText("Display Informations");
            igText("Resolution:%ix%i",VidConfigWidth->IValue,VidConfigHeight->IValue);
            igText("Refresh Rate:%i",VidConfigRefreshRate->IValue);
            igSeparator();
            igText("Camera Info");
            igText("Position:%f;%f;%f",Camera->Position[0],Camera->Position[1],Camera->Position[2]);
            igText("Rotation:%f;%f;%f",Camera->Rotation[PITCH],Camera->Rotation[YAW],Camera->Rotation[ROLL]);
        }
    }
    
    if( !GUI->DebugWindowHandle ) {
        GUIUpdateWindowStack(GUI,GUI->DebugWindowHandle);
    }
    igEnd();
}

void GUIDrawHelpOverlay()
{
//     int WindowFlags;
//     ImGuiViewport *Viewport;
//     ImVec2 WorkPosition;
//     ImVec2 WindowPosition;
//     ImVec2 WindowPivot;
//     
//     WindowFlags = /*ImGuiWindowFlags_NoDecoration |*/ ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | 
//                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | 
//                     ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
//     Viewport = igGetMainViewport();
//     WorkPosition = Viewport->WorkPos;
//     WindowPosition.x = (WorkPosition.x + 10.f);
//     WindowPosition.y = (WorkPosition.y + 10.f);
//     WindowPivot.x = 0.f;
//     WindowPivot.y = 0.f;
//     igSetNextWindowPos(WindowPosition, ImGuiCond_Once, WindowPivot);
// 
//     if( igBegin("Help", NULL, WindowFlags) ) {
//     }
//     igEnd();
}

void GUIDrawDebugOverlay(ComTimeInfo_t *TimeInfo)
{
    ImGuiViewport *Viewport;
    ImVec2 WorkPosition;
    ImVec2 WorkSize;
    ImVec2 WindowPosition;
    ImVec2 WindowPivot;
    int WindowFlags;
    
    WindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize | 
                    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | 
                    ImGuiWindowFlags_NoMove;
    Viewport = igGetMainViewport();
    WorkPosition = Viewport->WorkPos;
    WorkSize = Viewport->WorkSize;
    WindowPosition.x = (WorkPosition.x + WorkSize.x - 10.f);
    WindowPosition.y = (WorkPosition.y + 10.f);
    WindowPivot.x = 1.f;
    WindowPivot.y = 0.f;

    
    if( GUIShowFPS->IValue ) {
        igSetNextWindowPos(WindowPosition, ImGuiCond_Always, WindowPivot);
        if( igBegin("FPS", NULL, WindowFlags) ) {
            igText(TimeInfo->FPSString);
        }
        igEnd(); 
    }
}
void GUIDrawSceneWindow(RenderObjectManager_t *RenderObjectManager,Camera_t *Camera,ComTimeInfo_t *TimeInfo,const Byte *KeyState)
{
    ImGuiIO *IO;
    ImVec2 Size;
    ImVec4 TintColor;
    ImVec4 BorderColor;
    ImVec2 UV0;
    ImVec2 UV1;
    ImVec2 TextPosition;
    ImVec2 WindowPadding;
    ImDrawList *DrawList;
    BSDRenderObject_t *CurrentRenderObject;
    
    UV0.x = 0;
    UV0.y = 1;
    UV1.x = 1;
    UV1.y = 0;
    BorderColor.x = 1;
    BorderColor.y = 1;
    BorderColor.z = 1;
    BorderColor.w = 1;
    TintColor.x = 0;
    TintColor.y = 0;
    TintColor.z = 0;
    TintColor.w = 1;

    WindowPadding.x = 0;
    WindowPadding.y = 0;
    
    CurrentRenderObject = RenderObjectManagerGetSelectedRenderObject(RenderObjectManager);

    IO = igGetIO();
    
    //NOTE(Adriano):Disable window padding only for scene window.
    igPushStyleVar_Vec2(ImGuiStyleVar_WindowPadding, WindowPadding);
    
    if( igBegin("Scene Window", NULL, 0) ) {
        DrawList = igGetWindowDrawList();

        if( igIsWindowFocused(0) ) {
            CameraCheckKeyEvents(Camera,KeyState,TimeInfo->Delta);
        }

        igGetContentRegionAvail(&Size);
        igGetCursorScreenPos(&TextPosition);
        //NOTE(Adriano):Since we have disabled padding nudge position a bit in order to not overlap the text with
        //the window border.
        TextPosition.x += 2;
        igImageButton((void*)(long int) RenderObjectManager->FBOTexture, Size, UV0,UV1,0,TintColor,BorderColor);
        ImDrawList_AddText_Vec2(DrawList,TextPosition,0xFFFFFFFF,TimeInfo->FPSString,NULL);
        if( igIsItemActive() && igIsMouseDragging(ImGuiMouseButton_Left,0) && CurrentRenderObject != NULL) {
            //TODO(Adriano):Debugging code...this will be replaced by a call to RenderObjectManager to rotate the current
            //selected RenderObject.
            CurrentRenderObject->Rotation[1] += IO->MouseDelta.x;
            CurrentRenderObject->Rotation[0] -= IO->MouseDelta.y;
            if ( CurrentRenderObject->Rotation[0] > 89.0f ) {
                CurrentRenderObject->Rotation[0] = 89.0f;
            }
            if ( CurrentRenderObject->Rotation[0] < -89.0f ) {
                CurrentRenderObject->Rotation[0] = -89.0f;
            }

            if ( CurrentRenderObject->Rotation[1] > 180.0f ) {
                CurrentRenderObject->Rotation[1] -= 360.0f;
            }

            if ( CurrentRenderObject->Rotation[1] < -180.0f ) {
                CurrentRenderObject->Rotation[1] += 360.0f;
            }
        }
        igEnd();
    }
    igPopStyleVar(1);
}

void GUIDrawMainWindow(GUI_t *GUI,RenderObjectManager_t *RenderObjectManager,VideoSystem_t *VideoSystem)
{
    BSDRenderObjectPack_t *PackIterator;
    BSDRenderObject_t *RenderObjectIterator;
    BSDRenderObject_t *CurrentRenderObject;
    int TreeNodeFlags;
    int DisableNode;
    char SmallBuffer[32];
    char DeleteButtonId[32];
    ImVec2 ZeroSize;
    
    if( !igBegin("Main Window", NULL, ImGuiWindowFlags_AlwaysAutoResize) ) {
        return;
    }
    TreeNodeFlags = RenderObjectManager->BSDList != NULL ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_None;
    if( igCollapsingHeader_TreeNodeFlags("Animated RenderObjects List",TreeNodeFlags) ) {
        for(PackIterator = RenderObjectManager->BSDList; PackIterator; PackIterator = PackIterator->Next) {
            TreeNodeFlags = ImGuiTreeNodeFlags_None;
            if(  PackIterator == RenderObjectManagerGetSelectedBSDPack(RenderObjectManager) ) {
                TreeNodeFlags |= ImGuiTreeNodeFlags_DefaultOpen;
            }
            if( igTreeNodeEx_Str(PackIterator->Name,TreeNodeFlags) ) {
                igSameLine(0,-1);
                igText(PackIterator->GameVersion == MOH_GAME_STANDARD ? "MOH" : "MOH:Underground");
                igSameLine(0,-1);
                //NOTE(Adriano):We do not allow for duplicated BSD however since we support both MOH and MOH:Underground then
                //this could be confusing since the ID would for example be 2_1 for both versions when loading Mission 2 Level 1.
                //In order to solve this we just append the GameVersion in order to obtain the final Id which will be
                //in this example 2_1.BSD0 or 2_1.BSD1 thus solving any potential conflict.
                sprintf(DeleteButtonId,"Remove##%i",PackIterator->GameVersion);
                if( igSmallButton(DeleteButtonId) ) {
                    if( RenderObjectManagerDeleteBSDPack(RenderObjectManager,PackIterator->Name,PackIterator->GameVersion) ) {
                        igTreePop();
                        break;
                    }
                    GUISetErrorMessage(GUI,"Failed to remove BSD pack from list");
                }
                for( RenderObjectIterator = PackIterator->RenderObjectList; RenderObjectIterator; 
                    RenderObjectIterator = RenderObjectIterator->Next ) {
                    TreeNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet;
                    DisableNode = 0;
                    sprintf(SmallBuffer,"%u",RenderObjectIterator->Id);
                    if( RenderObjectIterator == RenderObjectManagerGetSelectedRenderObject(RenderObjectManager) ) {
                        TreeNodeFlags |= ImGuiTreeNodeFlags_Selected;
                        DisableNode = 1;
                    }
                    if( DisableNode ) {
                        igBeginDisabled(1);
                    }
                    if( igTreeNodeEx_Str(SmallBuffer,TreeNodeFlags) ) {
                        if (igIsMouseDoubleClicked(0) && igIsItemHovered(ImGuiHoveredFlags_None) ) {
                            RenderObjectManagerSetSelectedRenderObject(RenderObjectManager,PackIterator,RenderObjectIterator);
                        }
                    }
                    if( DisableNode ) {
                        igEndDisabled();
                    }
                }
                igTreePop();
            }
        }
    }
    if( igCollapsingHeader_TreeNodeFlags("Current RenderObject Informations",0) ) {
        CurrentRenderObject = RenderObjectManagerGetSelectedRenderObject(RenderObjectManager);
        if( !CurrentRenderObject ) {
            igText("No RenderObject selected.");
        } else {
            igText("ID:%u",CurrentRenderObject->Id);
            igText("References RenderObject Id:%u",CurrentRenderObject->ReferencedRenderObjectId);
            igText("Type:%i",CurrentRenderObject->Type);
            igText("Current Animation Index:%i",CurrentRenderObject->CurrentAnimationIndex);
            igText("NumAnimations:%i",CurrentRenderObject->NumAnimations);
            igText("Current Rotation:%f;%f;%f",CurrentRenderObject->Rotation[0],CurrentRenderObject->Rotation[1],CurrentRenderObject->Rotation[2]);
            igSeparator();
            igText("Export current pose");
            ZeroSize.x = 0.f;
            ZeroSize.y = 0.f;
            if( igButton("Export to Ply",ZeroSize) ) {
                RenderObjectManagerExportCurrentPose(RenderObjectManager,GUI,VideoSystem,RENDER_OBJECT_MANAGER_EXPORT_FORMAT_PLY);
            }
        }
    }
    igEnd();
}
void GUIDrawMenuBar(Engine_t *Engine)
{
    if( !igBeginMainMenuBar() ) {
        return;
    }
    if (igBeginMenu("File",true)) {
        if( igMenuItem_Bool("Open",NULL,false,true) ) {
            RenderObjectManagerOpenFileDialog(Engine->RenderObjectManager,Engine->VideoSystem);
        }
        if( igMenuItem_Bool("Exit",NULL,false,true) ) {
            Quit(Engine);
        }
//         igSeparator();
        igEndMenu();
    }
    igEndMainMenuBar();
}
void GUIDrawErrorMessage(GUI_t *GUI)
{
    ImVec2 ButtonSize;
    if( !GUI->ErrorMessage ) {
        return;
    }
    if( !GUI->ErrorDialogHandle ) {
        igOpenPopup_Str("Error",0);
        GUI->ErrorDialogHandle = 1;
    }
    ButtonSize.x = 120;
    ButtonSize.y = 0;
    GUIPrepareModalWindow();
    if( igBeginPopupModal("Error",NULL,ImGuiWindowFlags_AlwaysAutoResize) ) {
        igText(GUI->ErrorMessage);
        if (igButton("OK", ButtonSize) ) {
            GUIPopWindow(GUI);
            igCloseCurrentPopup();
            GUI->ErrorDialogHandle = 0;
            free(GUI->ErrorMessage);
            GUI->ErrorMessage = NULL;
        }
        igEndPopup();
    }
}
void GUIDraw(Engine_t *Engine)
{
    
    GUIBeginFrame();
    
    GUIDrawMenuBar(Engine);
    
//     GUIDrawDebugOverlay(TimeInfo);
    
//     if( !GUI->NumActiveWindows ) {
//         GUIDrawHelpOverlay();
//         GUIEndFrame();
//         return;
//     }
    
    GUIRenderFileDialogs(Engine->GUI);
    GUIDrawErrorMessage(Engine->GUI);
    GUIDrawSceneWindow(Engine->RenderObjectManager,Engine->Camera,Engine->TimeInfo,Engine->KeyState);
    GUIDrawMainWindow(Engine->GUI,Engine->RenderObjectManager,Engine->VideoSystem);
    GUIDrawDebugWindow(Engine->GUI,Engine->Camera,Engine->VideoSystem);
    GUIDrawVideoSettingsWindow(Engine->GUI,Engine->VideoSystem);
//     igShowDemoWindow(NULL);
    GUIEndFrame();
}

int GUIFileDialogIsOpen(GUIFileDialog_t *FileDialog)
{
    if( !FileDialog ) {
        DPrintf("GUIFileDialogIsOpen:Invalid dialog data\n");
        return 0;
    }
    
    return IGFD_IsOpened(FileDialog->Window);
}

void *GUIFileDialogGetUserData(GUIFileDialog_t *FileDialog)
{
    if( !FileDialog ) {
        DPrintf("GUIFileDialogIsOpen:Invalid dialog data\n");
        return 0;
    } 
    return IGFD_GetUserDatas(FileDialog->Window);
}
void GUIFileDialogOpenWithUserData(GUIFileDialog_t *FileDialog,void *UserData)
{
    if( !FileDialog ) {
        DPrintf("GUIFileDialogOpen:Invalid dialog data\n");
        return;
    }
    
    if( IGFD_IsOpened(FileDialog->Window) ) {
        return;
    }
    IGFD_OpenDialog2(FileDialog->Window,FileDialog->Key,FileDialog->WindowTitle,FileDialog->Filters,".",1,
                     UserData,ImGuiFileDialogFlags_DontShowHiddenFiles);
//     GUIPushWindow(GUI);
}
void GUIFileDialogOpen(GUIFileDialog_t *FileDialog)
{
    GUIFileDialogOpenWithUserData(FileDialog,NULL);
}

void GUIFileDialogClose(GUI_t *GUI,GUIFileDialog_t *FileDialog)
{
    if( !FileDialog ) {
        DPrintf("GUIFileDialogClose:Invalid dialog data\n");
        return;
    }
    
    if( !IGFD_IsOpened(FileDialog->Window) ) {
        return;
    }
    IGFD_CloseDialog(FileDialog->Window);
    GUIPopWindow(GUI);
}
/*
 Register a new file dialog.
 Filters can be NULL if we want a dir selection dialog or have a value based on ImGuiFileDialog documentation if we want to
 select a certain type of file.
 OnElementSelected and OnDialogCancelled are two callback that can be set to NULL if we are not interested in the result.
 NOTE that setting them to NULL or the cancel callback to NULL doesn't close the dialog.
 */
GUIFileDialog_t *GUIFileDialogRegister(GUI_t *GUI,const char *WindowTitle,const char *Filters,FileDialogSelectCallback_t OnElementSelected,
                                       FileDialogCancelCallback_t OnDialogCancelled)
{
    GUIFileDialog_t *FileDialog;
    
    if( !WindowTitle) {
        DPrintf("GUIFileDialogRegister:Invalid Window Title\n");
        return NULL;
    }

    FileDialog = malloc(sizeof(GUIFileDialog_t));
    
    if( !FileDialog ) {
        DPrintf("GUIFileDialogRegister:Couldn't allocate struct data.\n");
        return NULL;
    }
    asprintf(&FileDialog->Key,"FileDialog%i",GUI->NumRegisteredFileDialog);
    FileDialog->WindowTitle = StringCopy(WindowTitle);
    if( Filters ) {
        FileDialog->Filters = StringCopy(Filters);
    } else {
        FileDialog->Filters = NULL;
    }
    FileDialog->Window = IGFD_Create();
    FileDialog->OnElementSelected = OnElementSelected;
    FileDialog->OnDialogCancelled = OnDialogCancelled;
    FileDialog->Next = GUI->FileDialogList;
    GUI->FileDialogList = FileDialog;
    GUI->NumRegisteredFileDialog++;
    
    return FileDialog;
}
void GUIFileDialogSetTitle(GUIFileDialog_t *FileDialog,const char *Title)
{
    if(!FileDialog) {
        DPrintf("GUIFileDialogSetTitle:Invalid dialog\n");
        return;
    }
    if( !Title ) {
        DPrintf("GUIFileDialogSetTitle:Invalid title\n");
        return;
    }
    if( FileDialog->WindowTitle ) {
        free(FileDialog->WindowTitle);
    }
    FileDialog->WindowTitle = StringCopy(Title);
}

void GUIFileDialogSetOnElementSelectedCallback(GUIFileDialog_t *FileDialog,FileDialogSelectCallback_t OnElementSelected)
{
    if(!FileDialog) {
        DPrintf("GUIFileDialogSetOnElementSelectedCallback:Invalid dialog\n");
        return;
    }
    FileDialog->OnElementSelected = OnElementSelected;
}
void GUIFileDialogSetOnDialogCancelledCallback(GUIFileDialog_t *FileDialog,FileDialogCancelCallback_t OnDialogCancelled)
{
    if(!FileDialog) {
        DPrintf("GUIFileDialogSetOnDialogCancelledCallback:Invalid dialog\n");
        return;
    }
    FileDialog->OnDialogCancelled = OnDialogCancelled;
}

void GUIContextInit(ImGuiContext *Context,VideoSystem_t *VideoSystem,const char *ConfigFilePath)
{
    ImGuiIO *IO;
    ImGuiStyle *Style;
    ImFont *Font;
    ImFontConfig *FontConfig;
    
    IO = igGetIO();
    igSetCurrentContext(Context);
    ImGui_ImplSDL2_InitForOpenGL(VideoSystem->Window, &VideoSystem->GLContext);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    igStyleColorsDark(NULL);
    if( GUIFont->Value[0] ) {
        Font = ImFontAtlas_AddFontFromFileTTF(IO->Fonts,GUIFont->Value,floor(GUIFontSize->FValue * VideoSystem->DPIScale),NULL,NULL);
        if( !Font ) {
            DPrintf("GUIContextInit:Invalid font file...using default\n");
            ConfigSet("GUIFont","");
        }
    } else {
        FontConfig = ImFontConfig_ImFontConfig();
        FontConfig->OversampleH = 1;
        FontConfig->OversampleV = 1;
        FontConfig->PixelSnapH = true;
        FontConfig->SizePixels = floor(GUIFontSize->FValue * VideoSystem->DPIScale);
        ImFontAtlas_AddFontDefault(IO->Fonts,FontConfig);
        ImFontConfig_destroy(FontConfig);
    }
    Style = igGetStyle();
    Style->WindowTitleAlign.x = 0.5f;
    ImGuiStyle_ScaleAllSizes(Style,VideoSystem->DPIScale);
    IO->ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    IO->IniFilename = ConfigFilePath;
}
GUI_t *GUIInit(VideoSystem_t *VideoSystem)
{
    GUI_t *GUI;
    char *ConfigPath;

    GUI = malloc(sizeof(GUI_t));
    
    if( !GUI ) {
        DPrintf("GUIInit:Failed to allocate memory for struct\n");
        return NULL;
    }
    
    memset(GUI,0,sizeof(GUI_t));
    GUI->ProgressBar = malloc(sizeof(GUIProgressBar_t));
    GUI->ErrorMessage = NULL;
    GUI->ErrorDialogHandle = 0;
    
    ConfigPath = SysGetConfigPath();
    asprintf(&GUI->ConfigFilePath,"%simgui.ini",ConfigPath);
    free(ConfigPath);
    
    if( !GUI->ProgressBar ) {
        DPrintf("GUIInit:Failed to allocate memory for ProgressBar struct\n");
        free(GUI);
        return NULL;
    }
    GUI->NumRegisteredFileDialog = 0;
    
    GUI->FileDialogList = NULL;

    GUIFont = ConfigGet("GUIFont");
    GUIFontSize = ConfigGet("GUIFontSize");
    GUIShowFPS = ConfigGet("GUIShowFPS");
    
    GUI->DefaultContext = igCreateContext(NULL);
    GUI->ProgressBar->Context = igCreateContext(NULL);
    GUI->ProgressBar->DialogTitle = NULL;
    GUIContextInit(GUI->ProgressBar->Context,VideoSystem,GUI->ConfigFilePath);
    GUIContextInit(GUI->DefaultContext,VideoSystem,GUI->ConfigFilePath);
        
    GUI->NumActiveWindows = 0;

    return GUI;
}
