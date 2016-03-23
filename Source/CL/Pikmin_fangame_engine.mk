##
## Auto Generated makefile by CodeLite IDE
## any manual changes will be erased      
##
## Debug
ProjectName            :=Pikmin_fangame_engine
ConfigurationName      :=Debug
WorkspacePath          := "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/CL"
ProjectPath            := "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/CL"
IntermediateDirectory  :=./Debug
OutDir                 := $(IntermediateDirectory)
CurrentFileName        :=
CurrentFilePath        :=
CurrentFileFullPath    :=
User                   :=André Silva
Date                   :=23/03/16
CodeLitePath           :="/home/andre/.codelite"
LinkerName             :=/usr/bin/g++
SharedObjectLinkerName :=/usr/bin/g++ -shared -fPIC
ObjectSuffix           :=.o
DependSuffix           :=.o.d
PreprocessSuffix       :=.i
DebugSwitch            :=-g 
IncludeSwitch          :=-I
LibrarySwitch          :=-l
OutputSwitch           :=-o 
LibraryPathSwitch      :=-L
PreprocessorSwitch     :=-D
SourceSwitch           :=-c 
OutputFile             :=$(IntermediateDirectory)/$(ProjectName)
Preprocessors          :=
ObjectSwitch           :=-o 
ArchiveOutputSwitch    := 
PreprocessOnlySwitch   :=-E
ObjectsFileList        :="Pikmin_fangame_engine.txt"
PCHCompileFlags        :=
MakeDirCommand         :=mkdir -p
LinkOptions            :=  -lm `pkg-config --libs allegro-5.0 allegro_audio-5.0 allegro_image-5.0 allegro_font-5.0 allegro_acodec-5.0 allegro_primitives-5.0 allegro_dialog-5.0`
IncludePath            :=  $(IncludeSwitch). $(IncludeSwitch). 
IncludePCH             := 
RcIncludePath          := 
Libs                   := 
ArLibs                 :=  
LibPath                := $(LibraryPathSwitch). 

##
## Common variables
## AR, CXX, CC, AS, CXXFLAGS and CFLAGS can be overriden using an environment variables
##
AR       := /usr/bin/ar rcu
CXX      := /usr/bin/g++
CC       := /usr/bin/gcc
CXXFLAGS :=  -g -O0 -std=c++0x -D_GLIBCXX_USE_CXX11_ABI=0
 $(Preprocessors)
CFLAGS   :=  -g -O0 -Wall $(Preprocessors)
ASFLAGS  := 
AS       := /usr/bin/as


##
## User defined environment variables
##
CodeLiteDir:=/usr/share/codelite
Objects0=$(IntermediateDirectory)/source_status.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_weather.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_mob_script.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_game_state.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_menu_widgets.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_misc_structs.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_particle.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_functions.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_data_file.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_hitbox.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/source_interval.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_drawing.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_main.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_init.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_area_editor.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_menus.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_controls.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_logic.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_animation.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_animation_editor.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/source_spray_type.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_vars.cpp$(ObjectSuffix) $(IntermediateDirectory)/source_sector.cpp$(ObjectSuffix) $(IntermediateDirectory)/LAFI_frame.cpp$(ObjectSuffix) $(IntermediateDirectory)/LAFI_gui.cpp$(ObjectSuffix) $(IntermediateDirectory)/LAFI_label.cpp$(ObjectSuffix) $(IntermediateDirectory)/LAFI_angle_picker.cpp$(ObjectSuffix) $(IntermediateDirectory)/LAFI_widget.cpp$(ObjectSuffix) $(IntermediateDirectory)/LAFI_scrollbar.cpp$(ObjectSuffix) $(IntermediateDirectory)/LAFI_image.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/LAFI_checkbox.cpp$(ObjectSuffix) $(IntermediateDirectory)/LAFI_radio_button.cpp$(ObjectSuffix) $(IntermediateDirectory)/LAFI_minor.cpp$(ObjectSuffix) $(IntermediateDirectory)/LAFI_button.cpp$(ObjectSuffix) $(IntermediateDirectory)/LAFI_style.cpp$(ObjectSuffix) $(IntermediateDirectory)/LAFI_textbox.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_ship_type.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_leader.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_onion_fsm.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_ship.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/mobs_treasure_type.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_onion.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_gate_fsm.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_pikmin.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_leader_fsm.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_info_spot.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_pikmin_fsm.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_onion_type.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_mob.cpp$(ObjectSuffix) 

Objects1=$(IntermediateDirectory)/mobs_mob_type.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/mobs_nectar.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_leader_type.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_pikmin_type.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_mob_fsm.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_gate_type.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_enemy_type.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_treasure_fsm.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_gate.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_ship_fsm.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_pellet.cpp$(ObjectSuffix) \
	$(IntermediateDirectory)/mobs_enemy.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_pellet_fsm.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_bridge.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_treasure.cpp$(ObjectSuffix) $(IntermediateDirectory)/mobs_pellet_type.cpp$(ObjectSuffix) 



Objects=$(Objects0) $(Objects1) 

##
## Main Build Targets 
##
.PHONY: all clean PreBuild PrePreBuild PostBuild MakeIntermediateDirs
all: $(OutputFile)

$(OutputFile): $(IntermediateDirectory)/.d $(Objects) 
	@$(MakeDirCommand) $(@D)
	@echo "" > $(IntermediateDirectory)/.d
	@echo $(Objects0)  > $(ObjectsFileList)
	@echo $(Objects1) >> $(ObjectsFileList)
	$(LinkerName) $(OutputSwitch)$(OutputFile) @$(ObjectsFileList) $(LibPath) $(Libs) $(LinkOptions)

MakeIntermediateDirs:
	@test -d ./Debug || $(MakeDirCommand) ./Debug


$(IntermediateDirectory)/.d:
	@test -d ./Debug || $(MakeDirCommand) ./Debug

