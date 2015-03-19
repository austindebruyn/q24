
##Quest-24 Hash Function

Node.js 0.10.x required for now.

```
var q24 = require('q24');
q24.hashAsync('hash me', function (err, hash) {
	if (err) throw err;

	console.log(hash);
	// e97bb84c63cc81e90c3a6438c0a62b7700b439aa92b65cf9
});
```

Q24 produces a 24-byte hash by simulating a tabletop RPG. Using the hash input, the q24 algorithm simulates about 3500-7000 dice rolls to determine stats for the hero and hundreds or thousands of enemies. Each byte 1 through 24 represents the hero's HP at the end of each of the 24 quests.

This project was just for fun and presents no real cryptographic goals, although through normal usage I am convinced the algorithm is collision-free.

Run ```make``` to build the npm module and CLI tool. The CLI tool is a simple binary q24sum that is placed in the bin folder. You can move this to /bin to make the ```q24sum``` command available system-wide.

```
$ q24sum -i "three blind mice"
$ 3d164a33238d469757741a462ae5cd8ad41279ef421ce734
```

Run ```q24sum -h``` for more details.

##Install

```npm install q24```

If you have mocha, you can run the tests inside directory node_modules/q24 with the ```mocha``` command.

##License

MIT License
http://opensource.org/licenses/MIT
