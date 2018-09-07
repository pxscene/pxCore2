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

var files =    // https://github.com/gilbarbara/logos
[
    "100tb.svg", "500px.svg", "6px.svg", "admob.svg", "adroll.svg", "aerospike.svg", "airbnb.svg",
    "airbrake.svg", "airtable.svg", "akamai.svg", "akka.svg", "alfresco.svg", "algolia.svg",
    "altair.svg", "amazon-chime.svg", "amazon-connect.svg", "amex.svg", "ampersand.svg",
    "android-icon.svg", "android.svg", "angellist.svg", "angular-icon.svg", "angular.svg",
    "ansible.svg", "apache.svg", "apache_cloudstack.svg", "api-ai.svg", "apiary.svg",
    "apigee.svg", "apitools.svg", "apollostack.svg", "appbase.svg", "appcelerator.svg",
    "appcode.svg", "appdynamics.svg", "appfog.svg", "apphub.svg", "appium.svg", "apple.svg",
    "appmaker.svg", "apportable.svg", "appsignal.svg", "apptentive.svg", "appveyor.svg",
    "arangodb.svg", 
    "archlinux.svg", "arduino.svg", "armory.svg", "asana.svg", "astronomer.svg",
    "atom.svg", "atomic.svg", "aurelia.svg", "aurora.svg", "aurous.svg", "auth0.svg", "authy.svg",
    "autocode.svg", "autoit.svg", "autoprefixer.svg", "ava.svg", "awesome.svg", "aws-api-gateway.svg",
    "aws-cloudformation.svg", "aws-cloudfront.svg", "aws-cloudsearch.svg", "aws-cloudwatch.svg",
    "aws-codedeploy.svg", "aws-cognito.svg", "aws-dynamodb.svg", "aws-ec2.svg", "aws-elastic-cache.svg",
    "aws-glacier.svg", "aws-iam.svg", "aws-kinesis.svg", "aws-lambda.svg", "aws-mobilehub.svg",
    "aws-opsworks.svg", "aws-quicksight.svg", "aws-rds.svg", "aws-route53.svg", "aws-s3.svg",
    "aws-ses.svg", "aws-sns.svg", "aws-sqs.svg", "aws-waf.svg", "aws.svg", "azure.svg",
    "babel.svg",
    "backbone-icon.svg", "backbone.svg", "backerkit.svg", "baker-street.svg", 
   "base.svg",
    "basecamp.svg",
    "basekit.svg", "bash.svg", "batch.svg", "behance.svg", "bem-2.svg", "bem.svg", "bigpanda.svg", "bing.svg",
    "bitballoon.svg", "bitbucket.svg", "bitcoin.svg", "bitnami.svg", "bitrise.svg", "blocs.svg", "blogger.svg",
    "blossom.svg", "bluemix.svg", "blueprint.svg", "bluetooth.svg", "bootstrap.svg", "bosun.svg", "botanalytics.svg",
    "bourbon.svg", "bower.svg", "bowtie.svg", "box.svg", "brackets.svg", "branch.svg", "brandfolder.svg", "brave.svg",
    "braze.svg", "broccoli.svg", "browserify-icon.svg", "browserify.svg", "browserling.svg", "browserstack.svg",
    "browsersync.svg", "brunch.svg", "buck.svg", "buddy.svg", "buffer.svg", "bugherd.svg", "bugsee.svg", "bugsnag.svg",
    "c++.svg", "c.svg", "cachet.svg", "caffe2.svg", "cakephp.svg", "campaignmonitor.svg", "campfire.svg", "canjs.svg",
    "capistrano.svg", "carbide.svg", "cassandra.svg", "celluloid.svg", "centos-icon.svg", "centos.svg", "certbot.svg",
    "chai.svg", 
    "chalk.svg", 
    "changetip.svg", "chartblocks.svg", "chef.svg", "chevereto.svg", "chrome.svg", "circleci.svg",
    "cirrus.svg", "clickdeploy.svg", "clion.svg", "cljs.svg", "clojure.svg", "close.svg", "cloud9.svg", "cloudacademy.svg",
    "cloudant.svg", "cloudcraft.svg", "cloudera.svg", "cloudflare.svg", "cloudinary.svg", "cloudlinux.svg", "clusterhq.svg",
    "cobalt.svg", "cockpit.svg", "cocoapods.svg", "codebase.svg", "codebeat.svg", "codecademy.svg", "codeception.svg",
    "codeclimate.svg", "codecov.svg", "codefund.svg", "codeigniter.svg", "codepen-icon.svg", "codepen.svg",
    "codepicnic.svg", "codepush.svg", "coderwall.svg", "codeschool.svg", "codeship.svg", "codio.svg", "codrops.svg",
    "coffeescript.svg", "compass.svg", "component.svg", "componentkit.svg", "compose.svg", 
    "composer.svg",
    "concourse.svg", "concrete5.svg", "consul.svg", "containership.svg", "contentful.svg", "convox.svg",
    "copyleft-pirate.svg", "copyleft.svg", "cordova.svg", "coreos-icon.svg", "coreos.svg", "couchbase.svg",
    "couchdb.svg", "coursera.svg", "coveralls.svg", "coverity.svg", "cpanel.svg", "craft.svg", "crashlytics.svg",
    "crateio.svg", "createjs.svg", "crittercism.svg", "crossbrowsertesting.svg", "crowdprocess.svg",
    "crystal.svg", "css-3.svg", "css-3_official.svg", "cssnext.svg", "cucumber.svg", "customerio.svg",
    "cyclejs.svg", "cypress.svg", "d3.svg", "dapulse.svg", "dart.svg", "dashlane.svg", "dat.svg", "database-labs.svg",
    "dcos.svg", "debian.svg", "delicious-burger.svg", "delicious.svg", "delighted.svg", "dependencyci.svg",
    "deploy.svg", "deppbot.svg", "derby.svg", "designernews.svg", "desk.svg", "deviantart.svg", "digital-ocean.svg",
    "dinersclub.svg", "discord.svg", "discover.svg", "disqus.svg", "distelli.svg", "divshot.svg", "django.svg",
    "dockbit.svg", "docker.svg", "doctrine.svg", "dojo.svg", "dotcloud.svg", "dotnet.svg", "doubleclick.svg",
    "dreamfactory.svg", "dreamhost.svg", "dribbble-icon.svg", "dribbble.svg", "drift.svg", "drip.svg", "drone.svg",
    "dropbox.svg", "dropmark.svg", "dropzone.svg", "drupal.svg", "duckduckgo.svg", "dyndns.svg", "eager.svg",
    "ebanx.svg", "eclipse.svg", "egghead.svg", "elasticbox.svg", "elasticsearch.svg", "electron.svg",
    "elemental-ui.svg", "elementary.svg", "ello.svg", "elm.svg", "elo.svg", "emacs.svg", "embedly.svg",
    "ember-tomster.svg", "ember.svg", "emmet.svg", "engine-yard.svg", "envato.svg", "envoyer.svg", "enyo.svg",
    "erlang.svg", "es6.svg", "esdoc.svg", "eslint-old.svg", "eslint.svg", "eta-lang.svg", "etcd.svg",
    "ethereum.svg", "ethnio.svg", "eventbrite.svg", "eventsentry.svg", "expo.svg", "exponent.svg",
    "express.svg", "fabric.svg", "fabric_io.svg", "facebook.svg", "falcor.svg", "famous.svg", "fastlane.svg",
    "fastly.svg", "feathersjs.svg", "fedora.svg", "firebase.svg", "firefox.svg", "flannel.svg", "flarum.svg",
    "flask.svg", "flat-ui.svg", "flattr.svg", "fleep.svg", "flexible-gs.svg", "flickr-icon.svg", "flickr.svg",
    "flight.svg", "flocker.svg", "floodio.svg", "flow.svg", "flowxo.svg", "floydhub.svg", "flux.svg", "fluxxor.svg",
    "fly.svg", "flyjs.svg", "fomo.svg", "forest.svg", "forever.svg", "formkeep.svg", "foundation.svg", "framed.svg",
    "freebsd.svg", "freedcamp.svg", "freedomdefined.svg", "frontapp.svg", "fsharp.svg", "galliumos.svg",
    "game-analytics.svg", "gatsby.svg", "gaugeio.svg", "geekbot.svg", "get-satisfaction.svg", "ghost.svg",
    "giantswarm.svg", "git-icon.svg", "git.svg", "gitboard.svg", "github-icon.svg", "github-octocat.svg",
    "github.svg", "gitkraken.svg", "gitlab.svg", "gitter.svg", "gitup.svg", "glamorous.svg", "gleam.svg",
    "glimmerjs.svg", "glint.svg", 
   "gnu.svg", 
    "gocd.svg", "google-2014.svg", "google-adsense.svg", "google-adwords.svg",
    "google-analytics.svg", "google-cloud.svg", "google-developers.svg", "google-drive.svg", "google-gmail.svg",
    "google-icon.svg", "google-photos.svg", "google-plus.svg", "google.svg", "gopher.svg", "gordon.svg", "gradle.svg",
    "grails.svg", "grape.svg", "graphcool.svg", "graphene.svg", "graphql.svg", "gratipay.svg", "grav.svg", "gravatar.svg",
    "graylog.svg", "groovehq.svg", 
    "grove.svg", 
    "grunt.svg", "gulp.svg", "gunicorn.svg", "gusto.svg", "gwt.svg", "hack.svg",
    "hacker-one.svg", "hadoop.svg", "haml.svg", "handlebars.svg", 
     "hapi.svg",   // <<<<<<<< INTERESTING ... CPU intensive ? 
    "harrow.svg", "hashnode.svg", "haskell.svg",
    "hasura.svg", "haxe.svg", "haxl.svg", "hbase.svg", "heap.svg", "helpscout.svg", "heroku-redis.svg", "heroku.svg", "heron.svg",
    "hexo.svg", "hhvm.svg", "hibernate.svg", "highcharts.svg", "hipchat.svg", "hipercard.svg", "hoa.svg", "hoodie.svg",
    "horizon.svg", "hosted-graphite.svg", "hostgator.svg", "houndci.svg", "html-5.svg", "html5-boilerplate.svg", "hubspot.svg",
    "humongous.svg", "hyper.svg", "hyperapp.svg", "hyperdev.svg", "ibm.svg", "ieee.svg", "ifttt.svg", "imagemin.svg",
    "immutable.svg", "impala.svg", "importio.svg", "infer.svg", "inferno.svg", "influxdb.svg", "ink.svg", "instagram-icon.svg",
    "instagram.svg", "intellij-idea.svg", "intercom.svg", "internetexplorer.svg", "invision.svg", "io.svg", "ionic.svg",
    "ios.svg", "iron.svg",
   "itsalive.svg", 
    "jade.svg", "jamstack.svg", "jasmine.svg", "java.svg", "javascript.svg", "jcb.svg",
    "jekyll.svg", "jelastic.svg", "jenkins.svg", "jest.svg", "jetbrains.svg", "jhipster.svg", "jira.svg", "joomla.svg",
    "jquery-mobile.svg", "jquery.svg", "jruby.svg", "jsbin.svg", "jscs.svg", "jsdelivr.svg", "jsfiddle.svg", "json.svg",
    "jspm.svg", "juju.svg", "kafka.svg", "kallithea.svg", "karma.svg", "keen.svg", "kemal.svg", "keycdn.svg", "keymetrics.svg",
    "keystonejs.svg", "khan_academy.svg", "kibana.svg", "kickstarter.svg", 
"kinto.svg",
 "kinvey.svg", "kirby.svg",
    "kissmetrics-monochromatic.svg", "kissmetrics.svg", "kitematic.svg", "kloudless.svg", "knex.svg",
    "knockout.svg",
    "kong.svg", "kontena.svg", "kore.svg", "koreio.svg", "kotlin.svg", "kraken.svg", "krakenjs.svg", "kubernets.svg",
    "laravel.svg", "lastfm.svg", "lateral.svg", "launchkit.svg", "launchrock.svg", "leafjet.svg", "leankit.svg", "less.svg",
    "letsencrypt.svg", "leveldb.svg", "librato.svg", "liftweb.svg", "lighttpd.svg", "linkedin.svg", "linkerd.svg",
    "linode.svg", "linux-mint.svg", "linux-tux.svg", "litmus.svg", "loader.svg", "locent.svg", "lodash.svg", "logentries.svg",
    "loggly.svg", "logmatic.svg", "lookback.svg", "looker.svg", "loopback.svg", "losant.svg", "lotus.svg", "lua.svg",
    "lucene.net.svg", "lucene.svg", "lumen.svg", "lynda.svg", "macosx.svg", "maestro.svg", "mageia.svg", "magento.svg",
    "magneto.svg", "mailchimp-freddie.svg", "mailchimp.svg", "maildeveloper.svg", "mailgun.svg", "mandrill-shield.svg",
    "mandrill.svg", "manifoldjs.svg", "mantl.svg", "manuscript.svg", "mapbox.svg", "maps-me.svg", "mapzen.svg",
    "mariadb.svg", "marionette.svg", "markdown.svg", "marko.svg", "marvel.svg", "mastercard.svg", "material-ui.svg",
    "materializecss.svg", "mattermost.svg", "maxcdn.svg", "mdn.svg", "meanio.svg", "medium.svg", "memcached.svg",
    "memsql.svg", "mention.svg", "mercurial.svg", "mern.svg", "mesos.svg", "mesosphere.svg", "message.io.svg",
    "metabase.svg", "meteor-icon.svg", "meteor.svg", "microcosm.svg", "microsoft-edge.svg", "microsoft-windows.svg",
    "microsoft.svg", "middleman.svg", "milligram.svg", "mist.svg", "mithril.svg", "mixmax.svg", "mixpanel.svg",
    "mobx.svg", "mocha.svg", "mockflow.svg", "modernizr.svg", "modulus.svg", "modx.svg", "momentjs.svg", "monero.svg",
    "mongodb.svg", "mongolab.svg", "mono.svg", "moon.svg", "mootools.svg", "morpheus.svg", "mozilla.svg",
    "mparticle.svg", "mysql.svg", "myth.svg", "namecheap.svg", "nanonets.svg", "nativescript.svg", "neat.svg",
    "neo4j.svg", "neonmetrics.svg", "neovim.svg", "netbeans.svg", "netflix-icon.svg", "netflix.svg",
    "netlify.svg", "netuitive.svg", "new-relic.svg", "nextjs.svg", "nginx.svg", "nightwatch.svg",
    "nodal.svg", "node-sass.svg", "nodebots.svg", "nodejitsu.svg", "nodejs-icon.svg", "nodejs.svg",
    "nodemon.svg", "nodeos.svg", "nodewebkit.svg", "nomad.svg", "noysi.svg", "npm-2.svg", "npm.svg",
    "nuclide.svg", "nuodb.svg", "nuxt.svg", "oauth.svg", "ocaml.svg", "octodns.svg", "olapic.svg",
    "olark.svg", "onesignal.svg", "opbeat.svg", "open-graph.svg", "opencart.svg", "opencollective.svg",
    "opencv.svg", "opengl.svg", "openlayers.svg", "openshift.svg", "opensource.svg", "openstack.svg",
    "opera.svg", "opsee.svg", "opsgenie.svg", "opsmatic.svg", "optimizely.svg", "oracle.svg", "oreilly.svg",
    "origami.svg", "origin.svg", "oshw.svg", "osquery.svg", "otto.svg", "packer.svg", "pagekit.svg",
    "pagekite.svg", "panda.svg", "parse.svg", "parsehub.svg", "passbolt.svg", "passport.svg", "patreon.svg",
    "paypal.svg", "peer5.svg", "pepperoni.svg", "percona.svg", "percy.svg", "perf-rocks.svg", "periscope.svg",
    "perl.svg", "phalcon.svg", "phoenix.svg", "phonegap-bot.svg", "phonegap.svg", "php.svg", "phpstorm.svg",
    "picasa.svg", "pingdom.svg", "pingy.svg", "pinterest.svg", "pipedrive.svg", "pipefy.svg", "pivotal_tracker.svg",
    "pixate.svg", "pixelapse.svg", "pkg.svg", "planless.svg", "plastic-scm.svg", "platformio.svg", "play.svg", "pm2.svg",
    "podio.svg", "polymer.svg", "positionly.svg", "postcss.svg", "postgresql.svg", "postman.svg", "pouchdb.svg", "preact.svg",
    "precursor.svg", "prestashop.svg", "presto.svg", "prettier.svg", "processwire-icon.svg", "processwire.svg",
    "productboard.svg", "producteev.svg", "producthunt.svg", "progress.svg", "prometheus.svg", "promises.svg",
    "proofy.svg", "prospect.svg", "protactor.svg", "protoio.svg", "protonet.svg", "prott.svg", "pug.svg",
    "pumpkindb.svg", "puppet.svg", "puppy-linux.svg", "pushbullet.svg", "pusher.svg", "pycharm.svg", "python.svg",
    "pyup.svg", "q.svg", "qordoba.svg", "qt.svg", "quay.svg", "quobyte.svg", "quora.svg", "r-lang.svg", "rabbitmq.svg",
    "rackspace.svg", "rails.svg", "ramda.svg", "raml.svg", "rancher.svg", "randomcolor.svg", "raphael.svg", "raspberry-pi.svg",
    "rax.svg", "react-router.svg", "react.svg", "reactivex.svg", "realm.svg", "reapp.svg", "reasonml.svg", "recast.ai.svg",
    "reddit.svg", "redhat.svg", "redis.svg", "redsmin.svg", "redspread.svg", "redux-observable.svg", "redux-saga.svg",
    "redux.svg", "refactor.svg", "reindex.svg", "relay.svg", "remergr.svg", "require.svg", "rest-li.svg", "rest.svg",
    "rethinkdb.svg", "riak.svg", "riot.svg", "rkt.svg", "rocket-chat.svg", "rocksdb.svg", "rollbar.svg", "rollup.svg",
    "rollupjs.svg", "rsa.svg", "rsmq.svg", "rubocop.svg", "ruby.svg", "rubygems.svg", "rubymine.svg", "rum.svg",
    "run-above.svg", "runnable.svg", "runscope.svg", "rust.svg", "rxdb.svg", "safari.svg", "sagui.svg", "sails.svg",
    "salesforce.svg", "saltstack.svg", "sameroom.svg", "sanity.svg", "sass-doc.svg", "sass.svg", "saucelabs.svg", "scala.svg",
    "scaledrone.svg", "scaphold.svg", "scribd.svg", "sectionio.svg", "segment.svg", "selenium.svg", "semantic-ui.svg",
    "semaphore.svg", "sencha.svg", "sendgrid.svg", "seneca.svg", "sensu.svg", "sentry.svg", "sequelize.svg", "serveless.svg",
    "sherlock.svg", "shields.svg", "shipit.svg", "shippable.svg", "shogun.svg", "shopify.svg", "sidekick.svg", "sidekiq.svg",
    "sinatra.svg", "siphon.svg", "sitepoint.svg", "sketch.svg", "sketchapp.svg", "skylight.svg", "skype.svg", "slack.svg",
    "slides.svg", "slim.svg", "smashingmagazine.svg", "snap-svg.svg", "sninnaker.svg", "snupps.svg", "snyk.svg",
    "socket.io.svg", "solr.svg", "soundcloud.svg", "sourcegraph.svg", "sourcetree.svg", "spark.svg", "sparkcentral.svg",
    "sparkpost.svg", "speakerdeck.svg", "speedcurve.svg", "spring.svg", "sqldep.svg", "sqlite.svg", "square.svg",
    "squarespace.svg", "stackoverflow.svg", "stackshare.svg", "stacksmith.svg", "stash.svg", "statuspage.svg", "steam.svg",
    "steemit.svg", "steroids.svg", "stetho.svg", "stickermule.svg", "stoplight.svg", "stormpath.svg", "strider.svg",
    "stripe.svg", "strongloop.svg", "struts.svg", "styleci.svg", "stylefmt.svg", "stylelint.svg", "stylus.svg",
    "subversion.svg", "sugarss.svg", "supergiant.svg", "supersonic.svg", "supportkit.svg", "surge.svg", "survicate.svg",
    "suse.svg", "susy.svg", "svg.svg", "swift.svg", "swiftype.svg", "symfony.svg", "sysdig.svg", "t3.svg", "taiga.svg",
    "tapcart.svg", "targetprocess.svg", "tastejs.svg", "tealium.svg", "teamgrid.svg", "teamwork.svg", "tectonic.svg",
    "terminal.svg", "terraform.svg", "testlodge.svg", "testmunk.svg", "thimble.svg", "titon.svg", "todoist.svg", "todomvc.svg",
    "tomcat.svg", "torus.svg", "traackr.svg", "trac.svg", "trace.svg", "travis-ci-monochrome.svg", "travis-ci.svg",
    "treasuredata.svg", "treehouse.svg", "trello.svg", "tsu.svg", "tsuru.svg", "tumblr-icon.svg", "tumblr.svg", "tunein.svg",
    "turret.svg", "tutsplus.svg", "tutum.svg", "tux.svg", "twilio.svg", "twitch.svg", "twitter.svg", "typeform.svg",
    "typescript-icon.svg", "typescript.svg", "ubuntu.svg", "udacity.svg", "udemy.svg", "uikit.svg", "unbounce.svg",
    "undertow.svg", "unionpay.svg", "unitjs.svg", "unito.svg", "unity.svg", "upcase.svg", "upwork.svg", "user-testing.svg",
    "uservoice.svg", "v8-ignition.svg", "v8-turbofan.svg", "v8.svg", "vaadin.svg", "vaddy.svg", "vagrant.svg", "vault.svg",
    "vernemq.svg", "victorops.svg", "vim.svg", "vimeo-icon.svg", "vimeo.svg", "vine.svg", "visa.svg", "visaelectron.svg",
    "visual-studio-code.svg", "visual-studio.svg", "visual_website_optimizer.svg", "vivaldi.svg", "void.svg",
    "vue.svg", "vuetifyjs.svg", "vulkan.svg", "vultr.svg", "w3c.svg", "waffle.svg"
];


