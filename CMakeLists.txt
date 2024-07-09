cmake_minimum_required(VERSION 3.0)
project(calc)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(raylib)

add_executable(main
    calc/bit_register.cpp
    calc/bit_register.hpp
    calc/calc.cpp
    calc/calc.hpp
    calc/parse.cpp
    calc/parse.hpp
	calc/function.cpp
	calc/function.hpp
	calc/value.cpp
	calc/value.hpp
    main.cpp
    view/BitfieldDisplay.cpp
    view/BitfieldDisplay.hpp
    view/style.hpp
    view/view.cpp
    view/view.hpp
    viewmodel.cpp
    viewmodel.hpp
)

target_include_directories(main PRIVATE .)

target_compile_definitions(main PUBLIC __STDC_VERSION__=0)


if(NOT MSVC)
	target_compile_options(main
	PRIVATE
	    $<$<COMPILE_LANGUAGE:CXX>:
	    -Wall
	    -Wextra
		-Werror
	    -Wcast-align
	    -Wcast-qual
	    -Wctor-dtor-privacy
	    -Wdisabled-optimization
	    -Wformat=2
	    -Winit-self
	    -Wlogical-op
	    -Wmissing-declarations
	    -Wmissing-include-dirs
	    -Woverloaded-virtual
	    -Wredundant-decls
	    -Wshadow
	    -Wsign-promo
	    -Wstrict-null-sentinel
	    -Wstrict-overflow=5
	    -Wswitch
	    -Wundef
	    >
	)
endif()


target_link_libraries(main PRIVATE raylib)
