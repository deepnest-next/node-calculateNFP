const addon = require("./build/Release/addon.node")

const A = [
	{ X: 0, Y: 0 },
	{ X: 2, Y: 0 },
	{ X: 2, Y: 3 },
	{ X: 8, Y: 3 },
	{ X: 8, Y: 0 },
	{ X: 10, Y: 0 },
	{ X: 10, Y: 5 },
	{ X: 0, Y: 5 }
]
const B = [
	{ X: 10, Y: 0 },
	{ X: 15, Y: -5 },
	{ X: 20, Y: 0 },
	{ X: 15, Y: 5 }
]

const hasHoles = false
const nfps = addon.calculateNFP({ A: A, B: B, hasHoles: hasHoles })
console.log(nfps)