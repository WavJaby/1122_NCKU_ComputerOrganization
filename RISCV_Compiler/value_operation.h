#include "compiler_common.h"

#define toInt(val) (*(int*)&val)

bool valueOperation(char op, Object* a, Object* b, Object* out) {
    out->symbol = NULL;
    switch (a->type) {
    case OBJECT_TYPE_INT:
        switch (b->type) {
        case OBJECT_TYPE_INT:
            out->type = OBJECT_TYPE_INT;
            out->value = toInt(a->value) + toInt(b->value);
            return false;
        }
        break;
    }
    return false;
}