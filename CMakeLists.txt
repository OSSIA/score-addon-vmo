project(score_addon_vmo LANGUAGES CXX)

# General initialization
score_common_setup()

# Source files
set(HDRS
    vmo/Executor.hpp
    vmo/Inspector.hpp
    vmo/LocalTree.hpp
    vmo/Metadata.hpp
    vmo/Presenter.hpp
    vmo/Process.hpp
    vmo/View.hpp
    vmo/Layer.hpp
    vmo/CommandFactory.hpp

    score_addon_vmo.hpp
)
set(SRCS
    vmo/Executor.cpp
    vmo/Inspector.cpp
    vmo/LocalTree.cpp
    vmo/Presenter.cpp
    vmo/Process.cpp
    vmo/View.cpp

    score_addon_vmo.cpp
)


# Creation of the library
add_library(${PROJECT_NAME} ${SRCS} ${HDRS})

# Code generation
score_generate_command_list_file(${PROJECT_NAME} "${HDRS}")

# Link
target_link_libraries(${PROJECT_NAME} PUBLIC score_plugin_engine)

# Target-specific options
setup_score_plugin(${PROJECT_NAME})