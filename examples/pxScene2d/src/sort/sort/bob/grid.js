var fs = require('fs');

var MONTHS = ["January", "February", "March", "April", "May", "June", 
		"July", "August", "September", "October", "November", "December"];

function GridGuide(gridData, programData) {
	
	/*
	 * STARTUP PARAMETERS
	 */
	this._animateTime = 0.5;
	this._parentView = scene.create({t:"object", parent:scene.root, x:0, y:0, w:scene.root.w, h:scene.root.h});
	this._date = new Date("2015-06-18T05:59:45");	// current time... data is currently fixed on 6/18
	this._gridUnitWidth = 30;						// duration of each unit of time on grid
	this._gridUnits = 6;							// number of grid units to display 
	this._displayableChannels = 5;					// how many channels should be displayed concurrently
	this._channelWidth = 180;						// width in pixels of channel section of the grid.  Grid will occupy remaining width
	this._gridWidth = scene.root.w - this._channelWidth;	// width in pixels of program section of the grid.

	this._gridData = gridData;
	this._programData = programData;
	this._tickInterval = 1000;
	this._channels = this._gridData.getGridResponse.channels;
	
	this._dateText = null;
	this._timeBar = null;
	//this._gridUnitPixelWidth = (this._gridWidth - this._channelWidth) / (this._gridUnits -1);
	this._gridUnitPixelWidth = (this._gridWidth) / (this._gridUnits -1);
	this._times = new Array(this._gridUnits);
	this._timePointer = null;
	this._timesContainer = null;					// contains the times above the grid
	
	this._selectedChannelIndex = 0;					// which channel is currently selected
	this._selectedProgramIndex = 0;					// Which program is currently selected
	this._lastSelectedChannelIndex = -1;
	this._lastSelectedProgramIndex = -1;
	this._viewChannelStartIndex = 0;				// index of first channel at top of grid
	this._lastViewChannelStartIndex=-1;				// index of last channel at top of grid
	this._highlighter = null;
	
	this._gridContainer = null;
	this._channelView = null;
	this._programContainer = null;					// contains all of the programs
	this._programView = null;						// provides the clipped view through which programs are visible
	this._highlighterView = null;					// view that the highlighter floats on
	this._channelHeight = 0;						// height of channel in pixels
	this._originStartTime = this.calculateStartTime(this._date);	// the start time of the grid, which marks the origin, remains constant
	this._renderStartTime = this._originStartTime;			// the current rendered start time of the grid, 
															// shifts when program selection goes beyond the visible grid or when time changes
	this._lastRenderStartTime = -1					// last render start time
	this._pixelsPerms = 0;							// the number of pixels represented by a ms in time
	this._lastHorizStart = 0;						// start time of the last horizontal movement

	
	this._titleText = scene.create({t:"text", textColor:0xFF0000FF, pixelSize:48, parent:this._parentView, x:10, y:0, text:"Xfinity"});
	this._timeTempText = scene.create({t:"text", textColor:0xFF0000FF, pixelSize:36, parent:this._parentView, x:1000, y:10});

	this.createTimeBar({x:0, y:72, w:scene.root.w, h:50});
	this._programDetailView = new ProgramDetails(
		{x:0, y:500, w:scene.w, h:288},
		scene,
		scene.root,
		null);

	this.createGrid({x:0, y:122, w:scene.root.w, h:370}, scene.root);

	setInterval(this.tick, this._tickInterval, this);	// have to pass a reference to this... otherwise need to use closure
}

GridGuide.prototype.calculateStartTime = function() {
	// calculates the time floored to the nearest gridUnitWidth, example: if gridUnitWidth==30 and time is 6:48, returns 6:30
	var date = new Date(this._date.getTime());
	date.setMilliseconds(0);
	date.setSeconds(0);
	var mins = Math.floor((date.getMinutes()/ this._gridUnitWidth)) * this._gridUnitWidth;
	date.setMinutes(mins);
	console.log("calculateStartTime():  ", date);
	return date.getTime();
}