px.import({ scene:      'px:scene.1.js',
             http:      'http',
             keys:      'px:tools.keys.js',
}).then( function importsAreReady(imports)
{
  var scene = imports.scene;
  var keys  = imports.keys;
  var root  = imports.scene.root;
  var http  = imports.http;


  var base    = px.getPackageBaseFilePath();
  var fontRes = scene.create({ t: "fontResource", url: "FreeSans.ttf" });

  var bg = scene.create({t:"rect", parent: root, x:  0, y: 0, w: 100, h: 100, fillColor: 0xCCCCCCff, a: 1.0, id: "Background"});

  var title = scene.create({t:"textBox", text: "", parent: bg, pixelSize: 15, w: 800, h: 80, x: 0, y: 0,
                alignHorizontal: scene.alignHorizontal.LEFT, interactive: false,
                alignVertical:     scene.alignVertical.CENTER, textColor: 0x000000AF, a: 1.0});


  var startAtFile = 288;
  var file = "";

  var logo = null;
  var index = startAtFile;
  var prefix = 'https://raw.githubusercontent.com/gilbarbara/logos/master/logos/';

  var svgImage = null;

  var paused = false;

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //
  //    drawLogo
  //
  function drawLogo()
  {
        if(index >= files.length) 
        {
            console.log("Looping\nLooping\nLooping\nLooping\n ");
            index = 0;
        }

        file = files[index++];

        console.log("Loading [ "+file+" ]  ("+(index - 1)+" of "+files.length+") ...");

        var url = prefix + file;

      //  url = "//Users/hfitzp200/admob.svg"; // JUNK

        // Clean up...
        if(svgImage != null)
        {
            svgImage.remove();
            svgImage = null;
        }

        svgImage = scene.create({t:"image", url:url, parent:root});

        svgImage.ready.then(function(o)
        {
            svgImage.cx = svgImage.resource.w / 2;
            svgImage.cy = svgImage.resource.h / 2;

            console.log("SVG >> WxH " + svgImage.resource.w + " x " + svgImage.resource.h);
 
            svgImage.sx = 1.0;
            svgImage.sy = 1.0;

            svgImage.x = (bg.w - svgImage.resource.w) /2;
            svgImage.y = (bg.h - svgImage.resource.h) /2;

            title.text = "("+(index - 1)+" of "+files.length+")" + "  WxH: " + svgImage.resource.w + " x " + svgImage.resource.h + "    " + file ;

            setTimeout(drawLogo, 2200);  // NEXT !!!!
        },
        function(o) 
        {
          console.log("ERROR - moving on ...");
          setTimeout(drawLogo, 2500);  // NEXT !!!!
        });

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  function updateSize(w,h)
  {
     bg.w = w;
     bg.h = h;
  }

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  scene.on("onResize", function(e) { updateSize(e.w, e.h); });

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  Promise.all([ bg.ready, title.ready ])
      .catch( (err) =>
      {
          console.log("SVG >> Loading Assets ... err = " + err);
      })
      .then( (success, failure) =>
      {
          updateSize(scene.w, scene.h);

          title.x = 10;
          title.y = bg.h - title.h;

          drawLogo();  // <<<<<< RUN TESTS !!!

          bg.focus = true;
      });

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}).catch( function importFailed(err){
  console.error("SVG >> Import failed for LogosSVG.js: " + err);
});
