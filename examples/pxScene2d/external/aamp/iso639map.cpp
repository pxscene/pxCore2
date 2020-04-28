/*
* If not stated otherwise in this file or this component's license file the
* following copyright and licenses apply:
*
* Copyright 2018 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "iso639map.h"
#include <cstdlib>
#include <string.h>

#define ISO639_BTMAP_ENTRY_SIZE (3+3+1)
#define ISO639_BTMAP_ENTRY_COUNT 20

#define ISO639_23MAP_ENTRY_SIZE (2+3+1)
#define ISO639_23MAP_ENTRY_COUNT (204-ISO639_BTMAP_ENTRY_COUNT)

/**
 * sorted list of packed iso639-1, iso639-2 pairs
 * delimited by a nul terminator, allowing pointer to the iso632-2 entry as valid c string
 * entries sorted by the iso639-1 language code to facilitate binary search
 */
static const char mISO639_2_3_pairs[ISO639_23MAP_ENTRY_COUNT][ISO639_23MAP_ENTRY_SIZE] =
{
    "aa" "aar", // Afar
    "ab" "abk", // Abkhazian
    "ae" "ave", // Avestan
    "af" "afr", // Afrikaans
    "ak" "aka", // Akan
    "am" "amh", // Amharic
    "an" "arg", // Aragonese
    "ar" "ara", // Arabic
    "as" "asm", // Assamese
    "av" "ava", // Avaric
    "ay" "aym", // Aymara
    "az" "aze", // Azerbaijani
    "ba" "bak", // Bashkir
    "be" "bel", // Belarusian
    "bg" "bul", // Bulgarian
    "bh" "bih", // Bihari languages
    "bi" "bis", // Bislama
    "bm" "bam", // Bambara
    "bn" "ben", // Bengali
    "bo" "bod", // (T) Tibetan
    "br" "bre", // Breton
    "bs" "bos", // Bosnian
    "ca" "cat", // Catalan
    "ce" "che", // Chechen
    "ch" "cha", // Chamorro
    "co" "cos", // Corsican
    "cr" "cre", // Cree
    "cs" "ces", // (T) Czech
    "cu" "chu", // Church Slavic
    "cv" "chv", // Chuvash
    "cy" "cym", // (T) Welsh
    "da" "dan", // Danish
    "de" "deu", // (T) German
    "dv" "div", // Divehi
    "dz" "dzo", // Dzongkha
    "ee" "ewe", // Ewe
    "el" "ell", // (T) Greek, Modern
    "en" "eng", // English
    "eo" "epo", // Esperanto
    "es" "spa", // Spanish
    "et" "est", // Estonian
    "eu" "eus", // (T) Basque
    "fa" "fas", // (T) Persian
    "ff" "ful", // Fulah
    "fi" "fin", // Finnish
    "fj" "fij", // Fijian
    "fo" "fao", // Faroese
    "fr" "fra", // (T) French
    "fy" "fry", // Western Frisian
    "ga" "gle", // Irish
    "gd" "gla", // Gaelic
    "gl" "glg", // Galician
    "gn" "grn", // Guarani
    "gu" "guj", // Gujarati
    "gv" "glv", // Manx
    "ha" "hau", // Hausa
    "he" "heb", // Hebrew
    "hi" "hin", // Hindi
    "ho" "hmo", // Hiri Motu
    "hr" "hrv", // Croatian
    "ht" "hat", // Haitian
    "hu" "hun", // Hungarian
    "hy" "hye", // (T) Armenian
    "hz" "her", // Herero
    "ia" "ina", // Interlingua
    "id" "ind", // Indonesian
    "ie" "ile", // Interlingue
    "ig" "ibo", // Igbo
    "ii" "iii", // Sichuan Yi
    "ik" "ipk", // Inupiaq
    "io" "ido", // Ido
    "is" "isl", // (T) Icelandic
    "it" "ita", // Italian
    "iu" "iku", // Inuktitut
    "ja" "jpn", // Japanese
    "jv" "jav", // Javanese
    "ka" "kat", // (T) Georgian
    "kg" "kon", // Kongo
    "ki" "kik", // Kikuyu
    "kj" "kua", // Kuanyama
    "kk" "kaz", // Kazakh
    "kl" "kal", // Kalaallisut
    "km" "khm", // Central Khmer
    "kn" "kan", // Kannada
    "ko" "kor", // Korean
    "kr" "kau", // Kanuri
    "ks" "kas", // Kashmiri
    "ku" "kur", // Kurdish
    "kv" "kom", // Komi
    "kw" "cor", // Cornish
    "ky" "kir", // Kirghiz
    "la" "lat", // Latin
    "lb" "ltz", // Luxembourgish
    "lg" "lug", // Ganda
    "li" "lim", // Limburgan
    "ln" "lin", // Lingala
    "lo" "lao", // Lao
    "lt" "lit", // Lithuanian
    "lu" "lub", // Luba-Katanga
    "lv" "lav", // Latvian
    "mg" "mlg", // Malagasy
    "mh" "mah", // Marshallese
    "mi" "mri", // (T) Maori
    "mk" "mkd", // (T) Macedonian
    "ml" "mal", // Malayalam
    "mn" "mon", // Mongolian
    "mr" "mar", // Marathi
    "ms" "msa", // (T) Malay
    "mt" "mlt", // Maltese
    "my" "mya", // (T) Burmese
    "na" "nau", // Nauru
    "nb" "nob", // Bokmål, Norwegian
    "nd" "nde", // Ndebele, North
    "ne" "nep", // Nepali
    "ng" "ndo", // Ndonga
    "nl" "nld", // (T) Dutch
    "nn" "nno", // Norwegian Nynorsk
    "no" "nor", // Norwegian
    "nr" "nbl", // Ndebele, South
    "nv" "nav", // Navajo
    "ny" "nya", // Chichewa
    "oc" "oci", // Occitan
    "oj" "oji", // Ojibwa
    "om" "orm", // Oromo
    "or" "ori", // Oriya
    "os" "oss", // Ossetian
    "pa" "pan", // Panjabi
    "pi" "pli", // Pali
    "pl" "pol", // Polish
    "ps" "pus", // Pushto
    "pt" "por", // Portuguese
    "qu" "que", // Quechua
    "rm" "roh", // Romansh
    "rn" "run", // Rundi
    "ro" "ron", // (T) Romanian
    "ru" "rus", // Russian
    "rw" "kin", // Kinyarwanda
    "sa" "san", // Sanskrit
    "sc" "srd", // Sardinian
    "sd" "snd", // Sindhi
    "se" "sme", // Northern Sami
    "sg" "sag", // Sango
    "si" "sin", // Sinhala
    "sk" "slk", // (T) Slovak
    "sl" "slv", // Slovenian
    "sm" "smo", // Samoan
    "sn" "sna", // Shona
    "so" "som", // Somali
    "sq" "sqi", // (T) Albanian
    "sr" "srp", // Serbian
    "ss" "ssw", // Swati
    "st" "sot", // Sotho, Southern
    "su" "sun", // Sundanese
    "sv" "swe", // Swedish
    "sw" "swa", // Swahili
    "ta" "tam", // Tamil
    "te" "tel", // Telugu
    "tg" "tgk", // Tajik
    "th" "tha", // Thai
    "ti" "tir", // Tigrinya
    "tk" "tuk", // Turkmen
    "tl" "tgl", // Tagalog
    "tn" "tsn", // Tswana
    "to" "ton", // Tonga
    "tr" "tur", // Turkish
    "ts" "tso", // Tsonga
    "tt" "tat", // Tatar
    "tw" "twi", // Twi
    "ty" "tah", // Tahitian
    "ug" "uig", // Uighur
    "uk" "ukr", // Ukrainian
    "ur" "urd", // Urdu
    "uz" "uzb", // Uzbek
    "ve" "ven", // Venda
    "vi" "vie", // Vietnamese
    "vo" "vol", // Volapük
    "wa" "wln", // Walloon
    "wo" "wol", // Wolof
    "xh" "xho", // Xhosa
    "yi" "yid", // Yiddish
    "yo" "yor", // Yoruba
    "za" "zha", // Zhuang
    "zh" "zho", // (T) Chinese
    "zu" "zul", // Zulu
};

