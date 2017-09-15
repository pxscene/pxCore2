/*global describe, it*/
"use strict";

var fs = require("fs"),
	es = require("event-stream"),
	should = require("should");

require("mocha");

delete require.cache[require.resolve("../")];

var gutil = require("gulp-util"),
	jsoncombine = require("../");

describe("gulp-jsoncombine", function () {

	var expectedFile = new gutil.File({
		path: "test/expected/hello.txt",
		cwd: "test/",
		base: "test/expected",
		contents: fs.readFileSync("test/expected/hello.txt")
	});

	it("should produce expected file via buffer", function (done) {

		var srcFile = new gutil.File({
			path: "test/fixtures/hello.txt",
			cwd: "test/",
			base: "test/fixtures",
			contents: fs.readFileSync("test/fixtures/hello.txt")
		});

		var stream = jsoncombine("World", function (data) {
			var helloTxt = data.hell;
			return new Buffer(helloTxt + "\nWorld");
		});

		stream.on("data", function (newFile) {

			should.exist(newFile);
			should.exist(newFile.contents);

			String(newFile.contents).should.equal(String(expectedFile.contents));
			done();
		});

		stream.write(srcFile);
		stream.end();
	});

	it("should error on stream", function (done) {

		var srcFile = new gutil.File({
			path: "test/fixtures/hello.txt",
			cwd: "test/",
			base: "test/fixtures",
			contents: fs.createReadStream("test/fixtures/hello.txt")
		});

		var stream = jsoncombine("World", function () {});

		stream.on("error", function(err) {
			err.message.should.equal("Streaming not supported");
			done();
		});

		stream.on("data", function (newFile) {
			should.fail(null, null, "should never get here");
		});

		stream.write(srcFile);
		stream.end();
	});

	it("should error in converter", function (done) {
		var srcFile = new gutil.File({
			path: "test/fixtures/hello.txt",
			cwd: "test/",
			base: "test/fixtures",
			contents: fs.readFileSync("test/fixtures/hello.txt")
		});

		var stream = jsoncombine("World", function (data) {
			throw new Error("oops");
		});

		stream.on("error", function(err) {
			err.message.should.equal("oops");
			done();
		});

		stream.on("data", function (newFile) {
			should.fail(null, null, "should never get here");
		});

		stream.write(srcFile);
		stream.end();
	});

	it("should error when parsing JSON in source file", function (done) {
		var srcFile = new gutil.File({
			path: "test/fixtures/badHello.txt",
			cwd: "test/",
			base: "test/fixtures",
			contents: fs.readFileSync("test/fixtures/badHello.txt")
		});

		var stream = jsoncombine("World", function (data) { });

		stream.on("error", function(err) {
			err.message.should.equal("Error parsing JSON: SyntaxError: Unexpected token H, file: /badHello.txt");
			done();
		});

		stream.on("data", function (newFile) {
			should.fail(null, null, "should never get here");
		});

		stream.write(srcFile);
		stream.end();
	});
});
