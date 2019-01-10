'use strict';

px.import({
  scene: 'px:scene.1.js',
  scr_cap_utils: './screen_capture_utils.jar',
  assert: '../test-run/assert.js',
  manual: '../test-run/tools_manualTests.js'
})
.then(imports => {

  const scene = imports.scene
    , w = scene.root.w
    , h = scene.root.h
    , scr_cap_utils = imports.scr_cap_utils
    , assert = imports.assert.assert
    , manual = imports.manual
    , manualTest = manual.getManualTestValue();

  let img, info, link;

  module.exports.tests = {
    test1_drawImage: () => {
      return new Promise(resolve => {
        const img_w = Math.floor(w/2)
          , img_h = Math.floor(h/2)
          , img_url = 'https://picsum.photos/'+img_w+'/'+img_h +'?random';
        scene.create({
          t:'image',
          url:img_url,
          parent:scene.root,
          x:0.5*(w-img_w), y:0.5*(h-img_h), w:img_w, h:img_h,
          stretchX:scene.stretch.STRETCH,
          stretchY:scene.stretch.STRETCH
        }).ready.then(() => {
          resolve(assert(true));
        }, () => {
          resolve(assert(false, 'not loaded'));
        });
      });
    },
    test2_captureScreen: () => {
      return new Promise(resolve => {
        const delay_before_capture = 2000;
        setTimeout(() => {
          try {
            img = new scr_cap_utils.PNGImage(scr_cap_utils.capture());
            resolve(assert(img, 'not captured'));
          } catch (e) {
            resolve(assert(false, 'not captured: ' + e));
          }
        }, delay_before_capture);
      });
    },
    test3_verifyCapture: () => {
      return new Promise(resolve => {
        info = 'PNG\n'+img.getWidth() + 'x'+img.getHeight()+'\n'+img.getBitDepth()+' bit';
        let colorType;
        switch (img.getColorType()) {
          case 0: colorType = 'grayscale'; break;
          case 2: colorType = 'RGB'; break;
          case 3: colorType = 'palette'; break;
          case 4: colorType = 'grayscale with alpha'; break;
          case 6: colorType = 'RGBA'; break;
        }
        if (colorType) {
          info += '\n'+colorType;
        }
        resolve(assert(img.getWidth() > 0 && img.getHeight() > 0 &&
          img.getBitDepth() === 8 && img.getColorType() === 6,
          'wrong metadata: ' + info));
      });
    },
    test4_uploadCapture: () => {
      return new Promise(resolve => {
        // not mandatory for the test to pass because this function may not work with IPv6
        setTimeout(() => {
          resolve(assert(true));
        }, 10000);
        scr_cap_utils.generateOneTimeUrl(img.getBytes()).then(_link => {
          link = _link;
          console.log(`link: ${link}`);
          setTimeout(() => {
            resolve(assert(true));
          }, 1000);
        }, e => {
          console.log(`not uploaded: ${e}`);
          resolve(assert(true));
        });
      });
    },
    test5_displayPreview: () => {
      return new Promise(resolve => {
        const preview_w = 0.25*w
          , preview_h = preview_w * h / w
          , text_w = 0.15*w
          , text_h = preview_h
          , overlay_margin = 5
          , overlay_w = preview_w+text_w+4*overlay_margin
          , overlay_h = preview_h+2*overlay_margin
          , promises = [];
        const overlay = scene.create({
          t:'rect',
          parent:scene.root,
          x:0.5*(w-overlay_w), y:0.5*(h-overlay_h), w:overlay_w, h:overlay_h,
          fillColor:0x222222ff,
          lineWidth:0,
          a:0
        });
        promises.push(overlay);
        const text = scene.create({
          t:'text',
          parent:overlay,
          x:3*overlay_margin+preview_w, y:overlay_margin, w:text_w, h:text_h,
          textColor:0xffffffff,
          pixelSize:20,
          text:info
        });
        promises.push(text);
        if (link) {
          const image = scene.create({
            t:'image',
            url:link,
            parent:overlay,
            x:overlay_margin, y:overlay_margin, w:preview_w, h:preview_h,
            stretchX:scene.stretch.STRETCH,
            stretchY:scene.stretch.STRETCH
          });
          promises.push(image);
        }
        Promise.all(promises)
        .then(() => {
          overlay.animateTo({a:1},0.2).then(() => {
            resolve(assert(true));
          }, e => {
            resolve(assert(false, 'not animated: ' + e));
          });
        }, e => {
          resolve(assert(false, 'not displayed: ' + e));
        });
      });
    }
  };

  if (manualTest === true) {
    manual.runTestsManually(module.exports.tests, module.exports.beforeStart);
  }
});