GridGuide.prototype.tick = function(self) {
	var d = self._date = new Date(self._date.getTime() + self._tickInterval);

	self._timeTempText.text = d.getHours() + ":" + ((d.getMinutes() < 10) ? ('0' + d.getMinutes()) : d.getMinutes()) 
		+ ":" + ((d.getSeconds() < 10) ? ('0' + d.getSeconds()) : d.getSeconds()) + " / 83\u00B0";
	self.updateTimeBar();
}

GridGuide.prototype.programChanged = function(program, channel) {
	var programUrn = program.programInfo.programUrn;
	programUrn = programUrn.substring(programUrn.lastIndexOf(':') +1);
	var data = this._programData[programUrn];
	var date = new Date(program.startTime);
	var startStr = simpleDateFormat(date);
	date = new Date((parseInt(program.inWindowMinutes) * 60 * 1000) + parseInt(program.startTime));
	var endStr = simpleDateFormat(date);
	var imgurl = process.cwd() + "/bob/ql.jpg";
	console.log("data['p$mainImages'] : ", data["p$mainImages"]);

	var s = data["p$mediumSynopsis"];

	// let's manually word wrap the long text
	if (s) {
		var l = s.length;
		var i = 40;
		while (l > i) {
			var space = s.indexOf(' ', i);
			if (space < 0) break;
			var s1 = s.substring(0, space);
			var s2 = s.substring(space);
			if (s1.length < 120) {
				s = s1 + "\n" + s2;
			} else {
				s = s1;
			}
			i = space + 40;
		}
	}
	
		
	var pd = {
		imageUrl:imgurl, 			//data["p$mainImages"].main["media$url"],	// need to do null checks
		title:data["p$longTitle"],
		episode:data["e$episodeTitle"],
		description:s,
		startTime:startStr,
		endTime:endStr,
		channelNumber:channel.number,
		channelShortName:channel.stationInfo.title,
		hd:channel.stationInfo.hd,
		cc:program.captionType,
		rating:(program.contentRating) ? program.contentRating.rating : ""
	};
	this._programDetailView.setData(pd);
}

/*
 * Time Bar
 */
GridGuide.prototype.createTimeBar = function(rect) {
	var container = scene.create({t:"object", parent:this._parentView, x:rect.x, y:rect.y, w:rect.w, h:rect.h});
	this._dateText = scene.create({t:"text", parent:container, textColor:0xCCCCCCFF, pixelSize:24, x:10, y:0}); 
	this._timeBar = scene.create({t:"rect", parent:container, x:this._channelWidth, y:47, w:100, h:3, fillColor:0x0000FFFF});
	var timesViewPane = scene.create({t:"object", parent:container, x:this._channelWidth, y:0, w:this._gridWidth, h:rect.h, clip:true});
	this._timesContainer = scene.create({t:"object", parent:timesViewPane, x:0, y:0, h:rect.h});
	var timeWidth = this._gridWidth / this._gridUnits;
	for (var i=0; i<48; i++) {
		//var dx = timeWidth * i + this._channelWidth;
		var dx = timeWidth * i;
		this._times[i] = scene.create({t:"text", parent:this._timesContainer, x:dx, y:0, pixelSize:24, textColor:0xCCCCCCFF});
	}
	this._timePointer = scene.create({t:"image", parent:container, x:0, y:27, url:process.cwd() + "/bob/downArrow.png"});
	this.updateTimeBar();
	this.updateTimes()
}
 
