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
        # Remove Python.app (GUI wrapper for IDLE) - we don't need it
        COMMAND ${CMAKE_COMMAND} -E rm -rf "$<TARGET_BUNDLE_CONTENT_DIR:ladybird>/Resources/bundled_python/Versions/3.14/Resources/Python.app"
        # Patch python3.14 binary to remove hardcoded Python.app reference
        # The binary contains "Resources/Python.app/Contents/MacOS/Python" which venv tries to use
        COMMAND sed -i.bak "s|Resources/Python.app/Contents/MacOS/Python|\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00|g" "$<TARGET_BUNDLE_CONTENT_DIR:ladybird>/Resources/bundled_python/Versions/3.14/bin/python3.14"
        COMMAND rm -f "$<TARGET_BUNDLE_CONTENT_DIR:ladybird>/Resources/bundled_python/Versions/3.14/bin/python3.14.bak"
        COMMAND ${CMAKE_COMMAND} -E echo "Fixing Python dylib install names..."
        # Dynamically detect and fix the python3.14 executable dylib reference
        # Use otool to find the actual Homebrew path, then rewrite it
        COMMAND bash -c "OLD_PATH=$$(otool -L '$<TARGET_BUNDLE_CONTENT_DIR:ladybird>/Resources/bundled_python/Versions/3.14/bin/python3.14' | grep '/opt/homebrew/.*Python.framework' | awk '{print $$1}') && if [ -n \"$$OLD_PATH\" ]; then install_name_tool -change \"$$OLD_PATH\" '@executable_path/../Python' '$<TARGET_BUNDLE_CONTENT_DIR:ladybird>/Resources/bundled_python/Versions/3.14/bin/python3.14'; fi"
        COMMAND ${CMAKE_COMMAND} -E echo "Python bundled successfully into Resources"
        COMMENT "Bundling Python into Resources directory"
    )
endif()
