# Composant ESP-IDF ASTERIA.
# NOTE: ajoute ta bibliothèque AKA (gb_core/gb_graphics/gb_audio_player) dans REQUIRES,
# puis complète platform/gb_port_aka.cpp.
idf_component_register(
  SRCS
    "asteria_app.cpp"
    "player.cpp"
    "gamestate.cpp"
    "ui/text.cpp"
    "scene/scene.cpp"
    "scene/title_scene.cpp"
    "scene/create_scene.cpp"
    "scene/rules_scene.cpp"
    "scene/world_scene.cpp"
    "scene/combat_scene.cpp"
    "scene/shop_scene.cpp"
    "platform/gb_port_aka.cpp"
  INCLUDE_DIRS "."
  REQUIRES gamebuino aka_runtime
)

# Meme liste de tolerances que sur tous les autres jeux AKA de ce studio --
# le style compact du moteur (ex: "if(x) a; if(y) b;" sur une seule ligne
# dans world_scene.cpp) declenche -Werror=misleading-indentation sans ca.
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=reorder -Wno-error=switch -Wno-error=unused-variable
    -Wno-error=type-limits -Wno-error=narrowing -Wno-error=parentheses
    -Wno-error=maybe-uninitialized -Wno-error=unused-function -Wno-error=missing-field-initializers
    -Wno-error=unused-value -Wno-error=char-subscripts -Wno-error=unused-parameter
    -Wno-error=sign-compare -Wno-error=misleading-indentation -Wno-error=return-type
    -Wno-error=ignored-qualifiers)