PreBuild:


##
## Objects
##
$(IntermediateDirectory)/source_status.cpp$(ObjectSuffix): ../source/status.cpp $(IntermediateDirectory)/source_status.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/status.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_status.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_status.cpp$(DependSuffix): ../source/status.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_status.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_status.cpp$(DependSuffix) -MM "../source/status.cpp"

$(IntermediateDirectory)/source_status.cpp$(PreprocessSuffix): ../source/status.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_status.cpp$(PreprocessSuffix) "../source/status.cpp"

$(IntermediateDirectory)/source_weather.cpp$(ObjectSuffix): ../source/weather.cpp $(IntermediateDirectory)/source_weather.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/weather.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_weather.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_weather.cpp$(DependSuffix): ../source/weather.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_weather.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_weather.cpp$(DependSuffix) -MM "../source/weather.cpp"

$(IntermediateDirectory)/source_weather.cpp$(PreprocessSuffix): ../source/weather.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_weather.cpp$(PreprocessSuffix) "../source/weather.cpp"

$(IntermediateDirectory)/source_mob_script.cpp$(ObjectSuffix): ../source/mob_script.cpp $(IntermediateDirectory)/source_mob_script.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mob_script.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_mob_script.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_mob_script.cpp$(DependSuffix): ../source/mob_script.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_mob_script.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_mob_script.cpp$(DependSuffix) -MM "../source/mob_script.cpp"

$(IntermediateDirectory)/source_mob_script.cpp$(PreprocessSuffix): ../source/mob_script.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_mob_script.cpp$(PreprocessSuffix) "../source/mob_script.cpp"

$(IntermediateDirectory)/source_game_state.cpp$(ObjectSuffix): ../source/game_state.cpp $(IntermediateDirectory)/source_game_state.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/game_state.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_game_state.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_game_state.cpp$(DependSuffix): ../source/game_state.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_game_state.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_game_state.cpp$(DependSuffix) -MM "../source/game_state.cpp"

$(IntermediateDirectory)/source_game_state.cpp$(PreprocessSuffix): ../source/game_state.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_game_state.cpp$(PreprocessSuffix) "../source/game_state.cpp"

$(IntermediateDirectory)/source_menu_widgets.cpp$(ObjectSuffix): ../source/menu_widgets.cpp $(IntermediateDirectory)/source_menu_widgets.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/menu_widgets.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_menu_widgets.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_menu_widgets.cpp$(DependSuffix): ../source/menu_widgets.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_menu_widgets.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_menu_widgets.cpp$(DependSuffix) -MM "../source/menu_widgets.cpp"

$(IntermediateDirectory)/source_menu_widgets.cpp$(PreprocessSuffix): ../source/menu_widgets.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_menu_widgets.cpp$(PreprocessSuffix) "../source/menu_widgets.cpp"

$(IntermediateDirectory)/source_misc_structs.cpp$(ObjectSuffix): ../source/misc_structs.cpp $(IntermediateDirectory)/source_misc_structs.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/misc_structs.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_misc_structs.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_misc_structs.cpp$(DependSuffix): ../source/misc_structs.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_misc_structs.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_misc_structs.cpp$(DependSuffix) -MM "../source/misc_structs.cpp"

$(IntermediateDirectory)/source_misc_structs.cpp$(PreprocessSuffix): ../source/misc_structs.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_misc_structs.cpp$(PreprocessSuffix) "../source/misc_structs.cpp"

$(IntermediateDirectory)/source_particle.cpp$(ObjectSuffix): ../source/particle.cpp $(IntermediateDirectory)/source_particle.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/particle.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_particle.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_particle.cpp$(DependSuffix): ../source/particle.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_particle.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_particle.cpp$(DependSuffix) -MM "../source/particle.cpp"

$(IntermediateDirectory)/source_particle.cpp$(PreprocessSuffix): ../source/particle.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_particle.cpp$(PreprocessSuffix) "../source/particle.cpp"

$(IntermediateDirectory)/source_functions.cpp$(ObjectSuffix): ../source/functions.cpp $(IntermediateDirectory)/source_functions.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/functions.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_functions.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_functions.cpp$(DependSuffix): ../source/functions.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_functions.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_functions.cpp$(DependSuffix) -MM "../source/functions.cpp"

$(IntermediateDirectory)/source_functions.cpp$(PreprocessSuffix): ../source/functions.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_functions.cpp$(PreprocessSuffix) "../source/functions.cpp"

$(IntermediateDirectory)/source_data_file.cpp$(ObjectSuffix): ../source/data_file.cpp $(IntermediateDirectory)/source_data_file.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/data_file.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_data_file.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_data_file.cpp$(DependSuffix): ../source/data_file.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_data_file.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_data_file.cpp$(DependSuffix) -MM "../source/data_file.cpp"

$(IntermediateDirectory)/source_data_file.cpp$(PreprocessSuffix): ../source/data_file.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_data_file.cpp$(PreprocessSuffix) "../source/data_file.cpp"

$(IntermediateDirectory)/source_hitbox.cpp$(ObjectSuffix): ../source/hitbox.cpp $(IntermediateDirectory)/source_hitbox.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/hitbox.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_hitbox.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_hitbox.cpp$(DependSuffix): ../source/hitbox.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_hitbox.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_hitbox.cpp$(DependSuffix) -MM "../source/hitbox.cpp"

$(IntermediateDirectory)/source_hitbox.cpp$(PreprocessSuffix): ../source/hitbox.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_hitbox.cpp$(PreprocessSuffix) "../source/hitbox.cpp"

$(IntermediateDirectory)/source_interval.cpp$(ObjectSuffix): ../source/interval.cpp $(IntermediateDirectory)/source_interval.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/interval.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_interval.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_interval.cpp$(DependSuffix): ../source/interval.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_interval.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_interval.cpp$(DependSuffix) -MM "../source/interval.cpp"

