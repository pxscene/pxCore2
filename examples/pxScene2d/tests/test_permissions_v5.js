px.import({
  scene: 'px:scene.1.js',
  https: 'https',
  assert: '../test-run/assert.js',
  manual: '../test-run/tools_manualTests.js'
})
.then(imports => {

  const scene = imports.scene;
  const https = imports.https;
  const assert = imports.assert.assert;
  const manual = imports.manual;
  const manualTest = manual.getManualTestValue();
  const this_url = px.getPackageBaseFilePath() + '/test_permissions_v5.js?manualTest=0';

  //console.log(`this_url='${this_url}' manualTest='${manualTest}'`);

  module.exports.http_request = url => {
    return new Promise((resolve, reject) => {
      let req = https.request(url, resolve);
      req.on('response', resolve);
      req.on('error', reject);
      req.on('blocked', e => {
        if (e && e.message && e.message.indexOf('CORS') === 0) {
          // request permitted, but no CORS headers in response
          resolve();
        } else {
          reject();
        }
      });
      req.end();
    });
  };

  module.exports.tests = {
    byDefaultAllowGoogle: () => {
      return new Promise(resolve => {
        module.exports.http_request('https://google.com').then(() => {
          resolve(assert(true));
        }, () => {
          resolve(assert(false, 'blocked'));
        });
      });
    },
    explicitAllowGoogle: () => {
      return new Promise(resolve => {
        let new_scene = scene.create({
          t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: this_url,
          permissions: {
            'url' : {
              'allow' : [ '*', 'https://google.com' ],
              'block' : [ '*://google.com' ]
            }
          }
        });
        new_scene.ready.then(s => {
          s.api.http_request('https://google.com').then(() => {
            resolve(assert(true));
          }, () => {
            resolve(assert(false, 'blocked'));
          });
        }).catch(e => {
          resolve(assert(false, e));
        });
      });
    },
    explicitBlockGoogle: () => {
      return new Promise(resolve => {
        let new_scene = scene.create({
          t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: this_url,
          permissions: {
            'url' : {
              'allow' : [ '*' ],
              'block' : [ '*://google.com' ]
            }
          }
        });
        new_scene.ready.then(s => {
          s.api.http_request('https://google.com').then(() => {
            resolve(assert(false, 'not blocked'));
          }, () => {
            resolve(assert(true));
          });
        }).catch(e => {
          resolve(assert(false, e));
        });
      });
    },
    appCannotSetOwnPermissions: () => {
      return new Promise(resolve => {
        scene.permissions = {
          'url' : {
            'block' : [ '*' ]
          }
        };
        let new_scene = scene.create({
          t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: this_url
        });
        new_scene.ready.then(() => {
          resolve(assert(true));
        }).catch(e => {
          resolve(assert(false, e));
        });
      });
    },
    appCanSetChildAppPermissonsRuntime: () => {
      return new Promise(resolve => {
        let new_scene = scene.create({
          t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: this_url
        });
        new_scene.ready.then(s => {
          s.permissions = {
            'url' : {
              'block' : [ '*' ]
            }
          };
          s.api.http_request('https://google.com').then(() => {
            resolve(assert(false, 'not blocked'));
          }, () => {
            resolve(assert(true));
          });
        }).catch(e => {
          resolve(assert(false, e));
        });
      });
    },
    byDefaultAllowScreenCapture: () => {
      return new Promise(resolve => {
        try {
          let screenshot = scene.screenshot('image/png;base64');
          resolve(assert(screenshot, 'not captured'));
        } catch (e) {
          resolve(assert(false, e));
        }
      });
    },
    explicitAllowScreenCapture: () => {
      return new Promise(resolve => {
        let new_scene = scene.create({
          t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: this_url,
          permissions: {
            'url' : {
              'allow' : [ '*' ]
            },
            'features' : {
              'block' : [ '*' ],
              'allow' : [
                'screenshot'
              ]
            }
          }
        });
        new_scene.ready.then(s => {
          try {
            let screenshot = s.screenshot('image/png;base64');
            resolve(assert(screenshot, 'not captured'));
          } catch (e) {
            resolve(assert(false, e));
          }
        }).catch(e => {
          resolve(assert(false, e));
        });
      });
    },
    explicitBlockScreenCapture: () => {
      return new Promise(resolve => {
        let new_scene = scene.create({
          t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: this_url,
          permissions: {
            'url' : {
              'allow' : [ '*' ]
            },
            'features' : {
              'block' : [ '*' ]
            }
          }
        });
        new_scene.ready.then(s => {
          try {
            let screenshot = s.screenshot('image/png;base64');
            resolve(assert(!screenshot, 'captured'));
          } catch (e) {
            resolve(assert(true));
          }
        }).catch(e => {
          resolve(assert(false, e));
        });
      });
    },
    inexplicitBlockScreenCapture: () => {
      return new Promise(resolve => {
        let new_scene = scene.create({
          t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: this_url,
          permissions: {
            'url' : {
              'allow' : [ '*' ]
            }
          }
        });
        new_scene.ready.then(s => {
          try {
            let screenshot = s.screenshot('image/png;base64');
            resolve(assert(!screenshot, 'captured'));
          } catch (e) {
            resolve(assert(true));
          }
        }).catch(e => {
          resolve(assert(false, e));
        });
      });
    },
    byDefaultArbitraryAppCanAccessGoogle: () => {
      return new Promise(resolve => {
        let appLink = "https://tinyurl.com/y9ez78pa?module=https&url=https://www.google.com/";
        // -> https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/test_access_url.js?module=https&url=https://www.google.com/
        let new_scene = scene.create({
          t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: appLink
        });
        new_scene.ready.then(s => {
          s.api.test_access_url().then(blocked => {
            resolve(assert(!blocked, 'blocked'));
          }, e => {
            resolve(assert(false, e));
          });
        }).catch(e => {
          resolve(assert(false, e));
        });
      });
    },
    byDefaultArbitraryAppCanNotAccessLocalhost: () => {
      return new Promise(resolve => {
        let appLink = "https://tinyurl.com/y9ez78pa?module=https&url=https://127.0.0.1/";
        // -> https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/test_access_url.js?module=https&url=https://127.0.0.1/
        let new_scene = scene.create({
          t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: appLink
        });
        new_scene.ready.then(s => {
          s.api.test_access_url().then(blocked => {
            resolve(assert(blocked, 'not blocked'));
          }, e => {
            resolve(assert(false, e));
          });
        }).catch(e => {
          resolve(assert(false, e));
        });
      });
    }
  };

  if (manualTest === true) {
    manual.runTestsManually(module.exports.tests, module.exports.beforeStart);
  }
});