/**
 * sorted list of packed iso639-2 bibliographic, terminology language code pairs
 * delimited by a nul terminator, allowing pointer to the terminology entry as valid c string
 * entries sorted by the bibliographic language code to facilitate binary search
 *
 * Used to normalize use of the 'terminology' version of iso639-2 langauge codes.
 */
static const char mBibliographicTerminologyPairs[ISO639_BTMAP_ENTRY_COUNT][ISO639_BTMAP_ENTRY_SIZE] =
{ // bibliographic code, terminology code
    "alb" "sqi", // Albanian
    "arm" "hye", // Armenian
    "baq" "eus", // Basque
    "bur" "mya", // Burmese
    "chi" "zho", // Chinese
    "cze" "ces", // Czech
    "dut" "nld", // Dutch
    "fre" "fra", // French
    "geo" "kat", // Georgian
    "ger" "deu", // German
    "gre" "ell", // Greek, Modern
    "ice" "isl", // Icelandic
    "mac" "mkd", // Macedonian
    "mao" "mri", // Maori
    "may" "msa", // Malay
    "per" "fas", // Persian
    "rum" "ron", // Romanian
    "slo" "slk", // Slovak
    "tib" "bod", // Tibetan
    "wel" "cym", // Welsh
};

/**
 * @param pkey pointer to two-character ISO639-1 language key
 * @param pelem pointer to mISO639_2_3_pairs record
 *
 * @retval negative iff pkey is before pelem
 * @retval positive iff pkey is after pelem
 * @retval zero iff pkey matches pelem
 */