$(IntermediateDirectory)/source_interval.cpp$(PreprocessSuffix): ../source/interval.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_interval.cpp$(PreprocessSuffix) "../source/interval.cpp"

$(IntermediateDirectory)/source_drawing.cpp$(ObjectSuffix): ../source/drawing.cpp $(IntermediateDirectory)/source_drawing.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/drawing.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_drawing.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_drawing.cpp$(DependSuffix): ../source/drawing.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_drawing.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_drawing.cpp$(DependSuffix) -MM "../source/drawing.cpp"

$(IntermediateDirectory)/source_drawing.cpp$(PreprocessSuffix): ../source/drawing.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_drawing.cpp$(PreprocessSuffix) "../source/drawing.cpp"

$(IntermediateDirectory)/source_main.cpp$(ObjectSuffix): ../source/main.cpp $(IntermediateDirectory)/source_main.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/main.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_main.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_main.cpp$(DependSuffix): ../source/main.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_main.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_main.cpp$(DependSuffix) -MM "../source/main.cpp"

$(IntermediateDirectory)/source_main.cpp$(PreprocessSuffix): ../source/main.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_main.cpp$(PreprocessSuffix) "../source/main.cpp"

$(IntermediateDirectory)/source_init.cpp$(ObjectSuffix): ../source/init.cpp $(IntermediateDirectory)/source_init.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/init.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_init.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_init.cpp$(DependSuffix): ../source/init.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_init.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_init.cpp$(DependSuffix) -MM "../source/init.cpp"

$(IntermediateDirectory)/source_init.cpp$(PreprocessSuffix): ../source/init.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_init.cpp$(PreprocessSuffix) "../source/init.cpp"

$(IntermediateDirectory)/source_area_editor.cpp$(ObjectSuffix): ../source/area_editor.cpp $(IntermediateDirectory)/source_area_editor.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/area_editor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_area_editor.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_area_editor.cpp$(DependSuffix): ../source/area_editor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_area_editor.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_area_editor.cpp$(DependSuffix) -MM "../source/area_editor.cpp"

$(IntermediateDirectory)/source_area_editor.cpp$(PreprocessSuffix): ../source/area_editor.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_area_editor.cpp$(PreprocessSuffix) "../source/area_editor.cpp"

$(IntermediateDirectory)/source_menus.cpp$(ObjectSuffix): ../source/menus.cpp $(IntermediateDirectory)/source_menus.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/menus.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_menus.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_menus.cpp$(DependSuffix): ../source/menus.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_menus.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_menus.cpp$(DependSuffix) -MM "../source/menus.cpp"

$(IntermediateDirectory)/source_menus.cpp$(PreprocessSuffix): ../source/menus.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_menus.cpp$(PreprocessSuffix) "../source/menus.cpp"

$(IntermediateDirectory)/source_controls.cpp$(ObjectSuffix): ../source/controls.cpp $(IntermediateDirectory)/source_controls.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/controls.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_controls.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_controls.cpp$(DependSuffix): ../source/controls.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_controls.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_controls.cpp$(DependSuffix) -MM "../source/controls.cpp"

$(IntermediateDirectory)/source_controls.cpp$(PreprocessSuffix): ../source/controls.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_controls.cpp$(PreprocessSuffix) "../source/controls.cpp"

$(IntermediateDirectory)/source_logic.cpp$(ObjectSuffix): ../source/logic.cpp $(IntermediateDirectory)/source_logic.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/logic.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_logic.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_logic.cpp$(DependSuffix): ../source/logic.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_logic.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_logic.cpp$(DependSuffix) -MM "../source/logic.cpp"

$(IntermediateDirectory)/source_logic.cpp$(PreprocessSuffix): ../source/logic.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_logic.cpp$(PreprocessSuffix) "../source/logic.cpp"

$(IntermediateDirectory)/source_animation.cpp$(ObjectSuffix): ../source/animation.cpp $(IntermediateDirectory)/source_animation.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/animation.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_animation.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_animation.cpp$(DependSuffix): ../source/animation.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_animation.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_animation.cpp$(DependSuffix) -MM "../source/animation.cpp"

$(IntermediateDirectory)/source_animation.cpp$(PreprocessSuffix): ../source/animation.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_animation.cpp$(PreprocessSuffix) "../source/animation.cpp"

$(IntermediateDirectory)/source_animation_editor.cpp$(ObjectSuffix): ../source/animation_editor.cpp $(IntermediateDirectory)/source_animation_editor.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/animation_editor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_animation_editor.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_animation_editor.cpp$(DependSuffix): ../source/animation_editor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_animation_editor.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_animation_editor.cpp$(DependSuffix) -MM "../source/animation_editor.cpp"

$(IntermediateDirectory)/source_animation_editor.cpp$(PreprocessSuffix): ../source/animation_editor.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_animation_editor.cpp$(PreprocessSuffix) "../source/animation_editor.cpp"

$(IntermediateDirectory)/source_spray_type.cpp$(ObjectSuffix): ../source/spray_type.cpp $(IntermediateDirectory)/source_spray_type.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/spray_type.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_spray_type.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_spray_type.cpp$(DependSuffix): ../source/spray_type.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_spray_type.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_spray_type.cpp$(DependSuffix) -MM "../source/spray_type.cpp"

$(IntermediateDirectory)/source_spray_type.cpp$(PreprocessSuffix): ../source/spray_type.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_spray_type.cpp$(PreprocessSuffix) "../source/spray_type.cpp"

$(IntermediateDirectory)/source_vars.cpp$(ObjectSuffix): ../source/vars.cpp $(IntermediateDirectory)/source_vars.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/vars.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_vars.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_vars.cpp$(DependSuffix): ../source/vars.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_vars.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_vars.cpp$(DependSuffix) -MM "../source/vars.cpp"

