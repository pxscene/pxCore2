function Person(n) {
	this.name = n;
}

Person.prototype.getName = function() {
	return this.name;
}

Person.prototype.setName = function(n) {
	this.name = n;
}

module.exports = Person;
