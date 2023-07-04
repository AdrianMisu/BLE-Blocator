set(srcs "main.c"
    "board.c")

idf_component_register(SRCS "hardware_control.c" "main.c" "${srcs}"
                    INCLUDE_DIRS  ".")
