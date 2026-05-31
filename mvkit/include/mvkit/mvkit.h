#ifndef _MVKIT_H
#define _MVKIT_H

/*
 * MVKit -- an opinionated framework for building Multi-Vue applications on the
 * Tandy Color Computer 3 under NitrOS-9, loosely inspired by Apple's AppKit.
 *
 * This is the umbrella header: include <mvkit/mvkit.h> to pull in the whole
 * public API. Modules are added here as they are migrated out of the xmastree
 * example app (see PLAN.md, Phase 2):
 *
 *   mv_undo_manager   (NSUndoManager)
 *   mv_image          (NSImage)
 *   mv_file_dialog    (NSOpenPanel / NSSavePanel)
 *   mv_document       (NSDocument)
 *   mv_app            (NSApplication)
 */

#include "mvkit/mv_defs.h"
#include "mvkit/mv_version.h"
#include "mvkit/mv_undo_manager.h"
#include "mvkit/mv_image.h"
#include "mvkit/mv_file_dialog.h"
#include "mvkit/mv_app.h"
#include "mvkit/mv_document.h"

#endif /* _MVKIT_H */