static int myCompare23(const void*pkey,const void*pelem)
{
    int rc = ((char *)pkey)[0] - ((char *)pelem)[0];
    if( !rc )
    {
        rc = ((char *)pkey)[1] - ((char *)pelem)[1];
    }
    return rc;
}

/**
 * @param pkey pointer to three-character ISO639-2 language key
 * @param pelem pointer to mBibliographicTerminologyPairs record
 *
 * @retval negative iff pkey is before pelem
 * @retval positive iff pkey is after pelem
 * @retval zero iff pkey matches pelem
 */
static int myCompareBT(const void * pkey,const void * pelem)
{
    int rc = ((char *)pkey)[0] - ((char *)pelem)[0];
    if( !rc )
    {
        rc = ((char *)pkey)[1] - ((char *)pelem)[1];
        if( !rc )
        {
            rc = ((char *)pkey)[2] - ((char *)pelem)[2];
        }
    }
    return rc;
}

/**
 * @brief map "bibliographic" language code to corresponding "terminology" language code
 *
 * @param language3 three-character ISO639-2 language key
 
 * @retval corresponding terminology code if language3 is a bibliographic code
 * @retval echo language3 if not mapped
 */
static const char *MapISO639_BibliographicToTerminology( const char *language3 )
{
    const char *rc = (const char *)bsearch(
                                           (const void *)language3, // key
                                           (const void *)mBibliographicTerminologyPairs, // base
                                           ISO639_BTMAP_ENTRY_COUNT,
                                           ISO639_BTMAP_ENTRY_SIZE,
                                           myCompareBT);
    if( !rc )
    {
        rc = language3;
    }
    else
    {
        rc+=3; // skip past bibliographic key to terminology code
    }
    return rc;
}

static void MapISO639_TerminologyToBibliographic( char *language3 )
{
    for( int i=0; i<ISO639_BTMAP_ENTRY_COUNT; i++ )
    { // brute force search
        const char *entry = mBibliographicTerminologyPairs[i];
        if( memcmp(language3,&entry[3],3)==0 )
        {
            memcpy( language3, entry, 3 );
            break;
        }
    }
}

/**
 * @brief convert 2-character ISO639-1 code, map to 3-character ISO639-2 code
 * @param lang string to convert (if not already 3 character Terminology code
 */
static void ConvertLanguage2to3( char lang[], bool useTerminologyVariant )
{
    if( lang[0] && lang[1] )
    { // at least two characters
        if( lang[2] )
        { // at least three characters
            if( lang[3]==0x00 )
            { // exactly three characters
                if( useTerminologyVariant )
                {
                    const char *normalized = MapISO639_BibliographicToTerminology(lang);
                    strcpy( lang, normalized );
                }
                else
                {
                    MapISO639_TerminologyToBibliographic( lang );
                }
                return;
            }
        }
        else
        { // exactly two characters
            const char *rc = (const char *)bsearch(
                                                   (const void *)lang, // key
                                                   (const void *)mISO639_2_3_pairs, // base
                                                   ISO639_23MAP_ENTRY_COUNT,
                                                   ISO639_23MAP_ENTRY_SIZE,
                                                   myCompare23);
            if( rc )
            {
                rc+=2; // skip past ISO639-1 key to three-character ISO639-2 language code
                strcpy( lang, rc );
                if( !useTerminologyVariant )
                {
                    MapISO639_TerminologyToBibliographic( lang );
                }
                return;
            }
        }
    }
    strcpy( lang, "und" ); // default - error
}
/**
* @brief convert 3-character ISO639-2 code, map to 2-character ISO639-1 code
* @param lang string to convert (if not already 2 character  code
*/
static void ConvertLanguage3to2( char lang[] )
{
    size_t len = strlen(lang);
    if( len==3 )
    {
        const char *key = MapISO639_BibliographicToTerminology(lang);
        for( int i=0; i<ISO639_23MAP_ENTRY_COUNT; i++ )
        { // brute force search matching 3 character code to find corresponding two-character code
            const char *entry = mISO639_2_3_pairs[i]; // 2 character code followed by 3 character code
            if( memcmp(key,&entry[2],3)==0 )
            {
                lang[0] = entry[0];
                lang[1] = entry[1];
                lang[2] = 0;
                return;
            }
        }
        strcpy( lang, "un" ); // default - error
    }
}

