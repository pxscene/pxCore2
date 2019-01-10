const webpack    = require('webpack');
const path       = require('path');
const googleCC   = require('webpack-closure-compiler');
const copyPlugin = require('copy-webpack-plugin')

let config = {
    ////////////////////////////////////////////////////////////////////////////////
    //
    //  WEBPACK
    //
    mode: 'production',   // web-pack should default to 'development' build (production)
    watch: true,          // web-pack watches for changes .. and re-builds
    // devtool: 'source-map',
    // devtool: 'cheap-module-source-map', //cheap-source-map', //'source-map',

    ////////////////////////////////////////////////////////////////////////////////
    //
    //  INPUT FILES
    //
    entry:  [

        path.resolve(__dirname, './showcaseApp.js'),
    ],
    ////////////////////////////////////////////////////////////////////////////
    //
    //  OUTPUT FILES
    //
    output: {
        filename: 'output.js'
    },
    ////////////////////////////////////////////////////////////////////////////
    //
    //  FOLDER NAMES
    //
    resolve: {
        alias: {
                images: path.resolve(__dirname, './images/'),
            components: path.resolve(__dirname, './components/'),
                 utils: path.resolve(__dirname, './components/utils/'),
        }
    },
    ////////////////////////////////////////////////////////////////////////////
    //
    //  DEV SERVER
    //
    devServer:
    {
        contentBase: path.join(__dirname, "./dist/"),
        publicPath:  path.join(__dirname, "./dist/"),
        inline: false,
        // compress: true,
        port: 8080
    },
    ////////////////////////////////////////////////////////////////////////////
    //
    //  MODULES / RULES / LOADERS
    //
    module:
    {
        rules:
        [
        ////////////////////////////////////////////////////////////////////////////
        //
        //  LOADER: Spark
        //
        {
            test: /\.js$/,
            use: [
                {
                    loader: 'spark-resource-loader',
                        options: {
                        base:
                        {
                            "base"   : path.resolve(__dirname, './'),
                            "baseUrl": path.resolve(__dirname, './')
                        }
                    }
                },
                {
                    loader: 'spark-import-loader',
                        options: {
                        base:
                        {
                            "px.getPackageBaseFilePath()" : path.resolve(__dirname, './'), // definition for 'base' variable in the code
                            root:    path.resolve(__dirname),
                        }
                    }
                },
            ]
        },
        ////////////////////////////////////////////////////////////////////////////
        //
        //  LOADER: Images
        //
        {
            test: /\.(gif|png|jpe?g|svg)$/i,
            use: [
            {
                loader: 'file-loader?name=/images/[name].[ext]',
            },
            {
                loader: 'image-webpack-loader?name=/images/[name].[ext]',
                options: {
                    bypassOnDebug: true,  // webpack@1.x
                    disable:       false, // webpack@2.x and newer
                },
            }]

            //NOTE:  image-webpack-loader >>> will 'optimize' images to destination
        },
        ////////////////////////////////////////////////////////////////////////////
        //
        //  LOADER: Fonts
        //
        {
            test: /\.(woff(2)?|ttf|eot)(\?v=\d+\.\d+\.\d+)?$/,
            use: [
            {
                loader: 'file-loader',
                options: {
                    name: '[name].[ext]',
                    outputPath: 'fonts/'
                }
            }]
        }
        ]//rules
    },//modules

    ////////////////////////////////////////////////////////////////////////////////
    //
    //  PLUG-INS
    //
    plugins: 
    [
        ////////////////////////////////////////////////////////////////////////////
        //
        //  PLUG-IN: Google Closure Compiler
        //
        // new googleCC({
        //     compiler: {
        //         language_in:       'ECMASCRIPT6',
        //         language_out:      'ECMASCRIPT5',
        //         compilation_level: 'SIMPLE' //  'SIMPLE' or 'ADVANCED'
        //     },
        //     concurrency: 3,
        // }),

        ////////////////////////////////////////////////////////////////////////////
        //
        //  PLUG-IN: ProvidePlugin ... help "hint" paths
        //
        new webpack.ProvidePlugin({
            'utils': 'utils',
            'images': 'images',
        }),

        ////////////////////////////////////////////////////////////////////////////
        //
        //  PLUG-IN: DefinePlugin ... help define 'var' instances into JS context
        //
        // new webpack.DefinePlugin({
        //     myVarName: true
        // })

        ////////////////////////////////////////////////////////////////////////////
        //
        //  PLUG-IN: copy-webpack-plugin ... Just COPY to bundle
        //
        // new copyPlugin([
        //     { from:  path.resolve(__dirname, './appconfig.js') }
        // ])
    ]
    ////////////////////////////////////////////////////////////////////////////////
};

module.exports = config;
