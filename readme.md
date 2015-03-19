
#Quest-24 Hash Function

```var q24 = require('q24');
q24.hashAsync('the quick brown fox jumped over the lazy brown dog', function (err, hash) {
	if (err) throw err;

	console.log(hash);
	// 3d164a33238d469757741a462ae5cd8ad41279ef421ce734
});```

Q24 produces a 24-byte hash by simulating a tabletop RPG. Using the hash input, the q24 algorithm simulates about 3500-7000 dice rolls to determine stats for the hero and hundreds or thousands of enemies. Each byte 1 through 24 represents the hero's HP at the end of each of the 24 quests.

This project was just for fun and presents no real cryptographic goals, although through normal usage I am convinced the algorithm is collision-free.

Run ```make``` to build the npm module and CLI tool. The CLI tool is a simple binary q24sum that is placed in the bin folder. You can move this to /bin to make the q24sum command available system-wide.

```$ q24sum -i "three blind mice"
$ 3d164a33238d469757741a462ae5cd8ad41279ef421ce734```

Run ```q24sum -h``` for more details.

#License

MIT License
http://opensource.org/licenses/MIT