void iso639map_NormalizeLanguageCode( char lang[], LangCodePreference langCodePreference )
{
    switch( langCodePreference )
    {
        case ISO639_NO_LANGCODE_PREFERENCE:
            break;
        case ISO639_PREFER_3_CHAR_BIBLIOGRAPHIC_LANGCODE:
            ConvertLanguage2to3( lang, false );
            break;
        case ISO639_PREFER_3_CHAR_TERMINOLOGY_LANGCODE:
            ConvertLanguage2to3( lang, true );
            break;
        case ISO639_PREFER_2_CHAR_LANGCODE:
            ConvertLanguage3to2( lang );
            break;
    }
}

#ifdef INCLUDE_ISO639MAP_TESTS

static void TestHelper( const char *lang, LangCodePreference langCodePreference, const char *expectedResult )
{
    printf( "%s[", lang );
    switch( langCodePreference )
    {
    case ISO639_NO_LANGCODE_PREFERENCE:
        printf( "ISO639_NO_LANGCODE_PREFERENCE" );
        break;
    case ISO639_PREFER_3_CHAR_BIBLIOGRAPHIC_LANGCODE:
        printf( "ISO639_PREFER_3_CHAR_BIBLIOGRAPHIC_LANGCODE" );
        break;
    case ISO639_PREFER_3_CHAR_TERMINOLOGY_LANGCODE:
        printf( "ISO639_PREFER_3_CHAR_TERMINOLOGY_LANGCODE" );
        break;
    case ISO639_PREFER_2_CHAR_LANGCODE:
        printf( "ISO639_PREFER_2_CHAR_LANGCODE" );
        break;
    }
    char temp[256];
    strcpy( temp, lang );
    iso639map_NormalizeLanguageCode( temp, langCodePreference );
    printf( "] -> %s : ", temp );
    if( strcmp(temp,expectedResult)!=0 )
    {
        printf( "FAIL! expected %s\n", expectedResult );
    }
    else
    {
        printf( "PASS\n" );
    }
}

void iso639map_Test( void )
{
    TestHelper( "en", ISO639_NO_LANGCODE_PREFERENCE, "en" );
    TestHelper( "en", ISO639_PREFER_3_CHAR_BIBLIOGRAPHIC_LANGCODE, "eng" );
    TestHelper( "en", ISO639_PREFER_3_CHAR_TERMINOLOGY_LANGCODE, "eng" );
    TestHelper( "en", ISO639_PREFER_2_CHAR_LANGCODE, "en" );

    TestHelper( "eng", ISO639_NO_LANGCODE_PREFERENCE, "eng" );
    TestHelper( "eng", ISO639_PREFER_3_CHAR_BIBLIOGRAPHIC_LANGCODE, "eng" );
    TestHelper( "eng", ISO639_PREFER_3_CHAR_TERMINOLOGY_LANGCODE, "eng" );
    TestHelper( "eng", ISO639_PREFER_2_CHAR_LANGCODE, "en" );

    TestHelper( "de", ISO639_NO_LANGCODE_PREFERENCE, "de" );
    TestHelper( "de", ISO639_PREFER_3_CHAR_BIBLIOGRAPHIC_LANGCODE, "ger" );
    TestHelper( "de", ISO639_PREFER_3_CHAR_TERMINOLOGY_LANGCODE, "deu" );
    TestHelper( "de", ISO639_PREFER_2_CHAR_LANGCODE, "de" );
    
    TestHelper( "deu", ISO639_NO_LANGCODE_PREFERENCE, "deu" );
    TestHelper( "deu", ISO639_PREFER_3_CHAR_BIBLIOGRAPHIC_LANGCODE, "ger" );
    TestHelper( "deu", ISO639_PREFER_3_CHAR_TERMINOLOGY_LANGCODE, "deu" );
    TestHelper( "deu", ISO639_PREFER_2_CHAR_LANGCODE, "de" );
    
    TestHelper( "ger", ISO639_NO_LANGCODE_PREFERENCE, "ger" );
    TestHelper( "ger", ISO639_PREFER_3_CHAR_BIBLIOGRAPHIC_LANGCODE, "ger" );
    TestHelper( "ger", ISO639_PREFER_3_CHAR_TERMINOLOGY_LANGCODE, "deu" );
    TestHelper( "ger", ISO639_PREFER_2_CHAR_LANGCODE, "de" );
}

#endif // INCLUDE_ISO639MAP_TESTS
