list(APPEND GIO_INPUTS
	"gio.c"
	"gprintf.c"
)

add_library(gio STATIC ${GIO_INPUTS})
