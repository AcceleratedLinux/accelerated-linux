// xmlsocket_test.cpp
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <cppunit/extensions/HelperMacros.h>
#include <scew/scew.h>

#include <opengear/xmldb.h>

class XmlTest: public CPPUNIT_NS::TestFixture {
	CPPUNIT_TEST_SUITE(XmlTest);
	CPPUNIT_TEST(testCreation);
	CPPUNIT_TEST_SUITE_END();
public:
	XmlTest() {}
	void setUp() {}
	void tearDown() {}

	void testCreation();
};

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(XmlTest);

void
XmlTest::testCreation()
{
	xmldb_t *db = NULL;
	db = xmldb_open(NULL);
	if (db != NULL) {
		xmldb_close(db);
	}
	CPPUNIT_ASSERT(db != NULL);
}
