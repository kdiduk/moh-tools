// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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

#include "Config.h"
#include "MOHLevelViewer.h"

Config_t *ConfigList;

void ConfigFree()
{
    Config_t *Temp;
    
    while( ConfigList ) {
        free(ConfigList->Name);
        free(ConfigList->Value);
        if( ConfigList->Description ) {
            free(ConfigList->Description);
        }
        Temp = ConfigList;
        ConfigList = ConfigList->Next;
        free(Temp);
    }
}
void ConfigTokenizeSettings(char *String)
{
    char *Temp;
    char  ConfigBuffer[1024];
    char *ConfigLine[2];
    int   NumArgs;
    int i;
    
    Temp = String;
    NumArgs = 0;
    i = 0;
    ConfigLine[0] = ConfigLine[1] = NULL;

    while( 1 ) {
        
        if( NumArgs >= 2 ) {
            DPrintf("ConfigTokenizeSettings:Found a key with multiple values...discarding them\n");
            break;
        }
        //Skip any whitespace...
        while( *Temp && *Temp <= ' ' ) {
            Temp++;
        }

        if( !*Temp ) {
            ConfigBuffer[i++] = '\0';
            ConfigLine[NumArgs++] = StringCopy(ConfigBuffer);
            i = 0;
            break;
        }
        
        if( *Temp == '"' ) {
            Temp++;
            while ( *Temp && *Temp != '"' ) {
                ConfigBuffer[i++] = *Temp++;
            }
            if( *Temp != '"' ) { 
                DPrintf("ConfigTokenizeSettings:Malformed quote found expected \" found %c.\n",*Temp);
                break;
            }
            Temp++;
            continue;
        }
        //Found a string copy it...
        if( *Temp > ' ' ) {
            do {
                ConfigBuffer[i++] = *Temp++;
            } while( *Temp && *Temp != ' ');
            ConfigBuffer[i++] = 0;
            ConfigLine[NumArgs++] = StringCopy(ConfigBuffer);
            i = 0;
            continue;
        }
    }
    
    if( NumArgs <= 1 ) {
        DPrintf("ConfigTokenizeSettings:Found key %s but not value.\n",ConfigLine[0]);
    } else {
        DPrintf("Setting config %s to %s\n",ConfigLine[0],ConfigLine[1]);
        ConfigSet(ConfigLine[0],ConfigLine[1]);
    }
    if( ConfigLine[0] ) {
        free(ConfigLine[0]);
    }
    if( ConfigLine[1] ) {
        free(ConfigLine[1]);
    }
}

void ConfigLoadSettings()
{
    char *PrefPath;
    char *PrefFile;
    char  ConfigLine[1024];
    int   ConfigLineIndex;
    char *ConfigBuffer;
    char *Temp;
    
    PrefPath = SysGetConfigPath();
    asprintf(&PrefFile,"%sConfig.cfg",PrefPath);
    ConfigBuffer = ReadTextFile(PrefFile,0);
    
    //Settings didn't exists save the default ones.
    if( !ConfigBuffer ) {
        free(PrefFile);
        ConfigSaveSettings();
        return;
    }

    ConfigLineIndex = 0;
    Temp = ConfigBuffer;
    
    DPrintf("ConfigLoadSettings:Loading %s\n",PrefFile);

    while( 1 ) {
        if( !*Temp ) {
            break;
        }
        //Skip comments
        if( Temp[0] == '/' ) {
            if( Temp[1] == '/' ) {
                //This line is an inline comment and can be skipped!
                do {
                    Temp++;
                } while( *Temp && *Temp != '\n' );
                Temp++;
                continue;
            } else if ( Temp[1] == '*' ) {
                //This line contains a multi-line comment and can be skipped.
                Temp++;
                do {
                    Temp++;
                } while( *Temp && !(Temp[0] == '*' && Temp[1] == '/') );
//                 assert(*Temp);
                if( !*Temp ) {
                    break;
                }
                Temp += 2;
            }
        }
        
        if( *Temp > ' ' ) {
            do {
                ConfigLine[ConfigLineIndex++] = *Temp++;
            } while( *Temp && *Temp != '\n');
            ConfigLine[ConfigLineIndex++] = '\0';
            ConfigLineIndex = 0;
            ConfigTokenizeSettings(ConfigLine);
        }
        Temp++;
    }
    free(ConfigBuffer);
    free(PrefFile);
    free(PrefPath);
    return;
}

void ConfigSaveSettings()
{
    FILE *ConfigFile;
    char *PrefPath;
    char *PrefFile;
    Config_t *Config;
    PrefPath = SysGetConfigPath();
    asprintf(&PrefFile,"%sConfig.cfg",PrefPath);
    
    ConfigFile = fopen(PrefFile,"w+");
    
    fprintf(ConfigFile,"/*\n\t\t\t%s\n*/\n",CONFIG_FILE_HEADER);
    for(Config = ConfigList; Config; Config = Config->Next ){
        if( Config->Description ) {
            fprintf(ConfigFile,"/*\n%s:\n%s\n*/\n",Config->Name,Config->Description);
        }
        fprintf(ConfigFile,"%s \"%s\"\n",Config->Name,Config->Value);
    }
    
    free(PrefFile);
    fclose(ConfigFile);
    free(PrefPath);
}
Config_t *ConfigGet(char *Name)
{
    Config_t *Config;
    for(Config = ConfigList; Config; Config = Config->Next ){
        if( !strcmp(Config->Name,Name) ) {
            return Config;
        }
    }
    return NULL;
}

