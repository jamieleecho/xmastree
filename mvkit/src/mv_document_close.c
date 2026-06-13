#include <mvkit/mv_app.h>
#include "mv_document_internal.h"


bool mv_document_close(MVDocument *doc) {
    if (mv_document_is_dirty(doc)) {
        if (mv_app_show_message_box("Save changes before\r\nclosing?", MVMessageBoxType_YesNo) ==
             MVMessageBoxResult_Yes) {
            if (mv_document_save(doc)) {
                return false;   /* save failed/cancelled: keep the document open */
            }
        }
    }
    return true;
}
