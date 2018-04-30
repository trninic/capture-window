

#define COMMON_STRUCTS_CAPTURE_INFO 1
#define COMMON_STRUCTS_CAPTURE_PIXELSDATA 2


typedef struct {
	int width;
	int height;
	int cbsize;
	byte pixels[1];
} magCapturedData;
#define CAPTURED_DATA_SIZE(x) (sizeof(magCapturedData) + (sizeof(byte) * ((x) - 1)))


