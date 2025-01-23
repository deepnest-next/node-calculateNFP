const addon = require("./build/Release/addon.node")

const result = addon.calculateNFP({
	A: [
		{ x: 0, y: 0 },
		{ x: 20, y: 0 },
		{ x: 20, y: 20 },
		{ x: 0, y: 20 }
	],
	B: [
		{ x: 0, y: 0 },
		{ x: 5, y: 0 },
		{ x: 5, y: 5 },
		{ x: 0, y: 5 }
	]
})

console.dir(result, { depth: null })