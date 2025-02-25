// Copyright (c) 2021-2022, SIL International.
// Copyright (c) 2025, Seanghay Yath
// Licensed under MIT license: https://opensource.org/licenses/MIT
// Inspired from
// https://github.com/sillsdev/khmer-character-specification/blob/master/python/scripts/khnormal
#include "khnormal.hpp"
#include "re2/re2.h"
#include "utf8.h"
#include <algorithm>
#include <string>
#include <string_view>
#include <vector>

#define CAT_OTHER 0u
#define CAT_BASE 1u
#define CAT_COENG 3u
#define CAT_ZFCOENG 4u

namespace khnormal {

typedef struct {
  uint8_t type;
  utf8::utfchar32_t code;
  std::string text;
} GraphemeItem;

static const uint8_t CATS[] = {
    1, 1,  1,  1,  1,  1,  1, 1,  1, 1, 1, 1, 1, 1,  1,  1,  1,  1, 1,
    1, 1,  1,  1,  1,  1,  1, 1,  1, 1, 1, 1, 1, 1,  1,  1,  0,  0, 1,
    1, 1,  1,  1,  1,  1,  1, 1,  1, 1, 1, 1, 1, 1,  0,  0,  10, 9, 9,
    9, 9,  8,  8,  8,  7,  7, 7,  7, 7, 7, 7, 7, 11, 12, 12, 5,  5, 11,
    2, 11, 11, 11, 11, 11, 3, 11, 0, 0, 0, 0, 0, 0,  0,  0,  0,  11};

static const re2::RE2 PATTERN_COENG_DA_TA("(\\x{17D2})\\x{178A}");
static const re2::RE2
    PATTERN_COMPOUND_VOWEL1("\\x{17C1}([\\x{17BB}-\\x{17BD}]?)\\x{17B8}");
static const re2::RE2
    PATTERN_COMPOUND_VOWEL2("\\x{17C1}([\\x{17BB}-\\x{17BD}]?)\\x{17B6}");
static const re2::RE2 PATTERN_COMPOUND_VOWEL3("(\\x{17BE})(\\x{17BB})");

static const re2::RE2
    PATTERN_INVISIBLE_CHARS("([\\x{200C}\\x{200D}]\\x{17D2}?|\\x{17D2}\\x{200D}"
                            ")[\\x{17D2}\\x{200C}\\x{200D}]+");

static const re2::RE2 PATTERN_COENG_RO_SECOND(
    "(\\x{17D2}\\x{179A})(\\x{17D2}[\\x{1780}-\\x{17B3}])");

static const re2::RE2
    PATTERN_KHMER("([\\x{1780}-\\x{17ff}]+)|([^\\x{1780}-\\x{17ff}]+)");

static const re2::RE2 PATTERN_S2(
    "((?:[\\x{1784}\\x{1780}\\x{178E}\\x{1793}\\x{1794}\\x{1798}-\\x{179D}"
    "\\x{17A1}\\x{17A3}-\\x{17B3}]\\x{17CC}?(?:\\x{17D2}[\\x{1784}\\x{1780}"
    "\\x{178E}\\x{1793}\\x{1794}\\x{1798}-\\x{179D}\\x{17A1}\\x{17A3}-\\x{"
    "17B3}](?:\\x{17D2}[\\x{1784}\\x{1780}\\x{178E}\\x{1793}\\x{1794}\\x{"
    "1798}-\\x{179D}\\x{17A1}\\x{17A3}-\\x{17B3}])?)?|\\x{1794}\\x{17CC}?(?"
    ":(?:\\x{17D2}[\\x{1780}-\\x{1799}\\x{179B}-\\x{17A2}\\x{17A5}-\\x{"
    "17B3}])?\\x{17D2}[\\x{1780}-\\x{17A2}\\x{17A5}-\\x{17B3}\\x{25CC}])?|["
    "\\x{1780}-\\x{17A2}\\x{17A5}-\\x{17B3}\\x{25CC}]\\x{17CC}?(?:\\x{17D2}"
    "[\\x{1780}-\\x{1799}\\x{179B}-\\x{17A2}\\x{17A5}-\\x{17B3}]\\x{17D2}"
    "\\x{1794}|\\x{17D2}\\x{1794}(?:\\x{17D2}[\\x{1780}-\\x{17A2}\\x{17A5}-"
    "\\x{17B3}\\x{25CC}]))?)(?:\\x{17D2}\\x{200D}[\\x{1780}-\\x{1799}\\x{"
    "179B}-\\x{17A2}\\x{17A5}-\\x{17B3}])?[\\x{17C1}-\\x{17C5}]?)\\x{17BB}("
    "[\\x{17B7}-\\x{17BA}\\x{17BE}\\x{17BF}\\x{17DD}]|\\x{17B6}\\x{17C6}|"
    "\\x{17D0})");

static const re2::RE2 PATTERN_S1(
    "([\\x{1780}-\\x{1783}\\x{1785}-\\x{1788}\\x{178A}-\\x{178D}\\x{178F}-"
    "\\x{1792}\\x{1795}-\\x{1797}\\x{179E}-\\x{17A0}\\x{17A2}]\\x{17CC}?(?:"
    "\\x{17D2}[\\x{1780}-\\x{1793}\\x{1795}-\\x{17A2}\\x{17A5}-\\x{17B3}](?"
    ":\\x{17D2}[\\x{1780}-\\x{1793}\\x{1795}-\\x{17A2}\\x{17A5}-\\x{17B3}])"
    "?)?|[\\x{1780}-\\x{1793}\\x{1795}-\\x{17A2}\\x{17A5}-\\x{17B3}]\\x{"
    "17CC}?(?:\\x{17D2}[\\x{1780}-\\x{1783}\\x{1785}-\\x{1788}\\x{178A}-"
    "\\x{178D}\\x{178F}-\\x{1792}\\x{1795}-\\x{1797}\\x{179E}-\\x{17A0}\\x{"
    "17A2}](?:\\x{17D2}[\\x{1780}-\\x{1793}\\x{1795}-\\x{17A2}\\x{17A5}-"
    "\\x{17B3}])?|\\x{17D2}[\\x{1780}-\\x{1793}\\x{1795}-\\x{17A2}\\x{17A5}"
    "-\\x{17B3}]\\x{17D2}[\\x{1780}-\\x{1783}\\x{1785}-\\x{1788}\\x{178A}-"
    "\\x{178D}\\x{178F}-\\x{1792}\\x{1795}-\\x{1797}\\x{179E}-\\x{17A0}\\x{"
    "17A2}])(?:\\x{17D2}\\x{200D}[\\x{1780}-\\x{1799}\\x{179B}-\\x{17A2}"
    "\\x{17A5}-\\x{17B3}])?[\\x{17C1}-\\x{17C5}]?)\\x{17BB}([\\x{17B7}-\\x{"
    "17BA}\\x{17BE}\\x{17BF}\\x{17DD}]|\\x{17B6}\\x{17C6}|\\x{17D0})");

std::string khnormalize(std::string_view v) {
  std::vector<GraphemeItem> values;
  values.reserve(v.size());

  auto it = v.begin();
  auto end = v.end();
  int i = 0;
  while (it != end) {
    utf8::utfchar32_t c = utf8::next(it, end);
    uint8_t t = CAT_OTHER;

    if (0x1780u <= c && c <= 0x17ddu) {
      t = CATS[c - 0x1780u];
    }

    std::string text;
    utf8::append(c, std::back_inserter(text));

    if (i > 0 && values[i - 1].code == 0x17d2u &&
        (t == CAT_BASE || t == CAT_ZFCOENG)) {
      t = CAT_COENG;
    }

    values.push_back(GraphemeItem{.code = c, .type = t, .text = text});
    i++;
  }

  i = 0;

  std::string res;
  while (i < values.size()) {

    if (values[i].type != CAT_BASE) {
      res.append(values[i].text);
      i += 1;
      continue;
    }

    int j = i + 1;
    while (j < values.size() && values[j].type > CAT_BASE) {
      j += 1;
    }

    std::vector<uint32_t> newIndices;
    newIndices.reserve(j - i);

    for (uint32_t x = 0; x < j - i; x++) {
      newIndices.push_back(x + i);
    }

    std::sort(newIndices.begin(), newIndices.end(),
              [values](uint32_t a, uint32_t b) {
                return values[b].type > values[a].type || a < b;
              });

    std::string replaces;
    for (auto &idx : newIndices) {
      replaces.append(values[idx].text);
    }

    // remove multiple invisible chars
    re2::RE2::GlobalReplace(&replaces, PATTERN_INVISIBLE_CHARS, "\\1");
    // map compound vowel sequences to compounds with -u before to be converted
    re2::RE2::GlobalReplace(&replaces, PATTERN_COMPOUND_VOWEL1, "\u17BE\\1");
    re2::RE2::GlobalReplace(&replaces, PATTERN_COMPOUND_VOWEL2, "\u17C4\\1");
    re2::RE2::GlobalReplace(&replaces, PATTERN_COMPOUND_VOWEL3, "\\2\\1");
    re2::RE2::GlobalReplace(&replaces, PATTERN_S1, "\\1\u17CA\\2");
    re2::RE2::GlobalReplace(&replaces, PATTERN_S2, "\\1\u17C9\\2");
    // coeng ro second
    re2::RE2::GlobalReplace(&replaces, PATTERN_COENG_RO_SECOND, "\\2\\1");
    // coeng da->ta
    re2::RE2::GlobalReplace(&replaces, PATTERN_COENG_DA_TA, "\\1\u178F");
    res.append(replaces);

    i = j;
  }

  return res;
}

std::string normalize(std::string_view text) {
  std::string result = "";
  std::string m1;
  std::string m2;

  while (re2::RE2::FindAndConsume(&text, PATTERN_KHMER, &m1, &m2)) {
    if (!m1.empty()) {
      result.append(khnormalize(m1));
      continue;
    }

    result.append(m2);
  }

  return result;
}

} // namespace khnormal
