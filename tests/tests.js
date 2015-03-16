
var q24;
var assert = require('assert');

describe('Q24', function () {

	before(function () {
		q24 = require(__dirname + '/../bin/q24.node');
	});

	it('can run async', function (done) {
		q24.hashAsync('the quick brown fox jumped over the lazy brown dog', function (err, reply) {
			if (err)
				return done(err);

			assert.equal(reply, '3d164a33238d469757741a462ae5cd8ad41279ef421ce734');

			done();
		});
	});

	it('can run sync', function () {
		var reply = q24.hashSync('the quick brown fox jumped over the lazy brown dog');
		assert.equal(reply, '3d164a33238d469757741a462ae5cd8ad41279ef421ce734');
	});

});