$(IntermediateDirectory)/source_vars.cpp$(PreprocessSuffix): ../source/vars.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_vars.cpp$(PreprocessSuffix) "../source/vars.cpp"

$(IntermediateDirectory)/source_sector.cpp$(ObjectSuffix): ../source/sector.cpp $(IntermediateDirectory)/source_sector.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/sector.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/source_sector.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/source_sector.cpp$(DependSuffix): ../source/sector.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/source_sector.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/source_sector.cpp$(DependSuffix) -MM "../source/sector.cpp"

$(IntermediateDirectory)/source_sector.cpp$(PreprocessSuffix): ../source/sector.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/source_sector.cpp$(PreprocessSuffix) "../source/sector.cpp"

$(IntermediateDirectory)/LAFI_frame.cpp$(ObjectSuffix): ../source/LAFI/frame.cpp $(IntermediateDirectory)/LAFI_frame.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/LAFI/frame.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LAFI_frame.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LAFI_frame.cpp$(DependSuffix): ../source/LAFI/frame.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LAFI_frame.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/LAFI_frame.cpp$(DependSuffix) -MM "../source/LAFI/frame.cpp"

$(IntermediateDirectory)/LAFI_frame.cpp$(PreprocessSuffix): ../source/LAFI/frame.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LAFI_frame.cpp$(PreprocessSuffix) "../source/LAFI/frame.cpp"

$(IntermediateDirectory)/LAFI_gui.cpp$(ObjectSuffix): ../source/LAFI/gui.cpp $(IntermediateDirectory)/LAFI_gui.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/LAFI/gui.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LAFI_gui.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LAFI_gui.cpp$(DependSuffix): ../source/LAFI/gui.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LAFI_gui.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/LAFI_gui.cpp$(DependSuffix) -MM "../source/LAFI/gui.cpp"

$(IntermediateDirectory)/LAFI_gui.cpp$(PreprocessSuffix): ../source/LAFI/gui.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LAFI_gui.cpp$(PreprocessSuffix) "../source/LAFI/gui.cpp"

$(IntermediateDirectory)/LAFI_label.cpp$(ObjectSuffix): ../source/LAFI/label.cpp $(IntermediateDirectory)/LAFI_label.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/LAFI/label.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LAFI_label.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LAFI_label.cpp$(DependSuffix): ../source/LAFI/label.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LAFI_label.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/LAFI_label.cpp$(DependSuffix) -MM "../source/LAFI/label.cpp"

$(IntermediateDirectory)/LAFI_label.cpp$(PreprocessSuffix): ../source/LAFI/label.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LAFI_label.cpp$(PreprocessSuffix) "../source/LAFI/label.cpp"

$(IntermediateDirectory)/LAFI_angle_picker.cpp$(ObjectSuffix): ../source/LAFI/angle_picker.cpp $(IntermediateDirectory)/LAFI_angle_picker.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/LAFI/angle_picker.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LAFI_angle_picker.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LAFI_angle_picker.cpp$(DependSuffix): ../source/LAFI/angle_picker.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LAFI_angle_picker.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/LAFI_angle_picker.cpp$(DependSuffix) -MM "../source/LAFI/angle_picker.cpp"

$(IntermediateDirectory)/LAFI_angle_picker.cpp$(PreprocessSuffix): ../source/LAFI/angle_picker.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LAFI_angle_picker.cpp$(PreprocessSuffix) "../source/LAFI/angle_picker.cpp"

$(IntermediateDirectory)/LAFI_widget.cpp$(ObjectSuffix): ../source/LAFI/widget.cpp $(IntermediateDirectory)/LAFI_widget.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/LAFI/widget.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LAFI_widget.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LAFI_widget.cpp$(DependSuffix): ../source/LAFI/widget.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LAFI_widget.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/LAFI_widget.cpp$(DependSuffix) -MM "../source/LAFI/widget.cpp"

$(IntermediateDirectory)/LAFI_widget.cpp$(PreprocessSuffix): ../source/LAFI/widget.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LAFI_widget.cpp$(PreprocessSuffix) "../source/LAFI/widget.cpp"

$(IntermediateDirectory)/LAFI_scrollbar.cpp$(ObjectSuffix): ../source/LAFI/scrollbar.cpp $(IntermediateDirectory)/LAFI_scrollbar.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/LAFI/scrollbar.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LAFI_scrollbar.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LAFI_scrollbar.cpp$(DependSuffix): ../source/LAFI/scrollbar.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LAFI_scrollbar.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/LAFI_scrollbar.cpp$(DependSuffix) -MM "../source/LAFI/scrollbar.cpp"

$(IntermediateDirectory)/LAFI_scrollbar.cpp$(PreprocessSuffix): ../source/LAFI/scrollbar.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LAFI_scrollbar.cpp$(PreprocessSuffix) "../source/LAFI/scrollbar.cpp"

$(IntermediateDirectory)/LAFI_image.cpp$(ObjectSuffix): ../source/LAFI/image.cpp $(IntermediateDirectory)/LAFI_image.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/LAFI/image.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LAFI_image.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LAFI_image.cpp$(DependSuffix): ../source/LAFI/image.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LAFI_image.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/LAFI_image.cpp$(DependSuffix) -MM "../source/LAFI/image.cpp"

$(IntermediateDirectory)/LAFI_image.cpp$(PreprocessSuffix): ../source/LAFI/image.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LAFI_image.cpp$(PreprocessSuffix) "../source/LAFI/image.cpp"

$(IntermediateDirectory)/LAFI_checkbox.cpp$(ObjectSuffix): ../source/LAFI/checkbox.cpp $(IntermediateDirectory)/LAFI_checkbox.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/LAFI/checkbox.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LAFI_checkbox.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LAFI_checkbox.cpp$(DependSuffix): ../source/LAFI/checkbox.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LAFI_checkbox.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/LAFI_checkbox.cpp$(DependSuffix) -MM "../source/LAFI/checkbox.cpp"