GridGuide.prototype.updateTimeBar = function() {
	// update the time display
	var d = this._date;
	var dateString = MONTHS[d.getMonth()] + " " + d.getDate();
	this._dateText.text = dateString;
	//console.log("this._date, this._renderStartTime:  ", this._date, new Date(this._renderStartTime));
	// update the current time indicator and rectangular line associated with it
	//var tempTime = this._renderStartTime + (this._gridUnitWidth * 60 * 1000);
	//if (this._date.getTime() > tempTime) {
	if (this._date.getTime() > this._renderStartTime) {
	//console.log("CURRENT < TEMP - this._date.getTime(), tempTime:  ", this._date.getTime(), tempTime);
		var timeWidth = (((d.getMinutes() % 30) + (d.getSeconds() / 60) ) / this._gridUnitWidth ) * this._gridUnitPixelWidth;
		this._timeBar.w = timeWidth;
		this._timePointer.x = this._channelWidth + timeWidth - (this._timePointer.w/2);
		this._timeBar.a = 1;
		this._timePointer.a = 1;
	} else {
	//console.log("CURRENT >= TEMP - this._date.getTime(), tempTime:  ", this._date.getTime(), tempTime);
		this._timeBar.a = .0;
		this._timePointer.a = .0;
	}

	// TODO: Shift the full grid instead of changing only the time bar.
	// 		 Requires making a new selection if the current selection would be gone after the grid scrolls.
	if (d.getTime() >= (this._renderStartTime + this._gridUnitWidth * 60 * 1000)) {
		this._renderStartTime = this.calculateStartTime(this._date);
		//this.updateTimes();
		if (this._channels[this._selectedChannelIndex].stationInfo.listings[this._selectedProgramIndex].startTime < this._renderStartTime) {
			console.log("AM I HERE YET???");
			this._lastSelectedProgramIndex = this._selectedProgramIndex;
			this._selectedProgramIndex++;
		} 
		this.populateGrid(false);
		this.scrollGrid();
	}
 
 }
 
GridGuide.prototype.updateTimes = function() {
	var d = new Date(this._renderStartTime);
	var baseHour = d.getHours();
	var baseMinutes = d.getMinutes();
	for (var i=0; i<48; i++) {
		this._times[i].text = (baseHour > 12 ? baseHour -12 : baseHour) + ":" + (baseMinutes === 0 ? "00" : baseMinutes);
		if (baseMinutes == 30) {
			baseMinutes = 0;
			baseHour++;
		} else {
			baseMinutes = 30;
		}
	}
}

/*
 * Grid 
 */
GridGuide.prototype.createGrid = function (rect) {

	// create containers
	this._gridContainer = scene.create({t:"object", parent:this._parentView, x:rect.x, y:rect.y, w:rect.w, h:rect.h + 1, clip:true});
	this._channelView = scene.create({t:"object", parent:this._gridContainer, w:this._channelWidth});
	this._programContainer = scene.create({t:"object", parent:this._gridContainer, x:this._channelWidth, y:0, w:this._gridWidth, h:rect.h, clip:true});
	this._programView = scene.create({t:"object", parent:this._programContainer});
	this._highlighterView = scene.create({t:"object", parent:this._programContainer});
	this._channelHeight = rect.h / this._displayableChannels;
	
	this.populateGrid(true);

	var imgUrl = process.cwd() + "/bob/selector.png";
	console.log("this._selectedChannelIndex, this._selectedProgramIndex: ",this._selectedChannelIndex, this._selectedProgramIndex); 
	var first = this._channels[this._selectedChannelIndex].stationInfo.listings[this._selectedProgramIndex].ui;
	this.calculateHorizontalStart();
	this._highlighter = scene.create({t:"image9", x:first.x, y:first.y, w:first.w, h:first.h, parent:this._highlighterView,
								url:imgUrl, lInset:4, rInset:4, tInset:4, bInset:4});
	this._highlighter.animateTo({x:first.x, y:first.y, w:first.w, h:first.h}, .01, 0, 0);
	first.children[0].textColor=0xAAAAFFFF;
		
	// Adding key listener
	scene.root.on('onKeyDown', function(self) { 
		return function(e) {
		var valid = true;
		var vertical = false;
		switch(e.keyCode) {
			case 37:		// left
				{
					valid = self.horizontalMove(-1);
				}
				break;
			case 38:		// up
				{
					valid = self.verticalMove(-1);
					vertical = true;
				}
				break;
			case 39:		// right
				{
					valid = self.horizontalMove(1);
				}
				break;
			case 40:		// down
				{
					valid = self.verticalMove(1);
					vertical = true;
				}
				break;
			default:
				return;
		}
		console.log("Is the selection valid? ", valid);
		if (!valid) return;		
		self.selectNext(vertical);
	} }(this));
	this.selectNext(false);
}

