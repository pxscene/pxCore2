const webpack  = require('webpack');
const path     = require('path');
const glob = require("glob");
const CopyWebpackPlugin = require('copy-webpack-plugin');

let config = {
    mode: 'development',   // web-pack should default to 'development' build
    devtool:false,
    watch: false,           // web-pack watches for changes .. and re-builds
    entry:  glob.sync("./pack/**/*.js"),
    node: {
      fs: 'empty',
      global:false,
      process:false,
      Buffer:false
    },

        ////////////////////////////////////////////////////////////////////////////////
        //
        //  OUTPUT FILES
        //
    devServer:
    {
        ////////////////////////////////////////////////////////////////////////////////
        //
        //  DEV SEVER
        //
        contentBase: path.join(__dirname, "./dist/"),
        publicPath:  path.join(__dirname, "./dist/"),
        inline: false,
       // compress: true,
        port: 80
    },
    ////////////////////////////////////////////////////////////////////////////////
    //
    //  PLUG-IN: Google Closure Compiler
    //
    plugins: 
    [
      new CopyWebpackPlugin([
        { from: './pack/**/*.png' },
        { from: './pack/**/*.svg' },
        { from: './pack/**/*.ttf' },
        { from: './pack/**/*.jpg' }
      ])
    ]
    ////////////////////////////////////////////////////////////////////////////////
};

module.exports = config;
