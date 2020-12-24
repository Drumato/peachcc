#include "peachcc.h"
TranslationUnit *new_translation_unit(void)
{
    TranslationUnit *translation_unit = (TranslationUnit *)calloc(1, sizeof(TranslationUnit));
    translation_unit->functions = new_vec();
    translation_unit->global_variables = new_map();
    return translation_unit;
}