GridGuide.prototype.populateGrid = function(needsSelection) {
	var endTime = this._renderStartTime + (this._gridUnitWidth * this._gridUnits * 60000);  // end time of the grid
	var timeSpan = endTime - this._renderStartTime;
	this._pixelsPerMs = (this._gridWidth * 1.0) / timeSpan;
		
	// populate channels
	for (var i=this._viewChannelStartIndex; i<this._viewChannelStartIndex + this._displayableChannels; i++) {
		var channel = this._channels[i];
		var listings = channel.stationInfo.listings;
		this.createChannel(i, channel);
		var firstIndex = 0;
		var lastIndex = 0;
		for (var j=0; j<listings.length; j++) {
			//console.log("** channel.number, listings[j].startTime, this._renderStartTime, endTime, minutes:", channel.number, listings[j].startTime, this._renderStartTime, endTime, listings[j].inWindowMinutes);
			if (listings[j].startTime <= this._renderStartTime) {
				firstIndex = j;
			}
			if (listings[j].startTime < endTime) {
				lastIndex = j;
			} else {
				break;
			}
		}
		// If needed, mark the first program in the grid for selection
		if (needsSelection && i == this._viewChannelStartIndex) this._selectedProgramIndex = firstIndex;
		//console.log("channelIndex, firstProgramIndex, lastProgramIndex: ", i, firstIndex, lastIndex);
		// populate programs
		for (var j=firstIndex; j<=lastIndex; j++) {
			//console.log("createProgram:  channel, program, ui:  ", i, j, (listings[j].ui) ? true : false);
			this.createProgram(i, listings[j]);
		}
	}
}

GridGuide.prototype.horizontalMove = function(right) {
	var listings = this._channels[this._selectedChannelIndex].stationInfo.listings;
	// move left, but before the current time:  invalid
	if (right < 0 && listings[this._selectedProgramIndex].startTime < this._renderStartTime) return false;
	// move right, but beyond available programs: invalid (though in a real world case, new data would need to be loaded
	if (right + this._selectedProgramIndex >= listings.length) return false;

	this._lastSelectedChannelIndex = this._selectedChannelIndex;
	this._lastSelectedProgramIndex = this._selectedProgramIndex;
	this._selectedProgramIndex = this._selectedProgramIndex + right;
	this.calculateHorizontalStart();
	return true;	 
}

GridGuide.prototype.verticalMove = function(down) {
	var nextCh = this._selectedChannelIndex + down;
	// move up or down, but beyond available programs: invalid
	if (nextCh > this._channels.length -1) return false;
	if (nextCh < 0) return false;
	var listings = this._channels[this._selectedChannelIndex].stationInfo.listings;

	// move up or down: need to find the nearest program that is closest to lastHorizStart
	var currentProgramStart = listings[this._selectedProgramIndex].startTime;
	listings = this._channels[nextCh].stationInfo.listings;
	var index = -1;
	for (var i = 0; listings.length; i++) {
		if (listings[i].startTime <= this._lastHorizStart) {
			index = i;
		} else {
			break;
		}
	}

	this._lastSelectedChannelIndex = this._selectedChannelIndex;
	this._lastSelectedProgramIndex = this._selectedProgramIndex;
	this._selectedChannelIndex = nextCh;
	this._selectedProgramIndex = index;
	return true;
}

GridGuide.prototype.calculateHorizontalStart = function() {
		var programStart = this._channels[this._selectedChannelIndex].stationInfo.listings[this._selectedProgramIndex].startTime;
		this._lastHorizStart = (programStart < this._date.getTime()) ? this._date.getTime() : programStart;
}

