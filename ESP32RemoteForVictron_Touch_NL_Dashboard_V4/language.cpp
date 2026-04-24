#include "language.h"

#include "languages/lang_en.h"
#include "languages/lang_nl.h"
#include "languages/lang_de.h"
#include "languages/lang_fr.h"
#include "languages/lang_es.h"
#include "languages/lang_it.h"
#include "languages/lang_pl.h"
#include "languages/lang_ar.h"
#include "languages/lang_tr.h"

static const LanguagePack *ALL_LANGUAGES[LANG_COUNT] = {
    &LANG_EN,
    &LANG_NL,
    &LANG_DE,
    &LANG_FR,
    &LANG_ES,
    &LANG_IT,
    &LANG_PL,
    &LANG_AR,
    &LANG_TR};

const LanguagePack &GetLanguagePack(AppLanguage lang)
{
  int index = (int)lang;
  if (index < 0 || index >= (int)LANG_COUNT)
    index = 0;
  return *ALL_LANGUAGES[index];
}

AppLanguage NormalizeLanguageSetting(int languageValue)
{
  if (languageValue < 0 || languageValue >= (int)LANG_COUNT)
    return LANG_ENGLISH;
  return (AppLanguage)languageValue;
}

AppLanguage NextLanguage(AppLanguage lang)
{
  int next = ((int)lang + 1) % (int)LANG_COUNT;
  return (AppLanguage)next;
}
