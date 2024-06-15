#pragma once
#ifndef __EP_POPOVER_H__
#define __EP_POPOVER_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE (EpPopover, ep_popover, EP, POPOVER, GtkPopover)

#define EP_TYPE_POPOVER           (ep_popover_get_type ())
#define EP_POPOVER(o)             (G_TYPE_CHECK_INSTANCE_CAST ((o), EP_TYPE_POPOVER, EpPopover))
#define EP_POPOVER_CLASS(c)       (G_TYPE_CHECK_CLASS_CAST ((c), EP_TYPE_POPOVER, EpPopoverClass))
#define EP_IS_POPOVER(o)          (G_TYPE_CHECK_INSTANCE_TYPE ((o), EP_TYPE_POPOVER))
#define EP_IS_POPOVER_CLASS(o)    (G_TYPE_CHECK_CLASS_TYPE ((o), EP_TYPE_POPOVER))
#define EP_POPOVER_GET_CLASS(o)   (G_TYPE_INSTANCE_GET_CLASS ((o), EP_TYPE_POPOVER, EpPopoverClass))

typedef enum _EpPopoverField 
{
    EP_FIELD_NONE,
    EP_FIELD_QUANTITY,
    EP_FIELD_PRODUCT,
    EP_FIELD_TITLE,
    EP_FIELD_UDC,
    EP_FIELD_CALENDAR
} EpPopoverField;

typedef enum _EpPopoverType 
{
    EP_TYPE_QUANTITY,
    EP_TYPE_PRODUCT,
    EP_TYPE_PRODUCT_TITLE_QUANTITY,
    EP_TYPE_PRODUCT_QUANTITY,
    EP_TYPE_UDC_PRODUCT_QUANTITY,
    EP_TYPE_CALENDAR,
    EP_TYPE_UDC
} EpPopoverType;

void ep_popover_set_type (EpPopover *ep, EpPopoverType type);
GtkWidget *ep_popover_new (GtkWidget *parent, EpPopoverType type, GtkButtonsType buttons);
gint ep_popover_run (EpPopover *ep);
GtkWidget *ep_popover_get_field(EpPopover *ep, EpPopoverField field);

G_END_DECLS

#endif /* __EP_POPOVER_H__ */