GridGuide.prototype.selectNext = function(vertical) {
	// determine if the new selection is fully visible
	var prog = this._channels[this._selectedChannelIndex].stationInfo.listings[this._selectedProgramIndex];
	if (this._selectedChannelIndex >= this._viewChannelStartIndex &&
		this._selectedChannelIndex < (this._viewChannelStartIndex + this._displayableChannels) &&
		(prog.startTime >= this._renderStartTime || prog.startTime < this._originStartTime) &&
		(prog.inWindowMinutes * 60 * 1000 + prog.startTime) <= 
		(this._gridUnits * this._gridUnitWidth * 60 * 1000 + this._renderStartTime)) {
			console.log("Is this selection visible? ", true);
			this.moveSelection();
	} else {
		console.log("Is this selection visible? ", false);
		if (vertical) {
			console.log("* Case 1 - a vertical move");
			// there was a vertical move, channel has changed, determine firstChannel to display
			var delta = this._selectedChannelIndex - this._lastSelectedChannelIndex;
			this._lastViewChannelStartIndex = this._viewChannelStartIndex;
			this._viewChannelStartIndex += delta;
			// scroll channelBar
			this._channelView.animateTo({y:this._viewChannelStartIndex * this._channelHeight * -1}, this._animateTime, 0, 0);
		}
		this._lastRenderStartTime = this._renderStartTime;
		// determine the render startTime, even if there was a vertical move it could affect l/r scrolling
		// There are four scenarios:
		if (this._renderStartTime == this._originStartTime && prog.startTime < this._originStartTime) {
			// do nothing, this program is good.
			console.log("* Case 2");
			
			//this.moveSelection();
			//return;
		} else if (prog.startTime < this.calculateStartTime(this._date)) {
			// program calculated start time is < less than calculateStartTime of currentTime, use calculateStartTime(current)
			console.log("* Case 3");
			this._renderStartTime = this.calculateStartTime(this._date);
		} else if (prog.startTime < this._renderStartTime) {
			// program start time is < renderTime, renderTime = calculatedStartTime(programStart)
			console.log("* Case 4");
			this._renderStartTime = this.calculateStartTime(new Date(prog.startTime));
		} else if (prog.startTime > (this._gridUnitWidth * this._gridUnits) * 60 * 1000 + this._renderStartTime) {
			if (prog.startTime < this._renderStartTime) {
				console.log("* Case 5");
				// program start time is > render end time, but program startTime < render startTime, renderTime = programStartTime
				this._renderStartTime = this.calculateStartTime(new Date(prog.startTime));
			} else {
				console.log("* Case 6");
				// program start time is > render end time, renderTime = program end time + 30 mins - render view time
				this._renderStartTime = ((prog.inWindowMinutes + 30) * 60 * 1000 + prog.startTime) - ((this._gridUnitWidth * this._gridUnits) * 60 * 1000 + this._renderStartTime);
			}
		} else {
			var old = this._renderStartTime;
			this._renderStartTime = ((prog.inWindowMinutes + 30) * 60 * 1000 + prog.startTime) - ((this._gridUnitWidth * this._gridUnits) * 60 * 1000);
			var newer = this._renderStartTime;
			console.log("* Case 7");  // no changes necessary?
			console.log("old start, newer start, one hour: " , old, newer, 60 * 60 * 1000);
		}
		this.populateGrid();
		this.scrollGrid();
	}
}

GridGuide.prototype.moveSelection = function() {
	//console.log("this._selectedChannelIndex, this._selectedProgramIndex : ", this._selectedChannelIndex, this._selectedProgramIndex);
	if (this._lastSelectedChannelIndex >= 0) {
		this._channels[this._lastSelectedChannelIndex].stationInfo.listings[this._lastSelectedProgramIndex].ui.children[0].textColor=0xCCCCCCFF;
	}
	var next = this._channels[this._selectedChannelIndex].stationInfo.listings[this._selectedProgramIndex].ui;
	this._highlighter.animateTo({x:next.x, y:next.y, w:next.w, h:next.h}, this._animateTime, 0, 0);
	next.children[0].textColor=0xAAAAFFFF;
	this.programChanged(
				this._channels[this._selectedChannelIndex].stationInfo.listings[this._selectedProgramIndex], 
				this._channels[this._selectedChannelIndex]);
}

