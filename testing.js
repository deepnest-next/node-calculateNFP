const addon = require("./build/Release/addon.node")

const A = [
	{ X: 0, Y: 0 },
	{ X: 20, Y: 0 },
	{ X: 20, Y: 30 },
	{ X: 80, Y: 30 },
	{ X: 80, Y: 0 },
	{ X: 100, Y: 0 },
	{ X: 100, Y: 50 },
	{ X: 0, Y: 50 }
]

const B = [
	{ X: 100, Y: 0 },
	{ X: 140, Y: -40 },
	{ X: 180, Y: 0 },
	{ X: 140, Y: 40 }
]

const hasHoles = false
const nfps = addon.calculateNFP({ A: A, B: B, hasHoles: hasHoles })
console.log(nfps)