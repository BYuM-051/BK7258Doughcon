set(incs .
    ./common
    ./fonts
    /images
    ./screens
    ./ui
    ./ui/lifecycle
    )

set(srcs
    ap_main.c
    lvgl_vfs_init.c
    )

file(GLOB_RECURSE UI_EXTRA_SRCS "ui/*.c")
list(APPEND srcs ${UI_EXTRA_SRCS})
# Explicitly list new ui files so cmake detects them without a full clean
file(GLOB_RECURSE UI_LIFECYCLE_SRCS "ui/lifecycle/*.c")
list(APPEND srcs ${UI_LIFECYCLE_SRCS})
list(APPEND srcs
    ui/rtc_sync.c
    ui/fonts/lv_font_onoff_cn_32.c
    ui/fonts/lv_font_onoff_cn_72.c
    ui/fonts/lv_font_onoff_cn_90.c
)

    
    

if (CONFIG_LV_USE_DEMO_86BOX)
if (0)
list(APPEND srcs
    ui.c
    page_load_ctrol.c
    common/ui_helpers.c
    fonts/ui_font_FontHei96.c
    fonts/ui_font_Number.c
    fonts/ui_font_FontHei24.c
    fonts/ui_font_FontHei32.c
    images/ui_img_air_page_png.c
    images/ui_img_anime_lin2_png.c
    images/ui_img_anime_line1_png.c
    images/ui_img_arrow1_png.c
    images/ui_img_bg_img_png.c
    images/ui_img_cloud1_png.c
    images/ui_img_cold_symbol_png.c
    images/ui_img_common_switch1_png.c
    images/ui_img_curtain_off_png.c
    images/ui_img_curtain_on_png.c
    images/ui_img_curtain_page_png.c
    images/ui_img_curtain_stop_png.c
    images/ui_img_display_symbol_png.c
    images/ui_img_fan_symbol_png.c
    images/ui_img_first_page_png.c
    images/ui_img_info_symbol_png.c
    images/ui_img_light_page_png.c
    images/ui_img_light_symbol_off_png.c
    images/ui_img_light_symbol_on_png.c
    images/ui_img_mode_page_png.c
    images/ui_img_set_page_png.c
    images/ui_img_sun_png.c
    images/ui_img_switch_symbol_png.c
    images/ui_img_updata_symbol_png.c
    images/ui_img_voice_symbol_png.c
    images/ui_img_wifi_symbol_png.c
    images/ui_img_kongtiao_bg_png.c
    images/ui_img_cloud1_png.c
    images/ui_img_cloud_s_png.c
    images/ui_img_cloud_b_png.c
    screens/ui_UiArcPage.c
    screens/ui_UiPage1.c
    screens/ui_UiPage2.c
    screens/ui_UiPage3.c
    screens/ui_UiPage4.c
    screens/ui_UiPage5.c
    screens/ui_UiPage6.c
)
endif()
endif()

set(priv_req  media_service multimedia)

if ("${ARMINO_SOC}" STREQUAL "bk7258_ap")
list(APPEND priv_req  lvgl bk_display bk_cli media_service easy_flash key)
endif()


armino_component_register(SRCS "${srcs}" INCLUDE_DIRS "${incs}" PRIV_REQUIRES "${priv_req}"
)
add_compile_options(-Wno-error=stringop-truncation)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-unused-variable -O3)

# Copy .it.csv file to build directory
file(COPY ${CMAKE_CURRENT_LIST_DIR}/../.it.csv DESTINATION ${CMAKE_BINARY_DIR}/)