GridGuide.prototype.createProgram = function(channelIndex, data) {
	if (!data.ui) {
		// use this._originStartTime, this._programView, this._pixelsPerMs
		var x = Math.round((data.startTime - this._originStartTime) * this._pixelsPerMs);
		//console.log("myx: ", x);
		var w = Math.round(data.inWindowMinutes * 60 * 1000 * this._pixelsPerMs);
		var y = channelIndex * this._channelHeight;
		var h = this._channelHeight;
		//var container = scene.create({t:"object", parent:this._programView, x:x, y:y, w:w, h:h });
		var container = scene.create({t:"rect", parent:this._programView, x:x, y:y, w:w, h:h, fillColor:0x000000FF, lineColor:0x777777FF, lineWidth:1, clip:true});
		data.ui = container;
		scene.create({t:"text", parent:container, x:10, y:14, textColor:0xCCCCCCFF, pixelSize:32, text:data.programInfo.gridTitle});
		var imgUrl = process.cwd() + "/bob/black_fade_right.png";
		x = container.w - 76; // width of image
		scene.create({t:"image", parent:container, x:x, y:12, h:container.h, url:imgUrl});
	}
	//if program start time occurs before render start time, start text at render startTime
	//console.log("data.startTime, this._renderStartTime: ", data.startTime, this._renderStartTime);
	//console.log(data.startTime < this._renderStartTime);
	if (data.startTime < this._renderStartTime) {
		data.ui.children[0].x = Math.round((this._renderStartTime - this._originStartTime) * this._pixelsPerMs);
	} else {
		//TODO: data.ui.children[0].x = Math.round((data.startTime - this._originStartTime) * this._pixelsPerMs);
	}
	//console.log("l8x: ", data.ui.children[0].x);	
}

GridGuide.prototype.createChannel = function(index, data) {
	if (!data.ui) {
		var x = 0;
		var y = index * this._channelHeight;
		var w = this._channelWidth;
		var h = this._channelHeight;

		var container = scene.create({t:"object", parent:this._channelView, x:x, y:y, w:w, h:h});
		var rect = scene.create({t:"rect", parent:container, x:0, y:0, w:w, h:h, fillColor:0x000000FF, lineColor:0x777777FF, lineWidth:1 });

		var url = process.cwd() + "/bob/downArrow.png"; //TODO replace with data image below:
		
		//var url;
		for (var i=0; i < data.stationInfo.selectedImages.length; i++) {
			if (data.stationInfo.selectedImages[i].alias === "96x40-color") {
				url = data.stationInfo.selectedImages[i].url;
				break;
			}
		}
		
		var img = scene.create({t:"image", parent:container, x:6, y:19, w:96, h:40, url:url});
		var channelNumber = scene.create({t:"text", parent:container, x:110, y:14, textColor:0xCCCCCCFF, pixelSize:32, text:''+data.number});
		data.ui = container;
	}
}


GridGuide.prototype.scrollGrid = function() {
		// scroll grid
		var x = Math.round((this._renderStartTime - this._originStartTime) * this._pixelsPerMs);
		var y = this._viewChannelStartIndex * this._channelHeight;
		//console.log("-x, -y : ", -x, -y);
		this._programView.animateTo({x:-x, y:-y}, this._animateTime, 0, 0);
		this._highlighterView.animateTo({x:-x, y:-y}, this._animateTime, 0, 0);
		this._timesContainer.animateTo({x:-x}, this._animateTime, 0, 0);
		
		//if (this._renderStartTime != this._lastRenderStartTime) {
			// if renderStartTime changed, scroll time bar
			this.updateTimeBar();
		//}
		this.moveSelection();
}


/**
 * Program Details Definition
 */

function ProgramDetails(rect, scene, parent, data) {
	this._rect = rect;
	this._scene = scene;
	this._parent = parent;
	this._data = data;
	
	this._imageWidthPct = .35;
	this._infoWidthPct = .45;
	this._detailsWidthPct = .20;
	this._titleHeightPct = .25;
	this._titleFontSize = 36;
	this._descriptionHeightPct = .75;
	this._descriptionFontSize = 24;
	this._detailsHeightPct = .25;
	this._detailsFontSize = 30;
	var container = scene.create({t:"object", parent:parent, x:rect.x, y:rect.y, w:rect.w, h:rect.h});
	var progInfoContainer = scene.create({t:"object", parent:container, x:this._rect.w * this._imageWidthPct, y:0, w:485, h:rect.h, clip:true});
	this._channelX = progInfoContainer.x + progInfoContainer.h + 5;
	this._programImage = scene.create({t:"image", parent:container, clip:true});
	this._titleText = scene.create({t:"text", parent:progInfoContainer, textColor:0xAAAAFFFF, pixelSize:this._titleFontSize});
	this._descriptionText = scene.create({t:"text", parent:progInfoContainer, textColor:0xAAAAFFFF, pixelSize:this._descriptionFontSize, clip:true});
	this._programTimeText = scene.create({t:"text", parent:container, textColor:0xAAAAFFFF, pixelSize:this._detailsFontSize, });
	this._channelText = scene.create({t:"text", parent:container, textColor:0xAAAAFFFF, pixelSize:this._detailsFontSize});
	this._decorationsText = scene.create({t:"text", parent:container, textColor:0xAAAAFFFF, pixelSize:this._detailsFontSize});
	
	this.resize();
	//this.setData(data);
}


