#ifndef _IMAGES_ENUM_
#define _IMAGES_ENUM_

typedef enum {
	IMG_BULB,
	IMG_BULB_S,
	IMG_CHART,
	IMG_CHECK,
	IMG_COOLER,
	IMG_COOLER_S,
	IMG_DOOR,
	IMG_DOOR_S,
	IMG_GRASS,
	IMG_HUM,
	IMG_NET_NO,
	IMG_NET_OK,
	IMG_NEXT,
	IMG_PREV,
	IMG_PUMP,
	IMG_RU,
	IMG_SEEDS,
	IMG_TAP,
	IMG_TEMP,
	IMG_TIME_CAL,
	IMG_US,
	IMG_NO_WIFI,
	IMG_WIFI1,
	IMG_WIFI2,
	IMG_WIFI3,
	IMG_WIFI4,
	END_OF_IMAGES

} images_t;

const char* images[END_OF_IMAGES] = {
	"/bulb.jpg",
	"/bulb_small.jpg",
	"/chart.jpg",
	"/check.jpg",
	"/cooler.jpg",
	"/cooler_small.jpg",
	"/door.jpg",
	"/door_small.jpg",
	"/grass.jpg",
	"/hum.jpg",
	"/internet_no.jpg",
	"/internet_ok.jpg",
	"/next.jpg",
	"/prev.jpg",
	"/pump.jpg",
	"/ru.jpg",
	"/seeds.jpg",
	"/tap.jpg",
	"/temp.jpg",
	"/time-cal.jpg",
	"/us.jpg",
	"/wifi_no.jpg",
	"/wifi_ok1.jpg",
	"/wifi_ok2.jpg",
	"/wifi_ok3.jpg",
	"/wifi_ok4.jpg"
};


#endif