$(IntermediateDirectory)/LAFI_checkbox.cpp$(PreprocessSuffix): ../source/LAFI/checkbox.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LAFI_checkbox.cpp$(PreprocessSuffix) "../source/LAFI/checkbox.cpp"

$(IntermediateDirectory)/LAFI_radio_button.cpp$(ObjectSuffix): ../source/LAFI/radio_button.cpp $(IntermediateDirectory)/LAFI_radio_button.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/LAFI/radio_button.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LAFI_radio_button.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LAFI_radio_button.cpp$(DependSuffix): ../source/LAFI/radio_button.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LAFI_radio_button.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/LAFI_radio_button.cpp$(DependSuffix) -MM "../source/LAFI/radio_button.cpp"

$(IntermediateDirectory)/LAFI_radio_button.cpp$(PreprocessSuffix): ../source/LAFI/radio_button.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LAFI_radio_button.cpp$(PreprocessSuffix) "../source/LAFI/radio_button.cpp"

$(IntermediateDirectory)/LAFI_minor.cpp$(ObjectSuffix): ../source/LAFI/minor.cpp $(IntermediateDirectory)/LAFI_minor.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/LAFI/minor.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LAFI_minor.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LAFI_minor.cpp$(DependSuffix): ../source/LAFI/minor.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LAFI_minor.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/LAFI_minor.cpp$(DependSuffix) -MM "../source/LAFI/minor.cpp"

$(IntermediateDirectory)/LAFI_minor.cpp$(PreprocessSuffix): ../source/LAFI/minor.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LAFI_minor.cpp$(PreprocessSuffix) "../source/LAFI/minor.cpp"

$(IntermediateDirectory)/LAFI_button.cpp$(ObjectSuffix): ../source/LAFI/button.cpp $(IntermediateDirectory)/LAFI_button.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/LAFI/button.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LAFI_button.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LAFI_button.cpp$(DependSuffix): ../source/LAFI/button.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LAFI_button.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/LAFI_button.cpp$(DependSuffix) -MM "../source/LAFI/button.cpp"

$(IntermediateDirectory)/LAFI_button.cpp$(PreprocessSuffix): ../source/LAFI/button.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LAFI_button.cpp$(PreprocessSuffix) "../source/LAFI/button.cpp"

$(IntermediateDirectory)/LAFI_style.cpp$(ObjectSuffix): ../source/LAFI/style.cpp $(IntermediateDirectory)/LAFI_style.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/LAFI/style.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LAFI_style.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LAFI_style.cpp$(DependSuffix): ../source/LAFI/style.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LAFI_style.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/LAFI_style.cpp$(DependSuffix) -MM "../source/LAFI/style.cpp"

$(IntermediateDirectory)/LAFI_style.cpp$(PreprocessSuffix): ../source/LAFI/style.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LAFI_style.cpp$(PreprocessSuffix) "../source/LAFI/style.cpp"

$(IntermediateDirectory)/LAFI_textbox.cpp$(ObjectSuffix): ../source/LAFI/textbox.cpp $(IntermediateDirectory)/LAFI_textbox.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/LAFI/textbox.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/LAFI_textbox.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/LAFI_textbox.cpp$(DependSuffix): ../source/LAFI/textbox.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/LAFI_textbox.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/LAFI_textbox.cpp$(DependSuffix) -MM "../source/LAFI/textbox.cpp"

$(IntermediateDirectory)/LAFI_textbox.cpp$(PreprocessSuffix): ../source/LAFI/textbox.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/LAFI_textbox.cpp$(PreprocessSuffix) "../source/LAFI/textbox.cpp"

$(IntermediateDirectory)/mobs_ship_type.cpp$(ObjectSuffix): ../source/mobs/ship_type.cpp $(IntermediateDirectory)/mobs_ship_type.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/ship_type.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_ship_type.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_ship_type.cpp$(DependSuffix): ../source/mobs/ship_type.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_ship_type.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_ship_type.cpp$(DependSuffix) -MM "../source/mobs/ship_type.cpp"

$(IntermediateDirectory)/mobs_ship_type.cpp$(PreprocessSuffix): ../source/mobs/ship_type.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_ship_type.cpp$(PreprocessSuffix) "../source/mobs/ship_type.cpp"

$(IntermediateDirectory)/mobs_leader.cpp$(ObjectSuffix): ../source/mobs/leader.cpp $(IntermediateDirectory)/mobs_leader.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/leader.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_leader.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_leader.cpp$(DependSuffix): ../source/mobs/leader.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_leader.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_leader.cpp$(DependSuffix) -MM "../source/mobs/leader.cpp"

$(IntermediateDirectory)/mobs_leader.cpp$(PreprocessSuffix): ../source/mobs/leader.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_leader.cpp$(PreprocessSuffix) "../source/mobs/leader.cpp"

$(IntermediateDirectory)/mobs_onion_fsm.cpp$(ObjectSuffix): ../source/mobs/onion_fsm.cpp $(IntermediateDirectory)/mobs_onion_fsm.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/onion_fsm.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_onion_fsm.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_onion_fsm.cpp$(DependSuffix): ../source/mobs/onion_fsm.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_onion_fsm.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_onion_fsm.cpp$(DependSuffix) -MM "../source/mobs/onion_fsm.cpp"

$(IntermediateDirectory)/mobs_onion_fsm.cpp$(PreprocessSuffix): ../source/mobs/onion_fsm.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_onion_fsm.cpp$(PreprocessSuffix) "../source/mobs/onion_fsm.cpp"

$(IntermediateDirectory)/mobs_ship.cpp$(ObjectSuffix): ../source/mobs/ship.cpp $(IntermediateDirectory)/mobs_ship.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/ship.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_ship.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_ship.cpp$(DependSuffix): ../source/mobs/ship.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_ship.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_ship.cpp$(DependSuffix) -MM "../source/mobs/ship.cpp"

