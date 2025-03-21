/* C++ code produced by gperf version 3.0.1 */
/* Command-line: gperf --compare-strncmp --enum --null-strings --readonly-tables --language C++ --output-file PAG1Token.inc PAG1Token.gperf  */
/* Computed positions: -k'1,6' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 10 "PAG1Token.gperf"

#if defined __GNUC__
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

using namespace PAG1Token;
#line 18 "PAG1Token.gperf"
struct Token
{
  const char *name;
  int id;
};
#include <string.h>
/* maximum key range = 51, duplicates = 0 */

class Perfect_Hash
{
private:
  static inline unsigned int hash (const char *str, unsigned int len);
public:
  static const struct Token *in_word_set (const char *str, unsigned int len);
};

inline unsigned int
Perfect_Hash::hash (register const char *str, register unsigned int len)
{
  static const unsigned char asso_values[] =
    {
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 25, 55,  0, 55, 10,
      55, 55, 55, 55, 55, 55, 55, 10, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
       5, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55,  5, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55,  5, 25, 15,
      20,  0, 20, 15,  5, 55, 55,  5, 10, 55,
       0, 15,  5, 55,  0,  0,  0, 55,  5, 10,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
      55, 55, 55, 55, 55, 55
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
      case 4:
      case 3:
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

const struct Token *
Perfect_Hash::in_word_set (register const char *str, register unsigned int len)
{
  enum
    {
      TOTAL_KEYWORDS = 31,
      MIN_WORD_LENGTH = 4,
      MAX_WORD_LENGTH = 40,
      MIN_HASH_VALUE = 4,
      MAX_HASH_VALUE = 54
    };

  static const struct Token wordlist[] =
    {
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 38 "PAG1Token.gperf"
      {"note",note},
#line 49 "PAG1Token.gperf"
      {"rpage",rpage},
#line 39 "PAG1Token.gperf"
      {"number",number},
      {(char*)0}, {(char*)0},
#line 41 "PAG1Token.gperf"
      {"page",page},
#line 52 "PAG1Token.gperf"
      {"stylesheet",stylesheet},
#line 33 "PAG1Token.gperf"
      {"header",header},
#line 51 "PAG1Token.gperf"
      {"slprint-info",slprint_info},
      {(char*)0},
#line 47 "PAG1Token.gperf"
      {"prototype",prototype},
#line 44 "PAG1Token.gperf"
      {"page-scale",page_scale},
#line 37 "PAG1Token.gperf"
      {"layout",layout},
      {(char*)0}, {(char*)0},
#line 27 "PAG1Token.gperf"
      {"cell",cell},
#line 40 "PAG1Token.gperf"
      {"order",order},
#line 43 "PAG1Token.gperf"
      {"page-height",page_height},
#line 53 "PAG1Token.gperf"
      {"textbox",textbox},
      {(char*)0},
#line 28 "PAG1Token.gperf"
      {"date",date},
#line 45 "PAG1Token.gperf"
      {"page-width",page_width},
#line 31 "PAG1Token.gperf"
      {"footer",footer},
#line 54 "PAG1Token.gperf"
      {"version",version},
#line 29 "PAG1Token.gperf"
      {"document",document},
#line 26 "PAG1Token.gperf"
      {"body",body},
#line 42 "PAG1Token.gperf"
      {"page-group",page_group},
#line 24 "PAG1Token.gperf"
      {"92008102400",VERSION_STR_4},
#line 25 "PAG1Token.gperf"
      {"SLCreationDateProperty",SLCreationDateProperty},
#line 50 "PAG1Token.gperf"
      {"section-prototypes",section_prototypes},
#line 35 "PAG1Token.gperf"
      {"kSFWPFootnoteGapProperty",kSFWPFootnoteGapProperty},
#line 36 "PAG1Token.gperf"
      {"kSFWPFootnoteKindProperty",kSFWPFootnoteKindProperty},
#line 48 "PAG1Token.gperf"
      {"publication-info",publication_info},
      {(char*)0}, {(char*)0}, {(char*)0},
#line 46 "PAG1Token.gperf"
      {"print-info",print_info},
      {(char*)0}, {(char*)0},
#line 32 "PAG1Token.gperf"
      {"footnote",footnote},
      {(char*)0},
#line 34 "PAG1Token.gperf"
      {"http://developer.apple.com/namespaces/sl",NS_URI_SL},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
      {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 30 "PAG1Token.gperf"
      {"drawables",drawables}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = hash (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (s && *str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
            return &wordlist[key];
        }
    }
  return 0;
}
#line 55 "PAG1Token.gperf"

