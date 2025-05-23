zephyr_library()

zephyr_library_sources_ifdef(CONFIG_MIRAMESH
    integration/miramesh-integration.c
    integration/miramesh-integration-debug.c
    integration/miramesh-integration-memory.c
    integration/miramesh-integration-thread.c
    integration/swi-callback-handler.c
)

get_filename_component(DEFAULT_LIBMIRA_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../../vendor/libmira" ABSOLUTE)

if(NOT DEFINED LIBMIRA_LOCATION)
  set(LIBMIRA_LOCATION ${DEFAULT_LIBMIRA_LOCATION})
endif()

if (NOT EXISTS ${LIBMIRA_LOCATION})
  message(FATAL_ERROR "Looked for libmira at ${LIBMIRA_LOCATION} but directory does not exist. Set the libmira directory via the CMake variable LIBMIRA_LOCATION.")
endif()

if(DEFINED CONFIG_SOC_SERIES_NRF54LX)
    add_compile_definitions(MIRA_PLATFORM_NRF54L)
endif()

zephyr_include_directories_ifdef(CONFIG_MIRAMESH ${LIBMIRA_LOCATION}/include)
zephyr_include_directories_ifdef(CONFIG_MIRAMESH_INTEGRATION_VERIFICATION_LOG_EVENTS ${LIBMIRA_LOCATION}/diag_files)
zephyr_link_libraries_ifdef(CONFIG_SOC_NRF52840 ${LIBMIRA_LOCATION}/platform/nrf52840sdc-net/libmira.a)
zephyr_link_libraries_ifdef(CONFIG_SOC_NRF52832 ${LIBMIRA_LOCATION}/platform/nrf52832sdc-net/libmira.a)
zephyr_link_libraries_ifdef(CONFIG_SOC_SERIES_NRF54LX ${LIBMIRA_LOCATION}/platform/nrf54l-net/libmira.a)

# Since it depends on multiple definitions, use an intermediary definition
if(CONFIG_SOC_SERIES_NRF54LX AND CONFIG_TRUSTED_EXECUTION_NONSECURE)
    set(MIRA_NRF54_NS 1)
endif()
zephyr_link_libraries_ifdef(MIRA_NRF54_NS ${LIBMIRA_LOCATION}/platform/nrf54l-net_ns/libmira.a)

# Some modules causes the standard lib to be linked before miramesh
zephyr_link_libraries_ifdef(CONFIG_MIRAMESH -lc)

zephyr_library_include_directories_ifdef(CONFIG_MIRAMESH include)
target_include_directories(app PUBLIC include)
