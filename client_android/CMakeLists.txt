
add_custom_target(CortexClientAndroidApk ALL
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && gradlew assembleDebug
)
