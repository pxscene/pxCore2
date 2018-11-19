const webpack  = require('webpack');
const path     = require('path');
const UglifyJsPlugin = require('uglifyjs-webpack-plugin');
const glob = require("glob")
const process = require('process')
const env = process.env
const SparkPluginImports = require('spark-plugin-imports');
const WebpackShellPlugin = require('webpack-shell-plugin');
const CopyWebpackPlugin = require('copy-webpack-plugin');

let config = {
    mode: 'development',
    watch: false,
    devtool: 'cheap-source-map',
    optimization: {
        runtimeChunk: false,
    },
    entry:  glob.sync("./pack/**/*.js"),
    node: {
        fs: 'empty',
        global:false,
        process:false,
        Buffer:false
      },

    output: {
        filename: 'output.js'
    },
    resolve: {
        alias: {
            images: path.resolve(__dirname, './images/'),
            "px.getPackageBaseFilePath()": __dirname
        }
    },

    devServer:
    {
        contentBase: path.join(__dirname, "./dist/"),
        publicPath:  path.join(__dirname, "./dist/"),
        inline: false,
        port: 8080
    },

    plugins: 
    [
        new CopyWebpackPlugin([
            { from: './pack/**/*.png' },
            { from: './pack/**/*.svg' },
            { from: './pack/**/*.ttf' },
            { from: './pack/**/*.jpg' }
        ]),

        new webpack.ProvidePlugin({
            'components': 'components',
            'images': 'images',
        }),

        new SparkPluginImports({'entryFile':env.ENTRY_FILE}),
        new UglifyJsPlugin(),
        new WebpackShellPlugin({onBuildStart:[], onBuildEnd:['./node_modules/spark-plugin-imports/utils/bundle.sh']})
    ]
    ////////////////////////////////////////////////////////////////////////////////
};

module.exports = config;
