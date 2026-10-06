idf_component_register(
    SRCS 

            # Gamebuino low level : drivers
        "gb_ll/gb_ll_i2c.c"
        "gb_ll/gb_ll_adc.c"
        "gb_ll/gb_ll_audio.c"
        "gb_ll/gb_ll_lcd.c"
        "gb_ll/gb_ll_expander.c"
        "gb_ll/gb_ll_sdcard.c"
        "gb_ll/gb_ll_system.c"

            # Gamebuino mid level librarys : user entry level
        "gb_lib/gb_audio_player.cpp"
        "gb_lib/gb_audio_track_tone.cpp"
        "gb_lib/gb_audio_track_fx.cpp"
        "gb_lib/gb_audio_track_wav.cpp"
        "gb_lib/pmf_player/pmf_player.cpp"
        "gb_lib/gb_audio_track_pmf.cpp"
        "gb_lib/gb_core.cpp"
        "gb_lib/gb_graphics.cpp"

        "include_lib/gb_graphics_image.cpp"
        
    REQUIRES 
        esp_driver_ledc 
        esp_adc 
        esp_driver_i2c 
        esp_driver_gpio 
        spi_flash 
        esp_psram 
        esp_lcd 
        esp_timer 
        esp_driver_i2s 
        fatfs
    
    INCLUDE_DIRS "include_lib" "include_ll"
)

# Tolerances de warnings (memes que main/ et asteria/) : le style compact et
# certains patterns de la lib declenchent des warnings qui, en -Werror, feraient
# echouer le build sur un GCC/IDF different de celui de dev. On NE tolere PAS
# implicit-function-declaration (vraie erreur = fonction manquante).
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=reorder -Wno-error=switch -Wno-error=unused-variable
    -Wno-error=type-limits -Wno-error=narrowing -Wno-error=parentheses
    -Wno-error=maybe-uninitialized -Wno-error=unused-function -Wno-error=missing-field-initializers
    -Wno-error=unused-value -Wno-error=char-subscripts -Wno-error=unused-parameter
    -Wno-error=sign-compare -Wno-error=misleading-indentation -Wno-error=return-type
    -Wno-error=ignored-qualifiers)