$(IntermediateDirectory)/mobs_ship.cpp$(PreprocessSuffix): ../source/mobs/ship.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_ship.cpp$(PreprocessSuffix) "../source/mobs/ship.cpp"

$(IntermediateDirectory)/mobs_treasure_type.cpp$(ObjectSuffix): ../source/mobs/treasure_type.cpp $(IntermediateDirectory)/mobs_treasure_type.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/treasure_type.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_treasure_type.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_treasure_type.cpp$(DependSuffix): ../source/mobs/treasure_type.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_treasure_type.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_treasure_type.cpp$(DependSuffix) -MM "../source/mobs/treasure_type.cpp"

$(IntermediateDirectory)/mobs_treasure_type.cpp$(PreprocessSuffix): ../source/mobs/treasure_type.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_treasure_type.cpp$(PreprocessSuffix) "../source/mobs/treasure_type.cpp"

$(IntermediateDirectory)/mobs_onion.cpp$(ObjectSuffix): ../source/mobs/onion.cpp $(IntermediateDirectory)/mobs_onion.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/onion.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_onion.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_onion.cpp$(DependSuffix): ../source/mobs/onion.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_onion.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_onion.cpp$(DependSuffix) -MM "../source/mobs/onion.cpp"

$(IntermediateDirectory)/mobs_onion.cpp$(PreprocessSuffix): ../source/mobs/onion.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_onion.cpp$(PreprocessSuffix) "../source/mobs/onion.cpp"

$(IntermediateDirectory)/mobs_gate_fsm.cpp$(ObjectSuffix): ../source/mobs/gate_fsm.cpp $(IntermediateDirectory)/mobs_gate_fsm.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/gate_fsm.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_gate_fsm.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_gate_fsm.cpp$(DependSuffix): ../source/mobs/gate_fsm.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_gate_fsm.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_gate_fsm.cpp$(DependSuffix) -MM "../source/mobs/gate_fsm.cpp"

$(IntermediateDirectory)/mobs_gate_fsm.cpp$(PreprocessSuffix): ../source/mobs/gate_fsm.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_gate_fsm.cpp$(PreprocessSuffix) "../source/mobs/gate_fsm.cpp"

$(IntermediateDirectory)/mobs_pikmin.cpp$(ObjectSuffix): ../source/mobs/pikmin.cpp $(IntermediateDirectory)/mobs_pikmin.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/pikmin.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_pikmin.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_pikmin.cpp$(DependSuffix): ../source/mobs/pikmin.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_pikmin.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_pikmin.cpp$(DependSuffix) -MM "../source/mobs/pikmin.cpp"

$(IntermediateDirectory)/mobs_pikmin.cpp$(PreprocessSuffix): ../source/mobs/pikmin.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_pikmin.cpp$(PreprocessSuffix) "../source/mobs/pikmin.cpp"

$(IntermediateDirectory)/mobs_leader_fsm.cpp$(ObjectSuffix): ../source/mobs/leader_fsm.cpp $(IntermediateDirectory)/mobs_leader_fsm.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/leader_fsm.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_leader_fsm.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_leader_fsm.cpp$(DependSuffix): ../source/mobs/leader_fsm.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_leader_fsm.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_leader_fsm.cpp$(DependSuffix) -MM "../source/mobs/leader_fsm.cpp"

$(IntermediateDirectory)/mobs_leader_fsm.cpp$(PreprocessSuffix): ../source/mobs/leader_fsm.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_leader_fsm.cpp$(PreprocessSuffix) "../source/mobs/leader_fsm.cpp"

$(IntermediateDirectory)/mobs_info_spot.cpp$(ObjectSuffix): ../source/mobs/info_spot.cpp $(IntermediateDirectory)/mobs_info_spot.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/info_spot.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_info_spot.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_info_spot.cpp$(DependSuffix): ../source/mobs/info_spot.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_info_spot.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_info_spot.cpp$(DependSuffix) -MM "../source/mobs/info_spot.cpp"

$(IntermediateDirectory)/mobs_info_spot.cpp$(PreprocessSuffix): ../source/mobs/info_spot.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_info_spot.cpp$(PreprocessSuffix) "../source/mobs/info_spot.cpp"

$(IntermediateDirectory)/mobs_pikmin_fsm.cpp$(ObjectSuffix): ../source/mobs/pikmin_fsm.cpp $(IntermediateDirectory)/mobs_pikmin_fsm.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/pikmin_fsm.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_pikmin_fsm.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_pikmin_fsm.cpp$(DependSuffix): ../source/mobs/pikmin_fsm.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_pikmin_fsm.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_pikmin_fsm.cpp$(DependSuffix) -MM "../source/mobs/pikmin_fsm.cpp"

$(IntermediateDirectory)/mobs_pikmin_fsm.cpp$(PreprocessSuffix): ../source/mobs/pikmin_fsm.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_pikmin_fsm.cpp$(PreprocessSuffix) "../source/mobs/pikmin_fsm.cpp"

$(IntermediateDirectory)/mobs_onion_type.cpp$(ObjectSuffix): ../source/mobs/onion_type.cpp $(IntermediateDirectory)/mobs_onion_type.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/onion_type.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_onion_type.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_onion_type.cpp$(DependSuffix): ../source/mobs/onion_type.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_onion_type.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_onion_type.cpp$(DependSuffix) -MM "../source/mobs/onion_type.cpp"

$(IntermediateDirectory)/mobs_onion_type.cpp$(PreprocessSuffix): ../source/mobs/onion_type.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_onion_type.cpp$(PreprocessSuffix) "../source/mobs/onion_type.cpp"

$(IntermediateDirectory)/mobs_mob.cpp$(ObjectSuffix): ../source/mobs/mob.cpp $(IntermediateDirectory)/mobs_mob.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/mob.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_mob.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_mob.cpp$(DependSuffix): ../source/mobs/mob.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_mob.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_mob.cpp$(DependSuffix) -MM "../source/mobs/mob.cpp"

