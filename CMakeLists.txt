idf_component_register(
    SRC_DIRS
        "src"
        "src/video"
        "src/audio"
        "src/sdcard"
        "src/bus"
        "src/lcd"
        "src/font/font8x16"
        "src/esp-libhelix-mp3"
    INCLUDE_DIRS
        "src"
        "src/video"
        "src/audio"
        "src/sdcard"
        "src/bus"
        "src/lcd"
        "src/font/font8x16"
        "src/esp-libhelix-mp3"
        "src/esp_new_jpeg/src"
    REQUIRES 
        freertos
        esp_driver_gpio
        esp_driver_i2s
        esp_driver_ledc
        esp_timer
        esp_lcd
        fatfs
)

add_prebuilt_library(esp_new_jpeg "${CMAKE_CURRENT_SOURCE_DIR}/src/${CONFIG_IDF_TARGET}/libesp_new_jpeg.a")
target_link_libraries(${COMPONENT_LIB} INTERFACE "-Wl,--start-group" esp_new_jpeg "-Wl,--end-group")

# Some of warinings, block them.
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-but-set-variable)