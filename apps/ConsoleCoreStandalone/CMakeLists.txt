juce_add_gui_app(ConsoleCoreStandalone
    PRODUCT_NAME "ConsoleCore"
    COMPANY_NAME "ConsoleCore Audio"
    BUNDLE_ID "com.consolecore.standalone"
)

target_sources(ConsoleCoreStandalone PRIVATE
    Source/Main.cpp
    Source/MainComponent.h
    Source/MainComponent.cpp
)

target_link_libraries(ConsoleCoreStandalone PRIVATE
    juce::juce_audio_utils
    juce::juce_gui_extra
    console_core
    routing
    meters
    ipc
)

target_compile_definitions(ConsoleCoreStandalone PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
)
