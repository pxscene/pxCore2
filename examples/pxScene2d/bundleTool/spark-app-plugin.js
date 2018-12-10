"use strict";

const CopyWebpackPlugin = require('copy-webpack-plugin');
const UglifyJsPlugin = require('uglifyjs-webpack-plugin');
const WebpackShellPlugin = require('webpack-shell-plugin');

const process = require('process')
const env = process.env

const glob = require('glob')

const { ConcatSource } = require("webpack-sources");

var moduleExportsPre = "module.exports.*=.*__webpack_require__(?!.+?";
var moduleExportsPost = ".+?).+?;";

var webpackRequirePre = "^__webpack_require__(?!.+";
var webpackRequirePost = ".+).+?;";

var moduleExportsMatchPre = "module.exports.*=.*__webpack_require__(?=.+?";
var moduleExportsMatchPost = ".+?).+?;";

function escapeFileName(filename)
{
  return filename.replace(/[\/.]/gi,"\\$&");
}

var entryFile = "";
var extraPath = "";
var outputDir = "dist";
var outputFile = "output.js";
var outputJar = "bundle.jar";
var bundleType = "jar";
var projectPath = "";

module.exports = class SparkPluginImports {

  constructor() {
    // entry file provided by user
    projectPath = process.env.PWD;
  }

  apply(compiler)
  {
    /* add options for webpack */
    function addOptions(compiler)
    {
      if (compiler["options"].entry[null] == undefined)
      {
        console.log("please provide entry option for webpack execution");
        process.exit(0);
      }

      if (process.env.BUNDLE_TYPE != undefined)
      {
        var userInput = process.env.BUNDLE_TYPE;
        if ((userInput == "file") || (userInput == "jar")) 
          bundleType = userInput;
        else
          console.log("!! Warning : unsupported option for bundle type. only file and jar supported");
      }

      if ((compiler["options"].output["filename"] != undefined) && (compiler["options"].output["filename"] != '[name].js'))
      {
        if (bundleType == "jar")
        {
          outputJar = compiler["options"].output["filename"];
          compiler["options"].output["filename"] = outputFile;
        }
        else if(bundleType == "file")
        {
          outputFile = compiler["options"].output["filename"];
        }
        outputDir = compiler["options"].output["path"];
      }
      else
      {
        compiler["options"].output["filename"] = outputFile;
        compiler["options"].output["path"] = outputDir;
      }
      compiler["options"].optimization.namedModules = "true";
      var ignoreFiles = [];
      ignoreFiles.push("node_modules/**/*");
      compiler["options"].plugins.push(new CopyWebpackPlugin([
              { from: projectPath + '/**/*.png', ignore: ignoreFiles },
              { from: projectPath + '/**/*.svg', ignore: ignoreFiles },
              { from: projectPath + '/**/*.ttf', ignore: ignoreFiles },
              { from: projectPath + '/**/*.jpg', ignore: ignoreFiles }
          ]));
      //compiler["options"].plugins.push(new UglifyJsPlugin());

      if ("jar" == bundleType)
      {
        compiler["options"].plugins.push(new WebpackShellPlugin({onBuildStart:[], onBuildEnd:[__dirname + '/utils/bundle.sh ' + outputDir+" "+outputJar]}));
      }
      compiler["options"].devtool = 'cheap-source-map';
      compiler["options"].mode = 'production';
      compiler["options"].node = {
          fs: 'empty',
          global:false,
          process:false,
          Buffer:false
        };
      //console.log(compiler["options"]);
      compiler["options"].target = "node";
      entryFile = compiler["options"].entry[null];
      var lastSlash = entryFile.lastIndexOf("/");

      if (-1 != lastSlash)
      {
        extraPath = entryFile.substring(0,lastSlash+1);
      }
      else
        extraPath = "";
      if ((entryFile.length > 0) && (!entryFile.startsWith("./")) && (!entryFile.startsWith("/")))
        entryFile = "./" + entryFile;
      var files = glob.sync((extraPath=="")?"./**/*.js":extraPath+"**/*.js").filter(function(link){
                                            return !(link.includes("node_modules") || link.includes("webpack.config"))});
      if (undefined != compiler["options"].externals)
      {
        var externalfiles = compiler["options"].externals.files;
        var removeIndex = [];
        if (externalfiles != undefined)
        {
            for (var j=0; j<externalfiles.length; j++)
            {
              for (var i=0; i<files.length; i++)
              {
                if (files[i].includes(externalfiles[j]))
                {
                  files[i] = "deleted";
                }
              }
            }
        }
      }
      var newfiles = files.filter(a => a !== 'deleted')
      compiler["options"].entry = newfiles;
    }

    addOptions(compiler);

    /* function to update to spark module and exports instead of default one */
    function addPxModuleExports(findStr)
    {
        var myStr = findStr;
        var lastExportsIndex = myStr.lastIndexOf("function\(module, exports, __webpack_require__\)");
        if (-1 != lastExportsIndex)
        {
          var newStr = myStr.substring(0, lastExportsIndex);

          var reFunctionHeader1 = new RegExp(".*(function\\(module, exports, __webpack_require__\\))",'g');
          var matchFunctionHeader1 = null;

          while((matchFunctionHeader1 = reFunctionHeader1.exec(newStr)) != null)
          {
              newStr = newStr.replace(matchFunctionHeader1[1], "function(module=px.module, exports=px.exports)");//"// ## " + matchWebpackRequire[0]);
          }
          var remString = myStr.substring(lastExportsIndex);
          myStr = newStr + remString;
        }

        var reFunctionHeader = new RegExp(".*(function\\(module, exports\\))",'g');
        var matchFunctionHeader = null;

        while((matchFunctionHeader = reFunctionHeader.exec(myStr)) != null)
        {
            myStr = myStr.replace(matchFunctionHeader[1], "function(module=px.module, exports=px.exports)");//"// ## " + matchWebpackRequire[0]);
        }

        return myStr;
    }

    /* function to remove use strict lines */
    function removeUseStrict(findStr)
    {
        var myStr = findStr;
        // replace all use strict statements
        myStr = myStr.replace(/\"use strict\"\;/g," ");
        return myStr;
    }

    /* function to remove module.exports lines for all files except entry file */
    function removeModuleExportsExceptEntry(findStr, entryFile)
    {
      var myStr = findStr;
      var pattern = moduleExportsPre + escapeFileName(entryFile) + moduleExportsPost;
      var exp = new RegExp(pattern, "g");
      var matchWebpackModuleExports = null;
      while((matchWebpackModuleExports = exp.exec(myStr)) != null)
      {
          myStr = myStr.replace(matchWebpackModuleExports[0], "");
      }

      return myStr;
    }

    /* function to remove __webpack__require lines for all files except entry file */
    function removeWebPackRequireExceptEntry(findStr, entryFile)
    {
      var myStr = findStr;
      var pattern = webpackRequirePre + escapeFileName(entryFile) + webpackRequirePost;
      var exp = new RegExp(pattern, "gm");
      var matchWebpackRequire = null;
      while((matchWebpackRequire = exp.exec(myStr)) != null)
      {
          myStr = myStr.replace(matchWebpackRequire[0], "// " + matchWebpackRequire[0]);
      }
      return myStr;
    }

    /* function to replace module.exports for entry file with __webpack_require */
    function replaceEntryModuleExportsWithRequire(findStr, entryFile)
    {
      var myStr = findStr;
      var pattern = moduleExportsMatchPre + escapeFileName(entryFile) + moduleExportsMatchPost;
      var exp = new RegExp(pattern, "g");
      var matchWebpackModuleExports = null;
      while((matchWebpackModuleExports = exp.exec(myStr)) != null)
      {
          myStr = myStr.replace(matchWebpackModuleExports[0], "__webpack_require__(/*! "+entryFile+" */\"" + entryFile + "\");");
      }
      return myStr;
    }

    /* function to replace extra path */
    function replaceExtraPath(findStr, extraPath)
    {
        var reExtraPath = new RegExp(escapeFileName(extraPath),'g');
        var myStr = findStr; // to modify param
        var matchFunctionHeader = null;
        while((matchFunctionHeader = reExtraPath.exec(myStr)) != null)
        {
          myStr = myStr.replace(matchFunctionHeader, "");
        }
        return myStr;
    }

    /* function to add px.registerCode call before launch of entry js file */
    function addFinalPxRegisterCode(findStr, entryFile)
    {
        var myStr = findStr;
        myStr = myStr.replace("__webpack_require__(/*! "+entryFile+" */\"" + entryFile + "\");", "if (undefined != px.registerCode) { px.registerCode(__webpack_require__.m); __webpack_require__(/*! "+entryFile+" */\"" + entryFile + "\"); } else { console.log(\"!!! Bundled apps not supported in this version of Spark !!! \"); }");
        myStr = myStr.replace("__webpack_require__(\"" + entryFile + "\");", "if (undefined != px.registerCode) { px.registerCode(__webpack_require__.m); __webpack_require__(/*! "+entryFile+" */\"" + entryFile + "\"); } else { console.log(\"!!! Bundled apps not supported in this version of Spark !!! \"); }");
        return myStr;
    }

    if (compiler.hooks) {
      var plugin = { name: 'SparkPluginImports' };

      compiler.hooks.compilation.tap(plugin, function (compilation) {
        /* hook into optimizeChunkAssets event */
        compilation.hooks.optimizeChunkAssets.tapAsync(plugin, function(chunks, callback) {
          Object.keys(compilation.assets).forEach((key) =>
          {
            var source = compilation.assets[key];
            var sourceip = source.source();
            sourceip = removeModuleExportsExceptEntry(sourceip, entryFile);
            sourceip = removeWebPackRequireExceptEntry(sourceip, entryFile);
            sourceip = replaceEntryModuleExportsWithRequire(sourceip, entryFile);
            sourceip = addPxModuleExports(sourceip);
            sourceip = removeUseStrict(sourceip);
            sourceip = addFinalPxRegisterCode(sourceip, entryFile);
            if (extraPath != "")
              sourceip = replaceExtraPath(sourceip, extraPath);
            compilation.assets[key] = new ConcatSource(sourceip);
          });
          callback(); 
        }.bind(this));
      }.bind(this));
    }
  }
};