void ConfigUpdateValue(Config_t *Config,char *Value)
{
    if( !Config ) {
        DPrintf("ConfigUpdateValue:Invalid config.\n");
        return;
    }
    if( Config->Value ) {
        free(Config->Value);
    }
    Config->Value = StringCopy(Value);
    Config->IValue = StringToInt(Value);
    Config->FValue = atof(Value);
}
/*
 Sets the Value of a config by Name.
 If the config was found, it's value is updated and persisted inside
 the default config file.
 Returns 1 if operation succeeded 0 otherwise.
 */
int ConfigSet(char *Name,char *Value)
{
    Config_t *Config;
    if( !Name ) {
        DPrintf("ConfigSet:Invalid name\n");
        return 0;
    }
    if( !Value ) {
        DPrintf("ConfigSet:Invalid value\n");
        return 0;
    }
    for(Config = ConfigList; Config; Config = Config->Next ){
        if( !strcmp(Config->Name,Name) ) {
            ConfigUpdateValue(Config,Value);
            ConfigSaveSettings();
            return 1;
        }
    }
    DPrintf("ConfigSet:No config named \"%s\" was found in the list.\n",Name);
    return 0;
}

int ConfigSetNumber(char *Name,float Value)
{
    char SmallBuf[64];
    int Truncated;
    
    Truncated = (int) Value;
    //If truncating the variable results in the same variable
    //then it is probably just an int.
    if( Value == Truncated ) {
        sprintf(SmallBuf,"%i",Truncated);
    } else {
        sprintf(SmallBuf,"%f",Value);
    }
    return ConfigSet(Name,SmallBuf);
}

/*
 Register a new configuration key using the given Name,Value and Description.
 If the config has already been registered then 0 is returned,
 otherwise a new config entry is created and added to the list and the return value will be 1.
 If the config cannot be created due to memory allocation errors then -1 is returned.
 */
int ConfigRegister(char *Name,char *Value,char *Description)
{
    Config_t *Config;
    
    if( ConfigGet(Name) != NULL ) {
        return 0;
    }
    
    Config = malloc(sizeof(Config_t));
    
    if( !Config ) {
        return -1;
    }
    
    Config->Name = StringCopy(Name);
    Config->Value = NULL;
    ConfigUpdateValue(Config,Value);
    if( Description ) {
        Config->Description = StringCopy(Description);
    } else {
        Config->Description = NULL;
    }
    Config->Next = ConfigList;
    ConfigList = Config;
    
    return 1;
}

void ConfigRegisterDefaultSettings()
{
    ConfigRegister("VideoWidth","800",NULL);
    ConfigRegister("VideoHeight","600",NULL);
    ConfigRegister("VideoRefreshRate","60",NULL);
    ConfigRegister("VideoFullScreen","0",NULL);
    
    ConfigRegister("CameraSpeed","80.f",NULL);
    ConfigRegister("CameraMouseSensitivity","1.f",NULL);

    ConfigRegister("GameBasePath","","Sets the path from which the game will be loaded,any invalid path will result in this variable to "
                                    "being set to empty.");
    ConfigRegister("GUIFont","Fonts/DroidSans.ttf","Sets the file to be used as the GUI font,if not valid the application will use the default one");
    ConfigRegister("GUIFontSize","14.f",NULL);
    ConfigRegister("GUIShowFPS","1",NULL);

    ConfigRegister("LevelEnableWireFrameMode","0","Draw the level surfaces as lines");
    ConfigRegister("LevelDrawCollisionData","0","Draw the level collision data");
    ConfigRegister("LevelDrawBSPTree","0","When enabled draws the BSP tree for the current level.Red box represents a splitter while \n"
                                        "a yellow one a leaf (containing actual level data).");
    ConfigRegister("LevelDrawSurfaces","1","Draw the level surfaces");
    ConfigRegister("LevelDrawBSDNodesAsPoints","1","When enabled draws all the BSD nodes as points.");
    ConfigRegister("LevelDrawBSDRenderObjectsAsPoints","1","When enabled draws all the BSD RenderObjects as Points");
    ConfigRegister("LevelDrawBSDRenderObjects","1","When enabled draws all the supported render objects");
    ConfigRegister("LevelDrawBSDShowcase","0","When enabled draws all the loaded RenderObjects near the player spawn.");
    ConfigRegister("LevelEnableFrustumCulling","1","When enabled helps to skip non visibile nodes from the BSP tree improving the rendering speed");
    ConfigRegister("LevelEnableAmbientLight","1","When enabled the texture color is interpolated with the surface color to simulate lights on \n"
                                                    "surfaces");
    ConfigRegister("LevelEnableSemiTransparency","1","When enabled draw transparent surfaces as non-opaque");
    ConfigRegister("LevelEnableAnimatedLights","1","When enabled some surfaces will interpolate their color to simulate an animated surface.\n"
                                       "NOTE that this will only work if \"LevelEnableAmbientLight\" is enabled");
    ConfigRegister("LevelEnableAnimatedSurfaces","1","When enabled this will make some surfaces change their texture when the camera gets near.\n"
                                       "For example when hovering the camera near an explosive charge an overlay will pulse near the charge.");
    
    ConfigRegister("LevelEnableMusicTrack","1","When enabled sound will be played in background.\n"
                                       "There are 3 possible values:0 Disabled,1 Enable Music And Ambient sounds,2 Enable ambient sounds only.");
    ConfigRegister("SoundVolume","128","Sets the sound volume, the value must be in range 0-128, values outside that range will be clamped.");
}
void ConfigDumpSettings()
{
    Config_t *Config;
    
    for(Config = ConfigList; Config; Config = Config->Next ){
        DPrintf("Config:%s Value:%s %i %f\n",Config->Name,Config->Value,Config->IValue,Config->FValue);
    }
    
}
void ConfigInit()
{
    ConfigRegisterDefaultSettings();
    ConfigLoadSettings();
}
