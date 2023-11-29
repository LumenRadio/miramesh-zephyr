#ifndef SWI_CALLBACK_HANDLER_H
#define SWI_CALLBACK_HANDLER_H

typedef void (*swi_callback_t)(void);

void swi_callback_handler_init(void);

int register_swi_callback(swi_callback_t);

void invoke_swi_callback(swi_callback_t callback);

#endif /* SWI_CALLBACK_HANDLER_H */
