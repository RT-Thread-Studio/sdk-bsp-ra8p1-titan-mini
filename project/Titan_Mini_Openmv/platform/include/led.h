#ifndef RA8_LED_H
#define RA8_LED_H
enum { LED_RED, LED_GREEN, LED_BLUE };
void led_state(int led, int state);
void led_init(void);
#endif
