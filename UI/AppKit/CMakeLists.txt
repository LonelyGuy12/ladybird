add_library(ladybird_impl STATIC
    Application/Application.mm
    Application/ApplicationDelegate.mm
    Application/EventLoopImplementationMacOS.mm
    Interface/Autocomplete.mm
    Interface/Event.mm
    Interface/InfoBar.mm
    Interface/LadybirdWebView.mm
    Interface/LadybirdWebViewBridge.cpp
    Interface/LadybirdWebViewWindow.mm
    Interface/Menu.mm
    Interface/Palette.mm
    Interface/SearchPanel.mm
    Interface/Tab.mm
    Interface/TabController.mm
    Utilities/Conversions.mm
)
target_include_directories(ladybird_impl PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

target_compile_options(ladybird_impl PUBLIC
        $<$<COMPILE_LANGUAGE:CXX>:-fobjc-arc>
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-anon-enum-enum-conversion> # Required for CGImageCreate
)
target_compile_features(ladybird_impl PUBLIC cxx_std_23)

add_executable(ladybird MACOSX_BUNDLE
   main.mm
)
target_link_libraries(ladybird_impl PUBLIC "-framework Cocoa -framework UniformTypeIdentifiers" LibUnicode)
target_link_libraries(ladybird PRIVATE ladybird_impl)

create_ladybird_bundle(ladybird)

# Bundle Python.framework into app during build (not just install)
if(APPLE)
    add_custom_command(TARGET ladybird POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Bundling Python into Ladybird.app Resources..."
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_CONTENT_DIR:ladybird>/Resources/bundled_python"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "/opt/homebrew/opt/python@3.14/Frameworks/Python.framework"
            "$<TARGET_BUNDLE_CONTENT_DIR:ladybird>/Resources/bundled_python"
        # Fix Python dylib references (python3.14 → bundled Python dylib)
        COMMAND ${CMAKE_COMMAND} -DBUNDLE_DIR="$<TARGET_BUNDLE_CONTENT_DIR:ladybird>" -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/FixPythonBundle.cmake"
        COMMAND ${CMAKE_COMMAND} -E echo "Python bundled successfully into Resources"
        COMMENT "Bundling Python into Resources directory"
    )
endif()
