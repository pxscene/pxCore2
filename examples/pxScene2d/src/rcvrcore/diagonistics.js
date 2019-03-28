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

/* collect the complete tree of pxObjects for given scene and report the same */

function Diagonistics(){
  var scene = null;
  this.collect = collect;

  function collect(diagScene)
  {
    scene = diagScene;
    return new Promise(function (resolve, reject) {
      var node = {};
      node["estbMAC"] = "";
      node["timestamp"] = new Date();
      node["pxObjectCount"] = "";
      node["gfxmemory"] = scene.textureMemoryUsage();
      collectSnapshot("", scene.root, true, scene.filePath, true, resolve, reject, node);
    });
  }

  function populateMAC()
  {
    return new Promise(function (resolve, reject) {
      var systemService = scene.getService("systemService");
  
      if(systemService != undefined && systemService != null) {
        systemService.setApiVersionNumber(10);
        systemService.registerForAsyncResponses(true);
        systemService.onAsyncResponse = function(params, gid) {
          var macAddresses = JSON.parse(params);
          resolve(macAddresses.estb_mac);
        };
        var result = JSON.parse(systemService.callMethod("getMacAddresses", "1"));
        if (false == result.success)
        {
          resolve("");
          console.log("MAC address fetch failed !!");
        }
        else
        {
          console.log("MAC address fetch succeeded !!");
        }
      }
      else
      {
        console.log("unable to access system service");
        resolve("");
      }
    });
  }

  function collectSnapshot(identifier, element, isRoot, url, returnfn, resolve, reject, resultnode)
  {
    var node = {};
    var nodename = "";
    if (isRoot)
    {
      nodename = "root";
      if (url)
      {
        nodename = nodename + " (" + url + ")";
      }
    }
    else
    {
      var nodeType = element.description().substring(2);
      nodename = nodename + nodeType.toLowerCase() + "(" + identifier + ")";
    }
  
    if (undefined == node[nodename])
    {
      node[nodename] = {};
    }
  
    var props = element["allKeys"];
    for (var propertyIndex = 0; propertyIndex<props.length; propertyIndex++)
    {
      var property = props[propertyIndex];
      if ((property != "_pxObject") && (property != "parent") && (property != "children") && (element[property] != undefined) && (typeof element[property] != "function"))
      {
        node[nodename][props[propertyIndex]] = element[property];
      }
    }
    function populateChild(nodename,object,index,numChildren)
    {
      if (index >= numChildren)
      {
        return;
      }
  
      if (object.getChild(index).url != undefined && (object.getChild(index).url.endsWith(".js")))
      {
        var childurl = object.getChild(index).url; 
        var ispopulated = false;
        var newnodename = "root (" + childurl + ")";
        object.getChild(index).ready.then(function(data) { 
          if (data.root != undefined) { 
            var newnodename = "root (" + childurl + ")";
            node[nodename][newnodename] = collectSnapshot("", data.root, true, childurl)[newnodename];
            populateChild(nodename, object, index+1, numChildren);
          }
        });
      }
      else
      {
        var nodeType = object.getChild(index).description().substring(2);
        var newnodename = nodeType.toLowerCase() + "(" + (index+1) + ")";
        node[nodename][newnodename] = collectSnapshot(index+1, object.getChild(index),false)[newnodename];
        populateChild(nodename, object, index+1, numChildren);
      }
    }
    populateChild(nodename, element, 0, element.numChildren);
    if (true == returnfn)
    {
      resultnode["snapshot"] = node;
      populateMAC().then(function(mac) {
        var timedata = JSON.stringify(resultnode["timestamp"]);
        resultnode["estbMAC"] = mac;
        var diagService = scene.getService("org.rdk.diagonistics");    
        if ((undefined != diagService) && (null != diagService)) {
          diagService.saveData(JSON.stringify(resultnode), timedata.substring(1, timedata.length-1));
        }
        else
        {
          console.log(JSON.stringify(resultnode));
        }
        resolve(resultnode);
      });
    }
    else
      return node;
  }
}

module.exports = Diagonistics;
