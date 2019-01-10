"use strict";


/**
 * This test runs a series of steps over a set of real news stories in var 'textStories'.
 * 
 * The test will display the text in three locations on screen with various pixelSizes.
 * 
 * Usage: 
 * - Run this test with no parameters to cycle through all the titles/text of the stories (there are 30).
 * - Pass in '?index={number}' to run the test over just a single selection of text. This is helpful
 *   in debugging when a particular behavior is only seen with certain text.
 * 
 * */
var textStories = {
	"stories": [{
		"fullDescription": "Michael Phelps was defeated in 2012 by South Africa's Chad Le Clos. On Tuesday night, they'll compete for the gold.",
		"imageUrl": "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
		"shortDescription": "Rematch in Rio: Phelps vs. Le Clos",
		"startTimeMillis": 1470759667636,
		"articleText": "Michael Phelps was defeated in 2012 by South Africa's Chad Le Clos. On Tuesday night, they'll compete for the gold.",
		"type": "Rio Olympics",
		"articleUrl": "http://my.xfinity.com/articles/sports-general/20160809/OLY-SWM-Rematch-In-Rio/"
	}, {
		"fullDescription": "After Monday's massive system outage, another 250 flights have been canceled and Delta has offered passengers refunds.",
		"imageUrl": "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
		"shortDescription": "Delta Airlines Cancels 250 More Flights",
		"startTimeMillis": 1470759638430,
		"articleText": "After Monday’s massive system outage, another 250 flights have been canceled and Delta has offered passengers refunds.",
		"type": "US News",
		"articleUrl": "http://www.nbcnews.com/business/travel/delta-outage-more-pain-store-some-travelers-tuesday-n626006"
	}, {
		"fullDescription": "After six seasons, Taran Killam and Jay Pharoah will not rejoin SNL in the fall for the sketch comedy's 42nd season.",
		"imageUrl": "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
		"shortDescription": "Two SNL Cast Members Are Moving On",
		"startTimeMillis": 1470759607246,
		"articleText": "After six seasons, Taran Killam and Jay Pharoah will not rejoin SNL in the fall for the sketch comedy's 42nd season.",
		"type": "Entertainment",
		"articleUrl": "http://my.xfinity.com/articles/entertainment-eonline/20160809/b786240/?cid=enttab_media_snl"
	}, {
		"fullDescription": "The 30th player to reach 3,000 hits, Ichiro Suzuki has donated 3,000 items, including cleats and batting gloves.",
		"imageUrl": "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
		"shortDescription": "Ichiro Suzuki Donates Souvenirs to Hall of Fame",
		"startTimeMillis": 1470757861652,
		"articleText": "The 30th player to reach 3,000 hits, Ichiro Suzuki has donated 3,000 items, including cleats and batting gloves.",
		"type": "Baseball",
		"articleUrl": "http://my.xfinity.com/articles/sports-mlb/20160808/BBN--Ichiro-3000/"
	}, {
		"fullDescription": "Hillary Clinton holds a 10-point lead over Donald Trump, 51 percent to 41 percent, according to the latest NBC poll.",
		"imageUrl": "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
		"shortDescription": "Clinton Leads in Latest Polls",
		"startTimeMillis": 1470758277089,
		"articleText": "Hillary Clinton holds a 10-point lead over Donald Trump, 51 percent to 41 percent, according to the latest NBC poll.",
		"type": "U.S. News",
		"articleUrl": "http://www.nbcnews.com/politics/2016-election/poll-clinton-opens-double-digit-lead-over-trump-n625676"
	}, {
		"fullDescription": "Justin Timberlake shared a picture of all five ‘N Sync members getting together to celebrate JC Chasez’s 40th birthday.",
		"imageUrl": "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
		"shortDescription": "'N Sync Reunites for JC Chasez's Birthday",
		"startTimeMillis": 1470759962180,
		"articleText": "Justin Timberlake shared a picture of all five ‘N Sync members getting together to celebrate JC Chasez’s 40th birthday.",
		"type": "Entertainment",
		"articleUrl": "http://my.xfinity.com/articles/entertainment-eonline/20160809/b786265/?cid=enttab_media_nsync"
	}, {
		"fullDescription": "The 19-year-old set a new Olympic record to beat a late-charging Yulia Efimova of Russia in the 100m breaststroke.",
		"imageUrl": "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
		"shortDescription": "American Lilly King Tops Russian Rival",
		"startTimeMillis": 1470759152379,
		"articleText": "The 19-year-old set a new Olympic record to beat a late-charging Yulia Efimova of Russia in the 100m breaststroke.",
		"type": "Rio Olympics",
		"articleUrl": "http://www.nbcolympics.com/news/lilly-king-katie-meili-land-olympic-breaststroke-podium-their-first-games"
	}, {
		"fullDescription": "Fencer Ibtihaj Muhammad became the only American ever to compete in the Olympics wearing a hijab.",
		"imageUrl": "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
		"shortDescription": "Muslim American Makes Olympic History",
		"startTimeMillis": 1470759160633,
		"articleText": "Fencer Ibtihaj Muhammad became the only American ever to compete in the Olympics wearing a hijab.",
		"type": "Rio Olympics",
		"articleUrl": "http://my.xfinity.com/articles/sports-general/20160808/OLY--Olympics.Rdp/"
	}, {
		"fullDescription": "\"The Simpsons\" is doubling down with the series' first-ever hour-long episode in January.",
		"imageUrl": "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
		"shortDescription": "'The Simpsons' Prepares for a New First",
		"startTimeMillis": 1470759227047,
		"articleText": "\"The Simpsons\" is doubling down with the series' first-ever hour-long episode in January.",
		"type": "Entertainment",
		"articleUrl": "http://my.xfinity.com/articles/entertainment/20160808/US--TV-The.Simpsons/"
	}, {
		"fullDescription": "Shaking off a sluggish start, the Americans romped over Venezuela 113-69 on Monday.",
		"imageUrl": "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
		"shortDescription": "U.S. Men's Basketball Blasts Venezuela",
		"startTimeMillis": 1470759235698,
		"articleText": "Shaking off a sluggish start, the Americans romped over Venezuela 113-69 on Monday.",
		"type": "Rio Olympics",
		"articleUrl": "http://my.xfinity.com/articles/sports-general/20160809/OLY--BKO-US-Venezuela/"
	},{
      "fullDescription" : "Jack Jakubek, a former SUNY-Cortland swimmer, died during a lifeguard fitness test in Massachusetts over the weekend.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Ex-College Swimmer Dies During Lifeguard Test",
      "startTimeMillis" : 1464712401156,
      "articleText" : "Jack Jakubek, a former SUNY-Cortland swimmer, died during a lifeguard fitness test in Massachusetts over the weekend.",
      "type" : "Sports",
      "articleUrl" : "http://www.nbcnews.com/news/us-news/jack-jakubek-suny-cortland-swim-star-dies-during-lifeguard-test-n582901"
    }, {
      "fullDescription" : "A lawyer for Amber Heard called allegations that his client is blackmailing Johnny Depp \"unequivocally false.\"",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Amber Heard's Lawyer Slams Accusation",
      "startTimeMillis" : 1464711359448,
      "articleText" : "A lawyer for Amber Heard called allegations that his client is blackmailing Johnny Depp \"unequivocally false.\"",
      "type" : "Entertainment",
      "articleUrl" : "http://my.xfinity.com/articles/entertainment-eonline/20160531/b768884/"
    }, {
      "fullDescription" : "The State Department wants Americans traveling to Europe over the summer to know more about the risk of terror attacks.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "State Department Issues Warning About Travel",
      "startTimeMillis" : 1464711365876,
      "articleText" : "The State Department wants Americans traveling to Europe over the summer to know more about the risk of terror attacks.",
      "type" : "U.S. News",
      "articleUrl" : "http://www.nbcnews.com/news/us-news/state-department-issues-new-travel-alert-europe-n583161"
    }, {
      "fullDescription" : "The trial that cites soccer star Lionel Messi in a tax fraud case against Spain began Tuesday without Messi present.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Messi Skips Court in Tax Fraud Case",
      "startTimeMillis" : 1464711372384,
      "articleText" : "The trial that cites soccer star Lionel Messi in a tax fraud case against Spain began Tuesday without Messi present.",
      "type" : "Sports",
      "articleUrl" : "http://my.xfinity.com/articles/sports-general/20160531/SOC--Spain-Messi.Trial/"
    }, {
      "fullDescription" : "Two Hollywood directors backed Keira Knightley after filmmaker John Carney made comments slamming the actress' talent.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Filmmakers Defend Knightley After Criticism",
      "startTimeMillis" : 1464712442808,
      "articleText" : "Two Hollywood directors backed Keira Knightley after filmmaker John Carney made comments slamming the actress' talent.",
      "type" : "Entertainment",
      "articleUrl" : "http://my.xfinity.com/articles/entertainment-eonline/20160531/b768921/"
    }, {
      "fullDescription" : "The Warriors rallied from a 3-1 series deficit to beat the Thunder in game seven of the Western Conference Finals.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Golden State Punches Ticket to Finals in Style",
      "startTimeMillis" : 1464711291547,
      "articleText" : "The Warriors rallied from a 3-1 series deficit to beat the Thunder in game seven of the Western Conference Finals.",
      "type" : "Sports",
      "articleUrl" : "http://my.xfinity.com/articles/sports-general/20160531/BKN-Thunder-Warriors/"
    }, {
      "fullDescription" : "Public outcry continued in the form of protests at the Cincinnati Zoo, where a gorilla was killed to protect a child.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Outrage Grows over Zoo's Killing of Gorilla",
      "startTimeMillis" : 1464711299103,
      "articleText" : "Public outcry continued in the form of protests at the Cincinnati Zoo, where a gorilla was killed to protect a child.",
      "type" : "U.S. News",
      "articleUrl" : "http://www.nbcnews.com/news/us-news/outrage-grows-after-gorilla-harambe-shot-dead-cincinnati-zoo-save-n582706"
    }, {
      "fullDescription" : "A hacker reportedly got into the singer's Twitter account and began posting cryptic messages over the weekend.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Truth Behind Katy Perry's Nasty Tweets Revealed",
      "startTimeMillis" : 1464711305709,
      "articleText" : "A hacker reportedly got into the singer's Twitter account and began posting cryptic messages over the weekend.",
      "type" : "Entertainment",
      "articleUrl" : "http://my.xfinity.com/articles/entertainment-eonline/20160531/b768889/"
    }, {
      "fullDescription" : "Physicist Stephen Hawking said in an interview that GOP nominee Donald Trump appeals to the \"lowest common denominator.\"",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Hawking: I Can't Explain the Trump Phenomenon",
      "startTimeMillis" : 1464711311992,
      "articleText" : "Physicist Stephen Hawking said in an interview that GOP nominee Donald Trump appeals to the \"lowest common denominator.\"",
      "type" : "Politics",
      "articleUrl" : "http://www.nbcnews.com/politics/2016-election/stephen-hawking-donald-trump-appeals-lowest-common-denominator-n583026"
    }, {
      "fullDescription" : "Officials say a North Korea launch of a mid-range missile failed Tuesday when the missile exploded on the launch pad.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Officials: North Korea Missile Launch Failed",
      "startTimeMillis" : 1464711317353,
      "articleText" : "Officials say a North Korea launch of a mid-range missile failed Tuesday when the missile exploded on the launch pad.",
      "type" : "World News",
      "articleUrl" : "http://www.nbcnews.com/news/world/north-korea-s-test-launch-musudan-missile-ends-failure-south-n582911"
    },{
      "fullDescription" : "Toyota has recalled 3.37 million cars worldwide over possible air bag and emissions control unit defects.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Toyota Recalls More Than 3 Million Vehicles",
      "startTimeMillis" : 1467210177414,
      "articleText" : "Toyota has recalled 3.37 million cars worldwide over possible air bag and emissions control unit defects.",
      "type" : "World News",
      "articleUrl" : "http://www.nbcnews.com/business/business-news/toyota-recalls-3-37-million-cars-over-airbag-emissions-control-n600916"
    }, {
      "fullDescription" : "Joey Fatone is opening a new hot dog stand in the Florida Mall in Orlando later this year, NSYNC's Twitter confirmed.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "NSYNC Star Opening Mall Hot Dog Stand",
      "startTimeMillis" : 1467210223704,
      "articleText" : "Joey Fatone is opening a new hot dog stand in the Florida Mall in Orlando later this year, NSYNC's Twitter confirmed.",
      "type" : "Entertainment",
      "articleUrl" : "http://my.xfinity.com/articles/entertainment-eonline/20160628/b776402/"
    }, {
      "fullDescription" : "Luke Gatti, the ex-UConn student whose rant on jalapeño-bacon mac and cheese went viral, was arrested again in Florida.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Mac-and-Cheese Rant Student Arrested Again",
      "startTimeMillis" : 1467210245544,
      "articleText" : "Luke Gatti, the ex-UConn student whose rant on jalapeño-bacon mac and cheese went viral, was arrested again in Florida.",
      "type" : "U.S. News",
      "articleUrl" : "http://www.nbcnews.com/feature/college-game-plan/mac-cheese-rant-ex-uconn-student-luke-gatti-arrested-again-n600846"
    }, {
      "fullDescription" : "The Dodgers have sent ace Clayton Kershaw home to have his sore back tested, though he hopes to make his next start.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Dodgers Send Kershaw Home for Tests",
      "startTimeMillis" : 1467210252904,
      "articleText" : "The Dodgers have sent ace Clayton Kershaw home to have his sore back tested, though he hopes to make his next start.",
      "type" : "Sports",
      "articleUrl" : "http://my.xfinity.com/articles/sports-mlb/20160628/20160628191233219164408/"
    }, {
      "fullDescription" : "Scotty Moore, a pioneering rock guitarist best known for backing Elvis, has died at the age of 84, Moore's website says.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Elvis Presley's Guitarist Dies at 84",
      "startTimeMillis" : 1467210259111,
      "articleText" : "Scotty Moore, a pioneering rock guitarist best known for backing Elvis, has died at the age of 84, Moore's website says.",
      "type" : "Entertainment",
      "articleUrl" : "http://www.nbcnews.com/pop-culture/music/scotty-moore-elvis-presley-s-groundbreaking-guitarist-dies-84-n600821"
    }, {
      "fullDescription" : "Clinton said it is \"time to move on\" after a Congressional report on Benghazi found no new evidence of wrongdoing.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Hillary Clinton Reacts to New Benghazi Report",
      "startTimeMillis" : 1467210265065,
      "articleText" : "Clinton said it is \"time to move on\" after a Congressional report on Benghazi found no new evidence of wrongdoing.",
      "type" : "U.S. News",
      "articleUrl" : "http://www.nbcnews.com/politics/2016-election/hillary-clinton-time-move-after-benghazi-report-n600511"
    }, {
      "fullDescription" : "Tony Hawk landed a 900 17 years to the day after the skateboarding trick made him a household name.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Tony Hawk Recreates Breakthrough Trick at 48",
      "startTimeMillis" : 1467210271350,
      "articleText" : "Tony Hawk landed a 900 17 years to the day after the skateboarding trick made him a household name.",
      "type" : "Sports",
      "articleUrl" : "http://my.xfinity.com/articles/entertainment/20160628/US--Tony.Hawk-900.At.48/"
    }, {
      "fullDescription" : "Court records show Presley has filed for divorce from guitarist Michael Lockwood, her husband of more than 10 years.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Lisa Marie Presley Files for Divorce",
      "startTimeMillis" : 1467210279608,
      "articleText" : "Court records show Presley has filed for divorce from guitarist Michael Lockwood, her husband of more than 10 years.",
      "type" : "Entertainment",
      "articleUrl" : "http://my.xfinity.com/articles/entertainment/20160628/US--People-Lisa.Marie.Presley/"
    }, {
      "fullDescription" : "2012 gold medalist Missy Franklin struggled to a seventh-place finish in the 100 backstroke at the Olympic trials.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Franklin Misses Chance to Defend Gold Medal",
      "startTimeMillis" : 1467210284986,
      "articleText" : "2012 gold medalist Missy Franklin struggled to a seventh-place finish in the 100 backstroke at the Olympic trials.",
      "type" : "Sports",
      "articleUrl" : "http://my.xfinity.com/articles/sports-general/20160629/OLY--SWM-US.Swim.Trials/"
    }, {
      "fullDescription" : "Seattle and wide receiver Doug Baldwin have agreed to a four-year contract extension that runs through the 2020 season.",
      "imageUrl" : "http://por-img.cimcontent.net/cms/data/assets/bin-201312/9a850b96f522318976faa191521c24e1.png",
      "shortDescription" : "Seahawks, Baldwin Agree to 4-Year Deal",
      "startTimeMillis" : 1467210291265,
      "articleText" : "Seattle and wide receiver Doug Baldwin have agreed to a four-year contract extension that runs through the 2020 season.",
      "type" : "Sports",
      "articleUrl" : "http://my.xfinity.com/articles/sports-nfl/20160629/FBN--Seahawks-Baldwin.Extension/"
    }]

};
px.import({scene:"px:scene.1.js",ws:'ws',keys:'px:tools.keys.js'}).then( function ready(imports) {

  var scene = imports.scene;
  var root = scene.root;
  var http = imports.http;
  var url = imports.url;
  var ws = imports.ws;
  var keys = imports.keys;
  
  // Allow parm to indicate a single story to show in specific position
  console.log("index "+px.appQueryParams.index);

  var titleFont = scene.create({t:"fontResource", url:"http://96.118.11.20/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-Med.ttf"});
  var bodyFont = scene.create({t:"fontResource", url:"http://96.118.11.20/pxscene-samples/examples/px-reference/fonts/XFINITYSansTT-New-MedCond.ttf"});
  
  var stories = textStories.stories;
  var storyStartIndex = 0;
  var storiesLen = stories.length;
  var storyIndex = storyStartIndex;
  var loopIndex = 0;

  // Container and widgets for news stories
  var overlayContainerNews = scene.create({t:"object",parent:root,a:1});
  var title = scene.create({t:"textBox",parent:overlayContainerNews,y:10,x:25,h:80,w:400,pixelSize:30,textColor:0xffffffff,
                            wordWrap:true,truncation:scene.truncation.TRUNCATE_AT_WORD, ellipsis:true,
                            font:titleFont});
  var body = scene.create({t:"textBox",parent:overlayContainerNews,x:25,y:95,h:300,w:400,pixelSize:20,wordWrap:true,
                          textColor:0xffffffff, clip:true, truncation:scene.truncation.TRUNCATE_AT_WORD,
                          font:bodyFont});

  var doIt = function() {
    console.log( "stories.length="+storiesLen);
    if(storyIndex < storiesLen) {
      title.text = stories[storyIndex].shortDescription;
      body.text = stories[storyIndex].fullDescription;
      overlayContainerNews.animateTo({a:1},0.5,scene.animation.TWEEN_LINEAR,scene.animation.LOOP,1).
        then(function(c) {
          storyIndex++;
          setTimeout(function() {
            overlayContainerNews.animateTo({a:0},0.5,scene.animation.TWEEN_LINEAR,scene.animation.LOOP,1).
              then(function(d) {
                doIt();
              });
            },500);
        });
    } else if(loopIndex === 0){
      loopIndex = 1;
      storyIndex = storyStartIndex;
      overlayContainerNews.x = 100;
      overlayContainerNews.y = 200;
      doIt();
    } else if( loopIndex === 1) {
      loopIndex = 2;
      storyIndex = storyStartIndex;
      overlayContainerNews.x = 300;
      overlayContainerNews.y = 400;
      title.pixelSize = 35;
      body.pixelSize = 25;
      body.clip = false;
      doIt();
    }
    
      
  };
  
  // Make sure both fonts are loaded before beginning
  titleFont.ready.then(function(a) {
    console.log("titleFont is ready");
    bodyFont.ready.then(function(b) {
      console.log("bodyFont is ready");

      if(px.appQueryParams.index !== undefined) {
        storyStartIndex = parseInt(px.appQueryParams.index);
        storyIndex = storyStartIndex;
        storiesLen = storyStartIndex+1;
        console.log("storiesLen = "+storiesLen);
      }
  
      doIt();
    });
  });
                          
}).catch( function importFailed(err){
  console.error("Imports failed for screensaver.js: " + err)
});
