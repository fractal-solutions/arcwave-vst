cmake_minimum_required(VERSION 3.22)
project(ArcWave VERSION 0.3.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    if(NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "" FORCE)
    endif()
    if(NOT DEFINED CMAKE_OSX_ARCHITECTURES)
        set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "" FORCE)
    endif()
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-" CACHE STRING "" FORCE)
    set(CMAKE_XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME "NO" CACHE STRING "" FORCE)
endif()

add_subdirectory(third_party/JUCE)

juce_add_binary_data(ArcWaveFonts SOURCES
    assets/fonts/ChakraPetchBold.ttf
    assets/fonts/ChakraPetchSemiBold.ttf
    assets/fonts/ChakraPetchMedium.ttf
    assets/fonts/ShareTechMono.ttf
)

juce_add_plugin(ArcWave
    COMPANY_NAME "Qua$$a"
    BUNDLE_ID "com.quassa.arcwave"
    PLUGIN_MANUFACTURER_CODE QssA
    PLUGIN_CODE Arcw
    FORMATS VST3
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PRODUCT_NAME "ArcWave"
)

target_sources(ArcWave PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/SynthVoice.cpp
    src/WavetableBank.cpp
    src/Presets.cpp
    src/ArcWaveLookAndFeel.cpp
    src/Typography.cpp
)

target_compile_definitions(ArcWave PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(ArcWave PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    ArcWaveFonts
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

juce_add_console_app(ArcWaveRenderTest PRODUCT_NAME "ArcWave Render Test")

target_sources(ArcWaveRenderTest PRIVATE
    tests/RenderTest.cpp
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/SynthVoice.cpp
    src/WavetableBank.cpp
    src/Presets.cpp
    src/ArcWaveLookAndFeel.cpp
    src/Typography.cpp
)

target_include_directories(ArcWaveRenderTest PRIVATE src)

target_compile_definitions(ArcWaveRenderTest PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)

target_link_libraries(ArcWaveRenderTest PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    ArcWaveFonts
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
)
