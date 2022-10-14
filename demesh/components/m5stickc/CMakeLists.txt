# TMoor: attempt to adapt to IDF 4.4.2

cmake_minimum_required(VERSION 3.5)

set(srcs "m5stickc.c"
         "util/AXP192.c"
	 "util/MPU6886.c"
	 "util/button.c"
	 "util/i2smic.c"
         "util/spi_master_lobo.c"
	 "util/tft.c"
	 "util/tftspi.c"
	 "util/wire.c"
	 "util/font/DefaultFont.c"
	 "util/font/DejaVuSans18.c"
	 "util/font/DejaVuSans24.c"
	 "util/font/SmallFont.c"
	 "util/font/Ubuntu16.c"
	 "util/font/comic24.c"
	 "util/font/def_small.c"
	 "util/font/default_tiny_9pt.c"
	 "util/font/minya24.c"
	 "util/font/tooney32.c")

idf_component_register(SRCS "${srcs}"
                    INCLUDE_DIRS "include"
                    PRIV_INCLUDE_DIRS "util/include")



