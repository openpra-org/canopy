# for boost tests
find_package(_boost COMPONENTS unit_test_framework REQUIRED)

add_subdirectory(device_compilation)
add_subdirectory(core)
add_subdirectory(ext)
add_subdirectory(mef)