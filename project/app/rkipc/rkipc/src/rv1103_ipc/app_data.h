#ifndef __APP_DATA_H__
#define __APP_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
extern void stream_get_data(uint8_t *data, uint32_t len);
extern int video_init(uint32_t channel, uint32_t width, uint32_t height);
extern int video_start(uint8_t channel);
extern int video_stop(uint32_t channel);
#ifdef __cplusplus
}
#endif


#endif /* __APP_DATA_H__ */
