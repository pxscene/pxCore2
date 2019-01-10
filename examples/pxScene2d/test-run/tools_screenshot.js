/* Use file.io to capture a screenshot during test, then send link back to caller for verification */
px.import({scene:"px:scene.1.js", fs:"fs"
//          , http:"http"
        }).then(function(imports) {

/*
// DON'T USE file.io because it will return HTTP 429 if uploading more than 5 
// files in a short span of time.
function sharePNGToFileIo(buf){
    return new Promise(function (resolve, reject) {
      var originalBuf = buf;
      var b = '------------------------d51ea0dfc2e2b04b';
      var buf1 = new Buffer("--"+b+"\r\n"+
        'Content-Disposition: form-data; name="file"; filename="screenshot.png"\r\n'+
        'Content-Type: application/octet-stream\r\n\r\n');
      var buf2 = new Buffer("\r\n\r\n--"+b+"--");
      var length = buf1.length + buf.length + buf2.length;
      var body = Buffer.concat([buf1, buf, buf2], length);
      var req = imports.http.request({
        host: 'file.io',
        method: 'POST',
        headers: {
          'Content-Type': 'multipart/form-data; boundary=' + b,
          'Content-Length': body.length
        }
      }, function (res) {
        if (res.statusCode != 200) {
            reject("ERROR HTTP "+res.statusCode);

          return;
        }
        var str = '';
        res.on('data', function (chunk) {
          str += chunk;
        });
        res.on('end', function () {
          resolve(JSON.parse(str).link);
        });
      });
      req.on('error', function(e) {
        reject("ERROR "+e.message);
      });
      req.write(body);
      req.end();
    });
  }
*/
function getPNGForValidation(validateUrl)
{
    return new Promise(function (resolve, reject) {

      var filePromise = px.getFile(validateUrl);
      filePromise.then(function(data){
          //console.log("data is "+data);
          resolve(data);
      }).catch(function(exception) { 
        console.log("Error occurred trying to get file "+validateUrl);
        reject();
      });

    });
  }

  function takeScreenshot(consolePrint) 
  {
      var data = imports.scene.screenshot('image/png;base64');
      var buf = new Buffer(data.slice(data.indexOf(',')+1), 'base64');
        fs.writeFile("images/" + validationUrl, buf.toString('base64'), 'base64', function(err) {
          console.log(err);
        });
      if( consolePrint) {
        console.log("captured image is "+buf.toString('base64'))
      }

      return buf;

  }
  function validateScreenshot(validationUrl, consolePrint) { 
    var actualShot = new Buffer(takeScreenshot(consolePrint));

    console.log("validationUrl is "+validationUrl);
    return new Promise(function(resolve,reject) {
      var error = false;
      var validationPromise = getPNGForValidation(validationUrl);

      var validationPngBuff;
      //var actualPngBuff = new Buffer(actualShot.toString('utf8'), 'base64');
      var actualPngBuff = new Buffer(actualShot, 'base64');
      validationPromise.then(function(validationPng) {
        validationPngBuff = new Buffer(validationPng, 'base64');

        if(validationPngBuff.toString('base64').localeCompare(actualPngBuff.toString('base64')) === 0 )
           resolve(true);
        else 
          resolve(false);
      }).catch(function(exception){
        console.log("Exception while retrieving validation png");
        resolve(false);
      });
 

    });
  }

var getScreenshotEnabledValue = function() {
  var screenshotEnabled = false;
  if( px.appQueryParams.enableScreenshot !== undefined) {
    var param = px.appQueryParams.enableScreenshot;
    screenshotEnabled = (param == "true")?true:false;
  }
  return screenshotEnabled;
}

module.exports.takeScreenshot = takeScreenshot;
module.exports.validateScreenshot = validateScreenshot;
module.exports.getScreenshotEnabledValue = getScreenshotEnabledValue;
})
.catch(function(err){
  console.error("Imports failed for tools_screenshot: " + err)
});