ProgramDetails.prototype.resize = function() {

	this._programImage.x = 0;
	this._programImage.y = 0;
	this._programImage.w = this._rect.w * this._imageWidthPct;
	this._programImage.h = this._rect.h;
/*
	this._titleText.x = this._rect.w * this._imageWidthPct;
	this._titleText.y = 0;
	this._titleText.w = this._rect.w * this._infoWidthPct;
	this._titleText.h = this._rect.h * this._titleHeightPct;
	//console.log(this._titleText.x, this._titleText.y, this._titleText.w, this._titleText.h);
	this._descriptionText.x = this._titleText.x;
	this._descriptionText.y = this._titleText.h + 40;
	this._descriptionText.w = this._titleText.w;
	this._descriptionText.h = this._rect.h - this._titleText.h;
*/
	//console.log(this._descriptionText.x, this._descriptionText.y, this._descriptionText.w, this._descriptionText.h);
	this._programTimeText.x = 950;
	this._programTimeText.y = 0;
	this._programTimeText.w = this._rect.w * this._detailsWidthPct;
	this._programTimeText.h = this._rect.h * this._detailsHeightPct;
	//console.log(this._programTimeText.x, this._programTimeText.y, this._programTimeText.w, this._programTimeText.h);
	this._channelText.x = this._programTimeText.x;
	this._channelText.y = this._programTimeText.h;
	this._channelText.w = this._programTimeText.w;
	this._channelText.h = this._programTimeText.h;
	//console.log(this._channelText.x, this._channelText.y, this._channelText.w, this._channelText.h);
	this._decorationsText.x = this._programTimeText.x;
	this._decorationsText.y = this._channelText.y + this._channelText.h;
	this._decorationsText.w = this._programTimeText.w;
	this._decorationsText.h = this._programTimeText.h;
	//console.log(this._decorationsText.x, this._decorationsText.y, this._decorationsText.w, this._decorationsText.h);
}


ProgramDetails.prototype.setData = function(data) {
	this._data = data;
	
	if (data != null) {
		this._programImage.url = data.imageUrl;
		this._programImage.sx = .5;
		this._programImage.sy = .5;
		
		console.log("What is my titleText height? ", this._titleText.h);
		if (data.episode) {
			this._descriptionText.y = 101;			
			this._titleText.text = data.title + "\n" + data.episode;
		} else {
			this._descriptionText.y = 61;
			this._titleText.text = data.title;
		}
		this._descriptionText.text = data.description;
		this._programTimeText.text = data.startTime + " - " + data.endTime;
		this._channelText.text = data.channelNumber + " " + data.channelShortName;
		this._decorationsText.text = ((data.hd) ? "HD" : "") + " " + ((data.cc ? "CC" : "") + " " + data.rating);
	} 
}


/*
 * Helper Functions
 */
function simpleDateFormat(date) {
	return (date.getHours() > 12 ? date.getHours() - 12 : date.getHours()) + ":" 
		+ (date.getMinutes() < 10 ? '0' + date.getMinutes() : date.getMinutes());
}


/*
 * Run Application
 */
var gridFile = process.cwd() + "/bob/x_24h_grid_40hd.json";
var gridData = null;
var guide
fs.readFile(gridFile, 'utf8', function(err, data) {
	if (err) {
		console.log("Error loading grid file: " + err);
		return;
	}
	gridData = JSON.parse(data);
	fs.readFile(process.cwd() + "/bob/x_programLibrary24_40hd.json", 'utf8', function(err, data) {
		if (err) {
			console.log("Error loading grid file: " + err);
			return;
		}
		var programData = JSON.parse(data);
		console.log("Grid data? " + gridData.getGridResponse.channels.length);	
		guide = new GridGuide(gridData, programData);
	});
});