$(IntermediateDirectory)/mobs_mob.cpp$(PreprocessSuffix): ../source/mobs/mob.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_mob.cpp$(PreprocessSuffix) "../source/mobs/mob.cpp"

$(IntermediateDirectory)/mobs_mob_type.cpp$(ObjectSuffix): ../source/mobs/mob_type.cpp $(IntermediateDirectory)/mobs_mob_type.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/mob_type.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_mob_type.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_mob_type.cpp$(DependSuffix): ../source/mobs/mob_type.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_mob_type.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_mob_type.cpp$(DependSuffix) -MM "../source/mobs/mob_type.cpp"

$(IntermediateDirectory)/mobs_mob_type.cpp$(PreprocessSuffix): ../source/mobs/mob_type.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_mob_type.cpp$(PreprocessSuffix) "../source/mobs/mob_type.cpp"

$(IntermediateDirectory)/mobs_nectar.cpp$(ObjectSuffix): ../source/mobs/nectar.cpp $(IntermediateDirectory)/mobs_nectar.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/nectar.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_nectar.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_nectar.cpp$(DependSuffix): ../source/mobs/nectar.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_nectar.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_nectar.cpp$(DependSuffix) -MM "../source/mobs/nectar.cpp"

$(IntermediateDirectory)/mobs_nectar.cpp$(PreprocessSuffix): ../source/mobs/nectar.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_nectar.cpp$(PreprocessSuffix) "../source/mobs/nectar.cpp"

$(IntermediateDirectory)/mobs_leader_type.cpp$(ObjectSuffix): ../source/mobs/leader_type.cpp $(IntermediateDirectory)/mobs_leader_type.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/leader_type.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_leader_type.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_leader_type.cpp$(DependSuffix): ../source/mobs/leader_type.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_leader_type.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_leader_type.cpp$(DependSuffix) -MM "../source/mobs/leader_type.cpp"

$(IntermediateDirectory)/mobs_leader_type.cpp$(PreprocessSuffix): ../source/mobs/leader_type.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_leader_type.cpp$(PreprocessSuffix) "../source/mobs/leader_type.cpp"

$(IntermediateDirectory)/mobs_pikmin_type.cpp$(ObjectSuffix): ../source/mobs/pikmin_type.cpp $(IntermediateDirectory)/mobs_pikmin_type.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/pikmin_type.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_pikmin_type.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_pikmin_type.cpp$(DependSuffix): ../source/mobs/pikmin_type.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_pikmin_type.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_pikmin_type.cpp$(DependSuffix) -MM "../source/mobs/pikmin_type.cpp"

$(IntermediateDirectory)/mobs_pikmin_type.cpp$(PreprocessSuffix): ../source/mobs/pikmin_type.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_pikmin_type.cpp$(PreprocessSuffix) "../source/mobs/pikmin_type.cpp"

$(IntermediateDirectory)/mobs_mob_fsm.cpp$(ObjectSuffix): ../source/mobs/mob_fsm.cpp $(IntermediateDirectory)/mobs_mob_fsm.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/mob_fsm.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_mob_fsm.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_mob_fsm.cpp$(DependSuffix): ../source/mobs/mob_fsm.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_mob_fsm.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_mob_fsm.cpp$(DependSuffix) -MM "../source/mobs/mob_fsm.cpp"

$(IntermediateDirectory)/mobs_mob_fsm.cpp$(PreprocessSuffix): ../source/mobs/mob_fsm.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_mob_fsm.cpp$(PreprocessSuffix) "../source/mobs/mob_fsm.cpp"

$(IntermediateDirectory)/mobs_gate_type.cpp$(ObjectSuffix): ../source/mobs/gate_type.cpp $(IntermediateDirectory)/mobs_gate_type.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/gate_type.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_gate_type.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_gate_type.cpp$(DependSuffix): ../source/mobs/gate_type.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_gate_type.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_gate_type.cpp$(DependSuffix) -MM "../source/mobs/gate_type.cpp"

$(IntermediateDirectory)/mobs_gate_type.cpp$(PreprocessSuffix): ../source/mobs/gate_type.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_gate_type.cpp$(PreprocessSuffix) "../source/mobs/gate_type.cpp"

$(IntermediateDirectory)/mobs_enemy_type.cpp$(ObjectSuffix): ../source/mobs/enemy_type.cpp $(IntermediateDirectory)/mobs_enemy_type.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/enemy_type.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_enemy_type.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_enemy_type.cpp$(DependSuffix): ../source/mobs/enemy_type.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_enemy_type.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_enemy_type.cpp$(DependSuffix) -MM "../source/mobs/enemy_type.cpp"

$(IntermediateDirectory)/mobs_enemy_type.cpp$(PreprocessSuffix): ../source/mobs/enemy_type.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_enemy_type.cpp$(PreprocessSuffix) "../source/mobs/enemy_type.cpp"

$(IntermediateDirectory)/mobs_treasure_fsm.cpp$(ObjectSuffix): ../source/mobs/treasure_fsm.cpp $(IntermediateDirectory)/mobs_treasure_fsm.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/treasure_fsm.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_treasure_fsm.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_treasure_fsm.cpp$(DependSuffix): ../source/mobs/treasure_fsm.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_treasure_fsm.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_treasure_fsm.cpp$(DependSuffix) -MM "../source/mobs/treasure_fsm.cpp"

$(IntermediateDirectory)/mobs_treasure_fsm.cpp$(PreprocessSuffix): ../source/mobs/treasure_fsm.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_treasure_fsm.cpp$(PreprocessSuffix) "../source/mobs/treasure_fsm.cpp"

