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

#include "pxArchive.h"

#include "test_includes.h" // Needs to be included last

#define protected public
#define private public

class pxArchiveTest : public testing::Test
{
	public:
		virtual void SetUp()
		{
		}

		virtual void TearDown()
		{
		}

		void pxArchiveinitFromUrlTestHttp()
		{
			urlStr = "http://test.jpeg";
			pxArchivePtr = new pxArchive();
			EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
			EXPECT_EQ(RT_OK, pxArchivePtr->loadStatus(obj));
			(obj).get("sourceType", val);
			EXPECT_TRUE(strcmp(val.cString(), "http") == 0);
			(obj).get("statusCode", sCode);
			EXPECT_TRUE(-1 == sCode);
		}
		
		void pxArchiveinitFromUrlTestValidHttp()
                {
                        urlStr = "http://apng.onevcat.com/assets/elephant.png";
                        pxArchivePtr = new pxArchive();
                        EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
			EXPECT_EQ(RT_OK, pxArchivePtr->loadStatus(obj));
                        (obj).get("sourceType", val);
                        EXPECT_TRUE(strcmp(val.cString(), "http") == 0);
                        (obj).get("statusCode", sCode);
                        EXPECT_TRUE(-1 == sCode);
                }


		void pxArchiveinitFromUrlTestLoadFail()
		{
			urlStr = "test.zip";
			EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
			EXPECT_EQ(RT_OK, pxArchivePtr->loadStatus(obj));
			(obj).get("sourceType", val);
			EXPECT_TRUE(strcmp(val.cString(), "file") == 0);
			(obj).get("statusCode", sCode);
			EXPECT_TRUE(1 == sCode);
		}
		void pxArchiveinitFromUrlTestLoadSuccess()
		{
			urlStr = "supportfiles/file.txt";
			EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
			EXPECT_EQ(RT_OK, pxArchivePtr->loadStatus(obj));
			(obj).get("statusCode", sCode);
			EXPECT_TRUE(0 == sCode);
		}
		void pxArchiveinitFromUrlTestLoadZip()
		{
			urlStr = "supportfiles/empty.zip";
			EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
			EXPECT_EQ(RT_OK, pxArchivePtr->loadStatus(obj));
			(obj).get("statusCode", sCode);
			EXPECT_TRUE(0 == sCode);

		}

                void pxArchiveinitFromUrlFromZipTest()
                {
	  	        rtObjectRef pxArchiveParent;
                        pxArchiveParent = new pxArchive();
                        urlStr = "supportfiles/sample.zip";
	                EXPECT_EQ(RT_OK, ((pxArchive*)pxArchiveParent.getPtr())->initFromUrl(urlStr));
	                pxArchive pxArchiveChild;
                        EXPECT_EQ(RT_OK, pxArchiveChild.initFromUrl("test.html", NULL, pxArchiveParent));
                        rtData d;
                        EXPECT_EQ(RT_OK, pxArchiveChild.getFileData("", d));
                        EXPECT_TRUE(d.length() > 0);
                }

		void pxArchiveready()
		{
			pxArchive* pxArchivePtr = new pxArchive();
			EXPECT_EQ(RT_OK, pxArchivePtr->ready(obj));
		}
		void pxArchiveloadStatus()
		{
			pxArchive* pxArchivePtr = new pxArchive();
			EXPECT_EQ(RT_OK, pxArchivePtr->loadStatus(obj));
		}
		void pxArchivegetFileAsString()
		{
			pxArchive* pxArchivePtr = new pxArchive();

			urlStr = "supportfiles/file.txt";
			sStr = "";
			EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
			EXPECT_EQ(RT_OK, pxArchivePtr->loadStatus(obj));
			EXPECT_EQ(RT_OK, pxArchivePtr->getFileAsString(urlStr, sStr));

			urlStr = "supportfiles/sample.zip";
			EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
			EXPECT_EQ(RT_OK, pxArchivePtr->loadStatus(obj));
			(obj).get("statusCode", sCode);
			EXPECT_TRUE(0 == sCode);
			EXPECT_EQ(RT_FAIL, pxArchivePtr->getFileAsString(urlStr, sStr));

		}
		void pxArchivegetFileAsStringEmptyFile()
		{
			urlStr = "supportfiles/empty.zip";
			EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
			EXPECT_EQ(RT_OK, pxArchivePtr->loadStatus(obj));
			(obj).get("statusCode", sCode);
			EXPECT_TRUE(0 == sCode);

			EXPECT_EQ(RT_OK, pxArchivePtr->getFileAsString(urlStr, sStr));
		}
		void pxArchiveFileName()
		{
			pxArchivePtr = new pxArchive();
			urlStr = "supportfiles/sample.zip";
			EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
			EXPECT_EQ(RT_OK, pxArchivePtr->loadStatus(obj));
			EXPECT_EQ(RT_OK, pxArchivePtr->fileNames(obj));
			(obj).get("statusCode", sCode);
			EXPECT_TRUE(0 == sCode);

			pxArchivePtr = new pxArchive();
			urlStr = "supportfiles/file.txt";
			EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
			EXPECT_EQ(RT_OK, pxArchivePtr->loadStatus(obj));
			EXPECT_EQ(RT_OK, pxArchivePtr->fileNames(obj));
			(obj).get("statusCode", sCode);
			EXPECT_TRUE(0 == sCode);
		
		}

