/*
* TemplateSourceWindow declaration
*/
#pragma once

#ifndef __TEMPLATE_SOURCE_H__
#define __TEMPLATE_SOURCE_H__

G_BEGIN_DECLS

#include <gtk/gtk.h>

#define TEMPLATE_SOURCE_WINDOW_TYPE (template_source_window_get_type ())
G_DECLARE_FINAL_TYPE (TemplateSourceWindow, template_source_window, TEMPLATE_SOURCE, WINDOW, GtkDialog)

TemplateSourceWindow * template_source_window_new (GtkWindow *parent);

G_END_DECLS

#endif /* __TEMPLATE_SOURCE_H__ */
