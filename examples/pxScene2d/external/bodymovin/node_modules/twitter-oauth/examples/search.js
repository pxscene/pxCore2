

var express = require('express');
var http = require('http');
var path = require('path');

var config = {
        consumerKey: "sJaEIiV4X014iIC6uvzKDA", /* per appications - manage apps here: https://dev.twitter.com/apps */
     consumerSecret: "40wRAIu9a9bgAKWWltTP7NutSZxEMsPZJ5UhTZitmgw", /* per appications - manage apps here: https://dev.twitter.com/apps */
             domain: "http://YOURDOMAIN.com",
              login: "/twitter/sessions/connect",
             logout: "/twitter/sessions/logout",
      loginCallback: "/twitter/sessions/callback",  /* internal */
   completeCallback: "http://YOURDOMAIN.com/search/gosquared"  /* When oauth has finished - where should we take the user too */
};

var express = require('express');
twitterAuth = require('../server.js')(config);


var app = express();

app.configure(function(){
  app.set('port', 83);
});

app.use(express.cookieParser());
app.use(express.session({ secret: 'secret key' }));
app.use(express.bodyParser());

app.get('/', function(req, res){
  res.send('<a href="'+config.login+'">login with twitter</a>');
});

app.get('/search/:term', function(req, res){
  twitterAuth.search(req.params.term.split('|'),  req.session.oauthAccessToken, req.session.oauthAccessTokenSecret, function(error, data) {
    res.json(data);
  });
});

app.get('/mentions', function(req, res){
twitterAuth.retweets(req.session.oauthAccessToken, req.session.oauthAccessTokenSecret, function(error, data) {
    res.json(data);
  });
});

app.get('/user/:handle', function(req, res){
  twitterAuth.user(req.params.handle,  req.session.oauthAccessToken, req.session.oauthAccessTokenSecret, function(error, data) {
    res.json({
      error: error,
      data: data
    });
  });
});

app.get(config.login, twitterAuth.oauthConnect);
app.get(config.loginCallback, twitterAuth.oauthCallback);
app.get(config.logout, twitterAuth.logout);

http.createServer(app).listen(app.get('port'), function(){
  console.log("Express server listening on port " + app.get('port'));
});
