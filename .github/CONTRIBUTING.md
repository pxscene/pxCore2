# How to Contribute
Thank you for your interest in pxCore and pxscene! The following is a set of guidelines for contributing to pxCore and pxscene in GitHub. Can't find what you're looking for?  Find information that's missing?  Submit a pull request to add it!

## Testing
We use Travis CI for our continuous integration testing. 

### C++ Unit Tests
C++ Google tests can be added in the pxCore/tests/pxScene2d directory, then added to build scripts in pxCore/tests/pxScene2d.

### JavaScript Tests
We use [testRunner](https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/testRunner.js) to run JavaScript tests that exercise pxObjects and the JavaScript wrappers. By default, testRunner will run the tests located here: [tests.json](https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/tests.json).  For our Travis CI builds, however, the tests.json from pxCore/tests/pxScene2d/tests/testRunner will be used.

For more information on testRunner tests, see [TESTRUNNER](TESTRUNNER.md).


## Code Coverage
We use CodeCov to track our code coverage. Rule of thumb is that any new or changed code should have coverage >= 80%.  If you're creating a pull request, please be sure to include some tests for coverage.  See the [Testing section](#testing) above for information on tests. 

## Coding Conventions
* We indent using two spaces
* camelCase is preferred over underscores for names 
* Property getters and setters pattern:
  * Getter: use foo() rather than getFoo()
  * Setter: use setFoo()
* Use explicit width for ints, i.e., uint32_t rather than int
* Use rtLog, not printfs
* include <rtCore.h> instead of stdint.h and assert.h individually
* Short, descriptive comment per class to succinctly state purpose of class is desirable. Doxygen can be used within pxScene2d/src but not within rt or pxCore.

## Other Coding Guidelines
* rtCore should be lightweight classes to build stuff with
* No class inside of rtCore should leverage or vend any pxObjects
* pxResource should be lightweight wrapper around rtFileDownloader; then pxImageResource can handle/get texture and have that knowledge
* rtCore should be lightweight classes to build stuff with
* No commits that are pure style changes are allowed because they pollute code history