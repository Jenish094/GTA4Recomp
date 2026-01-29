project("GTA4RecompLib")

add_compile_options(
    -fno-strict-aliasing
)

if (WIN32)
    add_compile_options(/fp:strict)
else()
    add_compile_options(-ffp-model=strict)
endif()

set(FERNANDO_RECOMP_PPC_RECOMPILED_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_config.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_context.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_func_mapping.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_recomp_shared.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_indirect.cpp"
)

foreach(i RANGE 0 86)
    list(APPEND FERNANDO_RECOMP_PPC_RECOMPILED_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/ppc/ppc_recomp.${i}.cpp")
endforeach()

set_source_files_properties(
    ${FERNANDO_RECOMP_PPC_RECOMPILED_SOURCES}
    PROPERTIES GENERATED TRUE
)

add_custom_command(
    OUTPUT ${FERNANDO_RECOMP_PPC_RECOMPILED_SOURCES}
    COMMAND $<TARGET_FILE:XenonRecomp>
        "${CMAKE_CURRENT_SOURCE_DIR}/config/config.toml"
        "${FERNANDO_RECOMP_TOOLS_ROOT}/XenonRecomp/XenonUtils/ppc_context.h"
    DEPENDS 
        XenonRecomp
        "${CMAKE_CURRENT_SOURCE_DIR}/config/config.toml"
        "${CMAKE_CURRENT_SOURCE_DIR}/private/default.xex"
        "${FERNANDO_RECOMP_TOOLS_ROOT}/XenonRecomp/XenonUtils/ppc_context.h"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Generating PPC recompiled sources from default.xex..."
    USES_TERMINAL
    VERBATIM
)

add_custom_target(GeneratePPCRecomp
    DEPENDS ${FERNANDO_RECOMP_PPC_RECOMPILED_SOURCES}
)

set(XENOS_RECOMP_ROOT "${FERNANDO_RECOMP_TOOLS_ROOT}/XenosRecomp/XenosRecomp")
set(XENOS_RECOMP_INCLUDE "${XENOS_RECOMP_ROOT}/shader_common.h")

file(GLOB XENOS_RECOMP_SOURCES 
    "${XENOS_RECOMP_ROOT}/*.h"
    "${XENOS_RECOMP_ROOT}/*.cpp"
)

set_source_files_properties(
    "${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp"
    PROPERTIES GENERATED TRUE
)

# shader recompilation
add_custom_command(
    OUTPUT 
        "${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp"
    COMMAND
        $<TARGET_FILE:XenosRecomp>
    DEPENDS 
        XenosRecomp
        "${CMAKE_CURRENT_SOURCE_DIR}/private/shaders"
        ${XENOS_RECOMP_SOURCES}
        ${XENOS_RECOMP_INCLUDE}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Generating shader cache from extracted shaders..."
    USES_TERMINAL
    VERBATIM
)

add_custom_target(GenerateShaderCache
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shader/shader_cache.cpp"
)

add_dependencies(GenerateShaderCache GeneratePPCRecomp)

add_library(GTA4RecompLib
    ${FERNANDO_RECOMP_PPC_RECOMPILED_SOURCES}
    "shader/shader_cache.h"
    "shader/shader_cache.cpp"
)

add_dependencies(GTA4RecompLib GeneratePPCRecomp GenerateShaderCache)

target_include_directories(GTA4RecompLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(GTA4RecompLib PRIVATE "${FERNANDO_RECOMP_TOOLS_ROOT}/XenonRecomp/thirdparty/simde")
target_precompile_headers(GTA4RecompLib PUBLIC "ppc/ppc_recomp_shared.h")
