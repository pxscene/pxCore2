#About:

A simple API to work with twitters new (1.1) API. through oAuth with Express.js and Node.js.


Requests are cached in Redis for 60 seconds to avoid rate limits if you provide connection details.


##Installation:

npm install twitter-oauth


##Setup

You will need to ensure the following values are set correctly:

```
var twitterAuth = require('twitter-oauth')({
        consumerKey: "ENTER CONSUMER KEY HERE", /* per appication - create a comsumer key here: https://dev.twitter.com/apps */
        domain: 'YOUR DOMAIN HERE',
     consumerSecret: "ENTER CONSUMER SECRET FROM TWITTER HERE", /* create a comsumer key here: https://dev.twitter.com/apps */
      loginCallback: "http://yourdomain.com/twitter/sessions/callback",  /* internal */
   completeCallback:  "http://yourdomain.com/search/beagles"  /* When oauth has finished - where should we take the user too */
});
```

See examples for more details.


If a redis port and host are provided then each request will be cached for 60 seconds.

###Routes:

You will need to set up the routes:

```
app.get('/twitter/sessions/connect', twitterAuth.oauthConnect);
app.get('/twitter/sessions/callback', twitterAuth.oauthCallback);
app.get('/twitter/sessions/logout', twitterAuth.logout);
```


Then you can call the service like so:

```
  twitterAuth.search(req.params.term.split('|'),  req.session.oauthAccessToken, req.session.oauthAccessTokenSecret,  function(error, data) {
    res.json(data);
  });
```

See the search example to see all the possible searches.

Twitter 1.1

Twitter recently announced some changes to its API which resulted in us having to make some changes to our backend social service.

While the version number only went from 1 to 1.1 it actually contained some big changes.

To ensure our social offering was ready for twitter turning off its 1.0 API we build this library.

