set(EVENT_DRIVEN_SRCS  "event-driven/fsm.c" "event-driven/active.c" "event-driven/timer.c")

idf_component_register(SRCS ${EVENT_DRIVEN_SRCS} "user_input.c" "main.c"  "bsp.c"
                       INCLUDE_DIRS ".")
