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


px.import({ scene: 'px:scene.1', http: 'http', url: 'url' }).then( function importsAreReady(imports)
{
    var http = imports.http;
    var url = imports.url;

    var fetchCb = function (opt) {
        var respData = '';
    
        http.get(opt, function (resp) {
            console.log('http in func cb');
    
            resp.on('data', function (chunk) {
                console.log('http on data');
                respData += chunk;
            });
    
            resp.on('end', function () {
                console.log('http done. status_code = ' + resp.statusCode
                    + ' response size = ' + respData.length);
            });
        }).on('error', function (e) {
            console.log('error fetching data: ' + e.message);
        });
    };

    var fetchUrl = 'https://bellard.org/pi/pi2700e9';

    fetchCb(fetchUrl);
    fetchCb(url.parse(fetchUrl));

}).catch( function importFailed(err){
    console.error("Import failed for http_test.js: " + err);
});