		void pxArchiveFileNameValidation1()
		{	
			urlStr = "https://github.com/pxscene/pxscene/blob/gh-pages/examples/px-reference/gallery/tests/gallery.zip";
			EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
			EXPECT_EQ(RT_OK, pxArchivePtr->loadStatus(obj));
			(obj).set("statusCode", 0);
			pxArchivePtr->fileNames(obj);
			EXPECT_TRUE(0 == sCode);
		}

		void pxArchiveFileNameValidation2()
		{
			urlStr = "http://file.zip";
			EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
			EXPECT_EQ(RT_OK, pxArchivePtr->loadStatus(obj));
			(obj).set("statusCode", 0);
			pxArchivePtr->fileNames(obj);
			EXPECT_TRUE(0 == sCode);
			
		}

                void pxArchivegetFileNameTest()
                {
	            pxArchivePtr = new pxArchive();
	            urlStr = "supportfiles/sample.zip";
	            EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
	            EXPECT_TRUE(strcmp("supportfiles/sample.zip", pxArchivePtr->getName().cString()) == 0);
                }

                void pxArchiveisFileTrueTest()
                {
	            pxArchivePtr = new pxArchive();
	            urlStr = "supportfiles/simple.js";
	            EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
	            EXPECT_TRUE(true == pxArchivePtr->isFile());
                }

                void pxArchiveisFileFalseTest()
                {
	            pxArchivePtr = new pxArchive();
	            urlStr = "supportfiles/sample.zip";
	            EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
	            EXPECT_TRUE(false == pxArchivePtr->isFile());
                }

                void pxArchivegetFileDataNonZipTest()
                {
                    pxArchivePtr = new pxArchive();
                    urlStr = "supportfiles/simple.js";
                    EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
                    rtData d;
                    EXPECT_EQ(RT_OK, pxArchivePtr->getFileData("", d));
                    EXPECT_TRUE(d.length() > 0);
                }

                void pxArchivegetFileDataZipTest()
                {
                    pxArchivePtr = new pxArchive();
                    urlStr = "supportfiles/sample.zip";
                    EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
                    rtData d;
                    EXPECT_EQ(RT_OK, pxArchivePtr->getFileData("test.html", d));
                    EXPECT_TRUE(d.length() > 0);
                }

                void pxArchivegetFileDataUnavailableTest()
                {
                    pxArchivePtr = new pxArchive();
                    urlStr = "supportfiles/notthere.zip";
	            EXPECT_EQ(RT_OK, pxArchivePtr->initFromUrl(urlStr));
                    rtData d;
                    EXPECT_EQ(RT_ERROR, pxArchivePtr->getFileData("test.html",d));
                    EXPECT_TRUE(d.length() == 0);
                }

	private:
		rtObjectRef obj;
		int32_t sCode = 0;
		rtString val = "";
		rtString urlStr = "";
		rtString sStr = "";
		pxArchive* pxArchivePtr = NULL;
};

TEST_F(pxArchiveTest, pxArchiveCompleteTest)
{
	pxArchiveinitFromUrlTestHttp();
	pxArchiveinitFromUrlTestValidHttp();
	pxArchiveinitFromUrlTestLoadFail();
	pxArchiveinitFromUrlTestLoadSuccess();
	pxArchiveinitFromUrlTestLoadZip();	
	pxArchiveinitFromUrlFromZipTest();	
	pxArchiveready();
	pxArchiveloadStatus();
	pxArchivegetFileAsString();
	pxArchivegetFileAsStringEmptyFile();
	pxArchiveFileName();
	pxArchiveFileNameValidation1();
	pxArchiveFileNameValidation2();
    pxArchivegetFileNameTest();
	pxArchiveisFileTrueTest();
    pxArchiveisFileFalseTest();
    pxArchivegetFileDataNonZipTest();
    pxArchivegetFileDataZipTest();
    pxArchivegetFileDataUnavailableTest();
}

