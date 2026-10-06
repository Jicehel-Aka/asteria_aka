idf_component_register(
    SRCS "src/aka_runtime.cpp" "src/core/input.cpp"
    INCLUDE_DIRS "include"
    REQUIRES gamebuino app_update)

# Memes tolerances de warnings que les autres composants (comportement inchange).
target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-error=reorder -Wno-error=switch -Wno-error=unused-variable
    -Wno-error=type-limits -Wno-error=narrowing -Wno-error=parentheses
    -Wno-error=maybe-uninitialized -Wno-error=unused-function -Wno-error=missing-field-initializers
    -Wno-error=unused-value -Wno-error=char-subscripts -Wno-error=unused-parameter
    -Wno-error=sign-compare -Wno-error=misleading-indentation -Wno-error=return-type
    -Wno-error=ignored-qualifiers)
