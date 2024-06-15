#ifndef __RESOURCE__ep_H__
#define __RESOURCE__ep_H__

#include <gio/gio.h>

extern GResource *_ep_get_resource (void);

extern void _ep_register_resource (void);
extern void _ep_unregister_resource (void);

#endif
