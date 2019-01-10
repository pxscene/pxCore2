px.import({
  scene: 'px:scene.1.js',
  assert: '../test-run/assert.js',
  manual: '../test-run/tools_manualTests.js'
})
.then(imports => {

  const scene = imports.scene;
  const assert = imports.assert.assert;
  const manual = imports.manual;
  const manualTest = manual.getManualTestValue();
  const baseFilePath = px.getPackageBaseFilePath();

  module.exports.thisAppQueryParams = px.appQueryParams;
  module.exports.thisPackageUrl = module.appSceneContext.packageUrl;

  // https://www.sparkui.org/docs/frameworks_doc.html
  module.exports.tests = {
    thisModuleBaseFilePath: () => {
      return new Promise(resolve => {
        resolve(assert(baseFilePath === 'https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests' ||
          baseFilePath === 'https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/test-run/../tests',
          `Uri is: ${baseFilePath}`));
      });
    },
    pxImportSingle: () => {
      return new Promise(resolve => {
        px.import('px:scene.1.js').then(scene => {
          resolve(assert(scene.root, 'not imported'));
        }).catch(err => {
          resolve(assert(false, `${err}`));
        });
      });
    },
    pxImportMultiple: () => {
      return new Promise(resolve => {
        px.import({scene:'px:scene.1.js', keys:'px:tools.keys.js', url:'url'}).then(i => {
          resolve(assert(i.scene && i.keys && i.url && i.scene.root, 'not imported'));
        }).catch(err => {
          resolve(assert(false, `${err}`));
        });
      });
    },
    pxConfigImport: () => {
      return new Promise(resolve => {
        px.configImport({'pxFramework:':`${baseFilePath}/../frameworks/`});
        px.import({scene:'px:scene.1.js', TextInput: 'pxFramework:TextInput.js'}).then(i => {
          resolve(assert(i.scene && typeof i.TextInput === 'function' && i.scene.root, 'not imported'));
        }).catch(err => {
          resolve(assert(false, `${err}`));
        });
      });
    },
    pxAppQueryParams: () => {
      return new Promise(resolve => {
        resolve(assert(px.appQueryParams, 'no such property'));
      });
    },
    pxGetPackageBaseFilePath: () => {
      return new Promise(resolve => {
        const Uri = px.getPackageBaseFilePath();
        resolve(assert(Uri === baseFilePath, `Uri is: ${Uri}`));
      });
    },
    pxGetBaseFilePath: () => {
      return new Promise(resolve => {
        const Uri = px.getBaseFilePath();
        resolve(assert(Uri === baseFilePath, `Uri is: ${Uri}`));
      });
    },
    pxGetModuleFile: () => {
      return new Promise(resolve => {
        px.getModuleFile('images/ball.png').then(data => {
          resolve(assert(data.length === 28490, `data.length is: ${data.length}`));
        }).catch(err => {
          resolve(assert(false, `${err}`));
        });
      });
    },
    pxGetFile: () => {
      return new Promise(resolve => {
        px.getFile(`${baseFilePath}/images/ball.png`).then(data => {
          resolve(assert(data.length === 28490, `data.length is: ${data.length}`));
        }).catch(err => {
          resolve(assert(false, `${err}`));
        });
      });
    },
    pxLog: () => {
      return new Promise(resolve => {
        resolve(assert(px.log && px.log.warn, 'no such property'));
      });
    },
    childSceneWithParams: () => {
      return new Promise(resolve => {
        const childSceneUrl = `${baseFilePath}/test_moduleApi.js?manualTest=0&test=123`;
        const new_scene = scene.create({t: 'scene', parent: scene.root, w: scene.w, h: scene.h, url: childSceneUrl});
        new_scene.ready.then(s => {
          resolve(assert(s.api.thisAppQueryParams.manualTest === '0' &&
            s.api.thisAppQueryParams.test === '123' &&
            s.api.thisPackageUrl === childSceneUrl,
            'manualTest='+s.api.thisAppQueryParams.manualTest+' packageUrl='+s.api.thisPackageUrl));
        }).catch(err => {
          resolve(assert(false, `${err}`));
        });
      });
    },
    moduleAppSceneContextPackageUrl: () => {
      return new Promise(resolve => {
        const Uri = module.appSceneContext.packageUrl;
        resolve(assert(Uri === `${baseFilePath}/test_moduleApi.js?manualTest=0` ||
          Uri === `${baseFilePath}/test_moduleApi.js?manualTest=1` ||
          Uri === `${baseFilePath}/test_moduleApi.js`,
          `Uri is: ${Uri}`));
      });
    },
    moduleAppSceneContextGetPackageBaseFilePath: () => {
      return new Promise(resolve => {
        const Uri = module.appSceneContext.getPackageBaseFilePath();
        resolve(assert(Uri === baseFilePath, `Uri is: ${Uri}`));
      });
    },
    moduleAppSceneContextGetFile: () => {
      return new Promise(resolve => {
        module.appSceneContext.getFile(`${baseFilePath}/images/ball.png`).then(data => {
          resolve(assert(data.length === 28490, `data.length is: ${data.length}`));
        }).catch(err => {
          resolve(assert(false, `${err}`));
        });
      });
    },
    moduleProps: () => {
      return new Promise(resolve => {
        const k = Object.getOwnPropertyNames(module).sort();
        resolve(assert(k.length === 2 && k[0] === 'appSceneContext' && k[1] === 'exports',
          'length='+k.length+' [0]='+k[0]+' [1]='+k[1]));
      });
    },
    moduleExportsProps: () => {
      return new Promise(resolve => {
        const k = Object.getOwnPropertyNames(module.exports).sort();
        resolve(assert(k.length === 3 && k[0] === 'tests' && k[1] === 'thisAppQueryParams' && k[2] === 'thisPackageUrl',
          'length='+k.length+' [0]='+k[0]+' [1]='+k[1]+' [2]='+k[2]));
      });
    },
    moduleAppSceneContextProps: () => {
      return new Promise(resolve => {
        const k = Object.getOwnPropertyNames(module.appSceneContext).sort();
        resolve(assert(k.length === 3 && k[0] === 'getFile' && k[1] === 'getPackageBaseFilePath' && k[2] === 'packageUrl',
          'length='+k.length+' [0]='+k[0]+' [1]='+k[1]+' [2]='+k[2]));
      });
    },
  };

  if (manualTest === true) {
    manual.runTestsManually(module.exports.tests, module.exports.beforeStart);
  }
});
