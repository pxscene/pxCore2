/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include <sstream>
#include <string.h>
#include <unistd.h>
extern "C"
{
#include <utf8.h>
}

#include "test_includes.h" // Needs to be included last

#define STR_SIZE 32

using namespace std;

class UTF8Test : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void lengthTest()
    {
      char *str = (char*)malloc(sizeof(char)*8);
      strcpy(str,"\x46\x6F\x6F\x20\xC2");
      EXPECT_TRUE (u8_strlen(str) == 5);
    }

    void printTest()
    {
      char *str = (char*)malloc(sizeof(char)*4);
      strcpy(str,"\x46\x6F\x6E");
      EXPECT_TRUE (u8_printf((char*)"%s",str) == 3);
    }

    void charToUTF8Test()
    {
      char dest;
      u8_wc_toutf8(&dest,65);
      EXPECT_TRUE (dest == 'A');
    } 

    void charToByteOffsetTest()
    {
      char *str = (char*)malloc(sizeof(char)*4);
      strcpy(str,"\x46\x6F\x6E");
      EXPECT_TRUE(1 == u8_offset(str,1));
    }

    void byteOffsetTocharNumTest()
    {
      char *str = (char*)malloc(sizeof(char)*4);
      strcpy(str,"\x46\x6F\x6E");
      EXPECT_TRUE(2 == u8_charnum(str,2));
    }

    void isLocaleUTF8TrueTest()
    {
      EXPECT_TRUE (1 == u8_is_locale_utf8((char*)".UTF-8"));
    }

    void isLocaleUTF8FalseTest()
    {
      EXPECT_TRUE (0 == u8_is_locale_utf8((char*)"UTF-8"));
    }

    void strcharPresentTest()
    {
      char *str = (char*)malloc(sizeof(char)*4);
      strcpy(str,"\x46\x6F\x6E");
      int index = -1;
      char* ptr = NULL;
      ptr = u8_strchr(str,70, &index);
      EXPECT_TRUE (NULL != ptr);
      EXPECT_TRUE (0 == index);
    }

    void strcharAbsentTest()
    {
      char *str = (char*)malloc(sizeof(char)*4);
      strcpy(str,"\x46\x6F\x6E");
      int index = -1;
      char* ptr = NULL;
      ptr = u8_strchr(str,65, &index);
      EXPECT_TRUE (NULL == ptr);
      EXPECT_TRUE (3 == index);
    } 

    void memcharPresentTest()
    {
      char *str = (char*)malloc(sizeof(char)*4);
      strcpy(str,"\x46\x6F\x6E");
      int index = -1;
      char* ptr = NULL;
      ptr = u8_memchr(str,70, 2, &index);
      EXPECT_TRUE (NULL != ptr);
      EXPECT_TRUE (0 == index);
    }

    void memcharAbsentTest()
    {
      char *str = (char*)malloc(sizeof(char)*4);
      strcpy(str,"\x46\x6F\x6E");
      int index = -1;
      char* ptr = NULL;
      ptr = u8_memchr(str,70, 0,&index);
      EXPECT_TRUE (NULL == ptr);
      EXPECT_TRUE (0 == index);
    }

    void octalDigitTest()
    {
      EXPECT_TRUE (octal_digit('7') == 1);
    }

    void hexDigitTest()
    {
      EXPECT_TRUE (hex_digit('f') == 1);
    }

    void convertUTFToAsciiTest()
    {
      char *str = (char*)malloc(sizeof(char)*32);
      strcpy(str,"\x46\x6F\x6E");
      char buffer[10];
      memset (buffer, 0, sizeof(buffer));
      EXPECT_TRUE (3 == u8_escape(buffer, 32, str, 0));
      
      memset(str, 0x00, strlen(str) + 1);
      memset(buffer, 0x00, sizeof(buffer));
      strcpy(str,"\"u20\"");
      EXPECT_TRUE (7 == u8_escape(buffer, 32, str, 1));
}

    void convertAsciiToUTF8Test()
    {
      char str[STR_SIZE] = "Foo";
      char buffer[STR_SIZE];
      memset (buffer, 0, sizeof(buffer));
      EXPECT_TRUE (3 == u8_toutf8(buffer, 20, (u_int32_t*)str, 3));
    }

    void unescapeTest()
    {
      char str[STR_SIZE] = "\\u:";
      char buffer[STR_SIZE];
      memset (buffer, 0, sizeof(buffer));
      int ret = u8_unescape(buffer,sizeof(str),str);
      EXPECT_TRUE (2 == ret);
    }

    void u8_incTest()
    {
        char str[STR_SIZE] = "spark";
	int charPos = 0;
	u8_inc(str, &charPos); 
    }
    void u8_decTest()
    {
        char str[STR_SIZE] = "spark";
	int charPos = 0;
	u8_dec(str, &charPos);
    }
    void seqLengthTest()
    {
      char str[STR_SIZE] = "Foo";
      EXPECT_TRUE (1 == u8_seqlen(str));
    }

    void u8EscapeWcharTest()
    {
      char buffer[STR_SIZE];
      memset(buffer,0,STR_SIZE); 
      EXPECT_TRUE (2 == u8_escape_wchar(buffer, 100, L'\n'));
      EXPECT_TRUE (2 == u8_escape_wchar(buffer, 100, L'\t'));
      EXPECT_TRUE (2 == u8_escape_wchar(buffer, 100, L'\r'));
      EXPECT_TRUE (2 == u8_escape_wchar(buffer, 100, L'\b'));
      EXPECT_TRUE (2 == u8_escape_wchar(buffer, 100, L'\f'));
      EXPECT_TRUE (2 == u8_escape_wchar(buffer, 100, L'\f'));
      EXPECT_TRUE (2 == u8_escape_wchar(buffer, 100, L'\v'));
      EXPECT_TRUE (2 == u8_escape_wchar(buffer, 100, L'\a'));
      EXPECT_TRUE (2 == u8_escape_wchar(buffer, 100, L'\\'));
      EXPECT_TRUE (4 == u8_escape_wchar(buffer, 100, 0x7f));
      EXPECT_TRUE (6 == u8_escape_wchar(buffer, 100, 0x80));
}

    void toUTF8Test()
    {
      char dest[STR_SIZE];
      u_int32_t src1 = 35;
      memset(dest,0,sizeof(dest));
      EXPECT_TRUE (1 == u8_toutf8(dest, sizeof(dest), &src1,1));
      EXPECT_TRUE (0 == u8_toutf8(dest, 0, &src1,1));
      src1 = 131;
      EXPECT_TRUE (1 == u8_toutf8(dest, sizeof(dest), &src1,1));
      EXPECT_TRUE (0 == u8_toutf8(dest, 0, &src1,1));
      src1 = 65535;
      EXPECT_TRUE (1 == u8_toutf8(dest, sizeof(dest), &src1,1));
      EXPECT_TRUE (0 == u8_toutf8(dest, 0, &src1,1));
      src1 = 65537;
      EXPECT_TRUE (1 == u8_toutf8(dest, sizeof(dest), &src1,1));
      EXPECT_TRUE (0 == u8_toutf8(dest, 0, &src1,1));
    }

    void wctoUTF8Test()
    {
      char dest[STR_SIZE];
      u_int32_t src1 = 131;
      memset(dest,0,sizeof(dest));
      EXPECT_TRUE (2 == u8_wc_toutf8(dest, src1));
      src1 = 65530;
      EXPECT_TRUE (3 == u8_wc_toutf8(dest, src1));
      src1 = 65537;
      EXPECT_TRUE (4 == u8_wc_toutf8(dest, src1));
      src1 = 1120000;
      EXPECT_TRUE (0 == u8_wc_toutf8(dest, src1));
    }

    void readEscapeSequenceTest()
    {
      u_int32_t dest;
      char str[1], strA[STR_SIZE];
      str[0] = 'n';
      u8_read_escape_sequence(str, &dest);
      EXPECT_TRUE (10 == dest);
      str[0] = 't';
      u8_read_escape_sequence(str, &dest);
      EXPECT_TRUE ((uint32_t)('\t') == dest);
      str[0] = 'r';
      u8_read_escape_sequence(str, &dest);
      EXPECT_TRUE ((uint32_t)('\r') == dest);
      str[0] = 'b';
      u8_read_escape_sequence(str, &dest);
      EXPECT_TRUE ((uint32_t)('\b') == dest);
      str[0] = 'f';
      u8_read_escape_sequence(str, &dest);
      EXPECT_TRUE ((uint32_t)('\f') == dest);
      str[0] = 'v';
      u8_read_escape_sequence(str, &dest);
      EXPECT_TRUE ((uint32_t)('\v') == dest);
      str[0] = 'a';
      u8_read_escape_sequence(str, &dest);
      EXPECT_TRUE ((uint32_t)('\a') == dest);
      str[0] = '3';
      u8_read_escape_sequence(str, &dest);
      EXPECT_TRUE ((uint32_t)(3) == 3);
      dest = 0;  
      strcpy(strA, "x0F");
      u8_read_escape_sequence(strA, &dest);
      EXPECT_TRUE (15 == dest);

      memset(strA, 0x00, sizeof(strA)); 
      dest = 0;      
      strcpy(strA,"uAA");
      u8_read_escape_sequence(strA, &dest);
      EXPECT_TRUE (170 == dest);

      memset(strA, 0x00, sizeof(strA)); 
      dest = 0;      
      strcpy(strA,"U2026");
      u8_read_escape_sequence(strA, &dest);
      EXPECT_TRUE (8230 == dest);
    }

    void u8_toucsTest()
    {
	u_int32_t dest[STR_SIZE] = {0};
	char param[8] = "\u2026";
	int retVal = u8_toucs(dest, STR_SIZE, param, sizeof(param));
	EXPECT_TRUE (dest[0] == 8230); 
	EXPECT_TRUE (retVal == 6); 
    }
};

TEST_F(UTF8Test, UTF8Tests)
{
  lengthTest();
  printTest();
  charToUTF8Test();
  charToByteOffsetTest();
  byteOffsetTocharNumTest();
  isLocaleUTF8TrueTest();
  isLocaleUTF8FalseTest();
  strcharPresentTest();
  strcharAbsentTest();
  memcharPresentTest();
  memcharAbsentTest();
  octalDigitTest();
  hexDigitTest();
  convertUTFToAsciiTest();
  convertAsciiToUTF8Test();
  unescapeTest();
  seqLengthTest();
  u8EscapeWcharTest();
  toUTF8Test();
  wctoUTF8Test();
  readEscapeSequenceTest();
  u8_incTest();
  u8_decTest();
  u8_toucsTest();
}