$(IntermediateDirectory)/mobs_gate.cpp$(ObjectSuffix): ../source/mobs/gate.cpp $(IntermediateDirectory)/mobs_gate.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/gate.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_gate.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_gate.cpp$(DependSuffix): ../source/mobs/gate.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_gate.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_gate.cpp$(DependSuffix) -MM "../source/mobs/gate.cpp"

$(IntermediateDirectory)/mobs_gate.cpp$(PreprocessSuffix): ../source/mobs/gate.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_gate.cpp$(PreprocessSuffix) "../source/mobs/gate.cpp"

$(IntermediateDirectory)/mobs_ship_fsm.cpp$(ObjectSuffix): ../source/mobs/ship_fsm.cpp $(IntermediateDirectory)/mobs_ship_fsm.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/ship_fsm.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_ship_fsm.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_ship_fsm.cpp$(DependSuffix): ../source/mobs/ship_fsm.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_ship_fsm.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_ship_fsm.cpp$(DependSuffix) -MM "../source/mobs/ship_fsm.cpp"

$(IntermediateDirectory)/mobs_ship_fsm.cpp$(PreprocessSuffix): ../source/mobs/ship_fsm.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_ship_fsm.cpp$(PreprocessSuffix) "../source/mobs/ship_fsm.cpp"

$(IntermediateDirectory)/mobs_pellet.cpp$(ObjectSuffix): ../source/mobs/pellet.cpp $(IntermediateDirectory)/mobs_pellet.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/pellet.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_pellet.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_pellet.cpp$(DependSuffix): ../source/mobs/pellet.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_pellet.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_pellet.cpp$(DependSuffix) -MM "../source/mobs/pellet.cpp"

$(IntermediateDirectory)/mobs_pellet.cpp$(PreprocessSuffix): ../source/mobs/pellet.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_pellet.cpp$(PreprocessSuffix) "../source/mobs/pellet.cpp"

$(IntermediateDirectory)/mobs_enemy.cpp$(ObjectSuffix): ../source/mobs/enemy.cpp $(IntermediateDirectory)/mobs_enemy.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/enemy.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_enemy.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_enemy.cpp$(DependSuffix): ../source/mobs/enemy.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_enemy.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_enemy.cpp$(DependSuffix) -MM "../source/mobs/enemy.cpp"

$(IntermediateDirectory)/mobs_enemy.cpp$(PreprocessSuffix): ../source/mobs/enemy.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_enemy.cpp$(PreprocessSuffix) "../source/mobs/enemy.cpp"

$(IntermediateDirectory)/mobs_pellet_fsm.cpp$(ObjectSuffix): ../source/mobs/pellet_fsm.cpp $(IntermediateDirectory)/mobs_pellet_fsm.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/pellet_fsm.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_pellet_fsm.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_pellet_fsm.cpp$(DependSuffix): ../source/mobs/pellet_fsm.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_pellet_fsm.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_pellet_fsm.cpp$(DependSuffix) -MM "../source/mobs/pellet_fsm.cpp"

$(IntermediateDirectory)/mobs_pellet_fsm.cpp$(PreprocessSuffix): ../source/mobs/pellet_fsm.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_pellet_fsm.cpp$(PreprocessSuffix) "../source/mobs/pellet_fsm.cpp"

$(IntermediateDirectory)/mobs_bridge.cpp$(ObjectSuffix): ../source/mobs/bridge.cpp $(IntermediateDirectory)/mobs_bridge.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/bridge.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_bridge.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_bridge.cpp$(DependSuffix): ../source/mobs/bridge.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_bridge.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_bridge.cpp$(DependSuffix) -MM "../source/mobs/bridge.cpp"

$(IntermediateDirectory)/mobs_bridge.cpp$(PreprocessSuffix): ../source/mobs/bridge.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_bridge.cpp$(PreprocessSuffix) "../source/mobs/bridge.cpp"

$(IntermediateDirectory)/mobs_treasure.cpp$(ObjectSuffix): ../source/mobs/treasure.cpp $(IntermediateDirectory)/mobs_treasure.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/treasure.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_treasure.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_treasure.cpp$(DependSuffix): ../source/mobs/treasure.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_treasure.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_treasure.cpp$(DependSuffix) -MM "../source/mobs/treasure.cpp"

$(IntermediateDirectory)/mobs_treasure.cpp$(PreprocessSuffix): ../source/mobs/treasure.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_treasure.cpp$(PreprocessSuffix) "../source/mobs/treasure.cpp"

$(IntermediateDirectory)/mobs_pellet_type.cpp$(ObjectSuffix): ../source/mobs/pellet_type.cpp $(IntermediateDirectory)/mobs_pellet_type.cpp$(DependSuffix)
	$(CXX) $(IncludePCH) $(SourceSwitch) "/home/andre/Dropbox/Programacao/C++/Pikmin_fangame_engine/Source/source/mobs/pellet_type.cpp" $(CXXFLAGS) $(ObjectSwitch)$(IntermediateDirectory)/mobs_pellet_type.cpp$(ObjectSuffix) $(IncludePath)
$(IntermediateDirectory)/mobs_pellet_type.cpp$(DependSuffix): ../source/mobs/pellet_type.cpp
	@$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) -MG -MP -MT$(IntermediateDirectory)/mobs_pellet_type.cpp$(ObjectSuffix) -MF$(IntermediateDirectory)/mobs_pellet_type.cpp$(DependSuffix) -MM "../source/mobs/pellet_type.cpp"

$(IntermediateDirectory)/mobs_pellet_type.cpp$(PreprocessSuffix): ../source/mobs/pellet_type.cpp
	$(CXX) $(CXXFLAGS) $(IncludePCH) $(IncludePath) $(PreprocessOnlySwitch) $(OutputSwitch) $(IntermediateDirectory)/mobs_pellet_type.cpp$(PreprocessSuffix) "../source/mobs/pellet_type.cpp"


-include $(IntermediateDirectory)/*$(DependSuffix)
##
## Clean
##
clean:
	$(RM) -r ./Debug/


