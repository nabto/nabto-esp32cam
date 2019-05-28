#include <unabto/unabto_external_environment.h>
#include "esp_system.h"
#include <string.h>


#define MIN(a,b) (((a)<(b))?(a):(b))


/**
 * Note: esp_random can first be called after wifi has been initialized...
 * So don't use this implementation on on pure ethernet systems
 */
void nabto_random(uint8_t* buf, size_t len) {
  if(buf == NULL)
    return;

  uint32_t random;
  for(int i=0; i<len; i+=4) {
    random = esp_random();
    memcpy(buf+i, &random, MIN(4, len - i));
  }

}
