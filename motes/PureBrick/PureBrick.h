
#ifndef _PUREBRICK_H_
#define _PUREBRICK_H_


int32_t
relay_switch_get_all(uint32_t *state, uint32_t *mask);
void
relay_switch_set(uint8_t sw, uint8_t value);

#endif /* _PUREBRICK_H_ */
