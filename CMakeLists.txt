cmake_minimum_required(VERSION 3.22)
project(MiniKnobz VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(APPLE)
 set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum supported macOS version")
endif()
if(MSVC)
 set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")  # static CRT: nothing extra to install
endif()

find_package(JUCE CONFIG QUIET)
if(NOT JUCE_FOUND)
 include(FetchContent)
 FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.12)
 FetchContent_MakeAvailable(JUCE)
endif()

juce_add_binary_data(knobz_assets SOURCES
 Assets/Fonts/SpaceGrotesk-Bold.ttf Assets/Fonts/SpaceGrotesk-Medium.ttf
 Assets/Fonts/Inter-Bold.ttf Assets/Fonts/Inter-Regular.ttf)
set_target_properties(knobz_assets PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(KNOBZ_SOURCES
 Source/Dsp/Spec.cpp Source/Dsp/Spec.h
 Source/Dsp/Effects.cpp Source/Dsp/Effects.h
 Source/Dsp/Gasp.cpp Source/Dsp/Gasp.h
 Source/Common/Processor.cpp Source/Common/Processor.h
 Source/Common/Editor.cpp Source/Common/Editor.h
 Source/Common/UpdateChecker.cpp Source/Common/UpdateChecker.h)

set(KNOBZ_DEFINITIONS JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_USE_DIRECT2D=0 JUCE_VST3_CAN_REPLACE_VST2=0
 JUCE_DISPLAY_SPLASH_SCREEN=0)

# knobz_plugin(<Target> <index> <plugin code> <product name> <bundle suffix> [STANDALONE])
function(knobz_plugin target index code product suffix)
 set(formats VST3)
 if(APPLE)
  list(APPEND formats AU)
 endif()
 if("STANDALONE" IN_LIST ARGN)
  list(APPEND formats Standalone)
 endif()
 juce_add_plugin(${target}
  COMPANY_NAME "Audio Makers"
  BUNDLE_ID "com.audiomakers.knobz.${suffix}"
  PLUGIN_MANUFACTURER_CODE AuMk
  PLUGIN_CODE ${code}
  ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/Assets/icon-1024.png"
  FORMATS ${formats}
  PRODUCT_NAME "${product}"
  VST3_CATEGORIES Fx
  AU_MAIN_TYPE kAudioUnitType_Effect
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  VST3_AUTO_MANIFEST FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE)
 target_sources(${target} PRIVATE ${KNOBZ_SOURCES})
 target_include_directories(${target} PRIVATE Source Source/Common)
 target_compile_definitions(${target} PUBLIC ${KNOBZ_DEFINITIONS} KNOBZ_PLUGIN=${index})
 target_link_libraries(${target} PRIVATE knobz_assets juce::juce_audio_utils juce::juce_audio_processors juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
endfunction()

knobz_plugin(KnobzBite     0 KzBt "Knobz Bite"     bite)
knobz_plugin(KnobzPhat     1 KzPh "Knobz Phat"     phat)
knobz_plugin(KnobzKickside 2 KzKs "Knobz Kickside" kickside)
knobz_plugin(KnobzWide     3 KzWd "Knobz Wide"     wide)
knobz_plugin(KnobzGasp     4 KzGs "Knobz Gasp"     gasp STANDALONE)

# Tests: audio checks for every plugin, mode and knob position (+ editor snapshots with KNOBZ_SNAPSHOT_DIR)
juce_add_console_app(knobz_tests PRODUCT_NAME "Knobz Tests")
target_sources(knobz_tests PRIVATE Tests/AudioTests.cpp ${KNOBZ_SOURCES})
target_include_directories(knobz_tests PRIVATE Source Source/Common)
target_compile_definitions(knobz_tests PRIVATE ${KNOBZ_DEFINITIONS} KNOBZ_PLUGIN=0 JucePlugin_VersionString="1.0.0"
 JUCE_STANDALONE_APPLICATION=1)
target_link_libraries(knobz_tests PRIVATE knobz_assets juce::juce_audio_utils juce::juce_audio_processors juce::juce_dsp
 PUBLIC juce::juce_recommended_config_flags)

enable_testing()
add_test(NAME knobz_audio_tests COMMAND knobz_tests)
